// BrothersOS: tela de boot, area de trabalho estilo Windows 95 com icones e
// cursor, menu Iniciar e as janelas Wi-Fi e Brothers.

#include <WiFi.h>
#include "jogos.h"

#define WIFI_TEXTO_NOME  "BrothersOS"
#define WIFI_TEXTO_SENHA "doom1993"

static const char *const I_DOOM[] = {
  "................",
  ".....######.....",
  "...##########...",
  "..############..",
  "..############..",
  "..##..####..##..",
  "..#....##....#..",
  "..#....##....#..",
  "..##..####..##..",
  "..#####..#####..",
  "...####..####...",
  "....########....",
  "....#.#..#.#....",
  "....########....",
  ".....######.....",
  "................",
};
static const char *const I_TETRIS[] = {
  "................",
  ".###.###.###....",
  ".###.###.###....",
  ".###.###.###....",
  "................",
  ".....###........",
  ".....###........",
  ".....###........",
  "................",
  ".........###....",
  ".........###....",
  ".........###....",
  ".....###.###....",
  ".....###.###....",
  ".....###.###....",
  "................",
};
static const char *const I_COBRINHA[] = {
  "................",
  "..##########....",
  "..##########....",
  "..##............",
  "..##............",
  "..##########....",
  "..##########....",
  "..........##....",
  "..........##....",
  "..##########....",
  "..########o#....",
  "................",
  "................",
  "............##..",
  "...........####.",
  "............##..",
};
static const char *const I_FLAPPY[] = {
  "................",
  "................",
  "......#####.....",
  ".....#######....",
  "....#####o###...",
  "..#######o#####.",
  ".############...",
  ".###########....",
  "..#########.....",
  "....######......",
  "................",
  "................",
  "................",
  "................",
  "................",
  "................",
};
static const char *const I_INVASORES[] = {
  "................",
  "................",
  "................",
  "...#.......#....",
  "....#.....#.....",
  "...#########....",
  "..##.#####.##...",
  ".#############..",
  ".#.#########.#..",
  ".#.#.......#.#..",
  "....##...##.....",
  "................",
  "................",
  "................",
  "................",
  "................",
};
static const char *const I_CORRIDA[] = {
  "................",
  ".....######.....",
  "....########....",
  "..#.########.#..",
  "..#.#oooooo#.#..",
  "....#oooooo#....",
  "....########....",
  "....########....",
  "....########....",
  "....#oooooo#....",
  "..#.#oooooo#.#..",
  "..#.########.#..",
  "....########....",
  ".....######.....",
  "................",
  "................",
};
static const char *const I_DINO[] = {
  "................",
  ".........######.",
  "........##o#####",
  "........########",
  "........####....",
  "........######..",
  "#......####.....",
  "##....######....",
  "###..########...",
  "############.#..",
  ".##########.....",
  "..#########.....",
  "....###.##......",
  "....##...#......",
  "....#....##.....",
  "................",
};
static const char *const I_PONG[] = {
  "................",
  "................",
  ".##.............",
  ".##.............",
  ".##.......##....",
  ".##.......##....",
  ".##.............",
  ".##.......#..##.",
  "..........#..##.",
  "..........#..##.",
  "..........#..##.",
  "..........#..##.",
  "..........#.....",
  "................",
  "................",
  "................",
};
static const char *const I_TIJOLOS[] = {
  "................",
  ".###.###.###.##.",
  ".###.###.###.##.",
  "................",
  ".##.###.###.###.",
  ".##.###.###.###.",
  "................",
  ".###.....###.##.",
  ".###.....###.##.",
  "................",
  "................",
  "......##........",
  "......##........",
  "................",
  ".....######.....",
  ".....######.....",
};
static const char *const I_MINADO[] = {
  "................",
  ".......#........",
  "...#...#...#....",
  "....#######.....",
  "...#########....",
  "...##oo######...",
  ".#####o######...",
  "...##########...",
  "...##########...",
  "....########....",
  "...#.######.#...",
  "...#...#....#...",
  ".......#........",
  "................",
  "................",
  "................",
};
static const char *const I_TERMINAL[] = {
  "................",
  ".##############.",
  ".##############.",
  ".#oooooooooooo#.",
  ".#o#oooooooooo#.",
  ".#oo#ooooooooo#.",
  ".#ooo#oooooooo#.",
  ".#oo#ooooooooo#.",
  ".#o#ooo####ooo#.",
  ".#oooooooooooo#.",
  ".#oooooooooooo#.",
  ".##############.",
  ".....######.....",
  "...##########...",
  "................",
  "................",
};
static const char *const I_VERSICULO[] = {
  "................",
  "................",
  ".######..######.",
  "#......##......#",
  "#.####.##.####.#",
  "#......##......#",
  "#.####.##.####.#",
  "#......##......#",
  "#.####.##.####.#",
  "#......##......#",
  "#.###..##..###.#",
  "#......##......#",
  ".######..######.",
  ".........#......",
  "................",
  "................",
};
// Dois bonequinhos lado a lado: os brothers.
static const char *const I_BROTHERS[] = {
  "................",
  "...###....###...",
  "..#####..#####..",
  "..#####..#####..",
  "..#####..#####..",
  "...###....###...",
  "................",
  ".#######.######.",
  "#########.######",
  "##.###.###.##.##",
  "##.###.###.##.##",
  "...###....###...",
  "...#.#....#.#...",
  "...#.#....#.#...",
  "..##.##..##.##..",
  "................",
};

