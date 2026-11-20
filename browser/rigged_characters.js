/*
 * Optional, actual skeletal-mesh character rendering layer.
 * Three.js / GLTFLoader / SkeletonUtils stream via CDN.  No assets are copied
 * from Konami games. A failure returns to the complete offline WebGL game.
 *
 * Source GLB: 3d-fps soldier.glb @ pinned upstream commit (see ASSET_CREDITS).
 * The source file contains a genuine skinned rig with retargeted animation clips.
 * Requires online access; original standalone procedural characters remain offline.
 */
import * as THREE from 'three';
import {GLTFLoader} from 'three/addons/loaders/GLTFLoader.js';
import {clone as cloneSkeleton} from 'three/addons/utils/SkeletonUtils.js';

const CREDITS = 'assets/ASSET_CREDITS.md';
const MODEL_URL = 'https://raw.githubusercontent.com/cktang88/3d-fps/130ac2a5c856401ec36e7d35afcfefe40c5801f4/public/assets/models/characters/soldier.glb';
const status = document.getElementById('rigStatus');
const toggle = document.getElementById('rigToggle');
const originalCanvas = document.getElementById('gl');
const label = message => { if (status) status.textContent = message; };
let enabled = !!toggle?.checked;
let loaded = false;
let previousFrameTime = 0;
const actors = new Map();
const occluders = new Map();
const stage = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(61, 1, 0.075, 120);
const light = new THREE.HemisphereLight(0xcce9d9, 0x23302b, 2.0);
stage.add(light);
const keyLight = new THREE.DirectionalLight(0xf4f1ce, 2.15);
keyLight.position.set(-3.0, 8.5, 4.2);
stage.add(keyLight);
const fillLight = new THREE.DirectionalLight(0x63a9a1, 0.52);
fillLight.position.set(4.5, 3.4, -5.0);
stage.add(fillLight);

let renderer;
try {
  renderer = new THREE.WebGLRenderer({alpha:true, antialias:true, powerPreference:'default'});
  renderer.setClearColor(0x000000,0);
  renderer.outputColorSpace = THREE.SRGBColorSpace;
  renderer.toneMapping = THREE.ACESFilmicToneMapping;
  renderer.toneMappingExposure = 1.18;
  renderer.domElement.setAttribute('aria-hidden','true');
  renderer.domElement.id='rigged';
  Object.assign(renderer.domElement.style,{
    position:'fixed',top:0,left:0,width:'100%',height:'100%',pointerEvents:'none',
    zIndex:1,display:'none'
  });
  originalCanvas.after(renderer.domElement);
} catch(err) {
  label('RIG MODE UNAVAILABLE — ORIGINAL RENDERER ACTIVE');
  console.warn('Optional model renderer could not initialise',err);
}

const depthMaterial = new THREE.MeshBasicMaterial({
  colorWrite:false, depthWrite:true, depthTest:true,
  side:THREE.DoubleSide
});
const boxGeometry = new THREE.BoxGeometry(1,1,1);
let donor = null;
let sourceClips = [];
const clipAliases = {
  idle:['aim_idle','idle','mocap_idle','idle_unarmed','standing_idle'],
  walk:['mocap_walk_swat','walk','walking','walk_forward','mocap_walk'],
  run:['mocap_run','run','sprint','jog','mocap_jog'],
  crouch:['mocap_crouch','crouch_walk','crouch','crouched_walk','sneak'],
  prone:['crawl','prone','crawling','prone_idle'],
  hit:['hit','flinch','stagger','get_hit'],
  down:['death','die','death_front','fall']
};
const normalize = s=>s.toLowerCase().replace(/[^a-z0-9]+/g,'_');
function getClip(tag) {
  for(const candidate of clipAliases[tag]||[]) {
    let found = sourceClips.find(c=>normalize(c.name)===candidate);
    if(found)return found;
  }
  for(const candidate of clipAliases[tag]||[]) {
    let found = sourceClips.find(c=>normalize(c.name).includes(candidate));
    if(found)return found;
  }
  return null;
}
const stylePalette = {
  player: [0x83927e, 0x53695e],
  guard:  [0x9a9a83, 0x6d7466],
  fox:    [0xa49c84, 0x7a7167]
};

