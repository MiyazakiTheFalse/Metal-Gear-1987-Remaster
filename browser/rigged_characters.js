/*
 * Optional GLB review renderer for the ORIGINAL first-party 1995 cast.
 * Six source-controlled, separately skinned active models are shared with Unreal.
 * Gameplay, AI, collision, hit resolution and checkpoints stay in game.js.
 * Only the Three.js renderer/importer is streamed; character data is local.
 * When a role/animation/renderer is missing, keep the built-in WebGL rigs.
 */
import * as THREE from 'three';
import {GLTFLoader} from 'three/addons/loaders/GLTFLoader.js';
import {clone as cloneSkeleton} from 'three/addons/utils/SkeletonUtils.js';

const ROLE_FILES=Object.freeze({
 player:'assets/models/snake.glb',
 fox:'assets/models/fox.glb',
 guard:'assets/models/guard.glb',
 machinegun_kid:'assets/models/machinegun_kid.glb',
 pow_informant:'assets/models/pow_informant.glb',
 shoot_gunner:'assets/models/shoot_gunner.glb'
});
const ROLE_HEIGHTS=Object.freeze({
 player:1.78,fox:1.79,guard:1.76,shoot_gunner:1.83,
 machinegun_kid:1.78,pow_informant:1.72
}); // Prototype centimetres are reversible, not final character-bible approvals.
const status=document.getElementById('rigStatus');
const toggle=document.getElementById('rigToggle');
const originalCanvas=document.getElementById('gl');
const label=message=>{if(status)status.textContent=message;};
const intent=window.MGAnimationIntent;
if(!intent)throw new Error('Local animation intent script was not loaded.');
const sources=new Map(),actors=new Map(),occluders=new Map();
let enabled=!!toggle?.checked,ready=false,renderer=null;
const stage=new THREE.Scene();
const camera=new THREE.PerspectiveCamera(61,1,.075,120);
stage.add(new THREE.HemisphereLight(0xcce9d9,0x23302b,1.6));
const keyLight=new THREE.DirectionalLight(0xf4f1ce,2.1);
keyLight.position.set(-3,8.5,4.2);stage.add(keyLight);
const fillLight=new THREE.DirectionalLight(0x63a9a1,.5);
fillLight.position.set(4.5,3.4,-5);stage.add(fillLight);
const depthMaterial=new THREE.MeshBasicMaterial({
 colorWrite:false,depthWrite:true,depthTest:true,side:THREE.DoubleSide
});
const boxGeometry=new THREE.BoxGeometry(1,1,1);

try {
 renderer=new THREE.WebGLRenderer({alpha:true,antialias:true,powerPreference:'default'});
 renderer.setClearColor(0,0);
 renderer.outputColorSpace=THREE.SRGBColorSpace;
 renderer.toneMapping=THREE.ACESFilmicToneMapping;
 renderer.toneMappingExposure=1.12;
 renderer.domElement.id='rigged';
 renderer.domElement.setAttribute('aria-hidden','true');
 Object.assign(renderer.domElement.style,{
  position:'fixed',top:0,left:0,width:'100%',height:'100%',
  pointerEvents:'none',zIndex:1,display:'none'
 });
 originalCanvas.after(renderer.domElement);
}catch(error){
 label('GLB REVIEW UNAVAILABLE — LOCAL RIGS ACTIVE');
 console.warn('GLB review renderer failed',error);
}

