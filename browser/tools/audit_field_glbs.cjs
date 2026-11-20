#!/usr/bin/env node
// Inspect real exported GLB bytes; a source rig declaration is not a delivered model.
// node browser/tools/audit_field_glbs.cjs [directory]
const fs=require('node:fs'),path=require('node:path');
const dir=process.argv[2]||path.join(__dirname,'../assets/models');
let failed=0;
for(const role of ['snake','fox','guard','shoot_gunner']){
 try{
  const file=fs.readFileSync(path.join(dir,role+'.glb'));
  function check(ok,message){if(!ok)throw new Error(message);}
  check(file.length>=28&&file.toString('utf8',0,4)==='glTF','invalid glb header');
  check(file.readUInt32LE(4)===2,'expected glTF 2.0');
  check(file.readUInt32LE(8)===file.length,'incorrect binary length');
  const jsonSize=file.readUInt32LE(12);
  check(file.toString('utf8',16,20)==='JSON','missing JSON first chunk');
  const gltf=JSON.parse(file.toString('utf8',20,20+jsonSize));
  check(gltf.skins?.length===1,'expected one skinned body');
  check(gltf.skins[0].joints.length===18,'all cast must have 18 real joints');
  const names=new Set(gltf.nodes.map(n=>n.name));
  for(const joint of ['pelvis','chest','neck','head','jaw','brow','hand_L','hand_R','foot_L','foot_R'])
   check(names.has(joint),'missing joint '+joint);
  check(gltf.meshes?.[0]?.primitives?.length>0,'no renderable skinned mesh');
  const attrs=gltf.meshes[0].primitives[0].attributes;
  for(const a of ['POSITION','NORMAL','JOINTS_0','WEIGHTS_0'])
   check(attrs[a]!==undefined,'missing vertex channel '+a);
  const clips=new Map((gltf.animations||[]).map(a=>[a.name,a]));
  for(const name of ['Idle','IdleAlert','IdleLook','IdleWary','Walk','Run','Crouch','Prone','Down','Aim','Cover','Melee','Reaching','Hit','RadioListen','CrouchAim','ProneAim'])
   check(clips.has(name),'missing authored clip '+name);
  check(clips.get('RadioListen').channels.length===19,'facial/body channels not all exported');
  if(role==='fox')check(clips.has('Captive'),'Fox missing captive performance');
  if(role==='shoot_gunner')
   for(const name of ['Address','Shoulder','Recoil','Search'])
    check(clips.has(name),'boss missing '+name);
  for(const [name,animation] of clips){
   check(animation.samplers.length===animation.channels.length,
    'sampler/channel count mismatch for '+name);
   for(const channel of animation.channels)
    check(channel.target.node>=0&&channel.target.node<gltf.nodes.length,
     'animation points outside node graph in '+name);
  }
  const prims=gltf.meshes[0].primitives;
  const vertices=prims.reduce((sum,p)=>sum+gltf.accessors[p.attributes.POSITION].count,0);
  check(vertices>500,'mesh is an empty/static proxy');
  console.log('PASS',role,'bones',gltf.skins[0].joints.length,
              'clips',clips.size,'mesh vertices',vertices);
 }catch(err){failed++;console.error('FAIL',role,err.message);}
}
if(failed)process.exitCode=1;
