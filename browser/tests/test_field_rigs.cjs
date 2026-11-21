const assert=require('node:assert/strict');
const {FieldRigs:rig}=require('../assets/field_rigs.js');
assert.equal(rig.bones.length,18);
for(const [i,[name,parent]] of rig.bones.entries())assert.ok(parent<i,name);
for(const role of rig.castRoles){
 const [body,gear]=rig.model(role);
 assert.ok(body.length>3000 && body.length%3===0);
 assert.equal(gear.length>0,['snake','real_big_boss','venom_snake'].includes(role));
 assert.equal(rig.skeleton(role).length,18,'all cast members have facial joints');
 for(const v of [...body,...gear]){
  assert.ok(v.every(Number.isFinite));
  assert.ok(Math.abs(Math.hypot(...v.slice(3,6))-1)<1e-5);
  assert.ok(Number.isInteger(v[9])&&v[9]>=0&&v[9]<rig.skeleton(role).length&&v[10]>=0&&v[10]<=1);
 }
}
for(const state of [{},{stride:1,phase:1.4},{crouch:1},{prone:1},{down:1},{cover:1},{strike:1}]){
 const {rot,offset}=rig.pose(state);
 assert.equal(rot.length,18);assert.ok([...rot,...offset].every(Number.isFinite));
}
const talking=rig.pose({mouth:1,brow:1,radio:1});
const silent=rig.pose({mouth:0,brow:0,radio:0});
assert.notEqual(talking.rot[16],silent.rot[16],'jaw should articulate under speech');
assert.notEqual(talking.rot[17],silent.rot[17],'brow should articulate under acting cues');
assert.notEqual(talking.rot[2],silent.rot[2],'radio listen posture uses the body rig');
// Idles are actual looping joint animation, not frozen static stance meshes.
const idleA=rig.pose({time:0}),idleB=rig.pose({time:1.11});
assert.ok(Math.abs(idleA.rot[3]-idleB.rot[3])>.001,'idle head should scan');
assert.ok(Math.abs(idleA.rot[1]-rig.pose({time:.65}).rot[1])>.005,'idle ribcage breathing should animate');
const alert=rig.pose({time:1.11,idleAlert:1});
const wary=rig.pose({time:1.11,idleWary:1});
assert.notEqual(alert.rot[1],idleB.rot[1],'alert breathing varies from relaxed idle');
assert.notEqual(wary.rot[4],idleB.rot[4],'wary pose raises shoulders');
const moving=rig.pose({time:1.11,stride:1,phase:1});
assert.ok(Math.abs(moving.rot[3])<Math.abs(idleB.rot[3]),'movement suppresses gaze fidget');
assert.ok([...idleA.rot,...idleB.rot,...moving.rot].every(Number.isFinite));
console.log('PASS: costume layers, joint hierarchy, finite normals, skin bindings and gameplay poses.');
