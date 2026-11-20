/* Run with node browser/tests/test_campaign.cjs. DOM/WebGL calls are stubs;
   movement, obstruction, interaction, inventory, checkpoint and AI are real. */
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

function boot(storage = new Map(),search = '',getGamepads=()=>[]) {
  const elements = new Map();
  const documentListeners = new Map();
  const noop = () => {};
  const gl = new Proxy({}, { get(_, name) {
    if (name === 'getShaderParameter' || name === 'getProgramParameter') return () => true;
    return noop;
  }});
  function element(id) {
    if (!elements.has(id)) elements.set(id, { style:{}, dataset:{},
      hidden:id==='menuOptionsPanel'||id==='newGameConfirm',
      tagName:id==='rigToggle'||id==='tipsToggle'||id==='voiceToggle'?'INPUT':'BUTTON',
      classList:{add:noop},
      clientWidth:960, clientHeight:540, width:960, height:540,
      addEventListener:noop, getContext:() => gl });
    return elements.get(id);
  }
  const runtime = { console, performance, URLSearchParams, location:{search, hash:''},
    document:{getElementById:element, addEventListener:(id,handler)=>documentListeners.set(id,handler)}, addEventListener:noop,
    navigator:{getGamepads}, requestAnimationFrame:noop, setTimeout:noop,
    localStorage:{getItem:key=>storage.get(key),setItem:(key,value)=>storage.set(key,value)},
    __MG_TEST__:true };
  runtime.window=runtime;
  vm.createContext(runtime);
  for (const file of ['dialogue.js','tutorial.js','game.js'])
    vm.runInContext(fs.readFileSync(path.join(__dirname,'..',file),'utf8'),runtime,{filename:file});
  return {game:runtime.__mgGameDebug,button:id=>element(id).onclick(),element,storage,
    key:(which,targetId='start')=>documentListeners.get('keydown')?.({
      key:which,target:element(targetId),preventDefault:noop
    })};
}

// Title metagame is a real input path, not a static mock-up.
const title=boot();
assert.equal(title.element('resume').disabled,true,
 'Continue is unavailable without an actual checkpoint');
title.key('ArrowDown'); // disabled Continue must be skipped
assert.equal(title.element('brief').dataset.selected,'true',
 'keyboard title navigation skips unavailable saves');
title.key('Enter');
assert.equal(title.element('briefing').style.display,'flex',
 'briefing is a selectable non-gameplay title option');
assert.equal(title.game.state.started,false,
 'reading briefing never consumes or enters mission state');
title.button('menuOptions');
assert.equal(title.element('menuOptionsPanel').hidden,false,
 'title Options actually reveals accessibility and audio controls');
title.key(' ','rigToggle');
assert.equal(title.game.state.started,false,
 'Space on checkbox does not accidentally launch the mission');
title.button('menuOptions');
assert.equal(title.element('menuOptionsPanel').hidden,true,
 'title Options drawer is dismissible');
console.log('PASS: title options, keyboard selection, disabled Continue and briefing isolation.');
const alreadyPlayed=boot();
alreadyPlayed.button('start');
const savedOperation=boot(alreadyPlayed.storage);
assert.equal(savedOperation.element('resume').disabled,false,
 'Continue becomes available with a real checkpoint');
savedOperation.button('start');
assert.equal(savedOperation.element('newGameConfirm').hidden,false,
 'New Game requires explicit consent before deleting an existing checkpoint');
assert.equal(savedOperation.game.state.started,false,
 'checkpoint confirmation interrupts the destructive action');
assert.equal(savedOperation.element('cancelNewGame').dataset.selected,'true',
 'Cancel is default even for keyboard/controller navigation');
savedOperation.button('cancelNewGame');
assert.equal(savedOperation.element('newGameConfirm').hidden,true);
assert.equal(savedOperation.game.state.started,false,
 'cancel returns to the title with the old save intact');
savedOperation.button('start');
savedOperation.button('confirmNewGame');
assert.equal(savedOperation.game.state.started,true,
 'explicit confirmation starts the new operation');
