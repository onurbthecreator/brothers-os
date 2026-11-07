// Pagina do navegador (http://192.168.4.1): a tela da placa numa janela
// estilo Windows 95 e um controle (joystick + botoes, ou teclado) pra jogar
// pelo navegador. Sem nada de fora: o celular fica na rede da placa, sem internet.
#pragma once
#include <Arduino.h>

const char PAGINA[] PROGMEM = R"HTML(<!doctype html>
<html lang="pt-br"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="darkreader-lock">
<meta name="color-scheme" content="light">
<meta name="theme-color" content="#008080">
<title>BrothersOS</title>
<style>
:root{
  --teal:#008080; --cinza:#c0c0c0; --cinza-esc:#808080; --azul:#000080; --azul-claro:#1084d0;
  --texto:#000; --fonte:Tahoma,"MS Sans Serif","Segoe UI",system-ui,sans-serif;
  --alto:inset -1px -1px #000,inset 1px 1px #fff,inset -2px -2px #808080,inset 2px 2px #dfdfdf;
  --fundo-baixo:inset -1px -1px #fff,inset 1px 1px #808080,inset -2px -2px #dfdfdf,inset 2px 2px #000;
}
*{box-sizing:border-box}
body{margin:0;min-height:100vh;background:var(--teal);color:var(--texto);font-family:var(--fonte);
  padding:16px max(16px,env(safe-area-inset-left)) 56px max(16px,env(safe-area-inset-right));
  display:flex;flex-direction:column;align-items:center;gap:16px}
.janela{width:min(100%,720px);background:var(--cinza);box-shadow:var(--alto);padding:3px}
.titulo{display:flex;align-items:center;gap:6px;padding:3px 3px 3px 6px;color:#fff;font-weight:700;font-size:.95rem;
  background:linear-gradient(90deg,var(--azul),var(--azul-claro))}