// Wi-Fi na bandeja quando tem celular conectado.
static const char *const WIFI[] = {
  ".#####.",
  "#.....#",
  "..###..",
  ".#...#.",
  "...#...",
};

// Seta do mouse: '#' contorno preto, 'o' miolo branco.
static const char *const CURSOR[] = {
  "#......",
  "##.....",
  "#o#....",
  "#oo#...",
  "#ooo#..",
  "#oooo#.",
  "#ooooo#",
  "#oo####",
  "#o#....",
  "##.....",
};

static const int NUM_ICONES = 13;
static const char *const NOMES[NUM_ICONES] = {
  "Doom", "Tetris", "Cobrinha", "Flappy", "Invasores", "Corrida", "Dino",
  "Pong", "Tijolos", "Minado", "Terminal", "Versiculos", "Brothers"};
static const char *const *const ICONES[NUM_ICONES] = {
  I_DOOM, I_TETRIS, I_COBRINHA, I_FLAPPY, I_INVASORES, I_CORRIDA, I_DINO,
  I_PONG, I_TIJOLOS, I_MINADO, I_TERMINAL, I_VERSICULO, I_BROTHERS};
static const uint16_t CORES_ICONE[NUM_ICONES] = {
  rgb565(255, 80, 40), rgb565(0, 255, 255), rgb565(0, 255, 0), COR_AMARELO, rgb565(0, 255, 0),
  COR_VERMELHO, COR_CINZA, COR_BRANCO, COR_LARANJA, COR_PRETO,
  COR_CINZA, rgb565(255, 220, 120), rgb565(255, 200, 150)};

// 7 colunas x 2 linhas; a barra de tarefas fica em y 68 a 79.
static const int Y_BARRA = 68;
static int iconeX(int i) { return 4 + (i % 7) * 22; }
static int iconeY(int i) { return 6 + (i / 7) * 28; }

// ================= desenho estilo Windows 95 =================

