/* 1995 performance layer: interpersonal story follows the original plot.
 * This script has ZERO mission authority: no cards, flags, locks, targeting,
 * health or inventory. It serves only new, short, source-ranked dialogue and
 * player-earned optional context. All lines are original adaptation prose,
 * NOT extracted or transcribed original Metal Gear dialogue.
 */
(function(root){
'use strict';
const scenes=Object.freeze({
 'ST.P01.BRIEF':{
  stageMin:-2,stageMax:-2,delivery:'codec_nonblocking',lines:[
   {speaker:'BIG BOSS · 120.85',text:'Intrude N313. Find Gray Fox. I need what he actually saw, Snake.',duration:4.3},
   {speaker:'SNAKE',text:'If the report is wrong?',duration:2.0},
   {speaker:'BIG BOSS · 120.85',text:'Then bring me the evidence. Stay out of sight.',duration:3.1}
  ]},
 'ST.P03.CAPTURE':{
  stageMin:0,stageMax:0,delivery:'quiet_world',lines:[
   {speaker:'SNAKE',text:'No weapons. No radio. But I am inside.',duration:3.0}
  ]},
 'ST.P03.FOX':{
  stageMin:2,stageMax:2,delivery:'room_not_radio',lines:[
   {speaker:'SNAKE',text:'Can you stand?',duration:2.1},
   {speaker:'GRAY FOX',text:'Long enough. We can talk when we are clear of the cells.',duration:4.0}
  ]},
 'ST.P03.LISTEN':{
  stageMin:2,stageMax:3,delivery:'player_selected',lines:[
   {speaker:'GRAY FOX',text:'At every change of watch, the boots slow at the junction. They always check the same post.',duration:5.5},
   {speaker:'SNAKE',text:'You learned their patrols by listening?',duration:2.5},
   {speaker:'GRAY FOX',text:'Some days it was all there was to do.',duration:3.1}
  ]},
 'ST.P04.GEAR':{
  stageMin:4,stageMax:4,delivery:'quiet_world',lines:[
   {speaker:'SNAKE',text:'The seal is new. My kit has been handled.',duration:3.3}
  ]},
 'ST.P04.TRACKER':{
  stageMin:4,stageMax:5,delivery:'codec_optional',lines:[
   {speaker:'SNAKE',text:'Someone wanted me to carry that transmitter out of here.',duration:4.0},
   {speaker:'BIG BOSS · 120.85',text:'Remove it. Make sure they cannot use your position.',duration:4.1}
  ]},
 'ST.P04.BOSS':{
  stageMin:5,stageMax:6,delivery:'after_boss',lines:[
   {speaker:'SNAKE',text:'Quiet. Even the guards have stopped shouting.',duration:3.2}
  ]},
 'ST.P05.DOGS':{
  stageMin:7,stageMax:8,delivery:'optional_codec',lines:[
   {speaker:'SCHNEIDER · 120.79',text:'That old service run should be lower than the kennel floor. But they changed the walls after I left.',duration:5.7},
   {speaker:'SNAKE',text:'I will check the construction, not your memory.',duration:3.4},
   {speaker:'SCHNEIDER · 120.79',text:'Good. That is what I would do.',duration:2.4}
  ]},
 'ST.P05.KID':{
  stageMin:8,stageMax:8,delivery:'after_boss',lines:[
   {speaker:'SNAKE',text:'A weapon like that needs time to feed. There was my opening.',duration:3.5}
  ]}
});
function sceneLines(id,context={}){
 const s=scenes[id];
 if(!s||!Number.isFinite(context.stage)||context.stage<s.stageMin||
    context.stage>s.stageMax||context.seen?.includes?.(id)||
    (context.inVr===true)||context.alert>=3)return [];
 // A transcript/UI/VO performance is NOT allowed to change the mission.
 return s.lines.map((line,i)=>Object.freeze({
  id:id+'.'+i,speaker:line.speaker,text:line.text,duration:line.duration,
  direction:'Original 1995 adaptation, emotionally restrained; '+s.delivery,
  storyOnly:true
 }));
}
root.MGStoryPerformance=Object.freeze({
 scenes,sceneLines,has:(id)=>Object.prototype.hasOwnProperty.call(scenes,id)
});
})(typeof window==='undefined'?globalThis:window);
