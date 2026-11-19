const assert=require('node:assert/strict');
const {FieldRigs:rig}=require('../assets/field_rigs.js');
assert.equal(rig.bones.length,16);
for(const [i,[name,parent]] of rig.bones.entries())assert.ok(parent<i,name);
for(const role of ['snake','fox','guard']){
 const [body,gear]=rig.model(role);
 assert.ok(body.length>3000 && body.length%3===0);
 assert.equal(gear.length>0,role==='snake');
 for(const v of [...body,...gear]){
  assert.ok(v.every(Number.isFinite));
  assert.ok(Math.abs(Math.hypot(...v.slice(3,6))-1)<1e-5);
  assert.ok(Number.isInteger(v[9])&&v[9]>=0&&v[9]<rig.bones.length&&v[10]>=0&&v[10]<=1);
 }
}
for(const state of [{},{stride:1,phase:1.4},{crouch:1},{prone:1},{down:1},{cover:1},{strike:1}]){
 const {rot,offset}=rig.pose(state);
 assert.equal(rot.length,16);assert.ok([...rot,...offset].every(Number.isFinite));
}
console.log('PASS: costume layers, joint hierarchy, finite normals, skin bindings and gameplay poses.');