void corBorda3D(GFXcanvas16 &c, int x, int y, int w, int h, bool alto) {
  uint16_t claro = alto ? COR_BRANCO : COR_CINZA_ESC;
  uint16_t escuro = alto ? COR_PRETO : COR_BRANCO;
  c.drawFastHLine(x, y, w - 1, claro);
  c.drawFastVLine(x, y, h - 1, claro);
  c.drawFastHLine(x, y + h - 1, w, escuro);
  c.drawFastVLine(x + w - 1, y, h, escuro);
  if (alto) { // sombra de dentro
    c.drawFastHLine(x + 1, y + h - 2, w - 2, COR_CINZA_ESC);
    c.drawFastVLine(x + w - 2, y + 1, h - 2, COR_CINZA_ESC);
  }
}

static void barraAzul(GFXcanvas16 &c, int x, int y, int w, int h) {
  for (int i = 0; i < w; i++) {
    int k = i * 16 / max(1, w - 1);
    c.drawFastVLine(x + i, y, h, rgb565(16 * k / 16, 132 * k / 16, 128 + 80 * k / 16));
  }
}

int corJanela(GFXcanvas16 &c, int x, int y, int w, int h, const char *titulo) {
  c.fillRect(x, y, w, h, COR_CINZA);
  corBorda3D(c, x, y, w, h, true);
  barraAzul(c, x + 2, y + 2, w - 4, 9);
  c.setTextSize(1);
  c.setTextColor(COR_BRANCO);
  c.setCursor(x + 4, y + 3);
  c.print(titulo);
  // botao "x"
  int bx = x + w - 11;
  c.fillRect(bx, y + 3, 8, 7, COR_CINZA);
  corBorda3D(c, bx, y + 3, 8, 7, true);
  c.drawLine(bx + 2, y + 5, bx + 4, y + 7, COR_PRETO);
  c.drawLine(bx + 4, y + 5, bx + 2, y + 7, COR_PRETO);
  return y + 13;
}

void corTextoCentro(GFXcanvas16 &c, const char *texto, int y, int tamanho, uint16_t cor) {
  c.setTextSize(tamanho);
  c.setTextColor(cor);
  c.setCursor((TELA_W - (int)strlen(texto) * 6 * tamanho) / 2, y);
  c.print(texto);
}

void corBandeira(GFXcanvas16 &c, int x, int y, int s) {
  c.fillRect(x, y, s, s, COR_VERMELHO);
  c.fillRect(x + s + 1, y, s, s, COR_VERDE);
  c.fillRect(x, y + s + 1, s, s, rgb565(0, 80, 255));
  c.fillRect(x + s + 1, y + s + 1, s, s, COR_AMARELO);
}

// Sprite de texto ('#' na cor, 'o' preto) ampliado.
static void corSprite(GFXcanvas16 &c, int x, int y, const char *const *linhas, int altura, int escala,
                      uint16_t tinta, uint16_t miolo) {
  for (int l = 0; l < altura; l++)
    for (int k = 0; linhas[l][k]; k++) {
      char ch = linhas[l][k];
      if (ch == '#') c.fillRect(x + k * escala, y + l * escala, escala, escala, tinta);
      else if (ch == 'o') c.fillRect(x + k * escala, y + l * escala, escala, escala, miolo);
    }
}

// ================= boot =================

static void nuvem(GFXcanvas16 &c, int x, int y) {
  c.fillCircle(x, y, 5, COR_BRANCO);
  c.fillCircle(x + 7, y - 3, 6, COR_BRANCO);
  c.fillCircle(x + 15, y, 5, COR_BRANCO);
  c.fillRect(x, y, 16, 5, COR_BRANCO);
}

