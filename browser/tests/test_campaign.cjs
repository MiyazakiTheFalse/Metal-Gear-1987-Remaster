/* Run with node browser/tests/test_campaign.cjs. DOM/WebGL calls are stubs;
   movement, obstruction, interaction, inventory, checkpoint and AI are real. */
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

function boot(storage = new Map()) {
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
  const runtime = { console, performance, URLSearchParams, location:{search:'', hash:''},
    document:{getElementById:element, addEventListener:noop}, addEventListener:noop,
    navigator:{getGamepads:() => []}, requestAnimationFrame:noop, setTimeout:noop,
    localStorage:{getItem:key=>storage.get(key),setItem:(key,value)=>storage.set(key,value)},
    __MG_TEST__:true };
  runtime.window=runtime;
  vm.createContext(runtime);
  for (const file of ['dialogue.js','tutorial.js','game.js'])
    vm.runInContext(fs.readFileSync(path.join(__dirname,'..',file),'utf8'),runtime,{filename:file});
  return {game:runtime.__mgGameDebug,button:id=>element(id).onclick(),storage};
}

let run=boot();
run.button('start');
const stage=()=>run.game.state.stage;
const act=name=>run.game.act(name);
const next=()=>run.game.step(.31);
function walk(x,z) {
  const game=run.game, from=game.state.player;
  const route=game.findPath(from,{x,z});
  assert.ok(route.length,'walkable route to '+[x,z]);
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
for(let i=0;i<9&&stage()===4;i++){act('melee');run.game.step(.31)}
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
