// App VERSICULOS: um versiculo aleatorio por vez (Almeida Revista e
// Atualizada). Atirar ou direita sorteia outro, esquerda volta, clique sai.

#include "jogos.h"

struct Versiculo { const char *texto, *ref; };

static const Versiculo VERSICULOS[] = {
  {"Porque Deus amou ao mundo de tal maneira que deu o seu Filho unigênito, para que todo o que nele crê não pereça, mas tenha a vida eterna.", "João 3:16"},
  {"O Senhor é o meu pastor; nada me faltará.", "Salmos 23:1"},
  {"Tudo posso naquele que me fortalece.", "Filipenses 4:13"},
  {"Entrega o teu caminho ao Senhor, confia nele, e o mais ele fará.", "Salmos 37:5"},
  {"Agrada-te do Senhor, e ele satisfará os desejos do teu coração.", "Salmos 37:4"},
  {"O amor é paciente, é benigno; o amor não arde em ciúmes, não se ufana, não se ensoberbece.", "1 Coríntios 13:4"},
  {"Não temas, porque eu sou contigo; não te assombres, porque eu sou o teu Deus; eu te fortaleço, e te ajudo, e te sustento com a minha destra fiel.", "Isaías 41:10"},
  {"Lâmpada para os meus pés é a tua palavra e, luz para os meus caminhos.", "Salmos 119:105"},
  {"Confia no Senhor de todo o teu coração e não te estribes no teu próprio entendimento.", "Provérbios 3:5"},
  {"Vinde a mim, todos os que estais cansados e sobrecarregados, e eu vos aliviarei.", "Mateus 11:28"},
  {"Sabemos que todas as coisas cooperam para o bem daqueles que amam a Deus.", "Romanos 8:28"},
  {"Eu é que sei que pensamentos tenho a vosso respeito, diz o Senhor; pensamentos de paz e não de mal, para vos dar o fim que desejais.", "Jeremias 29:11"},
  {"Ao anoitecer, pode vir o choro, mas a alegria vem pela manhã.", "Salmos 30:5"},
  {"Alegrai-vos sempre no Senhor; outra vez digo: alegrai-vos.", "Filipenses 4:4"},
  {"Deus é o nosso refúgio e fortaleza, socorro bem presente nas tribulações.", "Salmos 46:1"},
  {"Aquietai-vos e sabei que eu sou Deus.", "Salmos 46:10"},
  {"O Senhor é a minha luz e a minha salvação; de quem terei medo?", "Salmos 27:1"},
  {"Lançando sobre ele toda a vossa ansiedade, porque ele tem cuidado de vós.", "1 Pedro 5:7"},
  {"Sê forte e corajoso; não temas, nem te espantes, porque o Senhor, teu Deus, é contigo por onde quer que andares.", "Josué 1:9"},
  {"Mas os que esperam no Senhor renovam as suas forças, sobem com asas como águias, correm e não se cansam, caminham e não se fatigam.", "Isaías 40:31"},
  {"Cria em mim, ó Deus, um coração puro e renova dentro de mim um espírito inabalável.", "Salmos 51:10"},
  {"Eu sou o caminho, e a verdade, e a vida.", "João 14:6"},
  {"Deixo-vos a paz, a minha paz vos dou.", "João 14:27"},
  {"No mundo, passais por aflições; mas tende bom ânimo; eu venci o mundo.", "João 16:33"},
  {"O amor jamais acaba.", "1 Coríntios 13:8"},
  {"Nós amamos porque ele nos amou primeiro.", "1 João 4:19"},
  {"Deus é amor.", "1 João 4:8"},
  {"Este é o dia que o Senhor fez; regozijemo-nos e alegremo-nos nele.", "Salmos 118:24"},
  {"Bem-aventurados os limpos de coração, porque verão a Deus.", "Mateus 5:8"},
  {"Vós sois a luz do mundo.", "Mateus 5:14"},
  {"A minha graça te basta, porque o poder se aperfeiçoa na fraqueza.", "2 Coríntios 12:9"},
  {"Se Deus é por nós, quem será contra nós?", "Romanos 8:31"},
  {"Em tudo, dai graças.", "1 Tessalonicenses 5:18"},
  {"Regozijai-vos sempre. Orai sem cessar.", "1 Tessalonicenses 5:16-17"},
  {"Sobre tudo o que se deve guardar, guarda o coração, porque dele procedem as fontes da vida.", "Provérbios 4:23"},
  {"Os céus proclamam a glória de Deus.", "Salmos 19:1"},
  {"Graças te dou, visto que por modo assombrosamente maravilhoso me formaste.", "Salmos 139:14"},
  {"O Senhor te abençoe e te guarde.", "Números 6:24"},
  {"Buscai, pois, em primeiro lugar, o seu reino e a sua justiça, e todas estas coisas vos serão acrescentadas.", "Mateus 6:33"},
  {"Amarás o teu próximo como a ti mesmo.", "Mateus 22:39"},
  {"Ainda que eu ande pelo vale da sombra da morte, não temerei mal nenhum, porque tu estás comigo.", "Salmos 23:4"},
  {"E lhes enxugará dos olhos toda lágrima.", "Apocalipse 21:4"},
  {"A alegria do Senhor é a vossa força.", "Neemias 8:10"},
  {"Invoca-me, e te responderei.", "Jeremias 33:3"},
  {"O Senhor, teu Deus, está no meio de ti, poderoso para salvar-te; ele se deleitará em ti com alegria.", "Sofonias 3:17"},
  {"As misericórdias do Senhor são a causa de não sermos consumidos, porque as suas misericórdias não têm fim; renovam-se cada manhã.", "Lamentações 3:22-23"},
  {"O meu socorro vem do Senhor, que fez o céu e a terra.", "Salmos 121:2"},
  {"De maneira alguma te deixarei, nunca jamais te abandonarei.", "Hebreus 13:5"},
  {"As muitas águas não poderiam apagar o amor.", "Cantares 8:7"},
  {"Mas o fruto do Espírito é: amor, alegria, paz, longanimidade, benignidade, bondade, fidelidade.", "Gálatas 5:22"},
};
static const int NUM_V = sizeof(VERSICULOS) / sizeof(VERSICULOS[0]);

