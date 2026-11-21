/* Zero-cost source regression: campaign plot vs earned character story.
 * node --test browser/tests/test_story_relationships.cjs
 * Runs without WebGL, network, Unreal Editor or generated human meshes.
 */
'use strict';
const test=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const path=require('node:path');
const vm=require('node:vm');
const ROOT=path.resolve(__dirname,'../..');
const read=p=>fs.readFileSync(path.join(ROOT,p),'utf8');
const storySrc=read('browser/story_relationships.js');
const filmSrc=read('browser/cinematic_director.js');
const gameSrc=read('browser/game.js');
const unrealSrc=read('unreal/MGRemaster/Source/MGRemaster/Private/MGGrayFoxMission.cpp');
const sceneLedger=JSON.parse(read('data/FULL_CAMPAIGN_CINEMATIC_SHOT_LEDGER_1995.json'));
const beatLedger=JSON.parse(read('data/FULL_CAMPAIGN_BEAT_BY_BEAT_IMPLEMENTATION_LEDGER.json'));
const arcs=JSON.parse(read('data/STORY_RELATIONSHIP_ARCS_1995.json'));
function loadStory(){const context={window:{}};vm.runInNewContext(storySrc,context);return context.window.MGCharacterStory}
test('Nine 1995 cast arcs each have motivated story/plot separation across all ten phases',()=>{
 assert.equal(arcs.characters.length,9);
 assert.equal(arcs.missions.length,10);
 const beats=new Set(beatLedger.missions.flatMap(m=>m.beats.map(b=>b.id)));
 for(const m of arcs.missions)
  for(const e of m.events){
   assert.ok(beats.has(e.triggerBeat),e.id+' must attach to a true beat');
   assert.ok(e.neverAutoReveal.includes('bigBoss_OuterHeaven_identity'));
  }
});
test('Fox optional listened-to patrol lesson is a human relationship and one-time persisted record',()=>{
 const story=loadStory();
 assert.equal(story.commit('P03.listen').length,2);
 assert.equal(story.commit('P03.listen').length,0);
 const saved=story.heard();assert.ok(saved.includes('P03.listen'));
 story.restore(saved);assert.equal(story.commit('P03.listen').length,0);
 assert.match(gameSrc,/FOX: BOOTS SLOWING/);
 assert.match(gameSrc,/world\.foxFollowup&&stage>=2&&stage<=3/);
 assert.match(gameSrc,/!g\.postGuard&&dist\(g,player\)<6\.2/);
});
test('Betrayal, Ellen and later character facts cannot arrive through early dossier or radio',()=>{
 const story=loadStory();
 for(const stage of [-2,0,2,4,8]){
  const dossier=story.dossier(stage);
  assert.doesNotMatch(dossier,/leader of Outer Heaven|Big Boss betrayed|Ellen was rescued/);
 }
 assert.equal(story.commit('P10.betrayal').length,0);
 assert.match(story.radio('kennel').text,/old utility channel/);
});
test('Story performances are presentation only; all pickups and alarm AI belong to game',()=>{
 assert.match(storySrc,/const SCENES=Object\.freeze/);
 assert.match(gameSrc,/storySeen:window\.MGCharacterStory\?\.heard\(\)/);
 assert.match(gameSrc,/MGCharacterStory\?\.restore\(data\.storySeen\)/);
 assert.match(gameSrc,/tellCharacterStory\('P05\.kid'\)/);
 assert.match(gameSrc,/tellCharacterStory\('P05\.log'\)/);
});
test('Seven early 3D cinematics exist but are not described as finished',()=>{
 const source=sceneLedger.scenes.filter(s=>s.implementation==='source_prototype');
 assert.equal(source.length,7);
 for(const scene of source){
  assert.equal(scene.unrealPIEVerified,false);
  assert.equal(scene.artistApproved,false);
  assert.ok(scene.unrealSource?.includes('unreal/MGRemaster/Source/MGRemaster/Private/MGStoryCameraDirector.cpp'));
 }
 assert.match(unrealSrc,/StartStoryCinematic\(TEXT\("capture"\), Player, Player\)/);
 assert.match(unrealSrc,/bCapturePending/);
 assert.match(unrealSrc,/ApplyAction\(MG_FOX_CAPTURE, Pending\)/);
});
test('Skipping browser cutscene calls the mission continuation exactly once',()=>{
 let card=null;
 const doc={
  createElement:()=>({
   classList:{add:()=>{},remove:()=>{}},
   setAttribute:()=>{},querySelector:()=>({textContent:''}),innerHTML:''
  }),
  head:{appendChild:()=>{}},body:{appendChild:()=>{}}
 };
 const ctx={window:{},document:doc};
 vm.runInNewContext(filmSrc,ctx);
 const camera=ctx.window.MGCinematicDirector;
 let fired=0;
 assert.ok(camera.start('capture',{snake:{x:-14,z:0}},{
  blocking:true,onDone:()=>{fired++}
 }));
 card=camera.update(.15,{});
 assert.ok(card&&card.eye.length===3);
 assert.equal(camera.skip(),true);
 assert.equal(camera.skip(),false);
 assert.equal(fired,1);
});
