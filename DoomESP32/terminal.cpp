// Terminal do BrothersOS: 21 colunas, 5 linhas de saida + a linha do prompt.
// Na placa: joystick cima/baixo escolhe o comando, atirar roda, clique do
// joystick sai. No celular: campo de texto na pagina (rota /cmd).

#include <Preferences.h>
#include <WiFi.h>
#include "jogos.h"

static const int COLS = 21, HIST = 40, VISIVEIS = 5;
static char linhas[HIST][COLS + 1];
static int total = 0; // linhas ja escritas (a mais nova e total - 1)

static void novaLinha() {
  memset(linhas[total % HIST], 0, COLS + 1);
  total++;
}

// Escreve quebrando em 21 colunas; '\n' pula linha.
static void escrever(const char *texto) {
  novaLinha();
  int col = 0;
  for (const char *p = texto; *p; p++) {
    if (*p == '\n' || col == COLS) {
      novaLinha();
      col = 0;
      if (*p == '\n') continue;
    }
    linhas[(total - 1) % HIST][col++] = *p;
  }
}

static void escreverf(const char *fmt, ...) {
  char buf[96];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  escrever(buf);
}

static void desenhar(const char *comando, bool cursor) {
  tela.clearDisplay();
  barraJanela("Terminal");
  tela.setTextSize(1);
  tela.setTextColor(SSD1306_WHITE);
  for (int k = 0; k < VISIVEIS; k++) {
    int n = total - VISIVEIS + k;
    if (n < 0) continue;
    tela.setCursor(1, AREA_Y + k * 8);
    tela.print(linhas[n % HIST]);
  }
  tela.setCursor(1, AREA_Y + VISIVEIS * 8);
  tela.print("$ ");
  tela.print(comando);
  if (cursor) tela.print("_");
  mostrarTela();
}

// ---------- comandos com animacao ----------

static void matrix() {
  int y[COLS];
  for (int c = 0; c < COLS; c++) y[c] = random(-40, 0);
  uint32_t fim = millis() + 4000, quadro = millis();
  while (millis() < fim) {
    Entrada e = lerEntrada();
    if (e.atirarToque || e.usarToque) break;
    tela.fillRect(0, AREA_Y, 128, AREA_H, SSD1306_BLACK);
    for (int c = 0; c < COLS; c++) {
      y[c] += random(2, 5);
      if (y[c] > AREA_H + 24) y[c] = random(-30, 0);
      for (int k = 0; k < 3; k++) {
        int yy = AREA_Y + y[c] - k * 8;
        if (yy < AREA_Y || yy > 56) continue;
        tela.drawChar(c * 6 + 1, yy, (char)random(33, 126), SSD1306_WHITE, SSD1306_BLACK, 1);
      }
    }
    barraJanela("Terminal");
    mostrarTela();
    esperarQuadro(quadro, 50);
  }
}

// Dois bonequinhos correm um pro outro e batem as maos no meio.
static void toca() {
  uint32_t quadro = millis();
  for (int t = 0; t <= 40; t++) {
    Entrada e = lerEntrada();
    if (e.atirarToque || e.usarToque) break;
    int d = t < 24 ? t * 2 : 48; // distancia andada por cada um
    int xa = 10 + d, xb = 110 - d, chao = AREA_Y + 34;
    tela.fillRect(0, AREA_Y, 128, AREA_H, SSD1306_BLACK);
    for (int lado = 0; lado < 2; lado++) {
      int x = lado ? xb : xa, s = lado ? -1 : 1;
      bool passo = (t / 3) % 2 && t < 24;
      tela.fillCircle(x, chao - 16, 3, SSD1306_WHITE);
      tela.drawFastVLine(x, chao - 12, 7, SSD1306_WHITE);
      tela.drawLine(x, chao - 5, x - 3 + (passo ? 2 : 0), chao, SSD1306_WHITE);
      tela.drawLine(x, chao - 5, x + 3 - (passo ? 2 : 0), chao, SSD1306_WHITE);
      // braco da frente sobe na hora da batida
      tela.drawLine(x, chao - 10, x + s * 4, t >= 24 ? chao - 16 : chao - 8, SSD1306_WHITE);
    }
    if (t == 24) som(1200, 2400, 120, 1, 3);
    if (t >= 24) {
      textoCentro("TOCA AQUI!", AREA_Y + 2, 1);
      tela.drawLine(58, chao - 22, 55, chao - 26, SSD1306_WHITE);
      tela.drawLine(70, chao - 22, 73, chao - 26, SSD1306_WHITE);
    }
    textoCentro("Bruno & Caio", 56, 1);
    barraJanela("Terminal");
    mostrarTela();
    esperarQuadro(quadro, 50);
  }
}

// ---------- interpretador ----------

static const char *const FRASES[] = {
  "funciona na minha maquina",
  "nao e bug, e feature",
  "Ctrl+C Ctrl+V, a base de tudo",
  "erro 404: sono nao encontrado",
  "brothers > bugs",
  "compilou de primeira? desconfie",
  "SENAI: onde o LED pisca",
  "64 KB devem bastar pra todo mundo",
};

static const char *const RECORDES[][2] = {
  {"tetris", "tetris"}, {"cobrinha", "cobrinha"}, {"flappy", "flappy"},
  {"invasores", "invasores"}, {"corrida", "corrida"}, {"dino", "dino"},
  {"pong", "pong"}, {"tijolos", "tijolos"}, {"minado", "minado"},
};

// Na mesma ordem dos ICONE_* (a partir do ICONE_DOOM).
static const char *const APPS[] = {
  "doom", "tetris", "cobrinha", "flappy", "invasores", "corrida", "dino",
  "pong", "tijolos", "minado", "terminal", "versiculo", "brothers"};