// UTF-8 -> fonte da tela (CP437). A fonte nao tem ã nem õ: viram a e o.
static void paraFonte(const char *in, char *out, size_t tamanho) {
  size_t n = 0;
  while (*in && n + 1 < tamanho) {
    uint8_t c = *in++;
    if (c != 0xC3 || !*in) { out[n++] = c < 0x80 ? c : '?'; continue; }
    uint8_t d = *in++;
    char m;
    switch (d) {
      case 0xA1: m = (char)0xA0; break; // á
      case 0xA0: m = (char)0x85; break; // à
      case 0xA2: m = (char)0x83; break; // â
      case 0xA3: m = 'a'; break;        // ã
      case 0xA9: m = (char)0x82; break; // é
      case 0xAA: m = (char)0x88; break; // ê
      case 0xAD: m = (char)0xA1; break; // í
      case 0xB3: m = (char)0xA2; break; // ó
      case 0xB4: m = (char)0x93; break; // ô
      case 0xB5: m = 'o'; break;        // õ
      case 0xBA: m = (char)0xA3; break; // ú
      case 0xBC: m = (char)0x81; break; // ü
      case 0xA7: m = (char)0x87; break; // ç
      case 0x87: m = (char)0x80; break; // Ç
      case 0x89: m = (char)0x90; break; // É
      case 0x81: m = 'A'; break;        // Á
      case 0x8D: m = 'I'; break;        // Í
      case 0x93: m = 'O'; break;        // Ó
      case 0x9A: m = 'U'; break;        // Ú
      default:   m = '?'; break;
    }
    out[n++] = m;
  }
  out[n] = 0;
}

// Quebra por palavra em linhas de ate 'cols' letras. Devolve quantas linhas.
static int quebrar(const char *texto, int cols, char linhas[][28], int max) {
  int n = 0;
  const char *p = texto;
  while (*p && n < max) {
    while (*p == ' ') p++;
    int len = strlen(p);
    int corte = len <= cols ? len : cols;
    if (len > cols) {
      int k = cols;
      while (k > 0 && p[k] != ' ') k--;
      if (k > 0) corte = k;
    }
    memcpy(linhas[n], p, corte);
    linhas[n][corte] = 0;
    n++;
    p += corte;
  }
  return n;
}

static int sortear(int atual) {
  int i;
  do i = random(NUM_V); while (NUM_V > 1 && i == atual);
  return i;
}

void textoVersiculo(char *buf, size_t tamanho) {
  const Versiculo &v = VERSICULOS[random(NUM_V)];
  char texto[200], ref[40];
  paraFonte(v.texto, texto, sizeof(texto));
  paraFonte(v.ref, ref, sizeof(ref));
  snprintf(buf, tamanho, "%s (%s)", texto, ref);
}

// Janela estilo Bloco de Notas: texto preto na area branca, referencia embaixo.
void abrirVersiculos() {
  GFXcanvas16 &c = tela.cor();
  c.cp437(true);
  int atual = sortear(-1), anterior = -1;
  uint32_t quadro = millis(), inicio = millis();
  som(1047, 1319, 120, 0, 2);
  delay(150);
  for (;;) {
    Entrada e = lerEntrada();
    if (e.sair || e.usarToque) break;
    if (e.atirarToque || e.dirToque) { anterior = atual; atual = sortear(atual); inicio = millis(); som(1319, 1568, 80, 0, 2); }
    if (e.esqToque && anterior >= 0) { int t = atual; atual = anterior; anterior = t; inicio = millis(); som(1047, 880, 60); }

    char texto[200], ref[40], linhas[10][28];
    paraFonte(VERSICULOS[atual].texto, texto, sizeof(texto));
    paraFonte(VERSICULOS[atual].ref, ref, sizeof(ref));
    int n = quebrar(texto, 25, linhas, 10);
    // 5 linhas cabem; versiculo comprido rola devagar
    int rolagem = n > 5 ? ((millis() - inicio) / 1800) % (n - 4) : 0;

    int y = corJanela(c, 0, 0, TELA_W, TELA_H, "Vers\xA1" "culos");
    c.fillRect(3, y, TELA_W - 6, 48, COR_BRANCO);
    corBorda3D(c, 3, y, TELA_W - 6, 48, false);
    c.setTextSize(1);
    c.setTextColor(COR_PRETO);
    for (int k = 0; k < 5 && rolagem + k < n; k++) {
      c.setCursor(5, y + 3 + k * 9);
      c.print(linhas[rolagem + k]);
    }
    c.setTextColor(COR_AZUL);
    c.setCursor(5, y + 52);
    c.print(ref);
    c.setTextColor(COR_CINZA_ESC);
    c.setCursor(TELA_W - 22, y + 52);
    c.print("< >");
    tela.mostrarCor();
    esperarQuadro(quadro, 30);
  }
  c.cp437(false);
}