function buildModel(actor){
 const source=sources.get(actor.kind);
 if(!source)throw new Error('Unknown prototype cast role: '+actor.kind);
 const subject=cloneSkeleton(source.scene);
 const root=new THREE.Group();root.add(subject);
 // GLB source and procedural field view both face +X at yaw zero.
 // Three.js rotates +X toward -Z for positive yaw, hence negative yaw.
 const bounds=new THREE.Box3().setFromObject(subject);
 const size=bounds.getSize(new THREE.Vector3());
 if(!Number.isFinite(size.y)||size.y<.25)
  throw new Error('Invalid local rig height: '+actor.kind);
 const scale=ROLE_HEIGHTS[actor.kind]/size.y;
 subject.scale.setScalar(scale);
 subject.position.y=-bounds.min.y*scale;
 subject.traverse(node=>{
  if(!node.isMesh)return;
  node.frustumCulled=false;
  if(Array.isArray(node.material))node.material=node.material.map(m=>m.clone());
  else if(node.material)node.material=node.material.clone();
 });
 const mixer=new THREE.AnimationMixer(subject),actions=new Map();
 for(const clip of source.animations)actions.set(clip.name,mixer.clipAction(clip));
 stage.add(root);
 const findBone=name=>{
  let result=null;
  subject.traverse(node=>{
   if(node.isBone&&node.name.toLowerCase()===name)result=node;
  });
  return result;
 };
 return {root,subject,mixer,actions,active:null,key:null,previous:null,
  kind:actor.kind,jaw:findBone('jaw'),brow:findBone('brow')};
}
function chooseAction(record,name){
 const next=record.actions.get(name)||record.actions.get('Idle');
 if(!next)return;
 if(record.key===name)return;
 const previous=record.active;
 next.enabled=true;
 next.reset();
 const looping=intent.looped(name);
 next.setLoop(looping?THREE.LoopRepeat:THREE.LoopOnce,looping?Infinity:1);
 next.clampWhenFinished=!looping;
 next.play();
 if(previous&&previous!==next)previous.crossFadeTo(next,.16,false);
 record.active=next;record.key=name;
}
function pose(record,actor,dt,time){
 record.root.position.set(actor.x,0,actor.z);
 record.root.rotation.y=-(Number.isFinite(actor.yaw)?actor.yaw:0);
 record.root.visible=actor.visible!==false;
 if(dt<=0)return; // Paused gameplay must not advance an acting loop.
 const moved=record.previous?
  Math.hypot(actor.x-record.previous[0],actor.z-record.previous[1]):0;
 record.previous=[actor.x,actor.z];
 // Teleports/checkpoint restore are never interpreted as a running stride.
 const speed=moved>4?0:moved/Math.max(dt,1/90);
 const name=intent.select(actor,speed,time);
 chooseAction(record,name);
 if(record.active){
  const pace=name==='Run'?4.7:name==='Walk'?2.45:1.0;
  const rate=name==='Run'||name==='Walk'?
   THREE.MathUtils.clamp(speed/pace,.65,1.55):1;
  record.active.setEffectiveTimeScale(rate);
 }
 record.mixer.update(Math.min(dt,.055));
 // Face motion is driven by an ACTUAL speaking actor's current dialogue,
 // not by source-blind looping expressions or invented voice clips.
 if(record.jaw&&actor.talking&&!actor.down)
  record.jaw.rotation.z-=.10*(.5+.5*Math.sin(time*15));
 if(record.brow&&actor.hit)record.brow.rotation.z-=.08;
}
function syncOccluders(solids){
 const keep=new Set();
 for(const o of solids){
  if(!o.active||(!o.solid&&!o.cameraSolid))continue;
  keep.add(o.id);
  let mesh=occluders.get(o.id);
  if(!mesh){mesh=new THREE.Mesh(boxGeometry,depthMaterial);
   mesh.renderOrder=-20;stage.add(mesh);occluders.set(o.id,mesh);}
  mesh.position.set(o.x,o.y,o.z);
  mesh.scale.set(o.w,o.h,o.d);
  mesh.rotation.set(0,-(o.yaw||0),0);
  mesh.visible=true;
 }
 for(const [key,mesh] of occluders)if(!keep.has(key))mesh.visible=false;
}
function renderFrame(frame){
 if(!renderer||!ready||!enabled||!frame?.camera)return;
 const dt=THREE.MathUtils.clamp(frame.dt??1/60,0,.06);
 const view=frame.camera;
 camera.fov=view.fov*180/Math.PI;
 camera.aspect=originalCanvas.clientWidth/Math.max(1,originalCanvas.clientHeight);
 camera.updateProjectionMatrix();
 camera.position.set(...view.eye);camera.lookAt(...view.target);
 renderer.setPixelRatio(Math.min(1.5,window.devicePixelRatio||1));
 renderer.setSize(originalCanvas.clientWidth,originalCanvas.clientHeight,false);
 syncOccluders(frame.solids||[]);
 const keep=new Set();
 for(const actor of frame.actors||[]){
  keep.add(actor.id);
  let record=actors.get(actor.id);
  if(!record){record=buildModel(actor);actors.set(actor.id,record);}
  pose(record,actor,dt,frame.time||0);
 }
 for(const [id,record] of actors)if(!keep.has(id))record.root.visible=false;
 renderer.render(stage,camera);
}
window.addEventListener('mg:rig-frame',evt=>{
 try{renderFrame(evt.detail)}
 catch(error){
  console.warn('GLB review failed: using original field renderer',error);
  ready=false;window.__MG_RIG_READY=false;
  if(renderer)renderer.domElement.style.display='none';
  label('GLB REVIEW ERROR — LOCAL RIGS ACTIVE');
 }
});
toggle?.addEventListener('change',()=>{
 enabled=toggle.checked;
 window.__MG_RIG_READY=enabled&&ready;
 if(renderer)renderer.domElement.style.display=enabled&&ready?'block':'none';
 label(enabled?(ready?'22-CAST SOURCE · 6 ACTIVE GLB ROLES':'GLB REVIEW LOADING'):
  'LOCAL ANIMATED FIELD RIGS ACTIVE');
});
async function loadRig(){
 if(!renderer)return;
 label('LOADING LOCAL CAST MODELS');
 try {
  const loader=new GLTFLoader();
  const loaded=await Promise.all(Object.entries(ROLE_FILES).map(async([kind,url])=>{
   const gltf=await loader.loadAsync(url);
   const bones=[];gltf.scene.traverse(node=>{if(node.isBone)bones.push(node.name);});
   if(bones.length!==18)throw new Error(kind+': expected 18 bones, got '+bones.length);
   const available=new Set((gltf.animations||[]).map(clip=>clip.name));
   const required=['Idle','Walk','Run','Crouch','Prone','Down','Hit'];
   if(kind==='fox')required.push('Captive');
   if(kind==='shoot_gunner')required.push('Address','Shoulder','Recoil','Search');
   for(const name of required)if(!available.has(name))
    throw new Error(kind+': missing source animation '+name);
   return [kind,gltf];
  }));
  for(const [kind,gltf] of loaded)sources.set(kind,gltf);
  ready=true;
  window.__MG_RIG_PRONE=true;window.__MG_RIG_CROUCH=true;
  window.__MG_RIG_READY=enabled;
  renderer.domElement.style.display=enabled?'block':'none';
  label(enabled?'SIX CAST-SPECIFIC GLB RIGS · ACTIVE':'LOCAL FIELD RIGS · GLB REVIEW READY');
 }catch(error){
  ready=false;window.__MG_RIG_READY=false;
  if(renderer)renderer.domElement.style.display='none';
  label('LOCAL RIGS ACTIVE — GLB REVIEW UNAVAILABLE');
  console.warn('Local cast GLB validation/loader failure',error);
 }
}
loadRig();