.titulo .bandeira{display:grid;grid-template-columns:7px 7px;gap:1px}
.titulo .bandeira i{width:7px;height:7px}
.titulo .acoes{margin-left:auto;display:flex;gap:2px}
.titulo .acoes span{width:18px;height:16px;background:var(--cinza);box-shadow:var(--alto);color:#000;font:700 11px/16px var(--fonte);text-align:center}
.corpo{padding:10px}
.status{font-size:.85rem;margin:0 0 8px}
.tela{background:#000;box-shadow:var(--fundo-baixo);padding:4px}
canvas{display:block;width:100%;aspect-ratio:2/1;image-rendering:pixelated;background:#000}
.fps{margin:8px 0 0;font-size:.8rem;color:#000}
.palco{width:min(100%,720px);display:flex;flex-direction:column;align-items:center;gap:16px}
.controle{width:100%;display:flex;justify-content:space-between;align-items:center;gap:16px;touch-action:none;user-select:none;-webkit-user-select:none;-webkit-touch-callout:none}
.analogico{position:relative;flex:none;width:140px;height:140px;border-radius:50%;background:var(--cinza);
  box-shadow:inset -3px -3px #808080,inset 3px 3px #fff,0 0 0 2px #000;touch-action:none;cursor:grab}
.bolinha{position:absolute;left:50%;top:50%;width:58px;height:58px;margin:-29px 0 0 -29px;border-radius:50%;background:var(--cinza);
  box-shadow:inset -3px -3px #808080,inset 3px 3px #fff,0 0 0 2px #000;pointer-events:none}
.botoes{display:grid;grid-template-columns:auto auto;grid-template-areas:". a" "b c";gap:10px;align-items:center;justify-items:center}
.bt{border:0;background:var(--cinza);box-shadow:var(--alto);color:#000;font:700 .85rem var(--fonte);touch-action:none;cursor:pointer;-webkit-tap-highlight-color:transparent}
.bt-a{grid-area:a;width:84px;height:72px;font-size:.95rem}
.bt-b{grid-area:b;width:66px;height:56px}
.bt-c{grid-area:c;width:66px;height:56px}
.bt.apertado{box-shadow:var(--fundo-baixo);padding-top:2px}
.extras{display:flex;gap:8px;flex-wrap:wrap;justify-content:center}
.pilula{border:0;background:var(--cinza);box-shadow:var(--alto);color:#000;padding:7px 14px;font:.9rem var(--fonte);cursor:pointer}
.pilula:active{box-shadow:var(--fundo-baixo)}
.bt:focus-visible,.pilula:focus-visible,.term button:focus-visible{outline:1px dotted #000;outline-offset:-5px}
.term{display:flex;align-items:center;gap:6px;background:#000;box-shadow:var(--fundo-baixo);padding:6px 6px 6px 10px}
.term span{font:700 .9rem ui-monospace,Consolas,monospace;color:#c0c0c0;white-space:nowrap}
.term input{flex:1;min-width:0;border:0;background:transparent;font:1rem ui-monospace,Consolas,monospace;color:#fff;outline:none}
.term input::placeholder{color:#808080}
.term button{border:0;background:var(--cinza);box-shadow:var(--alto);font:.9rem var(--fonte);padding:6px 12px;cursor:pointer}
.dica{margin:8px 0 0;font-size:.82rem}
/* tela larga (PC, celular deitado): joystick | tela | botoes */
@media (min-width:760px){
  .janela{width:min(100%,1180px)}
  .palco{width:min(100%,1180px);display:grid;grid-template-columns:auto minmax(0,1fr) auto;
    grid-template-areas:"ana tela bts" ". ext .";align-items:center;column-gap:30px;row-gap:16px}
  .palco>.janela{grid-area:tela;width:100%}
  .controle{display:contents}
  .analogico{grid-area:ana}
  .botoes{grid-area:bts}
  .extras{grid-area:ext}
}
/* tela cheia: tela no meio, controle por cima nos cantos */
.palco:fullscreen{width:100%;height:100%;max-width:none;display:flex;justify-content:center;padding:16px;background:var(--teal)}
.palco:fullscreen>.janela{width:min(100%,calc((100vh - 90px) * 2));max-width:none}
.palco:fullscreen .controle{display:flex;position:fixed;left:0;right:0;bottom:18px;padding:0 24px;pointer-events:none}
.palco:fullscreen .analogico,.palco:fullscreen .botoes{pointer-events:auto;opacity:.92}
.palco:fullscreen .extras{position:fixed;top:12px;right:12px}
.palco:fullscreen .fps{display:none}
.barra{position:fixed;left:0;right:0;bottom:0;height:40px;background:var(--cinza);border-top:2px solid #fff;display:flex;align-items:center;gap:8px;padding:4px 6px calc(4px + env(safe-area-inset-bottom))}
.barra .iniciar{display:flex;align-items:center;gap:6px;height:30px;padding:0 10px;box-shadow:var(--alto);font-weight:700}
.barra .relogio{margin-left:auto;height:30px;padding:0 12px;display:flex;align-items:center;box-shadow:var(--fundo-baixo);font-size:.85rem}
</style></head><body>
<div class="palco" id="palco">
<div class="janela">
  <div class="titulo">
    <span class="bandeira" aria-hidden="true"><i style="background:#f00"></i><i style="background:#0f0"></i><i style="background:#0050ff"></i><i style="background:#ff0"></i></span>
    BrothersOS
    <span class="acoes" aria-hidden="true"><span>_</span><span>&#9633;</span><span>x</span></span>
  </div>
  <div class="corpo">
    <p class="status" id="s">conectando...</p>
    <div class="tela"><canvas id="c" width="160" height="80"></canvas></div>
    <p class="fps" id="q">&nbsp;</p>
  </div>
</div>
<div class="controle" id="ctl">
  <div class="analogico" id="ana" aria-label="joystick"><div class="bolinha" id="bol"></div></div>
  <div class="botoes">
    <button type="button" class="bt bt-a" data-b="1">atirar</button>
    <button type="button" class="bt bt-b" data-b="2">arma</button>
    <button type="button" class="bt bt-c" data-b="4">usar</button>
  </div>
</div>
<div class="extras">
  <button type="button" class="pilula" id="cheia">tela cheia</button>
  <button type="button" class="pilula" id="voltar">voltar pra area de trabalho</button>
</div>
</div>
<div class="janela">
  <div class="titulo">Prompt do BrothersOS</div>
  <div class="corpo">
    <form class="term" id="t">
      <span>bros@os:~$</span>
      <input id="ti" autocomplete="off" autocapitalize="off" spellcheck="false" enterkeyhint="send" placeholder="digite um comando">
      <button type="submit">enviar</button>
    </form>
    <p class="dica">abra o Terminal na placa e digite aqui (tente: neofetch, toca, matrix, frase)</p>
    <p class="dica">no PC: setas ou WASD andam, espa&ccedil;o atira, E usa, Q troca a arma, Esc volta</p>
    <p class="dica" id="pad">controle de videogame: aperte um bot&atilde;o dele pra conectar</p>
  </div>
</div>
<div class="barra">
  <span class="iniciar"><span class="bandeira" style="display:grid;grid-template-columns:6px 6px;gap:1px" aria-hidden="true"><i style="width:6px;height:6px;background:#f00"></i><i style="width:6px;height:6px;background:#0f0"></i><i style="width:6px;height:6px;background:#0050ff"></i><i style="width:6px;height:6px;background:#ff0"></i></span>Iniciar</span>
  <span>Bruno &amp; Caio</span>
  <span class="relogio" id="rel">--:--</span>
</div>
<script>
const c=document.getElementById('c'),g=c.getContext('2d'),s=document.getElementById('s'),q=document.getElementById('q');
let img=g.createImageData(160,80),n=0,t0=performance.now();
function modo(w,h,ar){if(c.width!==w||c.height!==h){c.width=w;c.height=h;img=g.createImageData(w,h);c.style.aspectRatio=ar;}}
async function quadro(){
  try{
    const r=await fetch('/quadro?'+controle(),{cache:'no-store'});
    const b=new Uint8Array(await r.arrayBuffer());
    if(b[0]===4){
      modo(160,80,'2/1');
      const d=img.data;
      for(let i=0;i<160*80;i++){const v=b[1+i*2]|(b[2+i*2]<<8),j=i*4;
        d[j]=((v>>11)&31)*255/31;d[j+1]=((v>>5)&63)*255/63;d[j+2]=(v&31)*255/31;d[j+3]=255;}
    }
    else if(b[0]===1){
      modo(160,100,'8/5');
      const p=b.subarray(1,769),px=b.subarray(769),d=img.data;
      for(let i=0;i<px.length;i++){const k=px[i]*3,j=i*4;d[j]=p[k];d[j+1]=p[k+1];d[j+2]=p[k+2];d[j+3]=255;}
    }else{
      // folha de 1 bit, nas mesmas cores da placa: barra azul em cima, preto no branco embaixo
      modo(128,64,'2/1');
      const d=img.data;
      for(let y=0;y<64;y++)for(let x=0;x<128;x++){
        const on=(b[1+x+(y>>3)*128]>>(y&7))&1,j=(y*128+x)*4;
        if(y<16){if(on){d[j]=d[j+1]=d[j+2]=255;}else{d[j]=0;d[j+1]=0;d[j+2]=128;}}
        else{const v=on?0:255;d[j]=d[j+1]=d[j+2]=v;}
        d[j+3]=255;
      }
    }
    g.putImageData(img,0,0);
    s.textContent='Conectado ao BrothersOS';
    n++;const t=performance.now();
    if(t-t0>1000){q.textContent=(n*1000/(t-t0)).toFixed(0)+' quadros por segundo';n=0;t0=t;}
  }catch(e){s.textContent='Sem conexão. Tentando de novo...';await new Promise(r=>setTimeout(r,600));}
  requestAnimationFrame(quadro);
}
quadro();
// manda a hora do celular pra placa (ela nao tem internet nem relogio)
function hora(){fetch('/hora?t='+Date.now()).catch(()=>{});}
hora();setInterval(hora,60000);
const rel=document.getElementById('rel');
function relogio(){const d=new Date();rel.textContent=String(d.getHours()).padStart(2,'0')+':'+String(d.getMinutes()).padStart(2,'0');}
relogio();setInterval(relogio,10000);
const t=document.getElementById('t'),ti=document.getElementById('ti');
// ---- controle: vai junto com cada pedido de imagem ----
let jx=0,jy=0,bits=0,teclas={};
function controle(){
  let x=jx,y=jy,b=bits;
  if(teclas.l)x=-1;if(teclas.r)x=1;if(teclas.u)y=1;if(teclas.d)y=-1;
  if(teclas.a)b|=1;if(teclas.q)b|=2;if(teclas.e)b|=4;
  // controle de videogame (Xbox, PlayStation)
  const pads=navigator.getGamepads?navigator.getGamepads():[];
  for(const gp of pads){
    if(!gp)continue;
    const ax=gp.axes[0]||0,ay=gp.axes[1]||0,ap=(i)=>gp.buttons[i]&&gp.buttons[i].pressed;
    if(Math.abs(ax)>.15&&Math.abs(ax)>Math.abs(x))x=ax;
    if(Math.abs(ay)>.15&&Math.abs(ay)>Math.abs(y))y=-ay;
    if(ap(12))y=1;if(ap(13))y=-1;if(ap(14))x=-1;if(ap(15))x=1;
    if(ap(0)||ap(7))b|=1;          // A ou gatilho direito: atirar
    if(ap(1)||ap(6))b|=4;          // B ou gatilho esquerdo: usar
    if(ap(2)||ap(3)||ap(5))b|=2;   // X, Y ou RB: trocar arma
    if(ap(8)||ap(9)&&ap(4))b|=6;   // select: voltar
  }
  return 'x='+Math.round(x*100)+'&y='+Math.round(y*100)+'&b='+b;
}
const ana=document.getElementById('ana'),bol=document.getElementById('bol');let dedo=null;
function moverAna(ev){
  const r=ana.getBoundingClientRect(),R=r.width/2;
  let dx=(ev.clientX-r.left-R)/R*1.25,dy=(ev.clientY-r.top-R)/R*1.25;const m=Math.hypot(dx,dy);
  if(m>1){dx/=m;dy/=m;}
  jx=dx;jy=-dy;bol.style.transform='translate('+(dx*R*.55)+'px,'+(dy*R*.55)+'px)';
}
ana.addEventListener('pointerdown',ev=>{dedo=ev.pointerId;ana.setPointerCapture(dedo);moverAna(ev);ev.preventDefault();});
ana.addEventListener('pointermove',ev=>{if(ev.pointerId===dedo)moverAna(ev);});
function soltarAna(ev){if(ev.pointerId===dedo){dedo=null;jx=jy=0;bol.style.transform='';}}
ana.addEventListener('pointerup',soltarAna);ana.addEventListener('pointercancel',soltarAna);
document.querySelectorAll('.bt').forEach(bt=>{
  const v=+bt.dataset.b;
  bt.addEventListener('pointerdown',ev=>{bits|=v;bt.classList.add('apertado');bt.setPointerCapture(ev.pointerId);ev.preventDefault();if(navigator.vibrate)navigator.vibrate(12);});
  const solta=()=>{bits&=~v;bt.classList.remove('apertado');};
  bt.addEventListener('pointerup',solta);bt.addEventListener('pointercancel',solta);bt.addEventListener('lostpointercapture',solta);
});
// usar + trocar arma por 1,3 s = volta pra area de trabalho (no Doom a placa reinicia)
function voltar(){bits|=6;setTimeout(()=>{bits&=~6;},1300);}
document.getElementById('voltar').onclick=voltar;
document.getElementById('cheia').onclick=()=>{
  const p=document.getElementById('palco');
  if(document.fullscreenElement)document.exitFullscreen();
  else if(p.requestFullscreen)p.requestFullscreen().then(()=>{if(screen.orientation&&screen.orientation.lock)screen.orientation.lock('landscape').catch(()=>{});}).catch(()=>{});
};
document.getElementById('ctl').addEventListener('contextmenu',ev=>ev.preventDefault());
const MAPA={ArrowLeft:'l',KeyA:'l',ArrowRight:'r',KeyD:'r',ArrowUp:'u',KeyW:'u',ArrowDown:'d',KeyS:'d',Space:'a',KeyJ:'a',KeyE:'e',KeyK:'e',KeyQ:'q',KeyL:'q'};
addEventListener('keydown',ev=>{
  if(document.activeElement===ti)return;
  if(ev.code==='Escape'&&!document.fullscreenElement){voltar();return;}
  const k=MAPA[ev.code];if(k){teclas[k]=true;ev.preventDefault();}
});
addEventListener('keyup',ev=>{const k=MAPA[ev.code];if(k)teclas[k]=false;});
addEventListener('blur',()=>{teclas={};});
// mostra se o navegador esta vendo um controle de videogame
const pad=document.getElementById('pad');
if(!navigator.getGamepads||!window.isSecureContext){
  pad.textContent='controle de videogame: este navegador só libera em site com HTTPS (o da placa é HTTP); use o teclado ou a tela';
}
addEventListener('gamepadconnected',ev=>{pad.textContent='controle conectado: '+ev.gamepad.id;});
addEventListener('gamepaddisconnected',()=>{pad.textContent='controle desconectado';});
t.addEventListener('submit',ev=>{ev.preventDefault();const v=ti.value.trim();if(!v)return;fetch('/cmd?c='+encodeURIComponent(v)).catch(()=>{});ti.value='';});
</script></body></html>)HTML";
