#include <SPI.h>
#include <Adafruit_ST7735.h>
#include "tela.h"

// Tela 0,96" IPS: INITR_MINI160x80_PLUGIN ja liga a inversao de cores que o
// painel IPS precisa. Se as cores vierem trocadas (branco virando preto), troque
// por INITR_MINI160x80. Se ficar de cabeca pra baixo, troque a rotacao 3 por 1.
#define TFT_TIPO    INITR_MINI160x80_PLUGIN
#define TFT_ROTACAO 3

static Adafruit_ST7735 tft(&SPI, PINO_TFT_CS, PINO_TFT_DC, PINO_TFT_RST);

// No DoomESP32.ino: copia a tela colorida pro navegador.
void copiarCorWeb(const uint16_t *px);

// Folha na TFT: sem ampliar, no meio (16 px de cada lado, 8 em cima e embaixo).
static const int X_FOLHA = 16, Y_FOLHA = 8;

enum Modo { NADA, FOLHA, COR, DOOM };
static Modo modo = NADA;
static uint16_t linha[TELA_W];

void TelaBros::iniciar() {
  SPI.begin(PINO_TFT_SCK, -1, PINO_TFT_SDA, PINO_TFT_CS);
  tft.initR(TFT_TIPO);
  tft.setRotation(TFT_ROTACAO);
  tft.setSPISpeed(40000000);
  tft.fillScreen(ST77XX_BLACK);
  canvas = new GFXcanvas16(TELA_W, TELA_H); // 25 KB, vai pra PSRAM
  clearDisplay();
}

void TelaBros::drawPixel(int16_t x, int16_t y, uint16_t cor) {
  if (x < 0 || y < 0 || x >= 128 || y >= 64) return;
  uint8_t &b = folha[x + (y >> 3) * 128];
  uint8_t m = 1 << (y & 7);
  if (cor == SSD1306_WHITE) b |= m;
  else if (cor == SSD1306_INVERSE) b ^= m;
  else b &= ~m;
}

// Moldura da folha: barra de titulo do BrothersOS em cima, janela afundada no
// meio e a dica de como fechar embaixo, tudo no cinza do Windows 95.
static void molduraFolha() {
  tft.fillScreen(COR_CINZA);
  for (int x = 0; x < TELA_W; x++) {
    int k = x * 16 / (TELA_W - 1); // azul escuro -> azul claro
    tft.drawFastVLine(x, 0, 7, rgb565(16 * k / 16, 132 * k / 16, 128 + 80 * k / 16));
  }
  tft.setTextSize(1);
  tft.setTextColor(COR_BRANCO);
  tft.setCursor(3, 0);
  tft.print("BrothersOS");
  // borda afundada em volta da folha
  tft.drawFastHLine(X_FOLHA - 1, Y_FOLHA - 1, 130, COR_CINZA_ESC);
  tft.drawFastVLine(X_FOLHA - 1, Y_FOLHA - 1, 66, COR_CINZA_ESC);
  tft.drawFastHLine(X_FOLHA - 1, Y_FOLHA + 64, 130, COR_BRANCO);
  tft.drawFastVLine(X_FOLHA + 128, Y_FOLHA - 1, 66, COR_BRANCO);
  tft.setTextColor(COR_CINZA_ESC);
  tft.setCursor(26, 73);
  tft.print("usar+arma: fechar");
}

void TelaBros::display() {
  if (modo == DOOM) return;
  if (modo != FOLHA) { molduraFolha(); modo = FOLHA; }
  tft.startWrite();
  tft.setAddrWindow(X_FOLHA, Y_FOLHA, 128, 64);
  for (int y = 0; y < 64; y++) {
    // faixa de cima: barra de titulo (branco no azul); resto: preto no branco
    uint16_t fundo = y < 16 ? COR_AZUL : COR_BRANCO;
    uint16_t tinta = y < 16 ? COR_BRANCO : COR_PRETO;
    const uint8_t *col = folha + (y >> 3) * 128;
    uint8_t m = 1 << (y & 7);
    for (int x = 0; x < 128; x++) linha[x] = (col[x] & m) ? tinta : fundo;
    tft.writePixels(linha, 128, true, false);
  }
  tft.endWrite();
}

void TelaBros::mostrarCor() {
  if (!canvas || modo == DOOM) return;
  modo = COR;
  tft.startWrite();
  tft.setAddrWindow(0, 0, TELA_W, TELA_H);
  tft.writePixels(canvas->getBuffer(), TELA_W * TELA_H, true, false);
  tft.endWrite();
  copiarCorWeb(canvas->getBuffer());
}

void TelaBros::ligar(bool sim) {
  if (!sim) tft.fillScreen(ST77XX_BLACK);
  tft.enableDisplay(sim);
  if (sim) modo = NADA;
}

static void textoSombra(GFXcanvas16 &c, int x, int y, const char *texto, uint16_t cor) {
  c.setTextColor(COR_PRETO);
  c.setCursor(x + 1, y + 1);
  c.print(texto);
  c.setTextColor(cor);
  c.setCursor(x, y);
  c.print(texto);
}

void TelaBros::desenharDoom(const uint8_t *imagem, const uint16_t *paleta565, bool jogando, int vida, int municao) {
  modo = DOOM;
  uint16_t *px = canvas->getBuffer();
  for (int y = 0; y < TELA_H; y++) {
    // jogando: linhas 20 a 178 de 200 (um pixel a cada 2); menu: tudo achatado
    int sy = jogando ? 20 + y * 2 : y * 200 / TELA_H;
    const uint8_t *src = imagem + sy * 320;
    for (int x = 0; x < TELA_W; x++) *px++ = paleta565[src[x * 2]];
  }
  if (jogando) {
    char buf[8];
    canvas->setTextSize(1);
    snprintf(buf, sizeof(buf), "+%d", vida);
    textoSombra(*canvas, 2, 71, buf, COR_VERMELHO);
    if (municao >= 0) {
      snprintf(buf, sizeof(buf), "%d", municao);
      textoSombra(*canvas, TELA_W - 3 - (int)strlen(buf) * 6, 71, buf, COR_AMARELO);
    }
  }
  tft.startWrite();
  tft.setAddrWindow(0, 0, TELA_W, TELA_H);
  tft.writePixels(canvas->getBuffer(), TELA_W * TELA_H, true, false);
  tft.endWrite();
}
