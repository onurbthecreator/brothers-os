// Tetris: campo 10x16 com quadradinhos de 3 px na parte azul da tela.
// Joystick move (pra tras desce rapido), atirar ou frente gira,
// trocar arma derruba de vez.

#include "jogos.h"

// Cada peca em 4 rotacoes, grade 4x4 (bit 15 = canto de cima a esquerda).
static const uint16_t PECAS[7][4] = {
  {0x0F00, 0x2222, 0x00F0, 0x4444}, // I
  {0x44C0, 0x8E00, 0x6440, 0x0E20}, // J
  {0x4460, 0x0E80, 0xC440, 0x2E00}, // L
  {0xCC00, 0xCC00, 0xCC00, 0xCC00}, // O
  {0x06C0, 0x8C40, 0x6C00, 0x4620}, // S
  {0x0E40, 0x4C40, 0x4E00, 0x4640}, // T
  {0x0C60, 0x4C80, 0xC600, 0x2640}, // Z
};
static const int COLS = 10, LINS = 16, CEL = 3, OX = 49;
static uint8_t campo[LINS][COLS];

static bool cabe(int p, int r, int px, int py) {
  for (int i = 0; i < 16; i++) {
    if (!(PECAS[p][r] & (0x8000 >> i))) continue;
    int cx = px + i % 4, cy = py + i / 4;
    if (cx < 0 || cx >= COLS || cy >= LINS) return false;
    if (cy >= 0 && campo[cy][cx]) return false;
  }
  return true;
}

static void celula(int x, int y) {
  tela.fillRect(OX + x * CEL, AREA_Y + y * CEL, CEL - 1, CEL - 1, SSD1306_WHITE);
}

static int partida() {
  memset(campo, 0, sizeof(campo));
  int p = random(7), prox = random(7), r = 0, px = 3, py = -1;
  int pontos = 0, linhas = 0;
  uint32_t queda = 600, ultimaQueda = millis(), quadro = millis();

  for (;;) {
    Entrada e = lerEntrada();
    if (e.sair) return -1;

    if (e.esqToque && cabe(p, r, px - 1, py)) px--;
    if (e.dirToque && cabe(p, r, px + 1, py)) px++;
    if (e.atirarToque || e.cimaToque) {
      int nr = (r + 1) % 4;
      const int chutes[5] = {0, -1, 1, -2, 2}; // encosta na parede e ainda gira
      for (int d : chutes) {
        if (cabe(p, nr, px + d, py)) { r = nr; px += d; som(900, 1200, 20); break; }
      }
    }
    bool travar = false;
    if (e.armaToque) {
      while (cabe(p, r, px, py + 1)) py++;
      travar = true;
      som(400, 200, 40);
    }

    uint32_t intervalo = e.y < -0.5f ? 45 : queda;
    if (travar || millis() - ultimaQueda >= intervalo) {
      ultimaQueda = millis();
      if (!travar && cabe(p, r, px, py + 1)) {
        py++;
      } else {
        for (int i = 0; i < 16; i++) {
          if (!(PECAS[p][r] & (0x8000 >> i))) continue;
          int cy = py + i / 4;
          if (cy < 0) { som(400, 80, 600, 0, 5); return pontos; }
          campo[cy][px + i % 4] = 1;
        }
        int n = 0;
        for (int y = LINS - 1; y >= 0; y--) {
          bool cheia = true;
          for (int x = 0; x < COLS; x++) if (!campo[y][x]) { cheia = false; break; }
          if (cheia) {
            n++;
            for (int yy = y; yy > 0; yy--) memcpy(campo[yy], campo[yy - 1], COLS);
            memset(campo[0], 0, COLS);
            y++; // confere a mesma linha de novo (desceu uma de cima)
          }
        }
        const int TABELA[5] = {0, 40, 100, 300, 1200};
        pontos += TABELA[n];
        linhas += n;
        queda = max(90, 600 - (linhas / 5) * 50);
        if (n) som(600, 1500, 150, 0, 3); else som(200, 150, 30);
        p = prox; prox = random(7); r = 0; px = 3; py = -1;
        if (!cabe(p, r, px, py)) { som(400, 80, 600, 0, 5); return pontos; }
      }
    }

    tela.clearDisplay();
    tela.drawFastVLine(OX - 2, AREA_Y, AREA_H, SSD1306_WHITE);
    tela.drawFastVLine(OX + COLS * CEL, AREA_Y, AREA_H, SSD1306_WHITE);
    for (int y = 0; y < LINS; y++)
      for (int x = 0; x < COLS; x++)
        if (campo[y][x]) celula(x, y);
    for (int i = 0; i < 16; i++) {
      int cy = py + i / 4;
      if ((PECAS[p][r] & (0x8000 >> i)) && cy >= 0) celula(px + i % 4, cy);
    }
    tela.setTextSize(1);
    tela.setTextColor(SSD1306_WHITE);
    tela.setCursor(90, 20);
    tela.print("PROX");
    for (int i = 0; i < 16; i++)
      if (PECAS[prox][0] & (0x8000 >> i)) tela.fillRect(94 + (i % 4) * 4, 32 + (i / 4) * 4, 3, 3, SSD1306_WHITE);
    tela.setCursor(6, 20);
    tela.print("LIN");
    tela.setCursor(6, 32);
    tela.print(linhas);
    barraTitulo("TETRIS", pontos);
    mostrarTela();
    esperarQuadro(quadro, 20);
  }
}

void jogarTetris() {
  for (;;) {
    int p = partida();
    if (p < 0 || !fimDeJogo("tetris", p)) return;
  }
}
