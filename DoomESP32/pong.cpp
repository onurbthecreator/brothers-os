// Pong contra o computador. Joystick pra frente/tras move a sua raquete
// (esquerda). Cada gol seu deixa o computador mais rapido; ele marcando 5,
// acaba. Pontos = gols seus.

#include "jogos.h"

static const int ALT_RAQ = 11, X_EU = 3, X_CPU = 123, GOLS_FIM = 5;

static int partida() {
  float eu = AREA_Y + 18, cpu = AREA_Y + 18;
  float bx, by, vx, vy;
  int meus = 0, deles = 0;
  uint32_t quadro = millis(), espera = 0;

  auto sacar = [&](int lado) {
    bx = 63; by = AREA_Y + AREA_H / 2;
    vx = 1.4f * lado; vy = random(-8, 9) / 10.0f;
    espera = millis() + 700;
  };
  sacar(1);

  for (;;) {
    Entrada e = lerEntrada();
    if (e.sair) return -1;

    if (fabsf(e.y) > 0.15f) eu -= e.y * 2.6f; // frente = sobe
    eu = constrain(eu, AREA_Y, 63 - ALT_RAQ);

    // computador segue a bola, mais rapido a cada gol seu
    float velCpu = 0.9f + 0.18f * meus;
    float alvo = (vx > 0 ? by : AREA_Y + AREA_H / 2) - ALT_RAQ / 2;
    if (cpu < alvo - 1) cpu += min(velCpu, alvo - cpu);
    else if (cpu > alvo + 1) cpu -= min(velCpu, cpu - alvo);
    cpu = constrain(cpu, AREA_Y, 63 - ALT_RAQ);

    if (millis() >= espera) {
      bx += vx;
      by += vy;
      if (by < AREA_Y) { by = AREA_Y; vy = -vy; som(600, 600, 15); }
      if (by > 62) { by = 62; vy = -vy; som(600, 600, 15); }
      // raquetes: onde bate muda o angulo
      if (vx < 0 && bx <= X_EU + 2 && bx >= X_EU - 1 && by + 2 >= eu && by <= eu + ALT_RAQ) {
        vx = min(-vx * 1.06f, 3.2f);
        vy += (by + 1 - (eu + ALT_RAQ / 2.0f)) * 0.18f;
        bx = X_EU + 2;
        som(900, 1100, 25);
      }
      if (vx > 0 && bx + 2 >= X_CPU && bx <= X_CPU + 1 && by + 2 >= cpu && by <= cpu + ALT_RAQ) {
        vx = max(-vx * 1.06f, -3.2f);
        vy += (by + 1 - (cpu + ALT_RAQ / 2.0f)) * 0.18f;
        bx = X_CPU - 2;
        som(700, 900, 25);
      }
      vy = constrain(vy, -2.2f, 2.2f);
      if (bx < -2) { deles++; som(400, 120, 300, 0, 4); if (deles >= GOLS_FIM) return meus; sacar(1); }
      if (bx > 128) { meus++; som(900, 1800, 200, 0, 3); sacar(-1); }
    }

    tela.clearDisplay();
    for (int y = AREA_Y; y < 64; y += 4) tela.drawFastVLine(63, y, 2, SSD1306_WHITE); // rede
    tela.fillRect(X_EU, (int)eu, 2, ALT_RAQ, SSD1306_WHITE);
    tela.fillRect(X_CPU, (int)cpu, 2, ALT_RAQ, SSD1306_WHITE);
    if (millis() >= espera || (millis() / 120) % 2) tela.fillRect((int)bx, (int)by, 2, 2, SSD1306_WHITE);
    char titulo[16];
    snprintf(titulo, sizeof(titulo), "PONG cpu %d/%d", deles, GOLS_FIM);
    barraTitulo(titulo, meus);
    mostrarTela();
    esperarQuadro(quadro, 20);
  }
}

void jogarPong() {
  for (;;) {
    int p = partida();
    if (p < 0 || !fimDeJogo("pong", p)) return;
  }
}
