// Ponte entre o Doom (C) e o DoomESP32.ino: ajustes de inicio, dados do
// HUD e qual som tocar no buzzer pra cada efeito do jogo.

#include "doomstat.h"
#include "d_player.h"
#include "d_items.h"
#include "sounds.h"

extern int screenblocks;
extern int detailLevel;
extern int key_nextweapon;

// Implementado no DoomESP32.ino: toca de freq1 ate freq2 em ms.
// ruido = 1 sorteia a frequencia (tiro, explosao).
void DG_Tom(int freq1, int freq2, int ms, int ruido, int prioridade);

void DG_Ajustes(int tecla_proxima_arma, int com_barra)
{
    // TFT colorida: barra de status original. OLED: tela cheia, e o HUD vai
    // na faixa de cima da OLED.
    screenblocks = com_barra ? 10 : 11;
    detailLevel = 1;    // meia resolucao na horizontal: mais rapido e a tela so tem 128 colunas
    key_nextweapon = tecla_proxima_arma;
}

void DG_Hud(int *jogando, int *vida, int *municao, int *armadura)
{
    player_t *p = &players[consoleplayer];
    ammotype_t tipo;

    *jogando = gamestate == GS_LEVEL && !menuactive && !automapactive;
    *vida = p->health;
    *armadura = p->armorpoints;
    tipo = weaponinfo[p->readyweapon].ammo;
    *municao = tipo == am_noammo ? -1 : p->ammo[tipo];
}

void DG_Som(int sfx_id, int perto)
{
    if (!perto)
    {
        return;
    }

    switch (sfx_id)
    {
        case sfx_pistol:  DG_Tom(1800, 300, 60, 1, 3); break;
        case sfx_shotgn:
        case sfx_dshtgn:  DG_Tom(1200, 150, 130, 1, 4); break;
        case sfx_sgcock:  DG_Tom(900, 700, 40, 0, 1); break;
        case sfx_punch:   DG_Tom(400, 100, 40, 1, 2); break;
        case sfx_plasma:  DG_Tom(1500, 900, 60, 0, 3); break;
        case sfx_bfg:     DG_Tom(200, 1800, 400, 0, 4); break;
        case sfx_rlaunc:  DG_Tom(800, 200, 150, 1, 4); break;
        case sfx_rxplod:
        case sfx_barexp:
        case sfx_firxpl:  DG_Tom(500, 60, 250, 1, 5); break;
        case sfx_sawup:
        case sfx_sawidl:
        case sfx_sawful:
        case sfx_sawhit:  DG_Tom(180, 230, 80, 0, 2); break;
        case sfx_doropn:
        case sfx_bdopn:   DG_Tom(200, 500, 300, 0, 2); break;
        case sfx_dorcls:
        case sfx_bdcls:   DG_Tom(500, 200, 300, 0, 2); break;
        case sfx_swtchn:
        case sfx_swtchx:  DG_Tom(1200, 1200, 30, 0, 2); break;
        case sfx_pstart:  DG_Tom(300, 300, 100, 0, 1); break;
        case sfx_pstop:   DG_Tom(250, 250, 60, 0, 1); break;
        case sfx_itemup:  DG_Tom(1000, 1600, 60, 0, 2); break;
        case sfx_wpnup:   DG_Tom(700, 1400, 150, 0, 3); break;
        case sfx_getpow:  DG_Tom(600, 1800, 300, 0, 3); break;
        case sfx_oof:
        case sfx_noway:   DG_Tom(150, 120, 80, 0, 1); break;
        case sfx_plpain:  DG_Tom(400, 150, 150, 0, 4); break;
        case sfx_pldeth:
        case sfx_pdiehi:  DG_Tom(600, 80, 700, 0, 6); break;
        case sfx_telept:  DG_Tom(200, 2000, 400, 0, 3); break;
        case sfx_firsht:  DG_Tom(800, 300, 120, 0, 2); break;
        case sfx_podth1:
        case sfx_podth2:
        case sfx_podth3:
        case sfx_bgdth1:
        case sfx_bgdth2:
        case sfx_popain:
        case sfx_dmpain:  DG_Tom(300, 100, 120, 0, 2); break;
        default: break;
    }
}