console.log('PASS: Save-protecting New Game confirmation and safe cancellation.');

// The original 1987 three-truck discovery loop is available before custody.
// These are physical props with independent pickups, save state and useful items.
const supplies=boot();supplies.button('start');
supplies.game.teleport(-30.35,2.9);supplies.game.act('interact');
assert.equal(supplies.game.state.rationEarned,true,'west truck awards ration');
assert.equal(supplies.game.state.rationRemaining,1);
supplies.game.teleport(-27.55,2.9);supplies.game.act('interact');
assert.equal(supplies.game.state.firstCardEarned,true,'centre truck awards Card 1');
supplies.game.teleport(-24.75,2.9);supplies.game.act('interact');
assert.equal(supplies.game.state.binocularsEarned,true,'east truck awards scouting equipment');
supplies.game.act('equipitem');
for(let i=0;i<7;i++)supplies.game.act('next'); // Binoculars are the last item slot.
supplies.game.act('equipclose');
assert.equal(supplies.game.state.freight.scouting,true,'scout view is player-selected');
const supplyRestore=boot(supplies.storage);supplyRestore.button('resume');
assert.equal(supplyRestore.game.state.rationRemaining,1);
assert.equal(supplyRestore.game.state.rationWorld,null,'ration prop remains collected');
assert.equal(supplyRestore.game.state.binocularsWorld,null,'binoculars prop remains collected');
assert.equal(supplyRestore.game.state.card1World,null,'Card 1 prop remains collected');
assert.equal(supplyRestore.game.state.freight.scouting,false,'camera view resets safely on load');
assert.equal(supplyRestore.game.testApplyPlayerDamage(60),true,'test inflicted actual player health loss');
assert.equal(supplyRestore.game.state.health,40);
supplyRestore.game.act('equipitem');supplyRestore.game.act('next');supplyRestore.game.act('equipclose');
assert.equal(supplyRestore.game.state.health,85,'earned ration restores finite health');
assert.equal(supplyRestore.game.state.rationRemaining,0,'ration is actually consumed');
const spentRation=boot(supplyRestore.storage);spentRation.button('resume');
assert.equal(spentRation.game.state.health,85,'healed checkpoint persists');
assert.equal(spentRation.game.state.rationRemaining,0,'ration is not replenished by loading');
assert.equal(spentRation.game.state.rationWorld,null,'used ration never respawns');
console.log('PASS: 1987 truck rewards, optics, finite ration use and checkpoint persistence.');
// Card 1 must remain recoverable after custody; no softlocking a novice
// who took the optional opening route before inspecting the central truck.
const lateCardSave=new Map([['mg1987.grayfox.browser.v1',JSON.stringify({
 version:10,stage:4,recoveredGear:true,health:100,x:-10.6,z:0,
 firstCardEarned:false,rationEarned:false,binocularsEarned:false
})]]);
const lateCard=boot(lateCardSave);lateCard.button('resume');
assert.equal(lateCard.game.canOccupy(-12.6,0,0,0),true,
 'recovered gear opens a physical walkable backtrack gate');
lateCard.game.teleport(-27.55,2.9);lateCard.game.act('interact');
assert.equal(lateCard.game.state.firstCardEarned,true,
 'post-recovery return visit can still collect Card 1');
const backtrackReload=boot(lateCard.storage);backtrackReload.button('resume');
assert.equal(backtrackReload.game.state.firstCardEarned,true,
 'late Card 1 persists across the return-route checkpoint');
assert.equal(backtrackReload.game.state.player.x,-27.55,
 'a backtrack checkpoint remains in the visited truck, not teleported away');
console.log('PASS: non-missable Card 1 via physical after-recovery yard return.');

