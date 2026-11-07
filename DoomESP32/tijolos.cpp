// Tijolos (estilo Arkanoid): joystick pros lados move a raquete, atirar
// lanca a bola. 3 vidas; limpou a tela, a proxima fase vem mais rapida.

#include "jogos.h"

static const int COLS = 8, LINS = 4, TW = 15, TH = 4, Y_TIJ = AREA_Y + 3;
static const int RAQ_W = 18, Y_RAQ = 61;

static int partida() {
  uint8_t tijolo[LINS][COLS];
  float rx = 55, bx = 0, by = 0, vx = 0, vy = 0, vel = 1.5f;
  int pontos = 0, vidas = 3, restam = 0;
  bool presa = true; // bola parada em cima da raquete esperando o atirar
  uint32_t quadro = millis();

  auto novaFase = [&]() {
    memset(tijolo, 1, sizeof(tijolo));
    restam = LINS * COLS;
    presa = true;
  };
  novaFase();

  for (;;) {
    Entrada e = lerEntrada();
    if (e.sair) return -1;

    if (fabsf(e.x) > 0.15f) rx += e.x * 3.0f;
    rx = constrain(rx, 0, 128 - RAQ_W);

    if (presa) {
      bx = rx + RAQ_W / 2 - 1;
      by = Y_RAQ - 3;
      if (e.atirarToque) {
        presa = false;
        vx = vel * (random(2) ? 0.6f : -0.6f);
        vy = -vel;
        som(800, 1200, 30);
      }
    } else {
      bx += vx;
      by += vy;
      if (bx < 0) { bx = 0; vx = -vx; som(600, 600, 10); }
      if (bx > 126) { bx = 126; vx = -vx; som(600, 600, 10); }
      if (by < AREA_Y) { by = AREA_Y; vy = -vy; som(600, 600, 10); }
      // raquete: onde bate decide o angulo
      if (vy > 0 && by + 2 >= Y_RAQ && by <= Y_RAQ + 1 && bx + 2 >= rx && bx <= rx + RAQ_W) {
        float k = (bx + 1 - (rx + RAQ_W / 2.0f)) / (RAQ_W / 2.0f); // -1 a +1
        vx = vel * 1.2f * k;
        vy = -sqrtf(max(0.3f, vel * vel * 1.6f - vx * vx));
        by = Y_RAQ - 2;
        som(900, 1100, 20);
      }
      // tijolos: acha o que a bola tocou e rebate pro lado certo
      int c = ((int)bx + 1) / (TW + 1), l = ((int)by + 1 - Y_TIJ) / (TH + 1);
      if ((int)by + 1 >= Y_TIJ && c < COLS && l < LINS && tijolo[l][c]) {
        tijolo[l][c] = 0;
        restam--;
        pontos += 10 * (LINS - l);
        vy = -vy;
        som(1200 + l * 200, 1500 + l * 200, 30, 0, 2);
        if (restam == 0) {
          vel = min(vel + 0.3f, 3.0f);
          som(800, 1800, 300, 0, 4);
          novaFase();
        }
      }
      if (by > 64) {
        vidas--;
        som(400, 100, 400, 0, 5);
        if (vidas == 0) return pontos;
        presa = true;
      }
    }

    tela.clearDisplay();
    for (int l = 0; l < LINS; l++)
      for (int c = 0; c < COLS; c++)
        if (tijolo[l][c]) tela.fillRect(c * (TW + 1), Y_TIJ + l * (TH + 1), TW, TH, SSD1306_WHITE);
    tela.fillRect((int)rx, Y_RAQ, RAQ_W, 2, SSD1306_WHITE);
    tela.fillRect((int)bx, (int)by, 2, 2, SSD1306_WHITE);
    char titulo[16];
    snprintf(titulo, sizeof(titulo), "TIJOLOS v%d", vidas);
    barraTitulo(titulo, pontos);
    mostrarTela();
    esperarQuadro(quadro, 20);
  }
}

void jogarTijolos() {
  for (;;) {
    int p = partida();
    if (p < 0 || !fimDeJogo("tijolos", p)) return;
  }
}
