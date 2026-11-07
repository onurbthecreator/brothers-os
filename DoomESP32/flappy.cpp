// Flappy: atirar (ou empurrar pra frente, ou o clique) bate as asas.
// Cada cano passado vale 1 ponto e acelera um pouco.

#include "jogos.h"

static const int BX = 24, VAO = 20, CANO = 8, ESPACO = 48;

static int novoVao() { return random(AREA_Y + 3, 64 - 3 - VAO); }

static int partida() {
  float by = AREA_Y + 18, vy = 0, vel = 1.2f;
  float cx[3];
  int vao[3];
  bool passou[3];
  for (int i = 0; i < 3; i++) { cx[i] = 128 + i * ESPACO; vao[i] = novoVao(); passou[i] = false; }
  int pontos = 0;
  bool comecou = false;
  uint32_t quadro = millis();

  for (;;) {
    Entrada e = lerEntrada();
    if (e.sair) return -1;

    if (e.atirarToque || e.cimaToque || e.usarToque) {
      vy = -2.4f;
      comecou = true;
      som(700, 1100, 40);
    }

    if (comecou) {
      vy += 0.25f;
      by += vy;
      for (int i = 0; i < 3; i++) {
        cx[i] -= vel;
        if (cx[i] < -CANO) { cx[i] += 3 * ESPACO; vao[i] = novoVao(); passou[i] = false; }
        if (!passou[i] && cx[i] + CANO < BX) {
          passou[i] = true;
          pontos++;
          vel += 0.03f;
          som(1200, 1600, 30, 0, 2);
        }
      }
      bool bateu = by < AREA_Y || by + 5 > 63;
      for (int i = 0; i < 3; i++)
        if (BX + 6 > cx[i] && BX < cx[i] + CANO && (by < vao[i] || by + 5 > vao[i] + VAO)) bateu = true;
      if (bateu) {
        som(500, 100, 300, 0, 5);
        delay(400);
        return pontos;
      }
    }

    tela.clearDisplay();
    for (int i = 0; i < 3; i++) {
      int x = (int)cx[i];
      tela.fillRect(x, AREA_Y, CANO, vao[i] - AREA_Y, SSD1306_WHITE);
      tela.fillRect(x - 1, vao[i] - 3, CANO + 2, 3, SSD1306_WHITE);
      tela.fillRect(x, vao[i] + VAO, CANO, 64 - vao[i] - VAO, SSD1306_WHITE);
      tela.fillRect(x - 1, vao[i] + VAO, CANO + 2, 3, SSD1306_WHITE);
    }
    int y = (int)by;
    tela.fillRoundRect(BX, y, 7, 5, 2, SSD1306_WHITE);
    tela.drawPixel(BX + 5, y + 1, SSD1306_BLACK);              // olho
    tela.drawPixel(BX + 7, y + 2, SSD1306_WHITE);              // bico
    tela.drawPixel(BX + 1, vy < 0 ? y + 1 : y + 3, SSD1306_BLACK); // asa batendo
    if (!comecou) textoCentro("atirar = voar", 44, 1);
    barraTitulo("FLAPPY", pontos);
    mostrarTela();
    esperarQuadro(quadro, 25);
  }
}

void jogarFlappy() {
  for (;;) {
    int p = partida();
    if (p < 0 || !fimDeJogo("flappy", p)) return;
  }
}
