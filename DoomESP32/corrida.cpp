// Corrida: pista vista de cima com 3 faixas. Joystick pros lados desvia
// dos carros que vem descendo; a velocidade sobe com a distancia.

#include "jogos.h"

static const int ESQ = 34, DIR = 94; // bordas da pista (3 faixas de 20 px)
static const int CARRO_Y = 50;

struct Carro { float x, y; bool ativo; };

static void desenharCarro(int x, int y) {
  tela.fillRoundRect(x, y, 8, 12, 2, SSD1306_WHITE);
  tela.fillRect(x + 1, y + 3, 6, 2, SSD1306_BLACK); // para-brisa
  tela.fillRect(x + 1, y + 8, 6, 1, SSD1306_BLACK); // vidro de tras
}

static int partida() {
  float px = ESQ + 26, dist = 0, faixa = 0;
  Carro carros[4] = {};
  uint32_t proximoCarro = millis() + 800, quadro = millis();

  for (;;) {
    Entrada e = lerEntrada();
    if (e.sair) return -1;

    px = constrain(px + e.x * 2.2f, ESQ + 1, DIR - 9);
    float vel = min(4.0f, 1.4f + dist / 1500.0f);
    dist += vel;
    faixa = fmodf(faixa + vel, 8);

    if (millis() >= proximoCarro) {
      for (auto &c : carros) {
        if (c.ativo) continue;
        c = {(float)(ESQ + 6 + random(3) * 20), (float)(AREA_Y - 12), true};
        break;
      }
      proximoCarro = millis() + max(350, 1100 - (int)(dist / 20));
    }

    for (auto &c : carros) {
      if (!c.ativo) continue;
      c.y += vel * 0.7f;
      if (c.y > 64) { c.ativo = false; continue; }
      if (px < c.x + 8 && px + 8 > c.x && CARRO_Y < c.y + 12 && CARRO_Y + 12 > c.y) {
        som(600, 80, 500, 1, 5);
        delay(500);
        return (int)(dist / 10);
      }
    }

    tela.clearDisplay();
    tela.drawFastVLine(ESQ, AREA_Y, AREA_H, SSD1306_WHITE);
    tela.drawFastVLine(DIR, AREA_Y, AREA_H, SSD1306_WHITE);
    for (int k = 1; k < 3; k++) // faixas tracejadas andando
      for (int y = AREA_Y - 8 + (int)faixa; y < 64; y += 8)
        tela.drawFastVLine(ESQ + k * 20, y, 4, SSD1306_WHITE);
    for (int k = 0; k < 6; k++) { // matinho dos lados, pra dar sensacao de velocidade
      int y = AREA_Y + ((int)(k * 8 + faixa * 2)) % AREA_H;
      tela.drawPixel(12 + (k * 5) % 16, y, SSD1306_WHITE);
      tela.drawPixel(104 + (k * 7) % 16, y, SSD1306_WHITE);
    }
    for (auto &c : carros) if (c.ativo) desenharCarro((int)c.x, (int)c.y);
    desenharCarro((int)px, CARRO_Y);
    barraTitulo("CORRIDA", (long)(dist / 10));
    mostrarTela();
    esperarQuadro(quadro, 25);
  }
}

void jogarCorrida() {
  for (;;) {
    int p = partida();
    if (p < 0 || !fimDeJogo("corrida", p)) return;
  }
}
