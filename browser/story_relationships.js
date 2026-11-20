/* Character story is NOT the mission-state machine. The original MSX2
 * plot/locks remain authoritative; these independent moments add optional,
 * earned human context and are always recoverable through the intelligence log.
 * The 1995 voices below are ORIGINAL adaptation lines, not game quotations.
 */
(function(root){
'use strict';
const SCENES=Object.freeze({
 'P03.capture':{
  evidence:'Snake was taken into custody and lost his own kit.',
  lines:[{speaker:'SNAKE',text:'No radio. No equipment. I still know what I came here to do.',duration:3.8,
    direction:'Said quietly while testing the cell, not a triumphant speech.'}]},
 'P03.wall1':{
  evidence:'The first weakened cell wall was broken by Snake.',
  lines:[{speaker:'SNAKE',text:'That wall was repaired from this side. Someone was meant to stay here.',duration:4.0,
    direction:'Observes physical evidence; never asserts a location that he could not see.'}]},
 'P03.fox':{
  evidence:'Snake has physically reached Gray Fox and heard about Metal Gear.',
  lines:[{speaker:'GRAY FOX',text:'They sent a rookie. I thought they had stopped looking.',duration:3.7,
    direction:'Not cruel; exhaustion hides genuine relief.'},
   {speaker:'SNAKE',text:"I'm here, aren't I?",duration:2.1,
    direction:'Confident enough to answer simply, not boastful.'},
   {speaker:'GRAY FOX',text:'Then listen. I can tell you what I saw. Not what they told me.',duration:4.1,
    direction:'First genuine peer-to-peer trust, cautious and specific.'}]},
 'P04.bag':{
  evidence:'Snake has regained his confiscated equipment.',
  lines:[{speaker:'SNAKE',text:"They inventoried every piece. That doesn't mean they put it back untouched.",duration:3.6,
    direction:'Troubled by observed handling; no premature certainty about the identity of a traitor.'}]},
 'P04.tracker':{
  evidence:'Snake hears unexplained chirping from a transmitter in recovered gear.',
  lines:[{speaker:'SNAKE',text:'The chirp changes when I move. Someone is tracking the kit, not just the doors.',duration:4.0,
    direction:'Actual audible clue. Affects player suspicion, not an unearned lore flag.'}]},
 'P04.card3':{
  evidence:'Card 3 was physically collected; adjacent service-sector route is now accessible.',
  lines:[{speaker:'SCHNEIDER',text:"Another clearance? My plans showed an ordinary storeroom there. They rebuilt more than I knew.",duration:4.5,
    direction:'A human helper admitting limitations instead of supplying a perfect map.'}]}
});
const RADIO=Object.freeze({
 cell:{id:'storyRadioCell',speaker:'SCHNEIDER · 120.79',
  text:"I'm working from older plans, Snake. Tell me what you've seen; I won't guess which wall they changed.",duration:5.0,
  direction:'Calm, frustrated by imperfect information.'},
 rescued:{id:'storyRadioFox',speaker:'BIG BOSS · 120.85',
  text:'You found Fox. Good. Keep the objective in sight. Confirm his account before acting on it.',duration:4.9,
  direction:'Professional advice which gains different meaning later; never an explicit early confession.'},
 gear:{id:'storyRadioGear',speaker:'BIG BOSS · 120.85',
  text:'You recovered your gear. Check it carefully. Do not assume the prison staff left it alone.',duration:4.4,
  direction:'Plausible guidance, no privileged admission of who set the transmitter.'},
 foxContact:{id:'storyRadioFoxContact',speaker:'GRAY FOX',text:
  'The guard with the heavy boots always came second. The one carrying water came first. I could tell by the sound.',duration:6.0,
  direction:'Optional in-world observation; no enemy coordinates or permanent escort required.'},
 card3:{id:'storyRadioCard3',speaker:'SCHNEIDER · 120.79',
  text:"Card 3 changes what you can open, not what I know. I can help with older corridors; you'll have to check the new ones.",duration:5.8,
  direction:'Trust through honest limitations and actionable, earned contextual advice.'}
});
let seen=new Set();
const allowed=new Set(Object.keys(SCENES));
function restore(ids){
 seen=new Set(Array.isArray(ids)?ids.filter(id=>typeof id==='string'&&allowed.has(id)).slice(0,48):[]);
}
function commit(id){
 if(!allowed.has(id)||seen.has(id))return [];
 seen.add(id);
 return SCENES[id].lines.map((line,i)=>({id:'arc.'+id+'.'+i,...line}));
}
function heard(){return [...seen]}
function getDialogue(id){
 if(id.startsWith('arc.')){
  const rest=id.slice(4),at=rest.lastIndexOf('.'),key=rest.slice(0,at),n=Number(rest.slice(at+1));
  if(!allowed.has(key)||!Number.isSafeInteger(n))return null;
  return SCENES[key].lines[n]||null;
 }
 for(const v of Object.values(RADIO))if(v.id===id)return v;
 return null;
}
function radio(key){
 const v=RADIO[key];return v?{...v}:null;
}
function evidence(stage){
 const results=[];
 if(stage>=0)results.push('Custody cost: Snake lost his gear and access to command.');
 if(stage>=1)results.push('A repaired section of the cell wall revealed an undocumented passage.');
 if(stage>=2)results.push('Gray Fox has described a mobile, nuclear-capable weapons project.');
 if(stage>=4)results.push('Snake recovered his own equipment; its handling is suspect.');
 if(stage>=6)results.push('Card 3 opened further clearance while old facility plans remained incomplete.');
 return results;
}
function dossier(stage){
 return ['STORY / RELATIONSHIPS — OBSERVED EVIDENCE',
  'SOLID SNAKE: '+(stage>=2?'No longer working from command alone; has heard a fellow operative’s firsthand account.':'A FOXHOUND rookie acting under a limited mission briefing.'),
  'BIG BOSS: '+(stage>=0?'Commanding officer; currently out of reach during detention.':'Commanding officer. Orders and information remain limited.'),
  'GRAY FOX: '+(stage>=2?'Rescued operative. Relieved, exhausted, and a source of firsthand knowledge.':'Missing FOXHOUND operative; no confirmed contact yet.'),
  'SCHNEIDER: Resistance guide. Old construction plans may not match modified corridors.',
  ...evidence(stage).map(x=>'EVIDENCE: '+x)].join('\n');
}
root.MGCharacterStory=Object.freeze({commit,restore,heard,getDialogue,radio,dossier,
 events:SCENES,radioScenes:RADIO});
})(typeof window==='undefined'?globalThis:window);
