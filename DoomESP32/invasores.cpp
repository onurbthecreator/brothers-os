// Invasores: joystick move a nave, atirar (ou trocar arma) atira.
// 3 vidas; cada onda vencida volta mais rapida.

#include "jogos.h"

static const uint8_t ALIEN[2][6] PROGMEM = {
  {0x18, 0x3C, 0x7E, 0xDB, 0xFF, 0x24},
  {0x18, 0x3C, 0x7E, 0xDB, 0xFF, 0x5A},
};
static const int COLS = 7, LINS = 3, AW = 8, AH = 6, PASSO_X = 12, PASSO_Y = 9;
static const int NAVE_Y = 57;

static int partida() {
  int pontos = 0, vidas = 3, onda = 0;

  for (;;) {
    bool vivo[LINS][COLS];
    for (int r = 0; r < LINS; r++) for (int c = 0; c < COLS; c++) vivo[r][c] = true;
    int vivos = LINS * COLS;
    float ax = 6, ay = AREA_Y + 2 + min(onda, 3) * 2;
    int dir = 1, quadroAlien = 0;
    uint32_t passoMs = max(120, 450 - onda * 60), ultimoPasso = millis();
    float px = 60, tiroY = 0;
    int tiroX = -1;
    float bx[3], byy[3];
    bool bAtivo[3] = {false, false, false};
    uint32_t invencivel = 0, quadro = millis();

    while (vivos > 0) {
      Entrada e = lerEntrada();
      if (e.sair) return -1;

      px = constrain(px + e.x * 2.5f, 0, 119);
      if ((e.atirarToque || e.armaToque) && tiroX < 0) {
        tiroX = (int)px + 4;
        tiroY = NAVE_Y - 1;
        som(1500, 600, 60, 0, 2);
      }

      if (tiroX >= 0) {
        tiroY -= 3.5f;
        if (tiroY < AREA_Y) tiroX = -1;
        for (int r = 0; r < LINS && tiroX >= 0; r++) {
          for (int c = 0; c < COLS; c++) {
            if (!vivo[r][c]) continue;
            int x = ax + c * PASSO_X, y = ay + r * PASSO_Y;
            if (tiroX >= x && tiroX < x + AW && tiroY >= y && tiroY < y + AH) {
              vivo[r][c] = false;
              vivos--;
              pontos += (LINS - r) * 10;
              tiroX = -1;
              if (passoMs > 60) passoMs -= 8;
              som(300, 80, 120, 1, 3);
              break;
            }
          }
        }
      }

      if (millis() - ultimoPasso >= passoMs) {
        ultimoPasso = millis();
        quadroAlien ^= 1;
        int cmin = COLS, cmax = -1, rmax = -1;
        for (int r = 0; r < LINS; r++)
          for (int c = 0; c < COLS; c++)
            if (vivo[r][c]) { cmin = min(cmin, c); cmax = max(cmax, c); rmax = max(rmax, r); }
        float novoX = ax + dir * 2;
        if (novoX + cmin * PASSO_X < 0 || novoX + cmax * PASSO_X + AW > 127) { dir = -dir; ay += 3; }
        else ax = novoX;
        som(quadroAlien ? 130 : 160, quadroAlien ? 130 : 160, 40, 0, 0);
        if (ay + rmax * PASSO_Y + AH >= NAVE_Y) { som(800, 80, 600, 0, 5); return pontos; } // invadiram

        if (random(100) < 35) { // um alien da fileira de baixo atira
          int c = random(COLS);
          for (int r = LINS - 1; r >= 0; r--) {
            if (!vivo[r][c]) continue;
            for (int k = 0; k < 3; k++) {
              if (!bAtivo[k]) { bAtivo[k] = true; bx[k] = ax + c * PASSO_X + 4; byy[k] = ay + r * PASSO_Y + AH; break; }
            }
            break;
          }
        }
      }

      for (int k = 0; k < 3; k++) {
        if (!bAtivo[k]) continue;
        byy[k] += 1.6f;
        if (byy[k] > 63) { bAtivo[k] = false; continue; }
        if (bx[k] >= px && bx[k] < px + 9 && byy[k] >= NAVE_Y && byy[k] < NAVE_Y + 5 && millis() > invencivel) {
          bAtivo[k] = false;
          vidas--;
          invencivel = millis() + 1200;
          som(800, 100, 400, 1, 4);
          if (vidas <= 0) { delay(400); return pontos; }
        }
      }

      tela.clearDisplay();
      for (int r = 0; r < LINS; r++)
        for (int c = 0; c < COLS; c++)
          if (vivo[r][c]) tela.drawBitmap(ax + c * PASSO_X, ay + r * PASSO_Y, ALIEN[quadroAlien], AW, AH, SSD1306_WHITE);
      if (millis() > invencivel || (millis() / 100) % 2) {
        int x = (int)px;
        tela.drawPixel(x + 4, NAVE_Y, SSD1306_WHITE);
        tela.drawFastHLine(x + 1, NAVE_Y + 1, 7, SSD1306_WHITE);
        tela.fillRect(x, NAVE_Y + 2, 9, 3, SSD1306_WHITE);
      }
      if (tiroX >= 0) tela.drawFastVLine(tiroX, (int)tiroY, 3, SSD1306_WHITE);
      for (int k = 0; k < 3; k++)
        if (bAtivo[k]) tela.drawFastVLine((int)bx[k], (int)byy[k], 2, SSD1306_WHITE);
      barraTitulo("INVASORES", pontos);
      for (int v = 0; v < vidas; v++) tela.fillRect(58 + v * 7, 6, 5, 3, SSD1306_WHITE);
      mostrarTela();
      esperarQuadro(quadro, 25);
    }

    onda++;
    som(800, 1600, 300, 0, 3);
    delay(600);
  }
}

void jogarInvasores() {
  for (;;) {
    int p = partida();
    if (p < 0 || !fimDeJogo("invasores", p)) return;
  }
}
