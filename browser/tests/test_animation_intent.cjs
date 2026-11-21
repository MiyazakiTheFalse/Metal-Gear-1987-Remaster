const assert=require('node:assert/strict');
const intent=require('../animation_intent.js');
assert.equal(intent.select({kind:'player',id:'snake',stance:2,aiming:true},0),'ProneAim');
assert.equal(intent.select({kind:'player',id:'snake',stance:1,aiming:true},0),'CrouchAim');
assert.equal(intent.select({kind:'player',id:'snake',stance:2},0),'Prone');
assert.equal(intent.select({kind:'player',id:'snake',cover:true,aiming:true},0),'Aim');
assert.equal(intent.select({kind:'player',id:'snake',cover:true,knocking:true},0),'Melee');
assert.equal(intent.select({kind:'player',id:'snake',hit:true,aiming:true},0),'Hit');
assert.equal(intent.select({kind:'player',id:'snake',down:true,hit:true},0),'Down');
assert.equal(intent.select({kind:'fox',id:'fox',captive:true},0),'Captive');
assert.equal(intent.select({kind:'fox',id:'fox',radio:true},0),'RadioListen');
assert.equal(intent.select({kind:'guard',id:'g1',alert:3},0),'IdleAlert');
assert.equal(intent.select({kind:'guard',id:'g1',searching:true},0),'IdleLook');
assert.equal(intent.select({kind:'guard',id:'g1',alert:1},0),'IdleWary');
assert.equal(intent.select({kind:'guard',id:'g1'},1),'Walk');
assert.equal(intent.select({kind:'guard',id:'g1'},3.8),'Run');
for(const [args,want] of [
 [{kind:'shoot_gunner',id:'shotmaker',firing:true,reloading:true},'Recoil'],
 [{kind:'shoot_gunner',id:'shotmaker',reloading:true,telegraph:true},'Reaching'],
 [{kind:'shoot_gunner',id:'shotmaker',telegraph:true},'Shoulder'],
 [{kind:'shoot_gunner',id:'shotmaker',addressing:true},'Address'],
 [{kind:'shoot_gunner',id:'shotmaker',searching:true},'Search']
])assert.equal(intent.select(args,0),want);
assert.equal(intent.looped('Down'),false);
assert.equal(intent.looped('Walk'),true);
assert.equal(intent.idleVariant('guard4',12),intent.idleVariant('guard4',12),
 'idle variation is stable from role and time, never nondeterministic RNG');
for(const role of ['player','fox','guard','shoot_gunner'])
 for(const speed of [0,.3,1.5,4.8])
  for(const time of [0,1,8,19]){
   const selected=intent.select({kind:role,id:role,alert:0},speed,time);
   assert.ok(intent.clipSet.includes(selected),selected);
  }
console.log('PASS: source GLB animation intent has priority, stable idle variety, motion and boss-specific acting.');