// Ceu com nuvens, bandeirinha, nome e a barra colorida correndo embaixo.
void brosBoot() {
  GFXcanvas16 &c = tela.cor();
  const int NOTAS[] = {392, 523, 659, 784, 1047};
  for (int quadro = 0; quadro < 70; quadro++) {
    if (quadro % 5 == 0 && quadro / 5 < 5) som(NOTAS[quadro / 5], NOTAS[quadro / 5], 140, 0, 3);
    for (int y = 0; y < TELA_H; y++) {
      int k = y * 16 / (TELA_H - 1); // azul do ceu, mais claro embaixo
      c.drawFastHLine(0, y, TELA_W, rgb565(40 + 100 * k / 16, 110 + 90 * k / 16, 230 + 25 * k / 16));
    }
    int deriva = quadro / 3; // as nuvens andam devagar
    nuvem(c, (8 + deriva) % 190 - 20, 14);
    nuvem(c, (120 + deriva) % 190 - 20, 58);
    nuvem(c, (70 + deriva / 2) % 190 - 20, 8);
    corBandeira(c, 73, 4, 6);
    corTextoCentro(c, "BrothersOS", 22, 2, COR_PRETO);
    corTextoCentro(c, "BrothersOS", 21, 2, COR_BRANCO);
    corTextoCentro(c, "Bruno & Caio", 42, 1, COR_AZUL);
    corTextoCentro(c, "edicao Mundo SENAI", 53, 1, COR_AZUL);
    // barra colorida correndo, como no boot do Windows 95
    for (int x = 0; x < TELA_W; x++) {
      int f = (x * 4 + quadro * 6) % 320;
      int t = f < 160 ? f : 319 - f; // vai e volta 0..159
      c.drawFastVLine(x, 75, 5, rgb565(t * 255 / 159, 40, 255 - t * 255 / 159));
    }
    tela.mostrarCor();
    delay(25);
  }
}

// ================= area de trabalho =================

static void corIcone(GFXcanvas16 &c, int i, bool sobre) {
  int x = iconeX(i), y = iconeY(i);
  if (sobre) {
    c.fillRect(x - 2, y - 2, 20, 20, COR_AZUL);
    for (int k = 0; k < 20; k += 2) { // contorno pontilhado da selecao
      c.drawPixel(x - 2 + k, y - 2, COR_BRANCO);
      c.drawPixel(x - 2 + k, y + 17, COR_BRANCO);
      c.drawPixel(x - 2, y - 2 + k, COR_BRANCO);
      c.drawPixel(x + 17, y - 2 + k, COR_BRANCO);
    }
  } else {
    corSprite(c, x + 1, y + 1, ICONES[i], 16, 1, COR_PRETO, COR_PRETO); // sombra
  }
  uint16_t tinta = CORES_ICONE[i];
  if (tinta == COR_PRETO) tinta = sobre ? COR_CINZA : rgb565(40, 40, 40);
  corSprite(c, x, y, ICONES[i], 16, 1, tinta, COR_PRETO);
}

static void corBarraTarefas(GFXcanvas16 &c, int sobre, bool iniciarAberto) {
  c.fillRect(0, Y_BARRA, TELA_W, TELA_H - Y_BARRA, COR_CINZA);
  c.drawFastHLine(0, Y_BARRA, TELA_W, COR_BRANCO);
  // botao Iniciar (afunda quando o menu esta aberto ou o cursor em cima)
  bool afundado = iniciarAberto || sobre == -2;
  corBorda3D(c, 1, Y_BARRA + 1, 50, 11, !afundado);
  corBandeira(c, 4, Y_BARRA + 3, 3);
  c.setTextSize(1);
  c.setTextColor(COR_PRETO);
  c.setCursor(13, Y_BARRA + 3);
  c.print("Iniciar");
  if (sobre >= 0) {
    c.setCursor(55, Y_BARRA + 3);
    c.print(NOMES[sobre]);
  }
  // bandeja com o relogio
  corBorda3D(c, 125, Y_BARRA + 1, 34, 11, false);
  char relogio[6];
  textoHora(relogio, sizeof(relogio), (millis() / 1000) % 2);
  c.setCursor(127, Y_BARRA + 3);
  c.print(relogio);
  if (WiFi.softAPgetStationNum() > 0) corSprite(c, 116, Y_BARRA + 4, WIFI, 5, 1, COR_PRETO, COR_PRETO);
}

