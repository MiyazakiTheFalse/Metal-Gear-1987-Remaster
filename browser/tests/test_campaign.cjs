/* Run with node browser/tests/test_campaign.cjs. DOM/WebGL calls are stubs;
   movement, obstruction, interaction, inventory, checkpoint and AI are real. */
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

function boot(storage = new Map(),search = '',getGamepads=()=>[]) {
  const elements = new Map();
  const noop = () => {};
  const gl = new Proxy({}, { get(_, name) {
    if (name === 'getShaderParameter' || name === 'getProgramParameter') return () => true;
    return noop;
  }});
  function element(id) {
    if (!elements.has(id)) elements.set(id, { style:{}, dataset:{}, classList:{add:noop},
      clientWidth:960, clientHeight:540, width:960, height:540,
      addEventListener:noop, getContext:() => gl });
    return elements.get(id);
  }
  const runtime = { console, performance, URLSearchParams, location:{search, hash:''},
    document:{getElementById:element, addEventListener:noop}, addEventListener:noop,
    navigator:{getGamepads}, requestAnimationFrame:noop, setTimeout:noop,
    localStorage:{getItem:key=>storage.get(key),setItem:(key,value)=>storage.set(key,value)},
    __MG_TEST__:true };
  runtime.window=runtime;
  vm.createContext(runtime);
  for (const file of ['dialogue.js','tutorial.js','game.js'])
    vm.runInContext(fs.readFileSync(path.join(__dirname,'..',file),'utf8'),runtime,{filename:file});
  return {game:runtime.__mgGameDebug,button:id=>element(id).onclick(),element,storage};
}

let run=boot();
run.button('start');
const stage=()=>run.game.state.stage;
const act=name=>run.game.act(name);
const next=()=>run.game.step(.31);
function walk(x,z) {
  const game=run.game, from=game.state.player;
  const route=game.findPath(from,{x,z});
  assert.ok(route.length||Math.hypot(from.x-x,from.z-z)<.72,'walkable route to '+[x,z]);
  for(const point of [...route,{x,z}]) {
    for(let tick=0;tick<32;tick++) {
      const player=game.state.player, dx=point.x-player.x, dz=point.z-player.z;
      if(Math.hypot(dx,dz)<.07)break;
      game.moveBody(dx*8,dz*8,1/30);
    }
  }
  assert.ok(Math.hypot(game.state.player.x-x,game.state.player.z-z)<.4,'arrived at '+[x,z]);
}
function resume(){
 run=boot(run.storage);
 run.button('resume');
}
assert.equal(stage(),-2);
walk(-25.8,.6);act('interact');assert.equal(stage(),-1);
walk(-15.6,.2);act('interact');assert.equal(stage(),0);
assert.equal(run.game.state.equippedWeapon,'BARE HANDS');
walk(-5.4,0);act('interact');act('interact');assert.equal(run.game.state.wallHits[0],1);
resume();assert.equal(run.game.state.wallHits[0],1);
act('interact');next();act('interact');assert.equal(stage(),1);
walk(-.5,0);act('interact');assert.equal(stage(),2);
walk(2,-1.3);run.game.setCamera(-Math.PI/2);
for(let i=0;i<3;i++){next();act('interact')}
assert.equal(stage(),3);
assert.equal(run.game.canOccupy(14.6,0,0,0),false);
walk(7.1,-3.9);act('interact');assert.equal(stage(),4);
assert.equal(run.game.state.equippedWeapon,'STUN PISTOL');
assert.equal(run.game.state.stunAmmo,6);
resume();assert.equal(stage(),4);
assert.equal(run.game.canOccupy(27.3,0,0,0),false);
walk(24.15,.6);
run.game.setCamera(0);
for(let i=0;i<12&&stage()===4;i++){
 const target=run.game.state.bossLocation;
 if(Math.hypot(run.game.state.player.x-target.x,run.game.state.player.z-target.z)>1.28)
  walk(target.x-.75,target.z);
 run.game.setCamera(0);
 act('melee');run.game.step(.31);
}
assert.equal(stage(),5);
assert.equal(run.game.state.paused,false);
assert.equal(run.game.state.hasCard3,false);
assert.equal(run.game.canOccupy(27.3,0,0,0),false);
const dropped=run.game.state.cardDrop;
resume();assert.equal(stage(),5);
assert.deepEqual({...run.game.state.cardDrop},{...dropped});
walk(26.05,1.7);run.game.setCamera(0);act('interact');
assert.equal(stage(),5,'reader denies access without the card');
walk(dropped.x-.7,dropped.z);run.game.setCamera(Math.PI);
assert.equal(run.game.state.accessPrompt,'','facing away hides the pickup prompt');
run.game.setCamera(0);assert.equal(run.game.state.accessPrompt,'take');
act('interact');assert.equal(stage(),6);assert.equal(run.game.state.hasCard3,true);
resume();assert.equal(stage(),6);assert.equal(run.game.state.shutterOpen,false);
walk(26.05,1.7);run.game.setCamera(0);
assert.equal(run.game.state.accessPrompt,'scan');act('interact');
assert.equal(stage(),7);assert.equal(run.game.state.shutterOpen,false);
next();assert.equal(run.game.canOccupy(27.3,0,0,0),false);
next();next();assert.equal(run.game.state.shutterOpen,true);
resume();assert.equal(run.game.state.shutterOpen,true);
walk(28.4,0);next();assert.equal(stage(),8);
resume();assert.equal(stage(),8);
act('interact');assert.equal(run.game.state.paused,true);
run.button('returnToGame');assert.equal(run.game.state.paused,false);
console.log('PASS: new game, navigable route, arrest, multi-hit/reload, Fox, rotated breach, actual kit, boss, player-led exit and completed reload.');

