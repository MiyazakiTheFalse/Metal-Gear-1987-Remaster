// Rebuild editable glTF 2.0 assets from the mesh and motion data used in play.
const {FieldRigs:rig}=require('../assets/field_rigs.js');
const fs=require('node:fs'), path=require('node:path');
const out=process.argv[2]||path.join(__dirname,'../assets/models');
fs.mkdirSync(out,{recursive:true});
for(const role of ['snake','fox','guard']){
 const chunks=[], views=[], accessors=[], nodes=[], primitives=[], animations=[];
 let size=0;
 function accessor(values,type,component=5126,bounds=false){
  const width={SCALAR:1,VEC3:3,VEC4:4,MAT4:16}[type];
  const T=component===5126?Float32Array:Uint16Array;
  const data=Buffer.from(new T(values).buffer),index=views.length;
  views.push({buffer:0,byteOffset:size,byteLength:data.length});
  chunks.push(data);size+=data.length;
  if(size%4){chunks.push(Buffer.alloc(4-size%4));size+=4-size%4;}
  const a={bufferView:index,componentType:component,count:values.length/width,type};
  if(bounds){a.min=Array(width).fill(Infinity);a.max=Array(width).fill(-Infinity);
   values.forEach((v,i)=>{const k=i%width;a.min[k]=Math.min(v,a.min[k]);a.max[k]=Math.max(v,a.max[k]);});}
  return accessors.push(a)-1;
 }
 const inverses=[];
 rig.bones.forEach(([name,parent,x,y,z],i)=>{
  const b=rig.bones[parent]||['',0,0,0,0];
  nodes.push({name,translation:[x-b[2],y-b[3],z-b[4]]});
  if(parent>=0)(nodes[parent].children??=[]).push(i);
  inverses.push(1,0,0,0,0,1,0,0,0,0,1,0,-x,-y,-z,1);
 });
 for(const [layer,verts] of rig.model(role).entries()){
  if(!verts.length)continue;
  const keys=new Map(),pos=[],norm=[],col=[],joints=[],weights=[],indices=[];
  for(const v of verts){
   const key=v.join(',');
   if(!keys.has(key)){
    keys.set(key,keys.size);pos.push(...v.slice(0,3));norm.push(...v.slice(3,6));
    col.push(...v.slice(6,9));joints.push(v[9],Math.max(0,rig.bones[v[9]][1]),0,0);
    weights.push(1-v[10],v[10],0,0);
   }
   indices.push(keys.get(key));
  }
  primitives.push({attributes:{POSITION:accessor(pos,'VEC3',5126,true),NORMAL:accessor(norm,'VEC3'),
   COLOR_0:accessor(col,'VEC3'),JOINTS_0:accessor(joints,'VEC4',5123),WEIGHTS_0:accessor(weights,'VEC4')},
   indices:accessor(indices,'SCALAR',5123),material:0,extras:{layer:layer?'field gear':'body'}});
 }
 const keys=Array.from({length:25},(_,i)=>i/24);
 const times=accessor(keys,'SCALAR',5126,true);
 for(const [name,state] of Object.entries({Idle:{},Walk:{stride:.72},Run:{stride:1},Crouch:{crouch:1},Prone:{prone:1},Down:{down:1},...(role==='fox'?{Captive:{captive:true}}:{})})){
  const frames=keys.map(t=>rig.pose({...state,time:t*2*Math.PI/2.4,phase:t*2*Math.PI}));
  const channels=[],samplers=[];
  function track(bone,key,output){
   channels.push({sampler:samplers.length,target:{node:bone,path:key}});
   samplers.push({input:times,output,interpolation:'LINEAR'});
  }
  rig.bones.forEach((_,i)=>track(i,'rotation',accessor(frames.flatMap(f=>
   [0,0,Math.sin(f.rot[i]/2),Math.cos(f.rot[i]/2)]),'VEC4')));
  track(0,'translation',accessor(frames.flatMap(f=>f.offset.map((v,k)=>v+rig.bones[0][2+k])),'VEC3'));
  animations.push({name,channels,samplers});
 }
 const meshNode=nodes.push({name:role,mesh:0,skin:0})-1;
 const inverseBindMatrices=accessor(inverses,'MAT4');
 const doc={asset:{version:'2.0',generator:'Outer Heaven field rig exporter'},scene:0,scenes:[{nodes:[0,meshNode]}],
  nodes,meshes:[{name:role,primitives}],materials:[{name:'Uniform and skin',doubleSided:true,pbrMetallicRoughness:{metallicFactor:0,roughnessFactor:.86}}],
  skins:[{skeleton:0,joints:rig.bones.map((_,i)=>i),inverseBindMatrices}],animations,
  buffers:[{byteLength:size}],bufferViews:views,accessors};
 const json=Buffer.from(JSON.stringify(doc)),pad=Buffer.alloc((4-json.length%4)%4,32),binary=Buffer.concat(chunks);
 const header=Buffer.alloc(20),binHeader=Buffer.alloc(8);
 header.write('glTF');header.writeUInt32LE(2,4);header.writeUInt32LE(28+json.length+pad.length+size,8);
 header.writeUInt32LE(json.length+pad.length,12);header.write('JSON',16);
 binHeader.writeUInt32LE(size);binHeader.write('BIN\0',4);
 fs.writeFileSync(path.join(out,role+'.glb'),Buffer.concat([header,json,pad,binHeader,binary]));
}