static void papel(GFXcanvas16 &c) {
  c.fillScreen(COR_TEAL);
  c.setTextSize(1);
  c.setTextColor(rgb565(0, 150, 150)); // marca d'agua, quase da cor do fundo
  c.setCursor(96, 57);
  c.print("Brothers");
}

static void janelaWifi() {
  GFXcanvas16 &c = tela.cor();
  som(900, 1200, 40);
  delay(150);
  for (;;) {
    Entrada e = lerEntrada();
    papel(c);
    int y = corJanela(c, 6, 4, 148, 62, "Wi-Fi");
    c.setTextColor(COR_PRETO);
    c.setCursor(12, y + 3);
    c.print("rede:  " WIFI_TEXTO_NOME);
    c.setCursor(12, y + 15);
    c.print("senha: " WIFI_TEXTO_SENHA);
    c.setCursor(12, y + 27);
    c.print("http://192.168.4.1");
    corBarraTarefas(c, -1, false);
    tela.mostrarCor();
    if (e.atirarToque || e.usarToque || e.armaToque) return;
    delay(20);
  }
}

// Quem fez: janela "Sobre" com os nomes piscando nas cores da bandeira.
void abrirBrothers() {
  GFXcanvas16 &c = tela.cor();
  const uint16_t CORES[] = {COR_VERMELHO, rgb565(0, 160, 0), rgb565(0, 80, 255), rgb565(200, 140, 0)};
  som(523, 784, 120, 0, 2);
  delay(140);
  som(784, 1047, 160, 0, 2);
  uint32_t inicio = millis(), quadro = millis();
  for (;;) {
    Entrada e = lerEntrada();
    if (millis() - inicio > 400 && (e.atirarToque || e.usarToque || e.armaToque || e.sair)) return;
    papel(c);
    int y = corJanela(c, 2, 1, 156, 66, "Sobre o BrothersOS");
    corBandeira(c, 8, y + 3, 7);
    c.setTextSize(1);
    c.setTextColor(COR_PRETO);
    c.setCursor(28, y + 2);
    c.print("BrothersOS 95");
    c.setCursor(28, y + 11);
    c.print("feito por:");
    int fase = (millis() / 300) % 4;
    c.setTextColor(CORES[fase]);
    c.setCursor(28, y + 22);
    c.print("Bruno");
    c.setTextColor(COR_PRETO);
    c.print(" & ");
    c.setTextColor(CORES[(fase + 2) % 4]);
    c.print("Caio");
    c.setTextColor(COR_CINZA_ESC);
    c.setCursor(8, y + 34);
    c.print("Mundo SENAI - ESP32-S3");
    c.setCursor(8, y + 43);
    c.print("doom, 9 jogos, terminal");
    corBarraTarefas(c, -1, false);
    tela.mostrarCor();
    esperarQuadro(quadro, 30);
  }
}

