// Dino: corre sozinho. Atirar (ou frente) pula os cactos; joystick pra
// tras abaixa pra passar por baixo dos passaros.

#include "jogos.h"

static const int CHAO = 62;
static const int DINO_X = 10;

static const char *const DINO_A[] = {
  "......######",
  ".....##o####",
  ".....#######",
  ".....###....",
  "#...######..",
  "##.#####.#..",
  "#########...",
  ".#######....",
  "..#####.....",
  "..##..#.....",
  "..#...##....",
  "............",
};
static const char *const DINO_B[] = {
  "......######",
  ".....##o####",
  ".....#######",
  ".....###....",
  "#...######..",
  "##.#####.#..",
  "#########...",
  ".#######....",
  "..#####.....",
  "..#...##....",
  "..##........",
  "............",
};
static const char *const DINO_ABAIXADO[] = {
  "........######",
  "#.....###o####",
  "##############",
  ".############.",
  "..##.....##...",
  "..#.......#...",
};
static const char *const PASSARO_A[] = {
  "..#......",
  ".###.....",
  "#########",
  "....##...",
  "....#....",
};
static const char *const PASSARO_B[] = {
  "....#....",
  "....##...",
  "#########",
  ".###.....",
  "..#......",
};

enum { CACTO_PEQUENO, CACTO_GRANDE, PASSARO };
struct Obstaculo { float x; int tipo; };

static bool encosta(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
  return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

static int partida() {
  float altura = 0, vy = 0, falta = 140;
  Obstaculo obs[3];
  for (auto &o : obs) o.x = -100;
  long tempo = 0;
  int pontos = 0;
  uint32_t quadro = millis();

  for (;;) {
    Entrada e = lerEntrada();
    if (e.sair) return -1;

    bool noChao = altura <= 0 && vy <= 0;
    if (noChao && (e.atirarToque || e.cimaToque || e.usarToque)) { vy = 3.3f; som(600, 900, 50); }
    bool abaixado = noChao && e.y < -0.5f;
    if (!noChao) {
      altura += vy;
      vy -= 0.24f;
      if (altura <= 0) { altura = 0; vy = 0; }
    }

    float vel = min(5.0f, 2.0f + pontos / 400.0f);
    if (++tempo % 3 == 0) {
      pontos++;
      if (pontos % 100 == 0) som(900, 1400, 80, 0, 1);
    }

    falta -= vel;
    if (falta <= 0) {
      for (auto &o : obs) {
        if (o.x > -100) continue;
        o.x = 130;
        o.tipo = (pontos > 150 && random(4) == 0) ? PASSARO : random(2);
        break;
      }
      falta = random(60, 120) + vel * 8;
    }

    int dinoY = CHAO - 11 - (int)altura;
    for (auto &o : obs) {
      if (o.x <= -100) continue;
      o.x -= vel;
      if (o.x < -20) { o.x = -100; continue; }
      int ox = (int)o.x;
      bool bateu;
      int obx, oby, obw, obh;
      if (o.tipo == CACTO_PEQUENO) { obx = ox; oby = CHAO - 8; obw = 4; obh = 8; }
      else if (o.tipo == CACTO_GRANDE) { obx = ox; oby = CHAO - 12; obw = 5; obh = 12; }
      else { obx = ox; oby = CHAO - 13; obw = 9; obh = 5; }
      if (abaixado) bateu = encosta(DINO_X, CHAO - 5, 13, 5, obx, oby, obw, obh);
      else bateu = encosta(DINO_X + 2, dinoY + 1, 8, 10, obx, oby, obw, obh);
      if (bateu) {
        som(400, 80, 500, 0, 5);
        delay(400);
        return pontos;
      }
    }

    tela.clearDisplay();
    tela.drawFastHLine(0, CHAO + 1, 128, SSD1306_WHITE);
    for (int k = 0; k < 5; k++) { // pedrinhas no chao andando
      int x = 127 - (int)(tempo * vel + k * 29) % 128;
      tela.drawPixel(x, CHAO + 1, SSD1306_BLACK);
    }
    if (abaixado) desenharSprite(DINO_X, CHAO - 5, DINO_ABAIXADO, 6);
    else desenharSprite(DINO_X, dinoY, (noChao && (tempo / 4) % 2) ? DINO_B : DINO_A, 12);
    for (auto &o : obs) {
      if (o.x <= -100) continue;
      int ox = (int)o.x;
      if (o.tipo == CACTO_PEQUENO) {
        tela.fillRect(ox + 1, CHAO - 8, 2, 9, SSD1306_WHITE);
        tela.fillRect(ox, CHAO - 6, 1, 3, SSD1306_WHITE);
        tela.fillRect(ox + 3, CHAO - 5, 1, 2, SSD1306_WHITE);
      } else if (o.tipo == CACTO_GRANDE) {
        tela.fillRect(ox + 1, CHAO - 12, 3, 13, SSD1306_WHITE);
        tela.fillRect(ox, CHAO - 9, 1, 4, SSD1306_WHITE);
        tela.fillRect(ox + 4, CHAO - 10, 1, 4, SSD1306_WHITE);
      } else {
        desenharSprite(ox, CHAO - 13, (tempo / 6) % 2 ? PASSARO_A : PASSARO_B, 5);
      }
    }
    barraTitulo("DINO", pontos);
    mostrarTela();
    esperarQuadro(quadro, 20);
  }
}

void jogarDino() {
  for (;;) {
    int p = partida();
    if (p < 0 || !fimDeJogo("dino", p)) return;
  }
}