let run=boot();
run.button('start');
const initialCast=run.game.previewRigActors();
assert.ok(initialCast.some(a=>a.id==='snake'&&a.kind==='player'));
assert.ok(initialCast.some(a=>a.id==='fox'&&a.kind==='fox'&&a.captive));
assert.ok(initialCast.some(a=>a.id==='informant'&&a.kind==='pow_informant'));
assert.equal(initialCast.filter(a=>a.id==='snake').length,1);
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
walk(-28.65,1.25);act('interact');
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
walk(13,0);next();walk(17.4,-3.9);act('interact');assert.equal(stage(),4);
const bossCast=run.game.previewRigActors();
assert.equal(bossCast.filter(a=>a.id==='shotmaker').length,1,
 'one source-specific Shotmaker rig, not a generic extra guard');
assert.equal(bossCast.find(a=>a.id==='shotmaker').kind,'shoot_gunner');
assert.equal(run.game.state.equippedWeapon,'HANDGUN');
assert.equal(run.game.state.rounds,24);
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
walk(24.95,1.72);run.game.setCamera(Math.PI/2);
assert.equal(run.game.state.accessPrompt,'stores');act('interact');
resume();walk(dropped.x,dropped.z-.8);run.game.setCamera(-Math.PI/2);
assert.equal(run.game.state.accessPrompt,'','facing away hides the pickup prompt');
run.game.setCamera(Math.PI/2);assert.equal(run.game.state.accessPrompt,'take');
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
walk(29.4,-3.75);act('prone');run.game.setCamera(0);
assert.equal(run.game.canOccupy(35,-3.8,0,0),false);
assert.equal(run.game.canOccupy(35,-3.8,2,0),true);
walk(28.4,0);act('interact');assert.equal(run.game.state.paused,true);
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
depleted.game.teleport(13,0);depleted.game.step(.02);depleted.game.teleport(17.4,-3.9);
depleted.game.act('interact');depleted.game.act('interact');
assert.equal(depleted.game.state.stage,4);
assert.equal(depleted.game.state.rounds,0);
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
pg.teleport(-28.65,1.25);pg.act('interact');
activeXbox.buttons[7].value=.85;pg.readPad();assert.equal(pg.state.rounds,23);
listed=[];pg.readPad();pg.step(.01);assert.equal(pg.state.player.aim,false);
assert.equal(pg.state.activePadIndex,-1);
listed=[null,null,activeXbox];pg.readPad();assert.equal(pg.state.activePadIndex,2);
const blocked=boot(new Map(),'',()=>{throw Error('SecurityError')});
blocked.game.readPad();assert.match(blocked.element('padReadout').textContent,/blocked/i);
blocked.button('start');assert.equal(blocked.game.state.started,true);
console.log('PASS: Xbox input selection, A start, left stick, B, analog LT/RT, disconnect/reconnect and denied-API keyboard fallback.');

const dispatch=boot();
dispatch.button('start');
const freightGame=dispatch.game;
const selectItem=index=>{
 freightGame.act('equipitem');
 for(let i=0;i<index;i++)freightGame.act('next');
 freightGame.act('equipclose');
};
freightGame.teleport(-26.8,-2.3);
freightGame.act('interact');
assert.equal(freightGame.state.freight.box,true);
selectItem(3);
assert.equal(freightGame.state.freight.boxed,true);
freightGame.teleport(-26.65,-3.62);
for(let i=0;i<210;i++)freightGame.step(1/30);
assert.ok(Math.abs(freightGame.state.player.x+16)<.02,'parcel reaches its marked stop');
assert.equal(freightGame.state.freight.freightAmbientHeard,true,
 'moving freight triggers only optional authored worker chatter while advancing');
assert.equal(freightGame.state.freight.reported,false);
freightGame.teleport(-21.85,-3.1);
selectItem(4);
assert.equal(freightGame.state.freight.boxed,false);
assert.equal(freightGame.state.freight.smokeX,-21.85);
freightGame.teleport(-19.2,-4.15);
freightGame.act('interact');
assert.equal(freightGame.state.freight.cameraOff,true);
const reloaded=boot(dispatch.storage);
reloaded.button('resume');
assert.equal(reloaded.game.state.freight.cameraOff,true);
assert.equal(reloaded.game.state.freight.box,true);
assert.equal(reloaded.game.state.freight.freightAmbientHeard,true,
 'optional freight breather does not replay after checkpoint reload');