function buildModel(kind) {
  const root = new THREE.Group();
  const subject = cloneSkeleton(donor.scene);
  root.add(subject);
  const box = new THREE.Box3().setFromObject(subject);
  const size = new THREE.Vector3();box.getSize(size);
  const uniformScale = 1.76/Math.max(0.25,size.y);
  subject.scale.setScalar(uniformScale);
  subject.position.y -= box.min.y * uniformScale;
  const colors = stylePalette[kind] || stylePalette.guard;
  subject.traverse(node=> {
    if(!node.isMesh)return;
    node.frustumCulled = false;
    // Cloned materials must not leak colour edits into other characters.
    if(Array.isArray(node.material)) node.material=node.material.map(m=>m.clone());
    else node.material=node.material.clone();
    const decorate = mat => {
      // Tint is deliberately restrained: respect original PBR textures.
      const color = mat.color;
      if(color)color.lerp(new THREE.Color(colors[0]),kind==='fox'?0.28:0.14);
      if('roughness' in mat) mat.roughness = Math.max(mat.roughness ?? 0.55,0.55);
    };
    (Array.isArray(node.material)?node.material:[node.material]).forEach(decorate);
  });
  const mixer = new THREE.AnimationMixer(subject);
  const actions = new Map();
  for(const [key] of Object.entries(clipAliases)) {
    const clip=getClip(key);
    if(clip)actions.set(key,mixer.clipAction(clip));
  }
  if(!actions.has('idle') && sourceClips.length)actions.set('idle',mixer.clipAction(sourceClips[0]));
  root.rotation.y=Math.PI/2; // configurable: face the +X tactical-forward axis.
  // Optional upper-arm overlay for a low-cost wall bracing / knocking cue.
  // It is additive to source animation and intentionally avoids changing
  // locomotion or collision. The final hero rig needs authored montages.
  let braceArm=null,braceForearm=null;
  subject.traverse(node=> {
    if(!node.isBone)return;
    const name=String(node.name).toLowerCase().replace(/[^a-z]/g,'');
    if(name.endsWith('rightarm')&&!name.endsWith('forearm'))braceArm=node;
    if(name.endsWith('rightforearm'))braceForearm=node;
  });
  stage.add(root);
  return {root,subject,mixer,actions,active:null,key:null,position:null,kind,
    braceArm,braceForearm};
}

function switchAction(record,name) {
  if(record.key===name)return;
  const next=record.actions.get(name) || record.actions.get('walk') || record.actions.get('idle');
  if(!next) return;
  if(next===record.active){record.key=name;return;}
  next.enabled=true;
  next.setEffectiveWeight(1);
  next.setEffectiveTimeScale(1);
  next.reset().play();
  if(record.active)record.active.crossFadeTo(next,0.18,false);
  record.active=next;
  record.key=name;
}
function pose(record,world,dt) {
  record.root.position.set(world.x,0,world.z);
  record.root.rotation.y=-world.yaw+Math.PI/2;
  const move=record.position ?
    Math.hypot(world.x-record.position[0],world.z-record.position[1])/Math.max(dt,1/90):0;
  record.position=[world.x,world.z];
  const isDown=world.down;
  const posture=world.stance || 0;
  const action=isDown?'down':world.hit?'hit':posture===2?'prone':
    posture===1?'crouch':move>3.45?'run':move>.15?'walk':'idle';
  // Never render a standing rig while game mechanics say prone if the
  // downloaded animation set lacks a proper crawl/prone clip.
  const supported = !(posture===2 && !record.actions.has('prone')) &&
    !(posture===1 && !record.actions.has('crouch'));
  record.root.visible=world.visible && supported;
  switchAction(record,action);
  if(record.active)record.active.setEffectiveTimeScale(
    /walk|run|crouch/.test(action)?Math.max(.65,Math.min(1.35,move/(action==='run'?4.8:action==='walk'?2.6:1.4))):1);
  record.mixer.update(Math.min(dt,.055));
  const wallPose=world.kind==='player' && world.cover && posture!==2;
  // Modest lean reinforces contact without stealing the camera or snapping
  // the entire animated skeleton into a mechanically impossible pose.
  record.subject.rotation.z=THREE.MathUtils.damp(
    record.subject.rotation.z,wallPose?0.055:0,12,Math.min(dt,.055));
  if(wallPose&&record.braceArm){
    record.braceArm.rotation.z+=0.16;
    record.braceArm.rotation.x-=world.knocking?0.44:0.22;
    if(record.braceForearm)record.braceForearm.rotation.x-=world.knocking?0.22:0.12;
  }
  return supported;
}

