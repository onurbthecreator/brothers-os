// Campo Minado: 16x6 casas de 8 px, 14 minas. Joystick move o cursor,
// atirar abre a casa, trocar arma poe/tira a bandeira. A primeira casa
// aberta nunca tem mina. Ganhando, pontos = 1000 - segundos; perdendo,
// pontos = casas abertas.

#include "jogos.h"

static const int COLS = 16, LINS = 6, CEL = 8, MINAS = 14;

static uint8_t mina[LINS][COLS], vizinhas[LINS][COLS];
static uint8_t estado[LINS][COLS]; // 0 fechada, 1 aberta, 2 bandeira

static const char *const BANDEIRA[] = {
  "..o....",
  "..ooo..",
  "..oooo.",
  "..o....",
  "..o....",
  ".ooo...",
};

static void sortearMinas(int c0, int l0) {
  memset(mina, 0, sizeof(mina));
  for (int n = 0; n < MINAS;) {
    int c = random(COLS), l = random(LINS);
    if (mina[l][c] || (abs(c - c0) <= 1 && abs(l - l0) <= 1)) continue;
    mina[l][c] = 1;
    n++;
  }
  for (int l = 0; l < LINS; l++)
    for (int c = 0; c < COLS; c++) {
      int n = 0;
      for (int dl = -1; dl <= 1; dl++)
        for (int dc = -1; dc <= 1; dc++) {
          int ll = l + dl, cc = c + dc;
          if (ll >= 0 && ll < LINS && cc >= 0 && cc < COLS) n += mina[ll][cc];
        }
      vizinhas[l][c] = n;
    }
}

// Abre a casa; sem minas em volta, abre as vizinhas tambem (fila, sem recursao).
static int abrir(int c0, int l0) {
  int abertas = 0;
  int16_t fila[LINS * COLS];
  int ini = 0, fim = 0;
  fila[fim++] = l0 * COLS + c0;
  while (ini < fim) {
    int l = fila[ini] / COLS, c = fila[ini] % COLS;
    ini++;
    if (estado[l][c] != 0) continue;
    estado[l][c] = 1;
    abertas++;
    if (vizinhas[l][c]) continue;
    for (int dl = -1; dl <= 1; dl++)
      for (int dc = -1; dc <= 1; dc++) {
        int ll = l + dl, cc = c + dc;
        if (ll >= 0 && ll < LINS && cc >= 0 && cc < COLS && estado[ll][cc] == 0 && fim < LINS * COLS)
          fila[fim++] = ll * COLS + cc;
      }
  }
  return abertas;
}

static void desenhar(int cc, int cl, bool mostrarMinas, int bandeiras, uint32_t seg) {
  tela.clearDisplay();
  tela.setTextSize(1);
  for (int l = 0; l < LINS; l++)
    for (int c = 0; c < COLS; c++) {
      int x = c * CEL, y = AREA_Y + l * CEL;
      if (estado[l][c] == 1) {
        tela.drawPixel(x, y, SSD1306_WHITE); // pontinho no canto marca a grade
        if (vizinhas[l][c]) tela.drawChar(x + 1, y, '0' + vizinhas[l][c], SSD1306_WHITE, SSD1306_BLACK, 1);
      } else if (mostrarMinas && mina[l][c]) {
        tela.fillCircle(x + 3, y + 3, 2, SSD1306_WHITE);
      } else {
        tela.fillRect(x, y, CEL - 1, CEL - 1, SSD1306_WHITE);
        if (estado[l][c] == 2) desenharSprite(x, y, BANDEIRA, 6);
      }
    }
  if ((millis() / 250) % 2 || mostrarMinas) tela.drawRect(cc * CEL - 1, AREA_Y + cl * CEL - 1, CEL + 1, CEL + 1, SSD1306_INVERSE);
  char titulo[24];
  snprintf(titulo, sizeof(titulo), "MINADO bombas:%d %lus", MINAS - bandeiras, (unsigned long)min(seg, 999UL));
  tela.setTextColor(SSD1306_WHITE);
  tela.setCursor(0, 4);
  tela.print(titulo);
  mostrarTela();
}

static int partida() {
  memset(estado, 0, sizeof(estado));
  int cc = COLS / 2, cl = LINS / 2, abertas = 0, bandeiras = 0;
  bool comecou = false;
  uint32_t inicio = 0, quadro = millis();
  delay(150);

  for (;;) {
    Entrada e = lerEntrada();
    if (e.sair) return -1;
    if (e.esqToque) cc = (cc + COLS - 1) % COLS;
    if (e.dirToque) cc = (cc + 1) % COLS;
    if (e.cimaToque) cl = (cl + LINS - 1) % LINS;
    if (e.baixoToque) cl = (cl + 1) % LINS;

    if (e.armaToque && estado[cl][cc] != 1) {
      estado[cl][cc] = estado[cl][cc] == 2 ? 0 : 2;
      bandeiras += estado[cl][cc] == 2 ? 1 : -1;
      som(1000, 1300, 20);
    }
    if (e.atirarToque && estado[cl][cc] == 0) {
      if (!comecou) { sortearMinas(cc, cl); comecou = true; inicio = millis(); }
      if (mina[cl][cc]) {
        som(500, 60, 700, 1, 5);
        uint32_t fim = millis() + 2000;
        while (millis() < fim) { desenhar(cc, cl, true, bandeiras, (millis() - inicio) / 1000); delay(30); }
        return abertas;
      }
      abertas += abrir(cc, cl);
      som(900, 1200, 20);
      if (abertas == COLS * LINS - MINAS) {
        uint32_t seg = (millis() - inicio) / 1000;
        som(800, 1800, 400, 0, 4);
        uint32_t fim = millis() + 1500;
        while (millis() < fim) { desenhar(cc, cl, true, bandeiras, seg); delay(30); }
        return max(100, 1000 - (int)seg);
      }
    }

    desenhar(cc, cl, false, bandeiras, comecou ? (millis() - inicio) / 1000 : 0);
    esperarQuadro(quadro, 30);
  }
}

void jogarMinado() {
  for (;;) {
    int p = partida();
    if (p < 0 || !fimDeJogo("minado", p)) return;
  }
}