assert.equal(reloaded.game.state.freight.boxed,false);
console.log('PASS: carton pickup, belt arrival, cigarette use and checkpointed camera power.');

const curious=boot();
curious.button('start');
curious.game.teleport(-26.8,-2.3);
curious.game.act('interact');
curious.game.act('equipitem');
for(let i=0;i<3;i++)curious.game.act('next');
curious.game.act('equipclose');
assert.equal(curious.game.state.freight.boxed,true,'cardboard disguise equipped');
curious.game.teleport(-18,0);
curious.game.testSetGuard(0,{x:-19,z:0,yaw:0,state:0,suspicion:0,knocked:false});
curious.game.testStepGuards(.1);
assert.ok(curious.game.state.guards[0].state>=1 ||
 curious.game.state.guards[0].investigateUntil>0 ||
 curious.game.state.guards[0].last.x>-19.1,
 'a guard looking directly at a near stationary unexpected box investigates');
assert.ok(curious.game.state.guards[0].boxInspectionAge<1.3,
 'first glance reveals a suspicious crate, not Snake himself');
for(let i=0;i<13;i++){
 curious.game.testSetGuard(0,{x:-19,z:0,yaw:0,knocked:false});
 curious.game.testStepGuards(.1);
}
assert.ok(curious.game.state.guards[0].boxInspectionAge>=1.3,
 'steady optical attention completes a deliberate 1.3s inspection');
assert.equal(curious.game.state.guards[0].boxIdentitySpoken,true,
 'the guard identifies the person only after inspecting the box');
const distant=boot();
distant.button('start');
distant.game.teleport(-26.8,-2.3);distant.game.act('interact');
distant.game.act('equipitem');
for(let i=0;i<3;i++)distant.game.act('next');
distant.game.act('equipclose');
distant.game.teleport(-18,0);
distant.game.testSetGuard(0,{x:-25,z:0,yaw:0,state:0,suspicion:0,knocked:false});
distant.game.testStepGuards(.1);
assert.ok(distant.game.state.guards[0].state<2,
 'box disguise is not automatically recognized from beyond inspection distance');
console.log('PASS: stationary box is ordinary distant freight but prompts real close-up curiosity.');

assert.equal(curious.game.state.itemSlots.some(i=>i.name.startsWith('CARDBOARD BOX')&&i.usable),
 false,'authentic Card-2/third-floor cardboard box remains unearned');
const borrowed=boot();
borrowed.button('start');borrowed.game.teleport(-26.8,-2.3);borrowed.game.act('interact');
assert.equal(borrowed.game.state.itemSlots.some(i=>i.name.startsWith('FREIGHT CARTON')&&i.usable),
 true,'the freight disguise is only borrowed during the opening');
borrowed.game.teleport(-24.2,1.42);borrowed.game.act('interact');
assert.equal(borrowed.game.state.stage,-1,'POW contact is a real prerequisite');
borrowed.game.teleport(-14.3,.4);borrowed.game.act('interact');
assert.equal(borrowed.game.state.stage,0,'custody transaction occurs');
assert.equal(borrowed.game.state.itemSlots.some(i=>i.name.startsWith('FREIGHT CARTON')&&i.usable),
 false,'borrowed carton is lost at custody, not recovered as an original 1987 pickup');
console.log('PASS: borrowed freight carton is distinct from permanently earned MSX2 Card-2 box.');




