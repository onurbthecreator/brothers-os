/*
  BrothersOS: "sistema operacional" de brinquedo estilo Windows 95 no
  ESP32-S3 com tela TFT 0,96" 160x80, joystick, 2 botoes e buzzer. Feito
  pelo Bruno e pelo Caio pro Mundo SENAI. Area de trabalho com icones
  (brothersos.cpp) que abrem o Doom e 9 jogos (tetris.cpp, pong.cpp...).
  Motor do Doom: doomgeneric (GPL-2.0), em src/doom. O doom1.wad fica
  gravado cru na particao "wad" (partitions.csv desta pasta), ver README.md.

  Arduino IDE (Ferramentas):
    Placa: ESP32S3 Dev Module
    Flash Size: 16MB (128Mb)
    PSRAM: OPI PSRAM
    Partition Scheme: Huge APP (so pro limite de tamanho; a tabela usada
    vem sozinha do partitions.csv desta pasta, com os mesmos 3 MB)

  Ligacao (esquema completo no artifact "Fiacao do BrothersOS"):
    Tela TFT  0,96" ST7735S: SDA 8, SCL 9, CS 4, RES 5, DC 6, VCC e BLK 3V3
    Joystick  VRx 1 (virar), VRy 2 (andar), SW 42 (usar), +5V no 3V3
    Botoes    atirar 41, trocar arma 40 (cantos opostos, outro lado no GND)
    Buzzer    + no 12, - pelo resistor de 100 ohm no GND

  Controles no Doom:
    joystick         anda e vira (frente/tras ate o fim = corre)
    apertar joystick usar (porta, interruptor, elevador)
    atirar           atira; no menu confirma
    trocar arma      proxima arma que voce ja tem
    joystick + atirar  abre/fecha o menu do Doom
  Em qualquer jogo: usar + trocar arma por 1 s volta pra area de trabalho.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include <sys/time.h>
#include "jogos.h"
#include "pagina.h"

extern "C" {
#include "src/doom/doomgeneric.h"
#include "src/doom/doomkeys.h"
}

// O Doom e recursivo (desenho do mapa) e usa bastante pilha.
SET_LOOP_TASK_STACK_SIZE(32 * 1024);

#define PINO_JOY_X   1
#define PINO_JOY_Y   2
#define PINO_JOY_SW  42
#define PINO_ATIRAR  41
#define PINO_ARMA    40
#define PINO_BUZZER  12

// Rede Wi-Fi propria da placa: conecte nela e abra http://192.168.4.1
#define WIFI_NOME  "BrothersOS"
#define WIFI_SENHA "doom1993"

const float ZONA_MORTA    = 0.30; // folga no centro do joystick
const float LIMITE_CORRER = 0.90; // frente/tras quase ate o fim = corre
// Virar: abaixo deste ponto a tecla vai e volta (giro lento do Doom, bom
// pra mirar); acima, fica apertada e o giro acelera.
const float LIMITE_GIRO_RAPIDO = 0.90;
const uint32_t PULSO_GIRO_MS   = 120; // periodo do vai-e-volta

// Tecla "proxima arma" que o Doom recebe (qualquer uma que o jogo nao use).
#define TECLA_PROXIMA_ARMA ']'

extern "C" {
  extern uint32_t colors[256]; // paleta do Doom (struct color: b, g, r, a)
  extern int palette_changed;
  void DG_Ajustes(int tecla_proxima_arma, int com_barra);
  void DG_Hud(int *jogando, int *vida, int *municao, int *armadura);
}

TelaBros tela; // folha 128x64 e tela colorida 160x80 na TFT (tela.cpp)
uint16_t paleta565[256]; // paleta do Doom pra TFT

void atualizarPaleta() {
  for (int i = 0; i < 256; i++) {
    uint32_t c = colors[i];
    int r = (c >> 16) & 255, g = (c >> 8) & 255, b = c & 255;
    paleta565[i] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }
}

// ---------- Controles ----------

struct Evento { uint8_t apertou, tecla; };
Evento fila[32];
uint8_t filaIni = 0, filaFim = 0;

void enfileirar(bool apertou, uint8_t tecla) {
  uint8_t prox = (filaFim + 1) % 32;
  if (prox == filaIni) return; // fila cheia: perde o evento
  fila[filaFim].apertou = apertou;
  fila[filaFim].tecla = tecla;
  filaFim = prox;
}

// Estado de cada tecla que o Doom ja recebeu, pra mandar so as mudancas.
const uint8_t TECLAS[] = {KEY_UPARROW, KEY_DOWNARROW, KEY_LEFTARROW, KEY_RIGHTARROW, KEY_RSHIFT,
                          KEY_USE, KEY_FIRE, KEY_ENTER, 'y', KEY_ESCAPE, TECLA_PROXIMA_ARMA};
bool apertada[sizeof(TECLAS)];

void definir(uint8_t tecla, bool quer) {
  for (size_t i = 0; i < sizeof(TECLAS); i++) {
    if (TECLAS[i] == tecla) {
      if (apertada[i] != quer) {
        apertada[i] = quer;
        enfileirar(quer, tecla);
      }
      return;
    }
  }
}

// Eixo 0 = VRx (pino 1), eixo 1 = VRy (pino 2). Qual eixo anda/vira e
// pra que lado e frente/esquerda vem da calibracao na tela (calibrarDirecoes),
// guardada na memoria. Ligar segurando "atirar" refaz.
const int PINOS_EIXO[2] = {PINO_JOY_X, PINO_JOY_Y};
int centro[2] = {2048, 2048};
int eixoAndar = 1, sinalAndar = 1; // frente = positivo depois do sinal
int eixoVirar = 0, sinalVirar = 1; // direita = positivo depois do sinal

// -1 a +1, 0 no centro.
float lerEixo(int eixo) {
  int v = analogRead(PINOS_EIXO[eixo]);
  int c = centro[eixo];
  return v >= c ? (float)(v - c) / (4095 - c) : (float)(v - c) / c;
}

// Botao so muda depois de 2 leituras iguais seguidas (20 ms): tira o tremido.
struct Botao {
  int pino;
  bool bruto, firme;
  bool ler() {
    bool agora = digitalRead(pino) == LOW;
    if (agora == bruto) firme = agora;
    bruto = agora;
    return firme;
  }
};
Botao btUsar = {PINO_JOY_SW, false, false};
Botao btAtirar = {PINO_ATIRAR, false, false};
Botao btArma = {PINO_ARMA, false, false};

// Controle da pagina (quem esta no navegador tambem joga). Chega junto com
// cada pedido de imagem (/quadro?x=&y=&b=). x/y de -1 a +1 ja no sentido do
// jogo (x > 0 direita, y > 0 frente); b: 1 atirar, 2 trocar arma, 4 usar.
// Sem pedido por 0,5 s (celular fechou a aba) vale tudo solto.
volatile float webX = 0, webY = 0;
volatile uint8_t webBotoes = 0;
volatile uint32_t webControleMs = 0;

bool webAtivo() { return millis() - webControleMs < 500; }

// O lado mais empurrado ganha: placa e navegador jogam juntos.
float juntarEixo(float placa, float web) {
  if (!webAtivo()) return placa;
  return fabsf(web) > fabsf(placa) ? web : placa;
}

bool botaoWeb(uint8_t bit) { return webAtivo() && (webBotoes & bit); }

// Sobrevive ao ESP.restart(): 0xB805 = voltando do Doom, pula a tela de boot.
RTC_NOINIT_ATTR uint32_t brosVoltando;

void lerControles() {
  static uint32_t ultima = 0;
  if (millis() - ultima < 10) return;
  ultima = millis();

  float x = juntarEixo(lerEixo(eixoVirar) * sinalVirar, webX);
  float y = juntarEixo(lerEixo(eixoAndar) * sinalAndar, webY);
  definir(KEY_UPARROW, y > ZONA_MORTA);
  definir(KEY_DOWNARROW, y < -ZONA_MORTA);
  // Giro proporcional: quanto mais inclina, mais tempo a tecla fica apertada
  // em cada periodo. Soltar a tecla zera a aceleracao do giro no Doom.
  float ax = fabsf(x);
  bool girar = false;
  if (ax >= LIMITE_GIRO_RAPIDO) {
    girar = true;
  } else if (ax > ZONA_MORTA) {
    float parte = 0.35f + 0.65f * (ax - ZONA_MORTA) / (LIMITE_GIRO_RAPIDO - ZONA_MORTA);
    girar = (millis() % PULSO_GIRO_MS) < parte * PULSO_GIRO_MS;
  }
  definir(KEY_RIGHTARROW, girar && x > 0);
  definir(KEY_LEFTARROW, girar && x < 0);
  definir(KEY_RSHIFT, fabsf(y) > LIMITE_CORRER);

  bool usar = btUsar.ler() || botaoWeb(4);
  bool atirar = btAtirar.ler() || botaoWeb(1);
  bool menu = usar && atirar;
  bool tiro = atirar && !usar;
  definir(KEY_ESCAPE, menu);
  definir(KEY_USE, usar && !menu);
  // Atirar tambem e Enter e "y": confirma no menu e nas perguntas.
  definir(KEY_FIRE, tiro);
  definir(KEY_ENTER, tiro);
  definir('y', tiro);
  bool arma = btArma.ler() || botaoWeb(2);
  definir(TECLA_PROXIMA_ARMA, arma);

  // usar + trocar arma por 1 s: o Doom nao tem como fechar, entao reinicia
  // a placa direto na area de trabalho (sem a tela de boot).
  static uint32_t sairDesde = 0;
  if (usar && arma) {
    if (!sairDesde) sairDesde = millis();
    if (millis() - sairDesde > 1000) {
      brosVoltando = 0xB805;
      ESP.restart();
    }
  } else {
    sairDesde = 0;
  }
}

// ---------- Calibracao das direcoes (na tela) ----------

void mensagem(const char *linha1, const char *linha2);

// Espera o joystick ficar empurrado pra um lado por 300 ms e devolve o eixo
// e o sinal. eixoProibido = o eixo que ja foi usado (ou -1).
void esperarDirecao(int eixoProibido, int &eixo, int &sinal) {
  int candidato = -1, candSinal = 0;
  uint32_t desde = 0;
  for (;;) {
    int achou = -1, achouSinal = 0;
    for (int e = 0; e < 2; e++) {
      if (e == eixoProibido) continue;
      float v = lerEixo(e);
      if (fabsf(v) > 0.6f) { achou = e; achouSinal = v > 0 ? 1 : -1; }
    }
    if (achou >= 0 && achou == candidato && achouSinal == candSinal) {
      if (millis() - desde >= 300) { eixo = achou; sinal = achouSinal; return; }
    } else {
      candidato = achou; candSinal = achouSinal; desde = millis();
    }
    delay(10);
  }
}

void esperarCentro() {
  uint32_t desde = millis();
  while (millis() - desde < 300) {
    if (fabsf(lerEixo(0)) > 0.2f || fabsf(lerEixo(1)) > 0.2f) desde = millis();
    delay(10);
  }
}

void calibrarDirecoes(Preferences &mem) {
  int e, s;
  mensagem("FRENTE", "empurre o joystick pra frente e segure");
  esperarDirecao(-1, e, s);
  eixoAndar = e; sinalAndar = s;
  mensagem("SOLTE", "");
  esperarCentro();
  mensagem("ESQUERDA", "empurre pra esquerda e segure");
  esperarDirecao(eixoAndar, e, s);
  eixoVirar = e; sinalVirar = -s; // esquerda e o lado negativo
  mensagem("SOLTE", "");
  esperarCentro();
  mem.putInt("eixoAndar", eixoAndar);
  mem.putInt("sinalAndar", sinalAndar);
  mem.putInt("eixoVirar", eixoVirar);
  mem.putInt("sinalVirar", sinalVirar);
  Serial.printf("Direcoes salvas: andar eixo %d sinal %d, virar eixo %d sinal %d\n", eixoAndar, sinalAndar, eixoVirar, sinalVirar);
}

// ---------- Servidor web: a tela no navegador ----------
// A cada quadro, se alguem pediu imagem nos ultimos 3 s, guarda uma copia.
// Primeiro byte = formato: 1 = Doom colorido (paleta RGB + 160x100 indices,
// um pixel a cada 2), 2 = folha 1 bit (1024 bytes, igual ao buffer da folha),
// 4 = tela colorida (160x80 RGB565). A pagina pede /quadro em loop.

const size_t WEB_DOOM = 1 + 768 + 160 * 100;
const size_t WEB_OLED = 1 + 1024;
const size_t WEB_COR = 1 + TELA_W * TELA_H * 2;
uint8_t *webQuadro = nullptr; // preenchido pelo jogo
uint8_t *webEnvio = nullptr;  // copia usada no envio
size_t webTamanho = WEB_OLED;
SemaphoreHandle_t webTrava;
volatile uint32_t webUltimoPedido = 0;
WebServer servidor(80);
char cmdWeb[40];
volatile bool cmdWebNovo = false;

// A pagina fica em pagina.h.

void copiarQuadroWeb() {
  if (millis() - webUltimoPedido > 3000) return; // ninguem olhando
  if (xSemaphoreTake(webTrava, 0) != pdTRUE) return; // envio em andamento: pula este
  for (int i = 0; i < 256; i++) {
    uint32_t c = colors[i];
    webQuadro[1 + i * 3] = (c >> 16) & 255;
    webQuadro[1 + i * 3 + 1] = (c >> 8) & 255;
    webQuadro[1 + i * 3 + 2] = c & 255;
  }
  const uint8_t *src = (const uint8_t *)DG_ScreenBuffer;
  uint8_t *dst = webQuadro + 1 + 768;
  for (int y = 0; y < 100; y++) {
    const uint8_t *linha = src + y * 2 * 320;
    for (int x = 0; x < 160; x++) *dst++ = linha[x * 2];
  }
  webQuadro[0] = 1;
  webTamanho = WEB_DOOM;
  xSemaphoreGive(webTrava);
}

// Tela colorida (interface colorida): chamada por tela.mostrarCor().
void copiarCorWeb(const uint16_t *px) {
  if (millis() - webUltimoPedido > 3000) return;
  if (xSemaphoreTake(webTrava, 0) != pdTRUE) return;
  webQuadro[0] = 4;
  memcpy(webQuadro + 1, px, TELA_W * TELA_H * 2);
  webTamanho = WEB_COR;
  xSemaphoreGive(webTrava);
}

void copiarTelaWeb() {
  if (millis() - webUltimoPedido > 3000) return;
  if (xSemaphoreTake(webTrava, 0) != pdTRUE) return;
  webQuadro[0] = 2;
  memcpy(webQuadro + 1, tela.getBuffer(), 1024);
  webTamanho = WEB_OLED;
  xSemaphoreGive(webTrava);
}

void tarefaWeb(void *) {
  servidor.on("/", []() { servidor.send_P(200, "text/html; charset=utf-8", PAGINA); });
  // Hora do celular (milissegundos desde 1970), mandada pela pagina.
  servidor.on("/hora", []() {
    if (servidor.hasArg("t")) {
      timeval tv = {(time_t)(atoll(servidor.arg("t").c_str()) / 1000), 0};
      settimeofday(&tv, nullptr);
    }
    servidor.send(200, "text/plain", "ok");
  });
  servidor.on("/cmd", []() {
    if (servidor.hasArg("c")) {
      xSemaphoreTake(webTrava, portMAX_DELAY);
      strlcpy(cmdWeb, servidor.arg("c").c_str(), sizeof(cmdWeb));
      cmdWebNovo = true;
      xSemaphoreGive(webTrava);
    }
    servidor.send(200, "text/plain", "ok");
  });
  servidor.on("/quadro", []() {
    webUltimoPedido = millis();
    if (servidor.hasArg("b")) {
      webX = constrain(servidor.arg("x").toInt(), -100, 100) / 100.0f;
      webY = constrain(servidor.arg("y").toInt(), -100, 100) / 100.0f;
      webBotoes = servidor.arg("b").toInt() & 7;
      webControleMs = millis();
    }
    xSemaphoreTake(webTrava, portMAX_DELAY);
    size_t tam = webTamanho;
    memcpy(webEnvio, webQuadro, tam);
    xSemaphoreGive(webTrava);
    servidor.sendHeader("Cache-Control", "no-store");
    servidor.setContentLength(tam);
    servidor.send(200, "application/octet-stream", "");
    servidor.sendContent((const char *)webEnvio, tam);
  });
  servidor.begin();
  for (;;) {
    servidor.handleClient();
    vTaskDelay(1);
  }
}

void iniciarWeb() {
  webQuadro = (uint8_t *)ps_calloc(WEB_COR, 1); // o maior dos formatos
  webEnvio = (uint8_t *)ps_malloc(WEB_COR);
  webQuadro[0] = 2;
  webTrava = xSemaphoreCreateMutex();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_NOME, WIFI_SENHA);
  Serial.printf("Wi-Fi: rede %s, senha %s, abra http://%s\n", WIFI_NOME, WIFI_SENHA, WiFi.softAPIP().toString().c_str());
  xTaskCreatePinnedToCore(tarefaWeb, "web", 6144, NULL, 1, NULL, 0);
}

// ---------- Som no buzzer ----------
// O Doom chama DG_Tom (via dg_esp32.c) e uma tarefa no outro nucleo faz a
// varredura de frequencia, sem travar o jogo.

volatile int somF1, somF2, somDur, somRuido, somPrio;
volatile uint32_t somInicio;

extern "C" void DG_Tom(int freq1, int freq2, int ms, int ruido, int prioridade) {
  uint32_t agora = millis();
  bool tocando = agora - somInicio < (uint32_t)somDur;
  if (tocando && prioridade < somPrio) return; // nao corta um som mais importante
  somDur = 0; // para enquanto troca os valores
  somF1 = freq1; somF2 = freq2; somRuido = ruido; somPrio = prioridade;
  somInicio = agora;
  somDur = ms;
}

void tarefaSom(void *) {
  int ultimaFreq = -1;
  for (;;) {
    uint32_t t = millis() - somInicio;
    int dur = somDur, f = 0;
    if (dur > 0 && t < (uint32_t)dur) {
      f = somF1 + (somF2 - somF1) * (int)t / dur;
      if (somRuido) f = f / 2 + random(f);
      f = constrain(f, 120, 5000); // o buzzer (e o PWM) nao fazem frequencia muito baixa
    }
    if (f != ultimaFreq) {
      ledcWriteTone(PINO_BUZZER, f); // 0 = silencio
      ultimaFreq = f;
    }
    vTaskDelay(1);
  }
}

// ---------- Funcoes que o doomgeneric pede ----------

extern "C" void DG_Init() {
  // Sem a barra de status do Doom: na tela de 160x80 vida e municao vao por cima.
  DG_Ajustes(TECLA_PROXIMA_ARMA, 0);
}

extern "C" void DG_DrawFrame() {
  static uint32_t quadros = 0, ultimoRelatorio = 0;

  if (palette_changed) {
    palette_changed = 0;
    atualizarPaleta();
  }

  int jogando, vida, municao, armadura;
  DG_Hud(&jogando, &vida, &municao, &armadura);
  tela.desenharDoom((const uint8_t *)DG_ScreenBuffer, paleta565, jogando, vida, municao);
  copiarQuadroWeb();
  lerControles();

  quadros++;
  if (millis() - ultimoRelatorio >= 5000) {
    Serial.printf("%.1f quadros/s | PSRAM livre %u KB | RAM interna livre %u KB\n", quadros * 1000.0f / (millis() - ultimoRelatorio),
                  ESP.getFreePsram() / 1024, heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024);
    quadros = 0;
    ultimoRelatorio = millis();
  }
}

extern "C" void DG_SleepMs(uint32_t ms) {
  delay(ms);
}

extern "C" uint32_t DG_GetTicksMs() {
  return millis();
}

extern "C" int DG_GetKey(int *pressed, unsigned char *key) {
  if (filaIni == filaFim) lerControles();
  if (filaIni == filaFim) return 0;
  *pressed = fila[filaIni].apertou;
  *key = fila[filaIni].tecla;
  filaIni = (filaIni + 1) % 32;
  return 1;
}

extern "C" void DG_SetWindowTitle(const char *title) {
  (void)title;
}

void mensagem(const char *linha1, const char *linha2);

// Erro do motor do Doom (I_Error): mostra na tela e no Serial e para.
extern "C" void DG_Erro(const char *msg) {
  ledcWriteTone(PINO_BUZZER, 0);
  Serial.printf("\nERRO DO DOOM: %s\n", msg);
  Serial.flush();
  mensagem("ERRO", msg);
  for (;;) delay(1000);
}

// ---------- Inicio ----------

void mensagem(const char *linha1, const char *linha2) {
  tela.clearDisplay();
  tela.setTextSize(2);
  tela.setTextColor(SSD1306_WHITE);
  tela.setCursor(0, 0);
  tela.print(linha1);
  tela.setTextSize(1);
  tela.setCursor(0, 24);
  tela.print(linha2);
  tela.display();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("DOOM no ESP32-S3");

  pinMode(PINO_JOY_SW, INPUT_PULLUP);
  pinMode(PINO_ATIRAR, INPUT_PULLUP);
  pinMode(PINO_ARMA, INPUT_PULLUP);
  analogReadResolution(12);
  ledcAttach(PINO_BUZZER, 1000, 8);

  tela.iniciar();

  if (!psramFound()) {
    mensagem("SEM PSRAM", "Ferramentas > PSRAM: OPI PSRAM");
    Serial.println("PSRAM nao encontrada: em Ferramentas, PSRAM tem que ser \"OPI PSRAM\".");
    for (;;) delay(1000);
  }

  mensagem("Brothers", "nao mexa no joystick");

  // Centro do joystick: media com ele solto.
  for (int e = 0; e < 2; e++) {
    long soma = 0;
    for (int i = 0; i < 32; i++) {
      soma += analogRead(PINOS_EIXO[e]);
      delay(2);
    }
    centro[e] = constrain(soma / 32, 500, 3595);
  }
  Serial.printf("Joystick centro: X %d  Y %d\n", centro[0], centro[1]);

  // Direcoes: calibra na primeira vez ou se ligar segurando "atirar".
  Preferences mem;
  mem.begin("doom", false);
  if (!mem.isKey("eixoAndar") || digitalRead(PINO_ATIRAR) == LOW) {
    if (digitalRead(PINO_ATIRAR) == LOW) {
      mensagem("CALIBRAR", "solte o atirar");
      while (digitalRead(PINO_ATIRAR) == LOW) delay(10);
    }
    calibrarDirecoes(mem);
  } else {
    eixoAndar = mem.getInt("eixoAndar", 1);
    sinalAndar = mem.getInt("sinalAndar", 1);
    eixoVirar = mem.getInt("eixoVirar", 0);
    sinalVirar = mem.getInt("sinalVirar", 1);
  }
  mem.end();

  randomSeed(esp_random());
  // Fuso de Brasilia. A hora em si vem do celular e sobrevive ao ESP.restart().
  setenv("TZ", "<-03>3", 1);
  tzset();

  xTaskCreatePinnedToCore(tarefaSom, "som", 4096, NULL, 1, NULL, 0);
  iniciarWeb();

  bool voltandoDoDoom = brosVoltando == 0xB805;
  brosVoltando = 0;
  if (!voltandoDoDoom) brosBoot();
}

// ---------- Ajudantes do BrothersOS e dos jogos (jogos.h) ----------

Entrada lerEntrada() {
  static bool antAtirar = false, antArma = false, antUsar = false;
  static int direcaoAnt = -1;
  static uint32_t repete = 0, sairDesde = 0;
  Entrada e;
  e.x = juntarEixo(lerEixo(eixoVirar) * sinalVirar, webX);
  e.y = juntarEixo(lerEixo(eixoAndar) * sinalAndar, webY);
  e.atirar = btAtirar.ler() || botaoWeb(1);
  e.arma = btArma.ler() || botaoWeb(2);
  e.usar = btUsar.ler() || botaoWeb(4);
  e.atirarToque = e.atirar && !antAtirar;
  e.armaToque = e.arma && !antArma;
  e.usarToque = e.usar && !antUsar;
  antAtirar = e.atirar; antArma = e.arma; antUsar = e.usar;

  // 0 cima, 1 baixo, 2 esquerda, 3 direita; segurando repete
  int d = -1;
  if (fabsf(e.x) > 0.5f || fabsf(e.y) > 0.5f) {
    d = fabsf(e.x) > fabsf(e.y) ? (e.x > 0 ? 3 : 2) : (e.y > 0 ? 0 : 1);
  }
  uint32_t agora = millis();
  bool toque = false;
  if (d != direcaoAnt) { toque = d >= 0; repete = agora + 260; }
  else if (d >= 0 && (int32_t)(agora - repete) >= 0) { toque = true; repete = agora + 90; }
  direcaoAnt = d;
  e.cimaToque = toque && d == 0;
  e.baixoToque = toque && d == 1;
  e.esqToque = toque && d == 2;
  e.dirToque = toque && d == 3;

  if (e.usar && e.arma) { if (!sairDesde) sairDesde = agora; }
  else sairDesde = 0;
  e.sair = sairDesde && agora - sairDesde > 1000;
  return e;
}

void som(int freq1, int freq2, int ms, int ruido, int prioridade) {
  DG_Tom(freq1, freq2, ms, ruido, prioridade);
}

void mostrarTela() {
  tela.display();
  copiarTelaWeb();
}

void esperarQuadro(uint32_t &proximo, uint32_t ms) {
  int32_t falta = (int32_t)(proximo - millis());
  if (falta > 0) delay(falta);
  proximo += ms;
  if ((int32_t)(millis() - proximo) > 100) proximo = millis(); // atrasou muito: nao tenta compensar
}

void textoCentro(const char *texto, int y, int tamanho) {
  tela.setTextSize(tamanho);
  tela.setTextColor(SSD1306_WHITE);
  tela.setCursor((128 - (int)strlen(texto) * 6 * tamanho) / 2, y);
  tela.print(texto);
  tela.setTextSize(1);
}

void barraTitulo(const char *titulo, long pontos) {
  tela.fillRect(0, 0, 128, AREA_Y, SSD1306_BLACK);
  tela.setTextColor(SSD1306_WHITE);
  tela.setTextSize(1);
  tela.setCursor(0, 4);
  tela.print(titulo);
  char buf[12];
  snprintf(buf, sizeof(buf), "%ld", pontos);
  tela.setTextSize(2);
  tela.setCursor(128 - (int)strlen(buf) * 12, 1);
  tela.print(buf);
  tela.setTextSize(1);
}

void desenharSprite(int x, int y, const char *const *linhas, int altura) {
  for (int l = 0; l < altura; l++) {
    for (int c = 0; linhas[l][c]; c++) {
      char ch = linhas[l][c];
      if (ch == '#') tela.drawPixel(x + c, y + l, SSD1306_WHITE);
      else if (ch == 'o') tela.drawPixel(x + c, y + l, SSD1306_BLACK);
    }
  }
}

bool fimDeJogo(const char *chave, int pontos) {
  Preferences mem;
  mem.begin("recordes", false);
  int recorde = mem.getInt(chave, 0);
  bool novo = pontos > recorde && pontos > 0;
  if (novo) mem.putInt(chave, pontos);
  mem.end();
  if (novo) som(800, 1600, 300, 0, 4);

  uint32_t inicio = millis();
  for (;;) {
    Entrada e = lerEntrada();
    tela.fillRect(8, 18, 112, 45, SSD1306_BLACK);
    tela.drawRect(8, 18, 112, 45, SSD1306_WHITE);
    textoCentro("FIM DE JOGO", 21, 1);
    char linha[24];
    snprintf(linha, sizeof(linha), "pontos %d", pontos);
    textoCentro(linha, 31, 1);
    if (novo) textoCentro("NOVO RECORDE!", 41, 1);
    else { snprintf(linha, sizeof(linha), "recorde %d", recorde); textoCentro(linha, 41, 1); }
    textoCentro("atirar: de novo", 53, 1);
    mostrarTela();
    if (millis() - inicio > 500) { // nao pula a janela com o botao que ainda esta apertado
      if (e.atirarToque) return true;
      if (e.usarToque || e.armaToque || e.sair) return false;
    }
    delay(20);
  }
}

void barraJanela(const char *titulo) {
  tela.fillRoundRect(0, 1, 128, 13, 3, SSD1306_WHITE);
  tela.setTextSize(1);
  tela.setTextColor(SSD1306_BLACK);
  tela.setCursor(4, 4);
  tela.print(titulo);
  tela.setCursor(104, 4);
  tela.print("_ x");
  tela.setTextColor(SSD1306_WHITE);
}

bool horaCerta() {
  return time(nullptr) > 1700000000; // depois de 2023: alguem acertou
}

void acertarHora(int hora, int minuto) {
  time_t agora = time(nullptr);
  tm t;
  localtime_r(&agora, &t);
  if (!horaCerta()) { t.tm_year = 2026 - 1900; t.tm_mon = 0; t.tm_mday = 1; }
  t.tm_hour = hora; t.tm_min = minuto; t.tm_sec = 0;
  timeval tv = {mktime(&t), 0};
  settimeofday(&tv, nullptr);
}

void textoHora(char *buf, size_t tamanho, bool doisPontos) {
  if (!horaCerta()) { strlcpy(buf, doisPontos ? "--:--" : "-- --", tamanho); return; }
  time_t agora = time(nullptr);
  tm t;
  localtime_r(&agora, &t);
  snprintf(buf, tamanho, doisPontos ? "%02d:%02d" : "%02d %02d", t.tm_hour, t.tm_min);
}

void textoData(char *buf, size_t tamanho) {
  time_t agora = time(nullptr);
  tm t;
  localtime_r(&agora, &t);
  snprintf(buf, tamanho, "%02d/%02d/%04d %02d:%02d", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min);
}

bool pegarComandoWeb(char *destino, size_t tamanho) {
  if (!cmdWebNovo) return false;
  xSemaphoreTake(webTrava, portMAX_DELAY);
  strlcpy(destino, cmdWeb, tamanho);
  cmdWebNovo = false;
  xSemaphoreGive(webTrava);
  return true;
}

// Tela inteira de uma cor com duas linhas no meio (reiniciar/desligar).
static void avisoCor(const char *linha1, const char *linha2, uint16_t fundo, uint16_t tinta) {
  GFXcanvas16 &c = tela.cor();
  c.fillScreen(fundo);
  corTextoCentro(c, linha1, 28, 1, tinta);
  corTextoCentro(c, linha2, 42, 1, tinta);
  tela.mostrarCor();
}

void brosReiniciar() {
  avisoCor("Reiniciando o", "BrothersOS...", COR_TEAL, COR_BRANCO);
  som(900, 400, 300, 0, 4);
  delay(800);
  brosVoltando = 0; // mostra a tela de boot de novo
  ESP.restart();
}

// "Agora voce pode desligar": letra laranja no preto, como no Windows 95.
// Depois apaga a tela e espera um botao pra religar.
void brosDesligar() {
  avisoCor("Aguarde enquanto o", "BrothersOS desliga...", COR_TEAL, COR_BRANCO);
  som(1047, 523, 600, 0, 4);
  delay(1500);
  GFXcanvas16 &c = tela.cor();
  c.fillScreen(COR_PRETO);
  corTextoCentro(c, "Agora voce pode", 22, 1, COR_LARANJA);
  corTextoCentro(c, "desligar o BrothersOS", 34, 1, COR_LARANJA);
  corTextoCentro(c, "com seguranca.", 46, 1, COR_LARANJA);
  tela.mostrarCor();
  delay(3000);
  tela.ligar(false);
  while (digitalRead(PINO_ATIRAR) == HIGH && digitalRead(PINO_ARMA) == HIGH && digitalRead(PINO_JOY_SW) == HIGH) delay(30);
  tela.ligar(true);
  brosVoltando = 0;
  ESP.restart();
}

// ---------- Programas ----------

bool doomRodando = false;

void iniciarDoom() {
  mensagem("DOOM", "carregando...");
  // Comeca direto na fase 1 (E1M1), dificuldade media: o menu do Doom e
  // pequeno demais pra essa tela. Ele continua no joystick + atirar.
  static char a0[] = "doom", a1[] = "-iwad", a2[] = "doom1.wad";
  static char a3[] = "-skill", a4[] = "3", a5[] = "-warp", a6[] = "1", a7[] = "1";
  static char *args[] = {a0, a1, a2, a3, a4, a5, a6, a7, NULL};
  doomgeneric_Create(8, args);
  doomRodando = true;
}

// Abre um app; o terminal pode devolver outro pra abrir em seguida.
void abrirApp(int app) {
  while (app >= 0) {
    int proximo = -1;
    switch (app) {
    case ICONE_TERMINAL:  proximo = abrirTerminal(); break;
    case ICONE_DOOM:      iniciarDoom(); break;
    case ICONE_TETRIS:    jogarTetris(); break;
    case ICONE_COBRINHA:  jogarCobrinha(); break;
    case ICONE_FLAPPY:    jogarFlappy(); break;
    case ICONE_INVASORES: jogarInvasores(); break;
    case ICONE_CORRIDA:   jogarCorrida(); break;
    case ICONE_DINO:      jogarDino(); break;
    case ICONE_PONG:      jogarPong(); break;
    case ICONE_TIJOLOS:   jogarTijolos(); break;
    case ICONE_MINADO:    jogarMinado(); break;
    case ICONE_VERSICULO: abrirVersiculos(); break;
    case ICONE_BROTHERS:  abrirBrothers(); break;
    case ICONE_CALIBRAR: {
      Preferences mem;
      mem.begin("doom", false);
      calibrarDirecoes(mem);
      mem.end();
      break;
    }
    }
    if (doomRodando) return;
    app = proximo;
  }
}

void loop() {
  if (doomRodando) {
    doomgeneric_Tick();
    return;
  }
  abrirApp(brosDesktop());
}
