/* Independently written temporary scene dialogue; not transcribed J2ME text. */
window.MGDialogue = Object.freeze({
  informant:{speaker:'PRISONER',text:'Fox is in solitary. The guards at processing take prisoners straight through. It is the only way inside.',duration:6},
  schneider:{speaker:'SCHNEIDER · 120.79',text:'I worked on the buildings here. They have altered the original construction since then.',duration:4},
  bigBoss:{speaker:'BIG BOSS · 120.85',text:'Check the cell walls, Snake. Listen closely.',duration:3},
  continueMission:{speaker:'BIG BOSS · 120.85',text:'Stay on mission, Snake.',duration:2},
  capture: {speaker:'MISSION SYSTEM',text:'The bolt slides home. Your equipment is gone.',duration:5.2,direction:'Low, economical. No triumphant sting.'},
  breach1: {speaker:'SNAKE',text:'Hollow concrete. Someone patched this wall in a hurry.',duration:4.5,direction:'Observant; quiet disbelief.'},
  rescue1: {speaker:'GRAY FOX',text:'Snake? Keep your voice down. I thought they sent you to the same fate.',duration:5.9,direction:'Exhausted, subdued relief, whispered urgency.'},
  rescue2: {speaker:'SNAKE',text:'What did you find?',duration:3.2,direction:'Relieved but still assessing danger.'},
  rescue3: {speaker:'GRAY FOX',text:'Metal Gear is a walking tank that can launch a nuclear missile. Find Dr. Pettrovich. He designed it.',duration:7.3,direction:'A whispered confession, with a meaningful pause after Metal Gear.'},
  escortIntro: {speaker:'GRAY FOX',text:'I can move, Snake. Stay low. I will follow you, but I have no weapon.',duration:5.2,direction:'Tired but determined; not a robotic instruction.'},
  escortFail: {speaker:'MISSION SYSTEM',text:'Gray Fox is down. Extraction has failed.',duration:4.0,direction:'Clear, subdued failure announcement.'},
  equipment: {speaker:'SNAKE',text:'My equipment. Just as I left it.',duration:3.8,direction:'Controlled relief, no boasting.'},
  secure: {speaker:'GRAY FOX',text:'The shutter is locked. They cannot follow us here. You have to find Pettrovich.',duration:5.4,direction:'Out of breath, urgent clarity; not triumphant.'},
  exit: {speaker:'SNAKE',text:'Another weak seam. The guard corridor is open.',duration:4.0,direction:'Practical assessment, new alertness.'},
  detected: {speaker:'GUARD',text:'Contact! Intruder in the cell block!',duration:3.4,direction:'Sudden shout, genuinely surprised.'},
  searched: {speaker:'GUARD',text:'Area search. Check every corner.',duration:3.6,direction:'Measured urgency over radio.'},
  clear: {speaker:'GUARD',text:'Nothing confirmed. Resuming patrol.',duration:3.4,direction:'Fatigued, not omniscient.'},
  fail: {speaker:'MISSION SYSTEM',text:'Snake has been incapacitated. Restoring the last checkpoint.',duration:3.1,direction:'Neutral and precise.'}
});
window.MGVoiceDirection = Object.freeze({
  snake:{tone:'restrained, younger FOXHOUND operative',pace:'medium; deliberate rests',codec:'radio filtered only for remote communication'},
  fox:{tone:'exhausted, alert and deeply relieved',pace:'subtle hesitations and responsive phrasing',codec:'not a radio-filtered line when standing next to Snake'},
  guard:{tone:'unpolished military urgency',pace:'quick discovery, slower search',codec:'small radio filter only for transmitted messages'}
});