const late=loadFixture({version:6,stage:8,x:28.4,z:0,health:100}).game;
late.teleport(42.0,0);late.act('interact');
assert.equal(late.state.service.open,false);
late.teleport(35.8,1.75);late.act('interact');
assert.equal(late.state.service.charge,true);
late.teleport(42.0,0);late.act('interact');late.step(.4);late.act('interact');
assert.ok(late.state.service.fuse<2.1,'a second interaction cannot restart the timer');
for(let i=0;i<64;i++)late.step(1/30);
assert.equal(late.state.service.open,true);
late.teleport(49.4,-1.8);late.act('interact');assert.equal(late.state.service.intel,true);
late.teleport(61.1,.8);late.setCamera(0);
for(let i=0;i<7;i++){late.act('melee');late.step(.31);}
assert.equal(late.state.service.kidDown,true);
late.teleport(62.7,-2);late.act('interact');assert.equal(late.state.service.parachute,true);
console.log('PASS: old exit save through charge, timed breach, relay, Machinegun Kid and parachute.');


/* Connected roof continuity is tested with normal gameplay interactions,
 * not only by setting mission flags. The shot test asserts real ammo usage;
 * actual projectile collision must still be playtested in a browser. */
const roofSave=loadFixture({version:9,stage:8,x:63,z:0,health:100,
 service:{charge:true,open:true,intel:true,kidDown:true,parachute:true}});
assert.equal(roofSave.game.state.roof.active,false);
roofSave.game.act('interact');
assert.equal(roofSave.game.state.stage,9);
assert.equal(roofSave.game.state.roof.active,true);
assert.ok(roofSave.game.state.player.x>70,'roof transition is spatial, not a menu-only flag');
roofSave.game.teleport(76.6,-2.65);
const roofBefore=roofSave.game.state.roof.grenades;
roofSave.game.act('interact');
assert.equal(roofSave.game.state.roof.grenades,roofBefore-1,
 'using the exposed launcher consumes a finite grenade');
const roofReloaded=boot(roofSave.storage);roofReloaded.button('resume');
assert.equal(roofReloaded.game.state.roof.active,true);
assert.equal(roofReloaded.game.state.roof.grenades,roofBefore-1);
roofReloaded.game.teleport(71.3,0);roofReloaded.game.act('interact');
assert.equal(roofReloaded.game.state.roof.active,false);
assert.equal(roofReloaded.game.state.stage,9);
roofReloaded.game.teleport(63,0);roofReloaded.game.act('interact');
assert.equal(roofReloaded.game.state.roof.active,true,
 'return and re-entry remain possible without resetting the boss');
console.log('PASS: physical roof lift, persisted grenade count, return route and re-entry.');


const tankSlot='mg1987.grayfox.browser.v1';
const tankStorage=new Map([[tankSlot,JSON.stringify({version:10,stage:9,x:88.5,z:0,
 health:100,service:{charge:true,open:true,intel:true,kidDown:true,parachute:true},
 roof:{active:true,defeated:true,hp:0,grenades:3,phase:'disabled',clock:0,lap:0,flyX:82,flyZ:0}})]]);
const tankRun=boot(tankStorage);tankRun.button('resume');
assert.equal(tankRun.game.state.roof.defeated,true);
tankRun.game.act('interact');
assert.equal(tankRun.game.state.stage,10,'post-Hind rooftop exit enters Building 2');
assert.equal(tankRun.game.state.b2.active,true);
tankRun.game.teleport(105,1.1);
tankRun.game.act('interact');
assert.equal(tankRun.game.state.b2.mines,3,'a mined track uses an actual mine');
assert.equal(tankRun.game.state.b2.armed.length,1,'mine persists as track site');
const tankReload=boot(tankStorage);tankReload.button('resume');
assert.equal(tankReload.game.state.stage,10);
assert.equal(tankReload.game.state.b2.mines,3);
assert.equal(tankReload.game.state.b2.armed.length,1);
tankReload.game.teleport(98.5,0);tankReload.game.act('interact');
assert.equal(tankReload.game.state.b2.active,false,'retreat is reversible');
assert.equal(tankReload.game.state.roof.active,true);
tankReload.game.teleport(88.5,0);tankReload.game.act('interact');
assert.equal(tankReload.game.state.b2.active,true,'no tank progress reset after return');
assert.equal(tankReload.game.state.b2.mines,3);
console.log('PASS: Building 2 roof descent, physical mine placement, checkpoint, retreat and re-entry.');
