# BrothersOS

Uma interface estilo Windows 95 rodando num ESP32-S3, com o Doom original dentro. Fiz com o Caio pra apresentar no Mundo SENAI 2026, em Londrina.

Não é um sistema operacional de verdade. É um firmware (um programa só, gravado na placa) que desenha uma área de trabalho com cursor, ícones, menu Iniciar e barra de tarefas. O nome "OS" é brincadeira.

Uma versão anterior, que eu fiz antes desta, apareceu numa reportagem da RPC: [G1, Meio Dia Paraná Londrina](https://g1.globo.com/pr/norte-noroeste/videos-meio-dia-parana-londrina/video/evento-apresenta-novidades-tecnologicas-da-industria-com-portoes-abertos-em-londrina-15162232.ghtml).

## O que tem

- Doom (episódio shareware), Tetris, Cobrinha, Flappy, Invasores, Corrida, Dino, Pong, Tijolos e Campo Minado, com recorde salvo
- Terminal com comandos (neofetch, matrix, hora, recordes, memoria, wifi...)
- A placa cria a própria rede Wi-Fi. Pelo celular dá pra ver a tela e jogar, sem precisar de internet

## O que é meu e o que não é

O motor do Doom **não** fui eu que escrevi. É o [doomgeneric](https://github.com/ozkl/doomgeneric), uma versão do código original da id Software (em C), liberado como GPL. Eu adaptei pra essa placa: os ajustes estão marcados com "ESP32" nos comentários, em `DoomESP32/src/doom`.

O resto foi feito pra este projeto, em C++ (Arduino): a interface, os outros jogos, o terminal, a página web e a montagem do hardware.

Rodar Doom em ESP32 não é novidade, outras pessoas já fizeram. O que eu queria era um sistema inteiro em volta dele numa tela de 0,96".

Dificuldades que valeram a pena:
- **Memória:** o Doom e o Wi-Fi não cabiam juntos na RAM interna. A solução foi mandar a memória do jogo pra PSRAM (os 8 MB externos).
- **Tela de 160x80:** o Doom é 320x200. Em vez de achatar, a imagem é cortada em cima e embaixo, e a vida e a munição vão pros cantos.
- **Arquivo do jogo:** o WAD não fica num sistema de arquivos. Ele é gravado direto numa partição da flash e lido de lá.

## Peças

ESP32-S3 N16R8 (16 MB flash, 8 MB PSRAM), tela TFT 0,96" IPS 80x160 (ST7735S), joystick analógico, 2 botões e buzzer passivo.

| Peça | Pino da peça | ESP32-S3 |
|---|---|---|
| Tela TFT 0,96" | GND / VCC / SCL / SDA / RES / DC / CS / BLK | GND / 3V3 / 9 / 8 / 5 / 6 / 4 / 3V3 |
| Joystick | +5V / GND | 3V3 (não 5 V) / GND |
| Joystick | VRx / VRy / SW | 1 / 2 / 42 |
| Botão atirar | cantos opostos | 41 e GND |
| Botão trocar arma | cantos opostos | 40 e GND |
| Buzzer passivo | + / − | 12 / resistor 100 Ω até o GND |

Se as cores vierem invertidas, troque `INITR_MINI160x80_PLUGIN` por `INITR_MINI160x80` no `tela.cpp`. Se a imagem ficar de cabeça pra baixo, troque `TFT_ROTACAO 3` por `1`.

## Como gravar

1. Arduino IDE: placa **ESP32S3 Dev Module**, **Flash Size 16MB**, **PSRAM: OPI PSRAM**, **Partition Scheme: Huge APP**.
2. Bibliotecas: Adafruit GFX, Adafruit ST7735 and ST7789 e Adafruit SSD1306.
3. Upload do `DoomESP32`.
4. O arquivo do jogo não vem no repositório. Baixe o `doom1.wad` (Doom shareware v1.9, MD5 `f0cefca49926d00903cf57551d901abe`) e grave uma vez na partição dele:

```
esptool --chip esp32s3 --port COMx write-flash 0x310000 doom1.wad
```

O esptool vem junto com o Arduino IDE, em `%LOCALAPPDATA%\Arduino15\packages\esp32\tools\esptool_py\<versão>\`.

## Controles

- Joystick anda e vira (até o fim = corre). Apertar o joystick = usar porta e interruptor.
- Atirar e trocar arma nos dois botões. Joystick apertado + atirar abre o menu.
- Segurar usar + trocar arma por 1 s volta pra área de trabalho, em qualquer jogo.
- Na primeira vez a tela pede pra empurrar pra FRENTE e pra ESQUERDA e guarda a calibração. Pra refazer: Iniciar > Calibrar.

## Pelo celular

Conecte na rede `BrothersOS` (senha `doom1993`) e abra http://192.168.4.1.

## Licença

GPL-2.0, a mesma do doomgeneric (ver `LICENSE`).

Feito por Bruno Santos e Caio.
