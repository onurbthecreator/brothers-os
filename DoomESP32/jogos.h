// Coisas comuns ao BrothersOS e aos jogos (implementadas no DoomESP32.ino).
#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "tela.h"

extern TelaBros tela;

// Folha de 128x64 (ver tela.h). Faixa de cima (linhas 0 a 15, azul na TFT):
// barra de titulo. Resto (16 a 63): area do jogo.
const int AREA_Y = 16;
const int AREA_H = 48;

struct Entrada {
  float x, y;                                     // -1 a +1; x > 0 direita, y > 0 frente
  bool atirar, arma, usar;                        // segurando
  bool atirarToque, armaToque, usarToque;         // acabou de apertar
  bool cimaToque, baixoToque, esqToque, dirToque; // joystick, repete se segurar
  bool sair;                                      // usar + trocar arma por 1 s
};

Entrada lerEntrada();
void som(int freq1, int freq2, int ms, int ruido = 0, int prioridade = 1);
void mostrarTela(); // manda pra tela e pro navegador
void esperarQuadro(uint32_t &proximo, uint32_t ms);
void barraTitulo(const char *titulo, long pontos);
void textoCentro(const char *texto, int y, int tamanho);
// Linhas de texto: '#' acende, 'o' apaga, qualquer outro fica como esta.
void desenharSprite(int x, int y, const char *const *linhas, int altura);
// Janela de fim de jogo com o recorde. true = jogar de novo.
bool fimDeJogo(const char *chave, int pontos);
// Barra de titulo de janela (faixa azul), com "_ x" do lado direito.
void barraJanela(const char *titulo);

// Relogio: o celular manda a hora ao abrir a pagina (fuso de Brasilia).
bool horaCerta();
void acertarHora(int hora, int minuto);
void textoHora(char *buf, size_t tamanho, bool doisPontos); // "14:30" ou "--:--"
void textoData(char *buf, size_t tamanho);                  // "21/10/2026 14:30"

void brosReiniciar();
void brosDesligar();
// Comando digitado no campo do terminal da pagina. true = tinha um.
bool pegarComandoWeb(char *destino, size_t tamanho);

// Interface colorida estilo Windows 95, em brothersos.cpp.
// Borda 3D: alto = botao saltado (claro em cima/esquerda), senao afundado.
void corBorda3D(GFXcanvas16 &c, int x, int y, int w, int h, bool alto);
// Janela cinza com barra de titulo azul e o "x". Devolve o y onde comeca o conteudo.
int corJanela(GFXcanvas16 &c, int x, int y, int w, int h, const char *titulo);
void corTextoCentro(GFXcanvas16 &c, const char *texto, int y, int tamanho, uint16_t cor);
// Bandeirinha de 4 cores do botao Iniciar e do boot (s = lado de cada quadrado).
void corBandeira(GFXcanvas16 &c, int x, int y, int s);

void brosBoot();
int brosDesktop(); // devolve o icone clicado (ver ICONE_* abaixo)

// Icones da area de trabalho, na ordem. CALIBRAR so no menu Iniciar.
enum { ICONE_DOOM, ICONE_TETRIS, ICONE_COBRINHA, ICONE_FLAPPY, ICONE_INVASORES,
       ICONE_CORRIDA, ICONE_DINO, ICONE_PONG, ICONE_TIJOLOS, ICONE_MINADO,
       ICONE_TERMINAL, ICONE_VERSICULO, ICONE_BROTHERS, ICONE_CALIBRAR };

void jogarTetris();
void jogarCobrinha();
void jogarFlappy();
void jogarInvasores();
void jogarCorrida();
void jogarDino();
void jogarPong();    // pong.cpp
void jogarTijolos(); // tijolos.cpp
void jogarMinado();  // minado.cpp
int abrirTerminal(); // terminal.cpp; devolve um icone pra abrir ("abrir tetris") ou -1
void abrirVersiculos(); // versiculos.cpp
void textoVersiculo(char *buf, size_t tamanho); // um aleatorio, com a referencia
void abrirBrothers(); // brothersos.cpp: quem fez