// Menu Iniciar, subindo da barra de tarefas. Joystick escolhe, atirar abre,
// clique do joystick ou trocar arma fecha. Devolve um icone pra abrir ou -1.
static int menuIniciar() {
  GFXcanvas16 &c = tela.cor();
  const char *const ITENS[] = {"Terminal", "Versiculos", "Brothers", "Calibrar", "Wi-Fi", "Reiniciar", "Desligar"};
  const int N = 7, ALT = N * 9 + 4, Y = Y_BARRA - ALT, W = 84;
  int sel = 0;
  som(900, 1200, 30);
  delay(120);
  for (;;) {
    Entrada e = lerEntrada();
    if (e.usarToque || e.armaToque) return -1;
    if (e.cimaToque) { sel = (sel + N - 1) % N; som(1100, 1100, 10); }
    if (e.baixoToque) { sel = (sel + 1) % N; som(1100, 1100, 10); }
    if (e.atirarToque) {
      som(800, 1200, 40);
      switch (sel) {
        case 0: return ICONE_TERMINAL;
        case 1: return ICONE_VERSICULO;
        case 2: return ICONE_BROTHERS;
        case 3: return ICONE_CALIBRAR;
        case 4: janelaWifi(); return -1;
        case 5: brosReiniciar(); return -1;
        case 6: brosDesligar(); return -1;
      }
    }
    papel(c);
    for (int i = 0; i < NUM_ICONES; i++) corIcone(c, i, false);
    c.fillRect(0, Y, W, ALT, COR_CINZA);
    corBorda3D(c, 0, Y, W, ALT, true);
    // faixa azul da esquerda
    for (int y = Y + 2; y < Y + ALT - 2; y++) {
      int k = (y - Y) * 16 / ALT;
      c.drawFastHLine(2, y, 8, rgb565(0, 0, 128 + 6 * k));
    }
    c.setTextSize(1);
    for (int i = 0; i < N; i++) {
      int y = Y + 2 + i * 9;
      if (i == sel) c.fillRect(11, y, W - 13, 9, COR_AZUL);
      c.setTextColor(i == sel ? COR_BRANCO : COR_PRETO);
      c.setCursor(14, y + 1);
      c.print(ITENS[i]);
    }
    corBarraTarefas(c, -1, true);
    tela.mostrarCor();
    delay(25);
  }
}

// Retangulo crescendo do icone ate a tela toda, como ao abrir janela no 95.
static void animarAbertura(int i) {
  GFXcanvas16 &c = tela.cor();
  int x0 = iconeX(i) - 2, y0 = iconeY(i) - 2;
  for (int k = 1; k <= 8; k++) {
    int x = x0 - x0 * k / 8, y = y0 - y0 * k / 8;
    int w = 20 + (TELA_W - 20) * k / 8, h = 20 + (TELA_H - 20) * k / 8;
    c.drawRect(x, y, w, h, COR_CINZA);
    tela.mostrarCor();
    delay(16);
  }
}

int brosDesktop() {
  GFXcanvas16 &c = tela.cor();
  static float cx = 80, cy = 40;
  uint32_t proximo = millis();
  delay(200); // nao conta o botao que fechou o jogo como clique

  for (;;) {
    Entrada e = lerEntrada();
    const float folga = 0.12f;
    if (fabsf(e.x) > folga) cx += e.x * 3.2f;
    if (fabsf(e.y) > folga) cy -= e.y * 3.2f; // frente = sobe
    cx = constrain(cx, 0, TELA_W - 3);
    cy = constrain(cy, 0, TELA_H - 3);

    // o que esta embaixo do cursor: icone, botao Iniciar (-2) ou nada (-1)
    int sobre = -1;
    for (int i = 0; i < NUM_ICONES; i++) {
      int x = iconeX(i), y = iconeY(i);
      if (cx >= x - 2 && cx < x + 18 && cy >= y - 2 && cy < y + 18) sobre = i;
    }
    if (cy >= Y_BARRA && cx < 52) sobre = -2;

    // trocar arma pula pro proximo icone
    if (e.armaToque) {
      int prox = sobre >= 0 ? (sobre + 1) % NUM_ICONES : 0;
      cx = iconeX(prox) + 8;
      cy = iconeY(prox) + 8;
      sobre = prox;
      som(1000, 1000, 15);
    }

    if (e.atirarToque || e.usarToque) {
      if (sobre >= 0) {
        som(800, 1200, 60, 0, 2);
        animarAbertura(sobre);
        return sobre;
      }
      if (sobre == -2) {
        int app = menuIniciar();
        if (app >= 0) return app;
        proximo = millis();
        continue;
      }
    }

    papel(c);
    for (int i = 0; i < NUM_ICONES; i++) corIcone(c, i, i == sobre);
    corBarraTarefas(c, sobre, false);
    corSprite(c, (int)cx, (int)cy, CURSOR, 10, 1, COR_PRETO, COR_BRANCO);
    tela.mostrarCor();
    esperarQuadro(proximo, 30);
  }
}
