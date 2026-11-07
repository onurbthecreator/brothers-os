// Tela do BrothersOS: TFT 0,96" IPS 80x160 (ST7735S, SPI), usada deitada
// (160x80).
//
// Duas formas de desenhar:
//   folha (128x64, 1 bit): jogos e terminal. Vai sem ampliar no meio da TFT,
//     numa janela estilo Windows 95 (faixa de cima da folha = barra de titulo
//     azul, resto = area branca).
//   cor() (160x80 colorido): boot, area de trabalho, menu Iniciar, janelas.
//     mostrarCor() manda pra TFT e pro navegador.
// O Doom vai colorido direto na TFT (desenharDoom).
#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // so pelas constantes SSD1306_WHITE/BLACK/INVERSE

// TFT (SPI): ver o esquema "Fiacao do BrothersOS". BLK (luz de fundo) no 3V3.
#define PINO_TFT_SDA 8  // MOSI
#define PINO_TFT_SCK 9  // "SCL" na placa da tela
#define PINO_TFT_CS  4
#define PINO_TFT_RST 5  // "RES"
#define PINO_TFT_DC  6

static const int TELA_W = 160, TELA_H = 80;

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Paleta do Windows 95 (as mesmas cores da pagina do celular).
static const uint16_t COR_TEAL       = rgb565(0, 128, 128);  // area de trabalho
static const uint16_t COR_CINZA      = rgb565(192, 192, 192); // janelas e barra de tarefas
static const uint16_t COR_CINZA_ESC  = rgb565(128, 128, 128); // sombra dos botoes
static const uint16_t COR_AZUL       = rgb565(0, 0, 128);     // barra de titulo, selecao
static const uint16_t COR_AZUL_CLARO = rgb565(16, 132, 208);  // fim do degrade da barra
static const uint16_t COR_AMARELO    = rgb565(255, 255, 0);
static const uint16_t COR_VERMELHO   = rgb565(255, 0, 0);
static const uint16_t COR_VERDE      = rgb565(0, 255, 0);
static const uint16_t COR_LARANJA    = rgb565(255, 128, 0);
static const uint16_t COR_BRANCO     = 0xFFFF;
static const uint16_t COR_PRETO      = 0x0000;

class TelaBros : public Adafruit_GFX {
public:
  TelaBros() : Adafruit_GFX(128, 64) {}
  void iniciar();
  void drawPixel(int16_t x, int16_t y, uint16_t cor) override;
  void clearDisplay() { memset(folha, 0, sizeof(folha)); }
  uint8_t *getBuffer() { return folha; }
  void display();
  // Desligar/ligar a tela (o "Agora voce pode desligar" usa).
  void ligar(bool sim);
  // 320x200 com indices de paleta -> 160x80 colorido. Jogando, corta em cima
  // e embaixo (sem achatar) e desenha vida/municao por cima; no menu, achata.
  void desenharDoom(const uint8_t *imagem, const uint16_t *paleta565, bool jogando, int vida, int municao);
  // Tela colorida de 160x80.
  GFXcanvas16 &cor() { return *canvas; }
  void mostrarCor();

private:
  uint8_t folha[128 * 64 / 8];
  GFXcanvas16 *canvas = nullptr;
};