// Devolve o icone a abrir (comando "abrir"), -2 pra sair, -1 pra continuar.
static int executar(char *cmd) {
  while (*cmd == ' ') cmd++;
  for (char *p = cmd; *p; p++) *p = tolower(*p);
  char *arg = strchr(cmd, ' ');
  if (arg) { *arg++ = 0; while (*arg == ' ') arg++; }

  if (!*cmd) return -1;
  if (!strcmp(cmd, "ajuda") || !strcmp(cmd, "help")) {
    escrever("neofetch versiculo hora recordes memoria wifi ls frase toca matrix echo abrir limpar reiniciar desligar sair");
  } else if (!strcmp(cmd, "neofetch")) {
    uint32_t seg = millis() / 1000;
    escrever("[]  bros@os");
    escrever("[]  BrothersOS 95");
    escrever("    ESP32-S3");
    escrever("CPU   2x 240 MHz");
    escrever("RAM   8 MB PSRAM");
    escrever("disco 16 MB");
    escreverf("ligada %luh%02lum", (unsigned long)(seg / 3600), (unsigned long)(seg / 60 % 60));
    escreverf("temp  %.1f C", temperatureRead());
  } else if (!strcmp(cmd, "hora")) {
    if (arg && *arg) {
      int h, m;
      if (sscanf(arg, "%d:%d", &h, &m) == 2 && h >= 0 && h < 24 && m >= 0 && m < 60) {
        acertarHora(h, m);
        escreverf("hora ajustada: %02d:%02d", h, m);
      } else {
        escrever("use: hora 14:30");
      }
    } else if (horaCerta()) {
      char buf[24];
      textoData(buf, sizeof(buf));
      escrever(buf);
    } else {
      escrever("sem hora: abra a pagina no celular ou use hora 14:30");
    }
  } else if (!strcmp(cmd, "recordes")) {
    Preferences mem;
    mem.begin("recordes", true);
    for (auto &r : RECORDES) escreverf("%-10s %d", r[0], mem.getInt(r[1], 0));
    mem.end();
  } else if (!strcmp(cmd, "memoria")) {
    escreverf("RAM   %u KB livre", (unsigned)(heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024));
    escreverf("PSRAM %u KB livre", (unsigned)(ESP.getFreePsram() / 1024));
  } else if (!strcmp(cmd, "wifi")) {
    escrever("rede  BrothersOS");
    escrever("senha doom1993");
    escreverf("%d conectado(s)", WiFi.softAPgetStationNum());
  } else if (!strcmp(cmd, "ls")) {
    escrever("doom.wad tetris.exe cobrinha.exe flappy.exe invasor.exe corrida.exe dino.exe pong.exe tijolos.exe minado.exe");
  } else if (!strcmp(cmd, "versiculo")) {
    char v[240];
    textoVersiculo(v, sizeof(v));
    tela.cp437(true);
    escrever(v);
  } else if (!strcmp(cmd, "frase")) {
    escrever(FRASES[random(sizeof(FRASES) / sizeof(FRASES[0]))]);
  } else if (!strcmp(cmd, "toca") || !strcmp(cmd, "brothers")) {
    toca();
    escrever("Bruno & Caio, os brothers");
  } else if (!strcmp(cmd, "matrix")) {
    matrix();
    escrever("acorda, brother...");
  } else if (!strcmp(cmd, "echo")) {
    escrever(arg ? arg : "");
  } else if (!strcmp(cmd, "sudo")) {
    escrever("permissao negada: so o Bruno e o Caio sao admin");
  } else if (!strcmp(cmd, "abrir")) {
    for (int i = 0; i < (int)(sizeof(APPS) / sizeof(APPS[0])); i++)
      if (arg && !strcmp(arg, APPS[i])) return ICONE_DOOM + i;
    escrever("use: abrir tetris");
  } else if (!strcmp(cmd, "limpar") || !strcmp(cmd, "clear")) {
    total = 0;
  } else if (!strcmp(cmd, "reiniciar")) {
    brosReiniciar();
  } else if (!strcmp(cmd, "desligar")) {
    brosDesligar();
  } else if (!strcmp(cmd, "sair") || !strcmp(cmd, "exit")) {
    return -2;
  } else {
    escreverf("comando nao encontrado: %s", cmd);
  }
  return -1;
}

static const char *const MENU[] = {
  "ajuda", "neofetch", "versiculo", "hora", "recordes", "memoria", "wifi", "ls", "frase", "toca",
  "matrix", "abrir doom", "abrir minado", "limpar", "reiniciar", "desligar", "sair"};
static const int NUM_MENU = sizeof(MENU) / sizeof(MENU[0]);

int abrirTerminal() {
  if (total == 0) {
    escrever("BrothersOS 95");
    escrever("digite ajuda (ou use");
    escrever("o campo no celular)");
  }
  int sel = 0;
  uint32_t quadro = millis();
  delay(150);

  for (;;) {
    Entrada e = lerEntrada();
    if (e.sair || e.usarToque) return -1;
    if (e.cimaToque) sel = (sel + NUM_MENU - 1) % NUM_MENU;
    if (e.baixoToque) sel = (sel + 1) % NUM_MENU;

    char cmd[40] = "";
    bool rodar = false;
    if (pegarComandoWeb(cmd, sizeof(cmd))) {
      rodar = true;
    } else if (e.atirarToque) {
      strlcpy(cmd, MENU[sel], sizeof(cmd));
      rodar = true;
    }
    if (rodar) {
      escreverf("$ %s", cmd);
      som(1000, 1000, 15);
      int r = executar(cmd);
      if (r == -2) return -1;
      if (r >= 0) return r;
    }

    desenhar(MENU[sel], (millis() / 400) % 2);
    esperarQuadro(quadro, 30);
  }
}
