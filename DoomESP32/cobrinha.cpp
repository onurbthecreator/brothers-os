// Cobrinha: grade 32x12 de quadradinhos de 4 px. Bater na borda ou no
// proprio corpo acaba o jogo; cada comida deixa mais rapida.

#include "jogos.h"

static const int C = 4, W = 32, H = 12;
struct Ponto { int8_t x, y; };
static Ponto corpo[W * H + 1];
static int tamanho;
static Ponto comida;

static void novaComida() {
  for (;;) {
    comida = {(int8_t)random(W), (int8_t)random(H)};
    bool livre = true;
    for (int i = 0; i < tamanho; i++)
      if (corpo[i].x == comida.x && corpo[i].y == comida.y) { livre = false; break; }
    if (livre) return;
  }
}

static int partida() {
  tamanho = 3;
  for (int i = 0; i < tamanho; i++) corpo[i] = {(int8_t)(8 - i), 6};
  int dx = 1, dy = 0, ndx = 1, ndy = 0, pontos = 0;
  uint32_t passo = 160, ultimo = millis(), quadro = millis();
  novaComida();

  for (;;) {
    Entrada e = lerEntrada();
    if (e.sair) return -1;

    // direcao pelo lado mais empurrado; nao deixa voltar pra tras
    if (fabsf(e.x) > 0.5f || fabsf(e.y) > 0.5f) {
      if (fabsf(e.x) > fabsf(e.y)) {
        if (dx == 0) { ndx = e.x > 0 ? 1 : -1; ndy = 0; }
      } else {
        if (dy == 0) { ndx = 0; ndy = e.y > 0 ? -1 : 1; }
      }
    }

    if (millis() - ultimo >= passo) {
      ultimo = millis();
      dx = ndx; dy = ndy;
      Ponto cabeca = {(int8_t)(corpo[0].x + dx), (int8_t)(corpo[0].y + dy)};
      bool bateu = cabeca.x < 0 || cabeca.x >= W || cabeca.y < 0 || cabeca.y >= H;
      for (int i = 0; i < tamanho - 1 && !bateu; i++)
        if (corpo[i].x == cabeca.x && corpo[i].y == cabeca.y) bateu = true;
      if (bateu) { som(500, 100, 400, 0, 5); return pontos; }

      for (int i = tamanho; i > 0; i--) corpo[i] = corpo[i - 1];
      corpo[0] = cabeca;
      if (cabeca.x == comida.x && cabeca.y == comida.y) {
        tamanho++;
        pontos += 10;
        if (passo > 60) passo -= 5;
        som(1000, 1500, 40, 0, 2);
        novaComida();
      }
    }

    tela.clearDisplay();
    for (int i = 0; i < tamanho; i++) {
      int t = i == 0 ? C : C - 1; // cabeca inteira, corpo com frestinha
      tela.fillRect(corpo[i].x * C, AREA_Y + corpo[i].y * C, t, t, SSD1306_WHITE);
    }
    if ((millis() / 200) % 2) tela.fillRect(comida.x * C, AREA_Y + comida.y * C, C - 1, C - 1, SSD1306_WHITE);
    else tela.drawRect(comida.x * C, AREA_Y + comida.y * C, C - 1, C - 1, SSD1306_WHITE);
    barraTitulo("COBRINHA", pontos);
    mostrarTela();
    esperarQuadro(quadro, 16);
  }
}

void jogarCobrinha() {
  for (;;) {
    int p = partida();
    if (p < 0 || !fimDeJogo("cobrinha", p)) return;
  }
}
