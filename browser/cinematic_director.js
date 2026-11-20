/* Original in-engine 1995 cinematic camera director.
 * A cutscene NEVER awards inventory, runs AI, saves progress, arrests Snake or
 * moves a boss; the authoritative gameplay beat happens independently.
 * Nonblocking real-world camera accents are deliberately short and skippable.
 *
 * Future UE Sequencer should use the same scene IDs and lens intent.
 */
(function(root){
'use strict';
const SCENES=Object.freeze({
 opening:{label:'OUTER HEAVEN  //  1995',duration:3.3,
  tag:'OPERATION INTRUDE N313',
  shots:[
   {to:1.5,subject:'snake',yaw:2.05,radius:6.4,height:2.8,focusY:1.3,orbit:-.13},
   {to:3.3,subject:'snake',yaw:-2.4,radius:3.3,height:2.25,focusY:1.28,orbit:.18}
  ]},
 capture:{label:'PRISONER PROCESSING',duration:2.6,tag:'EQUIPMENT CONFISCATED',
  shots:[
   {to:1.18,subject:'snake',yaw:1.36,radius:3.6,height:1.68,focusY:1.0,orbit:-.16},
   {to:2.6,subject:'snake',yaw:-2.88,radius:2.45,height:1.8,focusY:1.0,orbit:.08}
  ]},
 fox:{label:'GRAY FOX',duration:2.9,tag:'PRISONER IDENTIFIED',
  shots:[
   {to:1.25,subject:'fox',yaw:2.5,radius:2.35,height:1.55,focusY:1.15,orbit:.11},
   {to:2.9,subject:'fox',yaw:1.45,radius:1.55,height:1.45,focusY:1.21,orbit:.10}
  ]},
 shotmaker:{label:'SHOOT GUNNER',duration:1.28,tag:'THREE-SHOT FIRING PATTERN',
  shots:[{to:1.28,subject:'boss',yaw:2.42,radius:3.0,height:1.68,focusY:1.26,orbit:-.15}]},
 card3:{label:'CLEARANCE UPDATED',duration:1.22,tag:'CARD 3  /  RECOVERED',
  shots:[{to:1.22,subject:'player',yaw:2.44,radius:2.42,height:1.77,focusY:1.0,orbit:.05}]}
});
let scene=null,view=null,seen=new Set();
function ensure(){
 if(view||typeof document==='undefined'||!document.createElement)return view;
 const css=document.createElement('style');
 css.textContent=`
 #mgFilmOverlay{position:fixed;inset:0;z-index:8;pointer-events:none;opacity:0;
 transition:opacity .22s;display:flex;justify-content:space-between;flex-direction:column;
 color:#e8e7d9;font-family:ui-monospace,Consolas,monospace;
 text-shadow:0 2px 6px #000}
 #mgFilmOverlay.visible{opacity:1}
 #mgFilmOverlay .filmBar{height:clamp(16px,4vh,43px);background:#03070aee;
 border-color:#8bb6a333;border-style:solid;border-width:0 0 1px}
 #mgFilmOverlay .filmBar.bottom{border-width:1px 0 0}
 #mgFilmOverlay .filmMeta{position:absolute;top:clamp(32px,8vh,72px);
 left:clamp(17px,4vw,60px);display:grid;gap:6px}
 #mgFilmOverlay .filmTag{font-size:10px;color:#92c9ad;letter-spacing:.22em}
 #mgFilmOverlay .filmName{font-size:clamp(15px,2.1vw,24px);
 font-weight:750;letter-spacing:.15em}
 #mgFilmOverlay .filmSkip{position:absolute;bottom:clamp(29px,8vh,75px);
 right:clamp(14px,3vw,42px);font-size:10px;letter-spacing:.11em;color:#c3d2bc}
 @media(prefers-reduced-motion:reduce){
 #mgFilmOverlay{transition:none}
 }`;
 document.head.appendChild(css);
 view=document.createElement('div');
 view.id='mgFilmOverlay';view.setAttribute('aria-hidden','true');
 view.innerHTML='<div class="filmBar"></div><div class="filmMeta"><div class="filmTag"></div><div class="filmName"></div></div><div class="filmSkip">SPACE / B · SKIP CINEMATIC</div><div class="filmBar bottom"></div>';
 document.body.appendChild(view);
 return view;
}
function start(id,subjects,flags={}){
 if(!SCENES[id]||scene||flags.combatAlert||flags.playerAiming||flags.inVr)return false;
 // Do not auto-repeat attention-stealing shots on re-entry.
 if(seen.has(id))return false;
 const presentation=ensure();
 if(!presentation)return false;
 scene={id,definition:SCENES[id],clock:0,subjects,locked:false};
 seen.add(id);
 presentation.querySelector('.filmTag').textContent=SCENES[id].tag;
 presentation.querySelector('.filmName').textContent=SCENES[id].label;
 presentation.classList.add('visible');
 return true;
}
function finish(){
 if(view)view.classList.remove('visible');
 scene=null;
}
function update(dt,flags={}){
 if(!scene)return null;
 if(flags.paused||dt<=0)return null;
 // Any real incoming damage or combat engagement takes priority over framing.
 // A camera can NEVER pull aim away when the player is currently shooting.
 if(flags.playerAiming||flags.playerHit||flags.combatAlert){finish();return null;}
 scene.clock+=Math.max(0,dt);
 const d=scene.definition;
 if(scene.clock>=d.duration){finish();return null;}
 const shot=d.shots.find(x=>scene.clock<x.to)||d.shots[d.shots.length-1];
 const entity=scene.subjects?.[shot.subject]||scene.subjects?.snake;
 if(!entity||!Number.isFinite(entity.x)||!Number.isFinite(entity.z)){
  finish();return null;
 }
 const time=scene.clock;
 const theta=shot.yaw+(shot.orbit||0)*time;
 const focus=[entity.x,shot.focusY,entity.z];
 const eye=[entity.x+Math.cos(theta)*shot.radius,
  shot.height,entity.z+Math.sin(theta)*shot.radius];
 return {id:scene.id,eye,focus,elapsed:time,duration:d.duration};
}
function skip(){if(!scene)return false;finish();return true}
root.MGCinematicDirector={start,update,skip,finish,
 get active(){return scene?.id||null},
 get scenes(){return SCENES},
 get seen(){return [...seen]}};
})(typeof window==='undefined'?globalThis:window);