const slot='mg1987.grayfox.browser.v1';
function loadFixture(data){
 const scenario=boot(new Map([[slot,JSON.stringify(data)]]));
 scenario.button('resume');
 return scenario;
}
const old=loadFixture({version:3,stage:4,recoveredGear:false,health:82,x:13.2,z:0});
assert.equal(old.game.state.stage,3);
assert.equal(old.game.state.recoveredGear,false);
const depleted=loadFixture({version:4,stage:3,health:88,x:7.1,z:-3.9,
 stunAmmo:0,equippedWeapon:'BARE HANDS',sealedWeapon:'BARE HANDS'});
depleted.game.act('interact');depleted.game.act('interact');
assert.equal(depleted.game.state.stage,4);
assert.equal(depleted.game.state.stunAmmo,0);
assert.equal(depleted.game.state.equippedWeapon,'BARE HANDS');
console.log('PASS: legacy corridor checkpoint and empty-ammo recovery without refills.');


for(const [oldStage,newStage] of [[5,7],[6,8]]){
 const previous=loadFixture({version:4,stage:oldStage,x:28,z:0,health:71});
 assert.equal(previous.game.state.stage,newStage);
 assert.equal(previous.game.state.shutterOpen,true);
 assert.equal(previous.game.state.hasCard3,true);
}
console.log('PASS: physical card pickup, denied reader, scan, shutter motion, checkpoint restoration and v4 exit migration.');

const encounter=loadFixture({version:5,stage:4,x:15.3,z:0,health:100}).game;
for(let i=0;i<4;i++)encounter.testSetGuard(i,{knocked:true});
function ticks(seconds){for(let i=0;i<Math.round(seconds*30);i++)encounter.step(1/30);}
encounter.teleport(13,0);ticks(2);
assert.equal(encounter.state.bossBeat,'duty','stays on duty outside the room');
encounter.teleport(23.4,.6);ticks(.1);
assert.equal(encounter.state.bossBeat,'address');
ticks(1.9);assert.equal(encounter.state.bossBeat,'shoulder');
assert.equal(encounter.state.health,100,'introduction gives the player time to react');
encounter.teleport(22.75,-3.5);
assert.ok(encounter.sightBlocked(encounter.state.bossLocation,encounter.state.player));
ticks(.4);assert.equal(encounter.state.bossBeat,'search');
assert.equal(encounter.state.health,100,'an interrupted windup deals no damage');
encounter.teleport(23.4,.6);ticks(1.2);
assert.equal(encounter.state.bossBeat,'recover');assert.equal(encounter.state.health,75);
const atShot=encounter.state.bossLocation;
ticks(2);assert.equal(encounter.state.bossBeat,'cross');
assert.ok(Math.hypot(encounter.state.bossLocation.x-atShot.x,encounter.state.bossLocation.z-atShot.z)>.25);
assert.equal(encounter.state.health,75,'recovery and crossover are quiet windows');
console.log('PASS: first sight, uninterrupted introduction, interrupted windup, single shot, recovery and collision-aware crossover.');