function syncOccluders(solids) {
  const keep = new Set();
  for(const o of solids) {
    if(!o.active)continue;
    // Mirror ONLY genuine solid scene obstacles (and roof). The depth-only
    // mask prevents the overlay drawing rigged bodies through real walls.
    if(!o.solid && !o.cameraSolid)continue;
    const key=o.id;
    keep.add(key);
    let mesh=occluders.get(key);
    if(!mesh) {
      mesh=new THREE.Mesh(boxGeometry,depthMaterial);
      mesh.renderOrder=-20;
      stage.add(mesh);
      occluders.set(key,mesh);
    }
    mesh.position.set(o.x,o.y,o.z);
    mesh.scale.set(o.w,o.h,o.d);
    mesh.rotation.set(0,-(o.yaw||0),0);
    mesh.visible=true;
  }
  for(const [key,m] of occluders)if(!keep.has(key))m.visible=false;
}

function renderFrame(frame) {
  if(!renderer || !loaded || !enabled || !frame?.camera)return;
  const dt=Math.min(.06,Math.max(0,frame.dt||1/60));
  const desired=frame.camera;
  camera.fov=desired.fov*180/Math.PI;
  camera.aspect=originalCanvas.clientWidth/Math.max(1,originalCanvas.clientHeight);
  camera.near=0.075;
  camera.far=120;
  camera.updateProjectionMatrix();
  camera.position.set(...desired.eye);
  camera.lookAt(...desired.target);
  const w=originalCanvas.clientWidth,h=originalCanvas.clientHeight;
  const scale=Math.min(1.5,window.devicePixelRatio || 1);
  renderer.setPixelRatio(scale);
  renderer.setSize(w,h,false);
  syncOccluders(frame.solids||[]);
  const visibleKeys=new Set();
  for(const actor of frame.actors||[]) {
    visibleKeys.add(actor.id);
    let rig=actors.get(actor.id);
    if(!rig){rig=buildModel(actor.kind);actors.set(actor.id,rig);}
    const supported=pose(rig,actor,dt);
    actor._rigSupported=supported;
  }
  for(const [key,rig] of actors)if(!visibleKeys.has(key))rig.root.visible=false;
  renderer.render(stage,camera);
}

window.addEventListener('mg:rig-frame',evt=>renderFrame(evt.detail));

toggle?.addEventListener('change',()=>{
  enabled=toggle.checked;
  window.__MG_RIG_READY=enabled&&loaded;
  if(renderer)renderer.domElement.style.display=enabled&&loaded?'block':'none';
  label(enabled?(loaded?'RIGGED CHARACTERS · ACTIVE':'RIGGED CHARACTERS · LOADING'):
    'PROCEDURAL CHARACTERS · ACTIVE');
});

async function loadRig() {
  if(!renderer)return;
  label('RIGGED CHARACTERS · DOWNLOADING MODEL');
  try {
    donor=await new GLTFLoader().loadAsync(MODEL_URL);
    sourceClips=donor.animations || [];
    const bones=[];
    donor.scene.traverse(o=> {if(o.isBone)bones.push(o);});
    if(bones.length<8 || sourceClips.length<2)
      throw new Error(`Model needs a substantial skeleton and at least two clips: ${bones.length} bones, ${sourceClips.length} clips`);
    loaded=true;
    window.__MG_RIG_PRONE=!!getClip('prone');
    window.__MG_RIG_CROUCH=!!getClip('crouch');
    window.__MG_RIG_READY=enabled;
    renderer.domElement.style.display=enabled?'block':'none';
    label(`RIGGED 3D · ${bones.length} BONES / ${sourceClips.length} CLIPS`);
    console.info('Rigged model ready:',bones.length,'bones,',sourceClips.map(c=>c.name));
  } catch(err) {
    console.warn('Rigged character asset unavailable; using full procedural fallback',err);
    label('OFFLINE / RIG UNAVAILABLE · ORIGINAL CHARACTERS');
    window.__MG_RIG_READY=false;
  }
}
loadRig();
