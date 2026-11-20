const assert=require('node:assert/strict');
const {FieldRigs:rig}=require('../assets/field_rigs.js');
assert.equal(rig.bones.length,18);
for(const [i,[name,parent]] of rig.bones.entries())assert.ok(parent<i,name);
for(const role of ['snake','fox','guard','shoot_gunner']){
 const [body,gear]=rig.model(role);
 assert.ok(body.length>3000 && body.length%3===0);
 assert.equal(gear.length>0,role==='snake');
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
console.log('PASS: costume layers, joint hierarchy, finite normals, skin bindings and gameplay poses.');