const shared=new Map([[slot,JSON.stringify({version:5,stage:2,x:0,z:0})]]);
const rehearsal=boot(shared,'?boss');
assert.equal(rehearsal.game.state.stage,4);
assert.equal(JSON.parse(shared.get(slot)).stage,2);
assert.ok(shared.has(slot+'.boss'));
const tallEye={x:21.1,z:.9},behindCart={x:21.1,z:3.35};
assert.equal(rehearsal.game.sightBlocked(tallEye,behindCart,1.58,1.52),false);
assert.equal(rehearsal.game.sightBlocked(tallEye,behindCart,1.58,.48),true);
console.log('PASS: rehearsal isolation and low freight cart sightlines.');

const tagRun=loadFixture({version:5,stage:4,x:24.0,z:1.4,health:100});
const tg=tagRun.game;
for(let i=0;i<4;i++)tg.testSetGuard(i,{knocked:true});
assert.equal(tg.state.tagState,'carried');
tg.act('equipitem');for(let i=0;i<3;i++)tg.act('next');
tg.act('equipclose');assert.equal(tg.state.tagState,'carried','browsing preserves the item');
tg.act('equipitem');for(let i=0;i<3;i++)tg.act('next');
tg.act('interact');assert.equal(tg.state.tagState,'placed');
const stored=boot(tagRun.storage);stored.button('resume');
assert.deepEqual({...stored.game.state.tagPoint},{x:24.0,z:1.4});
tg.teleport(13,0);
for(let i=0;i<14;i++)tg.step(1/30);
assert.equal(tg.state.bossBeat,'trace');
assert.deepEqual({...tg.state.signal},{x:24.0,z:1.4});
for(let i=0;i<70;i++)tg.step(1/30);
assert.equal(tg.state.tagState,'spent','the boss can silence the unattended transmitter');
assert.equal(tg.state.health,100);
const ended=boot(tagRun.storage);ended.button('resume');
assert.equal(ended.game.state.tagState,'spent');
console.log('PASS: item inspection, deliberate placement, signal-location investigation, decoy retirement and save restoration.');

const makePad=index=>({index,id:'Xbox '+index,mapping:'standard',connected:true,
 axes:[0,0,0,0],buttons:Array.from({length:17},()=>({pressed:false,value:0}))});
const idlePad=makePad(0),activeXbox=makePad(2);
let listed=[idlePad,null,activeXbox];
const pads=boot(new Map(),'',()=>listed),pg=pads.game;
pg.readPad();activeXbox.buttons[0].pressed=true;pg.readPad();
assert.equal(pg.state.started,true);assert.equal(pg.state.activePadIndex,2);
const before=pg.state.player.x;
activeXbox.axes[1]=-1;activeXbox.buttons[0].pressed=false;pg.readPad();pg.step(.12);
assert.ok(pg.state.player.x>before);activeXbox.axes[1]=0;
activeXbox.buttons[1].pressed=true;pg.readPad();assert.equal(pg.state.player.stance,1);
activeXbox.buttons[1].pressed=false;activeXbox.buttons[6].value=.85;pg.readPad();pg.step(.01);
assert.equal(pg.state.player.aim,true);
activeXbox.buttons[7].value=.85;pg.readPad();assert.equal(pg.state.stunAmmo,5);
listed=[];pg.readPad();pg.step(.01);assert.equal(pg.state.player.aim,false);
assert.equal(pg.state.activePadIndex,-1);
listed=[null,null,activeXbox];pg.readPad();assert.equal(pg.state.activePadIndex,2);
const blocked=boot(new Map(),'',()=>{throw Error('SecurityError')});
blocked.game.readPad();assert.match(blocked.element('padReadout').textContent,/blocked/i);
blocked.button('start');assert.equal(blocked.game.state.started,true);
console.log('PASS: Xbox input selection, A start, left stick, B, analog LT/RT, disconnect/reconnect and denied-API keyboard fallback.');
