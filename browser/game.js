/* Gray Fox Rescue: self-contained, original procedural browser proof-of-play.
   Native WebGL1, keyboard/mouse and Xbox-standard Gamepad API. No CDN/assets.
   Rooms/lines are provisional, not verified J2ME maps or transcribed dialogue. */
(()=>{'use strict';
const $=id=>document.getElementById(id);
const clamp=(v,a,b)=>Math.max(a,Math.min(b,v));
const lerp=(a,b,t)=>a+(b-a)*clamp(t,0,1);
const hypot=(x,z)=>Math.hypot(x,z);
const dist=(a,b)=>Math.hypot(a.x-b.x,a.z-b.z);
const nrm=(x,z)=>{let l=hypot(x,z)||1;return {x:x/l,z:z/l}};
const dx=(a,b)=>a.x-b.x;
const key={};let started=false, paused=false, showIntro=true, last=performance.now(), keysPressed=new Set(), pressedMouse=false;
const canvas=$('gl');const gl=canvas.getContext('webgl',{antialias:true,powerPreference:'high-performance',alpha:false});
if(!gl){$('support').textContent='WebGL is disabled. Enable hardware acceleration (or software WebGL) and try desktop Chrome/Edge/Firefox.';return;}
const V=`attribute vec3 aPos;attribute vec3 aN;uniform mat4 uPV;uniform vec3 uPos;uniform vec3 uScale;uniform float uYaw;uniform float uTilt;varying vec3 vLocal;varying vec3 vWorld;varying vec3 vNormal;void main(){vLocal=aPos;float c=cos(uYaw),s=sin(uYaw),ct=cos(uTilt),st=sin(uTilt);vec3 v=aPos*uScale;vec3 vt=vec3(ct*v.x+st*v.y,-st*v.x+ct*v.y,v.z);vec3 w=vec3(c*vt.x-s*vt.z,vt.y,s*vt.x+c*vt.z)+uPos;vWorld=w;vec3 n=normalize(aN/max(uScale,vec3(.00001)));vec3 nt=vec3(ct*n.x+st*n.y,-st*n.x+ct*n.y,n.z);vNormal=normalize(vec3(c*nt.x-s*nt.z,nt.y,s*nt.x+c*nt.z));gl_Position=uPV*vec4(w,1.0);}`;
const F=`#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
uniform vec3 uTint;uniform int uMaterial;uniform vec3 uEye;uniform float uTime;uniform sampler2D uAtlas;uniform sampler2D uFloorShade;uniform sampler2D uPlates;varying vec3 vLocal;varying vec3 vWorld;varying vec3 vNormal;float hash(vec3 p){return fract(sin(mod(dot(mod(p,32.),vec3(12.71,31.17,7.47)),6.28318))*1375.5453);}void main(){if(uMaterial==5){float r=length(vLocal.xz)*2.;gl_FragColor=vec4(.025,.034,.03,.59*(1.-smoothstep(.12,1.,r)));return;}vec3 n=normalize(vNormal);float ld=max(0.,dot(n,normalize(vec3(-0.6,1.4,0.5))));float light=.36+.57*ld;float point=1.2/(1.+length(vWorld-vec3(2.,2.7,-3.2))*.14);light*=.65+.35*point;vec3 base=uTint*light;
// Local fixture pools separate the yard, damp cells and the warmer exit.
vec3 lamp=vec3(0.);
for(int i=0;i<6;i++){
 vec3 centre=i==0?vec3(-25.,2.9,.3):i==1?vec3(-7.,3.,0.):i==2?vec3(1.,3.,.4):i==3?vec3(27.5,3.,.3):i==4?vec3(17.,3.02,.3):vec3(22.,3.02,.3);
 vec3 ray=centre-vWorld;
 float glow=max(.14,dot(n,normalize(ray)))/(1.+.30*dot(ray,ray));
 lamp+=glow*(i==1||i==5?vec3(.49,.83,1.03):vec3(1.65,1.10,.55));
}
base+=uTint*lamp;
// Four distinct original 256px surface tiles: concrete / steel / fabric / floor.
vec3 normalAbs=abs(n);vec2 projection=(normalAbs.y>normalAbs.x&&normalAbs.y>normalAbs.z)?vWorld.xz:((normalAbs.x>normalAbs.z)?vWorld.zy:vWorld.xy);
vec2 surface=fract(projection*(uMaterial==4?1.6:0.55));
vec2 corner=(uMaterial==2?vec2(.5,0.):uMaterial==4?vec2(0.,.5):uMaterial==1?vec2(.5,.5):vec2(0.,0.));
vec3 texel=texture2D(uAtlas,corner+surface*.48+vec2(.01)).rgb;
if(uMaterial!=3)base*=mix(vec3(1.),texel*1.55,.36);
if(uMaterial==2){float edge=pow(1.-max(dot(n,normalize(vec3(.35,.82,.3))),0.),2.);base+=vec3(.16,.23,.22)*edge*.35;}
if(uMaterial==4){base*=.96+.035*sin(projection.x*94.)*sin(projection.y*94.);}

// Large slab joints stay legible without the repeated high-frequency floor grid.
if(uMaterial==1){
 vec2 seam=abs(fract((vWorld.xz+vec2(.42,.21))/vec2(3.6,2.65))-.5);
 float joint=smoothstep(.494,.499,max(seam.x,seam.y));
 float occlusion=texture2D(uFloorShade,clamp((vWorld.xz+vec2(33.,5.5))/vec2(63.,11.),0.,1.)).r;
 base*=mix(.38,1.0,occlusion)*(1.-joint*.18);
 vec3 h=normalize(normalize(uEye-vWorld)+normalize(vec3(.15,1.,-.28)));
 base+=vec3(.10,.105,.093)*pow(max(dot(n,h),0.),22.)*occlusion*texel.r;
}else if(uMaterial==2){
 vec3 h=normalize(normalize(uEye-vWorld)+normalize(vec3(-.55,.85,.22)));
 base+=vec3(.22,.20,.15)*pow(max(dot(n,h),0.),26.);
 base*=.86+.22*texel.r;
}else if(uMaterial==0){
 base*=.87+.18*texel.g;
}
if(uMaterial==6||uMaterial==7){
 vec2 plateUV=uMaterial==6?vec2(vLocal.z+.5,.5-vLocal.y):vec2(vLocal.x+.5,.5-vLocal.y);
 plateUV.y=(clamp(plateUV.y,.015,.985)+(uMaterial==7?1.:0.))*.5;
 float ink=texture2D(uPlates,plateUV).r;
 base=mix(vec3(.063,.085,.08),vec3(.82,.79,.65),ink)*(.61+.4*ld)+lamp*.15;
}
if(uMaterial==3)base+=vec3(.32,.30,.19);
float fog=clamp((length(vWorld-uEye)-12.)/55.,0.,.52);
base=mix(base,vec3(.055,.083,.091),fog);
gl_FragColor=vec4(pow(max(base,vec3(0.)),vec3(.89)),1.);}`;
function shader(src,type){const s=gl.createShader(type);gl.shaderSource(s,src);gl.compileShader(s);if(!gl.getShaderParameter(s,gl.COMPILE_STATUS))throw Error(gl.getShaderInfoLog(s));return s;}
const program=gl.createProgram();gl.attachShader(program,shader(V,gl.VERTEX_SHADER));gl.attachShader(program,shader(F,gl.FRAGMENT_SHADER));gl.linkProgram(program);if(!gl.getProgramParameter(program,gl.LINK_STATUS))throw Error(gl.getProgramInfoLog(program));gl.useProgram(program);
const attr=gl.getAttribLocation(program,'aPos'),nor=gl.getAttribLocation(program,'aN');const loc={pv:gl.getUniformLocation(program,'uPV'),pos:gl.getUniformLocation(program,'uPos'),scale:gl.getUniformLocation(program,'uScale'),yaw:gl.getUniformLocation(program,'uYaw'),tilt:gl.getUniformLocation(program,'uTilt'),tint:gl.getUniformLocation(program,'uTint'),mat:gl.getUniformLocation(program,'uMaterial'),eye:gl.getUniformLocation(program,'uEye'),time:gl.getUniformLocation(program,'uTime')};
// Original small material atlas, generated once at startup. WebGL1 and file:// safe:
// no CDN, external cross-origin image or 4K memory cost. 512x512 RGBA (~1 MB).
const materialAtlas={resolution:512,tileSize:256,tiles:['painted concrete','worn gunmetal','woven blanket','painted floor']};
function buildMaterialAtlas(){
 const size=materialAtlas.resolution,half=size/2,data=new Uint8Array(size*size*4);
 function seed(x,y,k){let n=(x*374761393+y*668265263+k*1442695041)|0;n=(n^(n>>>13))*1274126177|0;return ((n^(n>>>16))>>>0)/4294967295}
 function valueNoise(x,y,k){const ix=Math.floor(x),iy=Math.floor(y),u=x-ix,v=y-iy;
  const smooth=t=>t*t*(3-2*t),a=smooth(u),b=smooth(v);
  const v00=seed(ix,iy,k),v10=seed(ix+1,iy,k),v01=seed(ix,iy+1,k),v11=seed(ix+1,iy+1,k);
  return (v00*(1-a)+v10*a)*(1-b)+(v01*(1-a)+v11*a)*b;
 }
 for(let y=0;y<size;y++)for(let x=0;x<size;x++){
  const tile=(x>=half?1:0)+(y>=half?2:0),u=x%half,v=y%half;
  const big=valueNoise(u/34,v/34,tile),medium=valueNoise(u/9,v/9,tile+5),fine=seed(u,v,tile+9);
  let gray=.78+big*.26+medium*.11+(fine-.5)*.12;
  let rgb;
  if(tile===0){ // chipped, stained concrete
   const chip=seed(Math.floor(u/4),Math.floor(v/4),4)>.952;
   gray*=chip?.61:1.;gray*=(.94+.11*Math.sin(u*.023+big));
   rgb=[.61*gray,.66*gray,.63*gray];
  }else if(tile===1){ // blue-green oxidised painted steel, vertical brush scratches
   const scrub=seed(Math.floor(u/2),Math.floor(v/11),31)>.94;
   const rust=valueNoise(u/12,v/14,24)>.78;
   gray*=scrub?1.28:.93;
   rgb=rust?[.55*gray,.37*gray,.25*gray]:[.45*gray,.62*gray,.61*gray];
  }else if(tile===2){ // coarse textile weave and wear
   const weave=(Math.sin(u*1.8)*Math.cos(v*1.8))*.065;
   gray*=.98+weave;rgb=[.84*gray,.83*gray,.76*gray];
  }else{ // grimy floor, distinct from vertical concrete
   const crack=.94+.06*medium;
   gray*=crack*(.79+.24*valueNoise(u/16,v/16,40));
   rgb=[.55*gray,.58*gray,.55*gray];
  }
  const idx=(y*size+x)*4;
  for(let j=0;j<3;j++)data[idx+j]=Math.max(0,Math.min(255,Math.round(rgb[j]*255)));
  data[idx+3]=255;
 }
 const texture=gl.createTexture();gl.activeTexture(gl.TEXTURE0);gl.bindTexture(gl.TEXTURE_2D,texture);
 gl.texImage2D(gl.TEXTURE_2D,0,gl.RGBA,size,size,0,gl.RGBA,gl.UNSIGNED_BYTE,data);
 gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,gl.CLAMP_TO_EDGE);
 gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,gl.CLAMP_TO_EDGE);
 gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.LINEAR_MIPMAP_LINEAR);
 gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.LINEAR);
 gl.generateMipmap(gl.TEXTURE_2D);gl.uniform1i(gl.getUniformLocation(program,'uAtlas'),0);
 materialAtlas.initialized=true;
 return texture;
}
const activeMaterialAtlas=buildMaterialAtlas();
function uploadPlates(){
 const pixels=new Uint8Array(256*128);let offset=0;
 const runs=window.freightPlateRuns||[];
 for(let i=0;i<runs.length;i+=2){pixels.fill(runs[i+1],offset,offset+runs[i]);offset+=runs[i];}
 gl.activeTexture(gl.TEXTURE2);gl.bindTexture(gl.TEXTURE_2D,gl.createTexture());
 gl.texImage2D(gl.TEXTURE_2D,0,gl.LUMINANCE,256,128,0,gl.LUMINANCE,gl.UNSIGNED_BYTE,pixels);
 gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.LINEAR_MIPMAP_LINEAR);
 gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.LINEAR);gl.generateMipmap(gl.TEXTURE_2D);
 gl.uniform1i(gl.getUniformLocation(program,'uPlates'),2);gl.activeTexture(gl.TEXTURE0);
}
uploadPlates();
// Single unit-cube mesh, 36 vertices; scene and characters share all draw code.
let verts=[],normals=[];const face=(pts,n)=>{const ix=[0,1,2,0,2,3];for(const i of ix){verts.push(...pts[i]);normals.push(...n)}};
face([[.5,-.5,-.5],[.5,-.5,.5],[.5,.5,.5],[.5,.5,-.5]],[1,0,0]);
face([[-.5,-.5,.5],[-.5,-.5,-.5],[-.5,.5,-.5],[-.5,.5,.5]],[-1,0,0]);
face([[-.5,.5,-.5],[.5,.5,-.5],[.5,.5,.5],[-.5,.5,.5]],[0,1,0]);
face([[-.5,-.5,.5],[.5,-.5,.5],[.5,-.5,-.5],[-.5,-.5,-.5]],[0,-1,0]);
face([[-.5,-.5,.5],[-.5,.5,.5],[.5,.5,.5],[.5,-.5,.5]],[0,0,1]);
face([[.5,-.5,-.5],[.5,.5,-.5],[-.5,.5,-.5],[-.5,-.5,-.5]],[0,0,-1]);
// Three original mesh shapes. Geometry is generated at runtime (no third-party IP).
const meshLibrary={};
function makeMesh(id,pos,norm){
 const vb=gl.createBuffer(),nb=gl.createBuffer();
 gl.bindBuffer(gl.ARRAY_BUFFER,vb);gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(pos),gl.STATIC_DRAW);
 gl.bindBuffer(gl.ARRAY_BUFFER,nb);gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(norm),gl.STATIC_DRAW);
 meshLibrary[id]={vb,nb,count:pos.length/3};
}
makeMesh('cube',verts,normals);
makeMesh('shadowPlane',[-.5,0,-.5,-.5,0,.5,.5,0,.5,-.5,0,-.5,.5,0,.5,.5,0,-.5],Array.from({length:18},(_,i)=>i%3===1?1:0));
{
 const p=[],n=[],lat=10,lon=14;
 function sph(a,b){const v=[Math.sin(a)*Math.cos(b),Math.cos(a),Math.sin(a)*Math.sin(b)];p.push(...v.map(x=>x*.5));n.push(...v)}
 for(let i=0;i<lat;i++)for(let j=0;j<lon;j++){
  const a=i*Math.PI/lat,c=(i+1)*Math.PI/lat,b=j*2*Math.PI/lon,d=(j+1)*2*Math.PI/lon;
  sph(a,b);sph(c,b);sph(c,d);sph(a,b);sph(c,d);sph(a,d);
 }
 makeMesh('sphere',p,n);
}
{
 const p=[],n=[],segments=12;
 function vert(x,y,z,nx,ny,nz){p.push(x,y,z);n.push(nx,ny,nz)}
 for(let i=0;i<segments;i++){
  const a=i*2*Math.PI/segments,b=(i+1)*2*Math.PI/segments;
  const ax=.5*Math.cos(a),az=.5*Math.sin(a),bx=.5*Math.cos(b),bz=.5*Math.sin(b);
  vert(ax,-.5,az,Math.cos(a),0,Math.sin(a));vert(bx,-.5,bz,Math.cos(b),0,Math.sin(b));vert(bx,.5,bz,Math.cos(b),0,Math.sin(b));
  vert(ax,-.5,az,Math.cos(a),0,Math.sin(a));vert(bx,.5,bz,Math.cos(b),0,Math.sin(b));vert(ax,.5,az,Math.cos(a),0,Math.sin(a));
  for(const y of [-.5,.5]){const ny=y<0?-1:1;vert(0,y,0,0,ny,0);vert(ax,y,az,0,ny,0);vert(bx,y,bz,0,ny,0)}
 }
 makeMesh('cylinder',p,n);
}
{
 // Shared rounded-chamfer cuboid: twelve edge transitions and eight rounded
 // corners, smooth normals, authored by code and instanced on hero props.
 const p=[],n=[],edges=[-.5,-.46,-.40,0,.40,.46,.5];
 function vertex(a){
  const radius=.10,inside=a.map(v=>clamp(v,-.5+radius,.5-radius));
  const dv=a.map((v,i)=>v-inside[i]);const ll=Math.hypot(...dv)||1;
  const normal=dv.map(v=>v/ll);
  const v=inside.map((c,i)=>c+normal[i]*radius);
  p.push(...v);n.push(...normal);
 }
 function faceAxis(axis,sgn){
  const u=(axis+1)%3,v=(axis+2)%3;
  for(let j=0;j<edges.length-1;j++)for(let i=0;i<edges.length-1;i++){
   function pt(k,l){const out=[0,0,0];out[axis]=sgn*.5;out[u]=edges[k];out[v]=edges[l];return out}
   const q=[pt(i,j),pt(i+1,j),pt(i+1,j+1),pt(i,j+1)];
   const winding=sgn>0?[0,1,2,0,2,3]:[0,2,1,0,3,2];
   for(const ix of winding)vertex(q[ix]);
  }
 }
 for(let a=0;a<3;a++){faceAxis(a,-1);faceAxis(a,1)}
 makeMesh('bevel',p,n);
}
let currentMesh='';
function useMesh(id){if(currentMesh===id)return;currentMesh=id;const m=meshLibrary[id];
 gl.bindBuffer(gl.ARRAY_BUFFER,m.vb);gl.enableVertexAttribArray(attr);gl.vertexAttribPointer(attr,3,gl.FLOAT,false,0,0);
 gl.bindBuffer(gl.ARRAY_BUFFER,m.nb);gl.enableVertexAttribArray(nor);gl.vertexAttribPointer(nor,3,gl.FLOAT,false,0,0);
}
gl.enable(gl.DEPTH_TEST);gl.depthFunc(gl.LEQUAL);gl.clearColor(.023,.058,.066,1);
const m4={persp:(fov,asp,n,f)=>{const q=1/Math.tan(fov/2),nf=1/(n-f);return [q/asp,0,0,0,0,q,0,0,0,0,(f+n)*nf,-1,0,0,2*f*n*nf,0]},look:(eye,center)=>{const z=unit3(sub3(eye,center)),x=unit3(cross([0,1,0],z)),y=cross(z,x);return [x[0],y[0],z[0],0,x[1],y[1],z[1],0,x[2],y[2],z[2],0,-dot3(x,eye),-dot3(y,eye),-dot3(z,eye),1]},mul:(a,b)=>{let o=new Array(16).fill(0);for(let c=0;c<4;c++)for(let r=0;r<4;r++)for(let k=0;k<4;k++)o[c*4+r]+=a[k*4+r]*b[c*4+k];return o}};
function sub3(a,b){return [a[0]-b[0],a[1]-b[1],a[2]-b[2]]}function dot3(a,b){return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]}function cross(a,b){return [a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]]}function unit3(a){const l=Math.hypot(...a)||1;return a.map(x=>x/l)}
const mat={concrete:0,grate:1,steel:2,light:3,fabric:4};const color={floor:[.27,.34,.31],wall:[.32,.38,.35],stripe:[.50,.48,.22],roof:[.23,.32,.31],metal:[.26,.36,.36],rust:[.36,.24,.17],bed:[.34,.30,.25],cloth:[.43,.46,.40],green:[.21,.34,.29],dark:[.13,.20,.23],skin:[.54,.41,.33],light:[.76,.80,.50],prisoner:[.49,.48,.34],guard:[.28,.37,.27],bloodless:[.54,.53,.39]};
const geometry=[];const obstacles=[];const add=(name,x,y,z,w,h,d,clr,material=0,collision=false)=>{const o={name,x,y,z,w,h,d,color:clr,material,active:true,mesh:"cube",yaw:0,tilt:0,detail:false};geometry.push(o);if(collision)obstacles.push(o);return o};
const wall=(name,x,z,w,d)=>add(name,x,1.6,z,w,3.25,d,color.wall,mat.concrete,true);
add('concrete floor',1.1,-.26,0,27.7,.5,10.4,color.floor,mat.grate,true);
const cameraOnlySolids=[add('ceiling',1.1,3.5,0,27.7,.28,10.4,color.roof,mat.concrete,false)];
wall('north corridor',1.1,5.15,27.6,.28);wall('south corridor',1.1,-5.15,27.6,.28);wall('west boundary',-12.6,0,.28,10.4);for (const z of [-3.4,3.4]) wall('arena entrance frame',14.6,z,.28,3.5);
const propertyGate=wall('arena entrance shutter',14.6,0,.28,3.4);
const finalGate=wall('east exit shutter',27.3,0,.24,3.2);
for (const z of [-3.4,3.4]) wall('exit masonry',27.3,z,.28,3.6);
for (const [centre,width] of [[-22.6,20],[22,15]]) {
 add('concrete floor',centre,-.26,0,width,.5,10.4,color.floor,mat.grate);
 for(const z of [-5.15,5.15])wall('perimeter boundary',centre,z,width,.28);
 cameraOnlySolids.push(add('ceiling',centre,3.5,0,width,.28,10.4,color.roof));
}
wall('yard boundary',-32.6,0,.28,10.4);
wall('exit boundary',29.5,0,.28,10.4);
for(const [x,z] of [[-21.4,.7],[18.4,-.9],[21.1,2.2],[23.3,-2.5]]){
 const height=x===21.1?1.12:1.90;
 add('cast steel cover',x,height/2,z,1.05,height,1.65,[.24,.32,.31],mat.steel,true).mesh='bevel';
 add('painted cap',x,height+.02,z,1.1,.065,1.7,color.stripe,mat.steel);
}
for(const x of [-25,-14.7,17,22,27.5]){
 add('fixture hood',x,3.21,.3,1.1,.16,.48,color.metal,mat.steel);
 add('fixture diffuser',x,3.11,.3,.91,.05,.33,[.91,.73,.40],mat.light);
 for(const z of [-4.9,4.9])add('steel jamb',x,1.52,z,.26,3.05,.33,color.metal,mat.steel);
}
const informant={x:-24.2,z:1.42,yaw:-2.7,stance:1,alive:true}, post={x:-14.3,z:.4};
const escortMode=new URLSearchParams(location.search).has('escort');

for(const x of [-4.1]){wall('partition north '+x,x,3.35,.28,3.62);wall('partition south '+x,x,-3.35,.28,3.62)}
const breach1=wall('damaged west divider',-4.1,0,.28,3.0),breach2=wall('damaged service divider',2,-2.5,3,.28);
wall('service partition',-1.8,-2.5,4.6,.28);
wall('service partition',4,-2.5,1,.28);
wall('east partition',4.5,1.35,.28,7.65);
// Readable damage seams and warm inspection lighting. These disappear with their wall.
const seam1=add('fracture west',-4.26,1.4,.2,.06,1.5,.13,color.rust),seam2=add('fracture service',2,1.4,-2.34,.13,1.5,.06,color.rust);
// Industrial prison cot: welded frame, slats, four legs, bedding, seams, headboard.
// Only the core cot is an obstacle; detail is purely visual and cannot block
// the camera, guard LOS, melee or a doorway. No new gameplay hazards.
const sceneDetail=[];
const detail=(name,x,y,z,w,h,d,clr,material=2,mesh='cube',yaw=0,tilt=0)=>{
 const o=add(name,x,y,z,w,h,d,clr,material,false);
 o.mesh=mesh;o.yaw=yaw;o.tilt=tilt;o.detail=true;sceneDetail.push(o);return o;
};
detail('property hall sign',20.75,2.72,-4.87,1.82,.48,.038,color.metal,7);
for(const x of [17.75,21.55,25.35]){
 detail('service conduit',x,2.98,-4.75,3.75,.052,.057,color.metal,mat.steel,'bevel');
 detail('hanger clamp',x+.82,2.98,-4.74,.055,.091,.08,[.43,.42,.36],mat.steel);
}
// Property returns: numbered bins, a cage window and a worn trolley route.
for(let bay=0;bay<3;bay++){
 const x=18.2+bay*2.55;
 detail('property cage back',x,1.38,-4.93,2.34,2.26,.08,color.dark,mat.steel);
 for(const y of [.34,1.16,2.34])detail('shelf crosspiece',x,y,-4.75,2.32,.065,.29,color.metal,mat.steel);
 for(const dx of [-1.12,-.56,0,.56,1.12])detail('cage upright',x+dx,1.36,-4.57,.036,2.1,.04,color.metal,mat.steel);
 for(let bin=0;bin<3;bin++){
  detail('sealed effects bin',x-.70+bin*.69,.73,-4.76,.60,.62,.29,
   bin===bay?[.42,.33,.23]:[.28,.34,.30],mat.steel,'bevel');
 }
 detail('enamel bay plate',x,2.60,-4.90,.56,.28,.035,[.20,.29,.29],mat.steel);
 for(let mark=0;mark<=bay;mark++)detail('bay index',x+(mark-bay/2)*.085,2.60,-4.875,.035,.14,.018,[.86,.77,.55],mat.fabric);
}
// Dress the actual collision volumes; the belts and casters identify the
// same safe faces that interrupt sight during the encounter.
for(const trunk of obstacles.filter(o=>o.name==='cast steel cover'&&o.x>15)){
 for(const side of [-1,1]){
  const z=trunk.z+side*.48;
  detail('freight band',trunk.x,trunk.h+.035,z,trunk.w+.055,.055,.105,[.56,.48,.32],mat.steel);
  detail('freight side strap',trunk.x-.54,trunk.h*.52,z,.045,trunk.h*.90,.095,[.49,.44,.31],mat.steel);
  detail('strap clasp',trunk.x-.568,trunk.h*.48,z,.042,.15,.14,color.metal,mat.steel,'bevel');
 }
 detail('return tag',trunk.x-.548,trunk.h*.70,trunk.z,.023,.19,.71,[.68,.64,.48],6);
}
for(const x of [16.4,18.6,20.8,23,25.2]){
 for(const z of [-1.85,-1.44])detail('trolley wheel wear',x,.006,z,1.72,.012,.042,[.36,.37,.31],mat.grate);
}
for(const z of [-2.65,2.65]){
 detail('intake lane lamp',15.02,2.55,z,.15,.38,.22,[.81,.52,.24],mat.light,'bevel');
}
const bedFrames=[];
for(const [x,z] of [[-9.3,3.2],[.1,3.2]]){
 bedFrames.push({x,z});
 add('cot',x,.35,z,2,.35,.95,[.15,.21,.20],mat.steel,true);
 detail('cot mattress padded edge',x,.57,z,1.84,.19,.88,[.60,.63,.55],mat.fabric,'bevel');
 detail('cot mattress top',x,.67,z,1.72,.06,.81,[.63,.66,.58],mat.fabric,'bevel');
 // Steel angle rails and sprung steel cross-bars.
 for(const sz of [-.45,.45]){
  detail('cot tubular side rail',x,.38,z+sz,2.05,.09,.075,[.33,.41,.40],mat.steel,'bevel');
  for(const ex of [-.83,.83]){
   detail('cot support foot',x+ex,.20,z+sz,.11,.42,.10,[.24,.31,.31],mat.steel,'bevel');
   detail('cot rubber foot',x+ex,.04,z+sz,.13,.08,.13,[.10,.12,.12],mat.steel,'bevel');
   detail('cot rivet head',x+ex,.38,z+sz*1.09,.045,.045,.025,[.66,.69,.62],mat.steel,'bevel');
  }
 }
 for(let i=0;i<9;i++){
  const sx=x-.79+i*.198;
  detail('cot spring slat',sx,.45,z,.045,.06,.83,[.24,.31,.30],mat.steel);
 }
 // Head and foot guard are two rails with visible round uprights.
 for(const ex of [-.96,.96]){
  for(const sz of [-.40,.40])detail('cot headboard post',x+ex,.57,z+sz,.075,.82,.075,[.27,.35,.33],mat.steel,'bevel');
  detail('cot headboard top',x+ex,.95,z,.08,.065,.88,[.38,.42,.40],mat.steel,'bevel');
 }
 // Cloth sheet folds and off-white pillow: local form, not flat boxes.
 detail('folded wool blanket',x+.43,.725,z, .63,.095,.78,[.43,.48,.40],mat.fabric,'bevel');
 for(let k=0;k<3;k++)detail('blanket fold ridge',x+.18+k*.16,.785,z, .055,.016,.73,[.32,.39,.32],mat.fabric,'bevel');
 detail('pillow bulged',x-.70,.755,z,.40,.15,.65,[.77,.74,.64],mat.fabric,'bevel');
 detail('pillow seam',x-.70,.825,z,.36,.012,.53,[.54,.54,.46],mat.fabric);
 detail('loose sheet fold',x+.09,.714,z,.08,.018,.78,[.51,.53,.47],mat.fabric,'bevel');
}

for(let x=-11;x<14;x+=3.7){add('cable',x,3.18,-4.96,2.4,.06,.10,color.dark,2);add('ceiling tube',x,3.27,-.8,2.3,.05,.06,color.metal,2)}
for(let x=-10;x<14;x+=5.2){add('fluorescent',x,3.28,0,1.6,.08,.14,color.light,3);add('fixture',x,3.24,0,1.8,.07,.19,color.metal,2)}
for(const x of [-10.6,-6.3,-2.3,1.3,6.7,12.7]){add('steel conduit',x,2.0,5.0,.10,2.8,.09,color.metal,2);add('vent box',x,2.72,4.83,1.0,.38,.27,color.dark,2)}
for(const [x,z] of [[-10,-3.9],[.8,-4],[7.1,-2.8],[11,2.8]]){
 add('locker',x,1.0,z,.9,2.0,.65,color.metal,2,true);
 const front=z-.335;
 detail('locker inset door',x,1.05,front,.76,1.83,.032,[.28,.35,.33],mat.steel,'bevel');
 detail('locker door hinge edge',x-.35,1.05,front-.025,.035,1.79,.04,[.11,.18,.18],mat.steel);
 detail('locker latch bezel',x+.27,1.20,front-.054,.095,.31,.035,[.49,.54,.50],mat.steel,'bevel');
 detail('locker recessed handle',x+.27,1.19,front-.079,.022,.23,.017,[.12,.18,.17],mat.steel,'bevel');
 detail('locker asset number plate',x,1.66,front-.055,.30,.13,.025,[.61,.62,.50],mat.steel,'bevel');
 for(let i=0;i<6;i++){
  detail('locker ventilation louver',x,1.49-i*.07,front-.054,.42,.018,.018,[.12,.20,.20],mat.steel,'bevel');
 }
 for(const yy of [.38,1.7])detail('locker hinge',x-.35,yy,front-.075,.05,.13,.06,[.55,.59,.54],mat.steel,'bevel');
 for(const xx of [-.3,.3])detail('locker foot',x+xx,.08,z,.075,.16,.48,[.13,.18,.19],mat.steel,'bevel');
}

for(const [x,z] of [[7.8,1.9],[10.6,-2.4]]){add('crate',x,.72,z,1.55,1.4,1.35,color.rust,2,true);add('crate strap',x,.88,z,1.6,.11,1.39,color.metal,2)}
for(const x of [-8.8,-1,8.8]){add('floor stencil',x,.028,3.95,2.2,.013,.11,color.stripe,1)}
// Small debris and discarded containers are deliberate, nonblocking set dressing.
for(let i=0;i<35;i++){const x=-11+(i*7.37%25.1),z=-4.55+(i*3.79%9.15);if(Math.abs(x+4.1)<1||Math.abs(x-4.5)<1)continue;add('scattered debris',x,.038,z,.09+(i%3)*.07,.07,.09+(i%2)*.07,i%2?color.rust:color.metal,2)}
// Visual dressing pass: modular 1995 military-industrial *adaptation*.
// These are non-colliding environmental props. Source-specific props still
// require J2ME verification before final level reconstruction.
const darkSteel=[.115,.175,.17],steelEdge=[.33,.40,.38],hazard=[.54,.41,.18],warning=[.56,.17,.13];
for(let x=-11.6;x<14.0;x+=2.9){
 for(const side of [-1,1]){
  const z=side*4.95;
  add('reinforced wall pillar',x,1.53,z,.18,3.05,.28,darkSteel,2);
  add('pillar lower guard',x,.42,z,.27,.75,.34,steelEdge,2);
  add('conduit cross-bracket',x,2.73,z,.34,.08,.43,steelEdge,2);
  add('painted safety line',x,1.14,z-side*.13,.27,.045,.08,hazard,1);
 }
 add('cross ceiling spar',x,3.22,0,.14,.19,9.9,darkSteel,2);
 // Low-cost ceiling light housing with emissive surface and protective ribs.
 if(Math.round((x+11.6)/2.9)%2===0){
  add('lamp guard',x,3.30,.95,.72,.12,.24,steelEdge,2);
  add('lamp glow',x,3.235,.95,.63,.035,.18,color.light,3);
 }
}
for(let x=-11.7;x<13.5;x+=1.3){
 add('floor expansion joint',x,.013,0,.018,.01,9.1,darkSteel,2);
 for(const z of [-4.38,4.38]){
  add('wall base track',x,.15,z,.91,.22,.15,darkSteel,2);
  add('floor hazard chevron',x,.011,z>0?3.92:-3.92,.30,.009,.08,hazard,1);
 }
}
// Electrical banks and emergency call boxes positioned to aid navigation.
for(const [x,z] of [[-10.4,-4.81],[-1.8,-4.81],[8.0,-4.81]]){
 add('electrical panel',x,1.65,z,.88,.98,.16,darkSteel,2);
 add('panel inset',x,1.65,z+(z<0?.09:-.09),.70,.73,.02,steelEdge,2);
 add('panel light',x+.22,1.90,z+(z<0?.12:-.12),.11,.10,.04,color.light,3);
 add('panel latch',x+.30,1.48,z+(z<0?.13:-.13),.06,.19,.04,hazard,2);
}
for(const [x,z] of [[-11.2,4.83],[-2.75,4.83],[5.82,4.83]]){
 add('sector marking plate',x,2.33,z,.68,.34,.05,steelEdge,2);
 add('sector stencil bar',x,2.33,z-(z>0?.043:-.043),.48,.07,.02,hazard,2);
}
for(const x of [-3.6]){
 add('blast frame left',x,.33,-1.63,.32,.65,.32,steelEdge,2);
 add('blast frame right',x,.33,1.63,.32,.65,.32,steelEdge,2);
 add('blast frame crown',x,3.16,0,.38,.24,3.58,steelEdge,2);
 add('breach painted caution',x,.69,0,.025,.12,2.67,hazard,1);
}
// Prison fixtures give the captivity sequence a lived-in sense of scale.
for(const [x,z] of [[-11.0,-3.75],[-1.4,-3.75]]){
 add('utility wash basin',x,.65,z,.61,.19,.40,steelEdge,2);
 add('wash basin bracket',x,.42,z,.09,.43,.16,darkSteel,2);
 add('drain pipe',x,.24,z,.075,.31,.075,darkSteel,2);
 add('faucet',x,.79,z-.08,.045,.19,.08,steelEdge,2);
}
// Guard lane: defensive infrastructure and lighting; no premature alarms.
for(const x of [6.1,9.45,12.8]){
 add('security gantry',x,2.45,4.65,.20,1.45,.24,darkSteel,2);
 add('security console',x,1.12,4.30,.76,.48,.42,steelEdge,2);
 add('status display',x,1.17,4.06,.61,.33,.035,[.21,.44,.34],3);
}
// Deliberate stealth route. A tall cabinet breaks STANDING sightlines;
// low barricades protect crouched/prone players but not a standing head.
// Every piece uses the same obstacle list for navigation and guard sight.
const coverAssets = [
  {name:'low equipment barricade',x:6.6,z:-2.35,w:1.45,d:.56,h:1.28,kind:'low'},
  {name:'tall steel locker bank',x:9.15,z:.65,w:.75,d:1.40,h:2.14,kind:'tall'},
  {name:'low power cabinet',x:10.65,z:-.65,w:1.36,d:.62,h:1.32,kind:'low'},
  {name:'tall signal enclosure',x:12.3,z:2.2,w:.72,d:1.20,h:2.06,kind:'tall'}
];
for(const c of coverAssets){
  add(c.name,c.x,c.h/2,c.z,c.w,c.h,c.d,c.kind==='tall'?[.21,.31,.30]:[.24,.29,.26],2,true);
  add(c.name+' lid',c.x,c.h+.045,c.z,c.w+.07,.09,c.d+.07,[.39,.45,.40],2);
  add(c.name+' danger stripe',c.x,c.h*.55,c.z-c.d/2-.016,c.w*.75,.055,.026,[.72,.52,.18],2);
  if(c.kind==='tall'){
    for(let i=0;i<3;i++)add(c.name+' vent',c.x,c.h*.36+i*.32,c.z-c.d/2-.024,c.w*.65,.045,.026,[.10,.16,.17],2);
  }
}
// Cabinet close-up detail: inset panels, steel frame corners, fasteners,
// blackout ventilation slots and the small cues that make cover believable.
for(const c of coverAssets){
 const faceZ=c.z-c.d*.5-.03;
 detail(c.name+' front plate',c.x,c.h*.50,faceZ,c.w*.88,c.h*.70,.045,[.27,.35,.32],mat.steel,'bevel');
 for(const sx of [-1,1]){
  const edgeX=c.x+sx*c.w*.40;
  detail(c.name+' front trim',edgeX,c.h*.53,faceZ-.03,.045,c.h*.81,.055,[.43,.47,.41],mat.steel,'bevel');
  for(const yy of [.26,.73])detail(c.name+' bolt head',edgeX,yy*c.h,faceZ-.06,.043,.043,.022,[.65,.67,.59],mat.steel,'bevel');
 }
 if(c.kind==='tall'){
  detail(c.name+' pull handle',c.x+c.w*.29,c.h*.57,faceZ-.06,.065,.31,.055,[.66,.68,.62],mat.steel,'bevel');
  for(let v=0;v<7;v++)detail(c.name+' louver',c.x,c.h*.30+v*.105,faceZ-.07,c.w*.51,.021,.018,[.09,.16,.17],mat.steel,'bevel');
 }
}
// This is a REMASTER-ADDED secure service lift for the escort-mode slice,
// not a claim about the 1987 or J2ME prison architecture. The lift closes
// the player's immediate danger loop without pretending N313 has ended.
const safeLift={x:13.2,z:0.0,r:1.15};
add('maintenance lintel',13.91,2.96,0,.25,.25,2.2,[.24,.33,.34],mat.steel);
add('service lift shutter left',13.70,1.35,-.62,.075,2.40,.12,[.48,.55,.48],mat.steel);
add('service lift shutter right',13.70,1.35,.62,.075,2.40,.12,[.48,.55,.48],mat.steel);
add('service lift overhead beacon',13.55,2.79,0,.27,.16,.42,[.47,.84,.70],mat.light);
for(let i=0;i<5;i++)detail('lift threshold marking',12.28+i*.15,.015,-1.1,.07,.012,2.0,[.45,.57,.46],mat.steel);
// Interaction beacon above recoverable gear in this PROTOTYPE.
const gearLocker={x:7.1,z:-2.8,r:1.8};
// Physical identifiers for the processing desk, sealed kit and final threshold.
for (const [x,z] of [[-25.8,2.9],[-14.55,-1.25],[26.9,-1.95]]) {
 detail('desk steel frame',x,.47,z,1.12,.88,.62,[.19,.25,.24],mat.steel,'bevel');
 detail('brushed counter lip',x,.96,z,1.32,.085,.76,[.53,.55,.47],mat.steel,'bevel');
 detail('fixture bracket',x,1.24,z+.19,.065,.5,.065,color.metal,mat.steel);
 detail('caged task lamp',x,1.52,z+.10,.32,.12,.24,[.93,.70,.37],mat.light,'bevel');
}
for(const x of [-25,-14.8,18,23.5,27]){
 for(const side of [-1,1]){
  detail('roof truss foot',x,2.97,side*4.8,.28,.48,.23,color.metal,mat.steel);
  detail('conduit joint',x,2.65,side*4.93,.25,.20,.14,color.rust,mat.steel);
  detail('painted warning stripe',x,1.14,side*4.8,.32,.08,.06,color.stripe,mat.steel);
 }
 detail('ceiling rail',x,3.26,0,.13,.18,9.82,color.metal,mat.steel);
}
const propertyTag=detail('sealed property tag',7.1,1.15,-3.23,.32,.20,.025,[.83,.78,.59],mat.fabric);

// The reader sits on the approach face of the jamb, clear of the moving panel.
const exitReader={x:27.07,z:1.82};
detail('reader enclosure',27.12,1.35,1.82,.14,.39,.26,color.metal,mat.steel,'bevel');
detail('reader slot',27.04,1.33,1.82,.015,.026,.20,[.07,.09,.09],mat.steel);
for(const side of [-.042,0,.042])
 detail('reader level three emboss',27.043,1.24,1.82+side,.016,.047,.016,[.83,.81,.67],mat.steel);
const readerLamp=detail('reader indicator',27.035,1.45,1.82,.016,.04,.13,color.rust,mat.light);
const shutterSlats=Array.from({length:8},(_,i)=>
 detail('shutter folded seam',27.165,.24+i*.39,0,.035,.038,3.16,color.metal,mat.steel));
const cardHome={x:25,z:.6};
let cardDrop={...cardHome},dropTime=-10,shutterRise=0;
const liftSignal=add('secure passage status lamp',13.52,2.78,-.26,.08,.11,.13,[.29,.76,.44],mat.light);liftSignal.active=false;
const gearBeacon=add('recover equipment beacon',7.1,2.42,-2.8,.18,.18,.18,[.85,.59,.31],mat.light);
const rubble=[];
const fox={x:.35,z:0,yaw:Math.PI*.5,alive:true,hp:100,stance:0,mode:'captive',waypoints:[],target:null,pathTime:0,moveSpeed:0,hitTimer:0,calloutAt:-100,footstepAt:-100},guards=[{x:8.05,z:3.25,yaw:-Math.PI/2,state:0,suspicion:0,alert:0,search:0,caution:0,last:{x:8.05,z:3.25},patrol:0,target:null,waypoints:[],pathTime:0,knocked:false,hp:100,radioTimer:0,fireCooldown:1.0,aimTime:0,shotVisual:0},{x:11.8,z:-3.1,yaw:Math.PI/2,state:0,suspicion:0,alert:0,search:0,caution:0,last:{x:11.8,z:-3.1},patrol:1,target:null,waypoints:[],pathTime:0,knocked:false,hp:100,radioTimer:0,fireCooldown:1.65,aimTime:0,shotVisual:0}];
let stage=0,elapsedStage=0,gameTime=0,levelTime=0,health=100,missionFailed=false,lastEscortReminder=-100,player={x:-10.6,z:0,yaw:0,stance:0,aim:false,sprint:false,cover:false,coverNormal:null,knockUntil:0,attackUntil:0,renderCrouch:0,renderStride:0},cam={yaw:0,pitch:.15,dist:4.5,eye:[-14,2.2,3],focusY:1.56,shoulder:1},discovery={moved:false,looked:false,crouched:false,openedEquip:false,showTips:true,finished:false};
let footsteps=[],noiseEvents=[],nextNoiseId=0,particles=[],shots=[],notes=[],message=null,dialogueQueue=[],tipVisible='',lastStep=0,lastSound=0,awareness=0,alertTimer=0,achievement=[],lastRadar=0,frameCount=0,frameStart=performance.now();
let equipOpen=null,eqIndex=0,activePadIndex=-1,recoveredGear=false,stunAmmo=0,equippedWeapon='BARE HANDS',sectorSecured=false;const inventories={weapons:[{name:'BARE HANDS',usable:true},{name:'STUN PISTOL',usable:false},{name:'SUPPRESSOR',usable:false}],items:[{name:'NO ITEM',usable:true},{name:'RATION',usable:false},{name:'CARD 3',usable:false}]};
// This item enters the inventory only when the sealed kit comes back.
const tagSlot={name:'UNLISTED PROPERTY TAG',usable:true};
const tag={state:'absent',point:null,next:0,flash:-10,sampled:false};
function armPropertyTag(){
 tag.state='carried';tag.flash=-10;tag.next=gameTime+2;tag.point=null;
}
function tagPulse(origin){
 tag.flash=gameTime;
 playCue('propertyPing',origin);
}
function updatePropertyTag(){
 if(stage===-1&&!tag.sampled&&dist(player,post)<4.8){
  tag.sampled=true;tagPulse(post);
 }
 if(stage!==4||gameTime<tag.next||!['carried','placed'].includes(tag.state))return null;
 tag.next=gameTime+5.5;
 const point=tag.state==='placed'?tag.point:player;
 tagPulse(point);

 return {x:point.x,z:point.z};
}
function setDownTag(){
 tag.point={x:player.x,z:player.z};tag.state='placed';tag.next=gameTime+.4;
 syncEquipment();save();popup('Property tag set down.');
}
const rehearsal=new URLSearchParams(location.search).has('boss');
const STORAGE='mg1987.grayfox.browser.v1'+(rehearsal?'.boss':'')+(escortMode?'.escort':'');let context=null,audioReady=false,audioBed=null,beatTime=0,prevAlert=0,padPrev=[],padUsing=false,tipToggle=$('tipsToggle');
const wallHits=[0,0];
let nextImpact=0, sealedWeapon='STUN PISTOL', quietSince=0, movementSample=null;
const boss={...guards[0],x:25,z:.6,yaw:Math.PI,hp:240,boss:true,waypoints:[],last:{x:25,z:.6},knocked:false,cycle:0};
const yardGuard={...guards[0],x:-17.2,z:2.4,yaw:Math.PI,waypoints:[],last:{x:-17.2,z:2.4},yard:true};
const arrestGuard={...yardGuard,x:post.x,z:post.z,postGuard:true,waypoints:[],last:{...post}};
guards.push(yardGuard,arrestGuard);
// Bake contact and cast shadows for fixed scenery once. Dynamic shutters,
 // actors and breakable panels are intentionally excluded from this map.
function bakeFloorShade(){
 const w=512,h=128,bytes=new Uint8Array(w*h).fill(255);
 const solids=obstacles.filter(o=>o.active&&o.h>.3&&o.w<8&&o.d<8&&
  ![finalGate,propertyGate,breach1,breach2].includes(o)&&o.y-o.h/2<.45);
 for(const o of solids){
  const lampX=o.x>14?(o.x<19.5?17:o.x<24.7?22:27.5):Math.round(o.x/6)*6;
  const sx=clamp((o.x-lampX)*.45,-1.35,1.35),sz=clamp((o.z-.3)*.42,-1.2,1.2);
  const x0=Math.max(0,Math.floor((o.x-o.w/2-2.2+33)*w/63));
  const x1=Math.min(w-1,Math.ceil((o.x+o.w/2+2.2+33)*w/63));
  const z0=Math.max(0,Math.floor((o.z-o.d/2-2.2+5.5)*h/11));
  const z1=Math.min(h-1,Math.ceil((o.z+o.d/2+2.2+5.5)*h/11));
  for(let z=z0;z<=z1;z++)for(let x=x0;x<=x1;x++){
   const dx=(x+.5)*63/w-33-o.x,dz=(z+.5)*11/h-5.5-o.z;
   let darkness=0;
   for(let j=0;j<=4;j++){
    const t=j/4;
    const edge=Math.hypot(Math.max(0,Math.abs(dx-sx*t)-o.w/2),Math.max(0,Math.abs(dz-sz*t)-o.d/2));
    darkness=Math.max(darkness,(1-t*.56)*Math.max(0,1-edge/(.38+t*.26)));
   }
   bytes[z*w+x]=Math.min(bytes[z*w+x],255-Math.round(224*darkness));
  }
 }
 const texture=gl.createTexture();gl.activeTexture(gl.TEXTURE1);gl.bindTexture(gl.TEXTURE_2D,texture);
 gl.texImage2D(gl.TEXTURE_2D,0,gl.LUMINANCE,w,h,0,gl.LUMINANCE,gl.UNSIGNED_BYTE,bytes);
 gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MIN_FILTER,gl.LINEAR);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_MAG_FILTER,gl.LINEAR);
 gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_WRAP_S,gl.CLAMP_TO_EDGE);gl.texParameteri(gl.TEXTURE_2D,gl.TEXTURE_WRAP_T,gl.CLAMP_TO_EDGE);
 gl.uniform1i(gl.getUniformLocation(program,'uFloorShade'),1);gl.activeTexture(gl.TEXTURE0);
}
bakeFloorShade();
const scars=[[],[]];
function kitAvailable(){return stage<0||recoveredGear}
function syncEquipment(){
 inventories.weapons[1].usable=kitAvailable();
 inventories.items[1].usable=kitAvailable();
 propertyTag.active=!recoveredGear;
 propertyGate.active=!recoveredGear;
 inventories.items[2].usable=stage>=6;
 const slotIndex=inventories.items.indexOf(tagSlot);
 if(tag.state==='carried'&&slotIndex<0)inventories.items.push(tagSlot);
 if(tag.state!=='carried'&&slotIndex>=0)inventories.items.splice(slotIndex,1);
 updateShutter(0);
 gearBeacon.active=stage===3&&!recoveredGear;
 liftSignal.active=stage>=5;
}
function updateShutter(dt){
 if(stage<7)shutterRise=0;
 else shutterRise=Math.min(1,shutterRise+dt/.85);
 const ease=shutterRise*shutterRise*(3-2*shutterRise);
 finalGate.y=1.6+3.28*ease;
 finalGate.active=shutterRise<1;
 for(const [i,slat] of shutterSlats.entries()){
  slat.y=.24+i*.39+3.28*ease;slat.active=finalGate.active;
 }
 readerLamp.color=stage>=7?[.30,.87,.62]:[.96,.53,.20];
}
function validCardPosition(point){
 return Number.isFinite(point?.x)&&Number.isFinite(point?.z)&&point.x<26.93&&
        point.x>-12.2&&canOccupyBody(point.x,point.z,0,0);
}
function cardReachable(target){
 return dist(player,target)<1.55&&faces(target)&&!lineBlocked(player,target);
}
function exitInteraction(){
 if(stage===8&&player.x>27.85&&Math.abs(player.z)<1.4)return 'debrief';
 if(stage===5&&cardReachable(cardDrop))return 'take';
 if(stage>=4&&stage<7&&cardReachable(exitReader))return stage===6?'scan':'locked';
 return '';
}
function syncBoss(){
 if(stage>=4&&!guards.includes(boss))guards.push(boss);
 boss.knocked=stage>=5;
}
function record(event){
 performance.mark('grayfox.'+event);
}
function save(){
 if(missionFailed||health<=0)return;
 try{
  localStorage.setItem(STORAGE,JSON.stringify({version:6,propertyTag:{state:tag.state,point:tag.point},stage,cardDrop,x:player.x,z:player.z,
   stunAmmo,equippedWeapon,sealedWeapon,recoveredGear,wallHits,health,
   bossHP:boss.hp,bossRest:stage>=5?{x:boss.x,z:boss.z,yaw:boss.yaw}:null,showTips:discovery.showTips,foxX:fox.x,foxZ:fox.z,foxHP:fox.hp}));
 }catch(e){}
}
function load(){
 let data;
 try{data=JSON.parse(localStorage.getItem(STORAGE))}catch(e){return false}
 if(!data||![1,2,3,4,5,6].includes(data.version)||!Number.isInteger(data.stage))return false;
 const legacy=data.version<4;
 if(data.stage<(legacy?0:-2)||data.stage>(legacy?4:data.version===4?6:8))return false;
 stage=legacy?Math.min(3,data.stage):data.stage;
 // Version 4 opened the exit on boss defeat. Preserve that earned access.
 if(data.version===4&&stage>=5)stage+=2;
 shutterRise=stage>=7?1:0;
 recoveredGear=(stage===3&&legacy&&!!data.recoveredGear)||stage>=4;
 if(recoveredGear&&stage===3)stage=4;
 stunAmmo=clamp(Number.isFinite(data.stunAmmo)?data.stunAmmo:6,0,6);
 sealedWeapon=data.sealedWeapon||'STUN PISTOL';
 equippedWeapon=kitAvailable()?(data.equippedWeapon||sealedWeapon):'BARE HANDS';
 health=clamp(Number(data.health)||100,1,100);
 boss.hp=clamp(Number(data.bossHP)||240,1,240);duel.lastHp=boss.hp;
 fox.x=escortMode?(data.foxX??.35):.35;fox.z=escortMode?(data.foxZ??0):0;
 fox.hp=escortMode?clamp(Number(data.foxHP)||100,1,100):100;
 fox.mode=stage>=2?(escortMode?'following':'resting'):'captive';
 discovery.showTips=data.showTips!==false;tipToggle.checked=discovery.showTips;
 if(stage>=4){
  const saved=data.propertyTag;
  if(data.version===6&&['placed','spent'].includes(saved?.state)&&
     Number.isFinite(saved.point?.x)&&Number.isFinite(saved.point?.z)&&
     Math.abs(saved.point.z)<=4.8&&saved.point.x>=-12.6&&saved.point.x<=29.1){
   tag.state=saved.state;tag.point={x:saved.point.x,z:saved.point.z};tag.next=gameTime+2;
  }else if(stage===4||saved?.state==='carried')armPropertyTag();
 }
 syncEquipment();syncBoss();
 for(const [i,wall,seam,opened] of [[0,breach1,seam1,stage>=1],[1,breach2,seam2,stage>=3]]){
  wallHits[i]=opened?3:clamp(Math.floor(data.wallHits?.[i]||0),0,2);
  if(opened)activateBreach(wall,seam,false);
  else for(let n=1;n<=wallHits[i];n++)makeScar(i,n);
 }
 cardDrop=validCardPosition(data.cardDrop)?{x:data.cardDrop.x,z:data.cardDrop.z}:{...cardHome};
 if(stage>=5){
  const rest=validCardPosition(data.bossRest)?data.bossRest:cardDrop;
  boss.x=rest.x;boss.z=rest.z;boss.yaw=Number.isFinite(rest.yaw)?rest.yaw:Math.PI;
 }
 const fallback=stage<0?[-27,0]:stage===0?[-10.6,0]:stage<3?[-.5,0]:stage===3?[5.4,-3.7]:stage===4?[15.7,0]:[26,0];
 const withinStage=stage<0?data.x<-12.6:stage===0?data.x>-12.6&&data.x<-4.1:
  stage<3?data.x>-12.6&&data.x<4.5:stage===3?data.x>-12.6&&data.x<14.6:
  stage<7?data.x>-12.6&&data.x<27.3:data.x>-12.6;
 const safe=withinStage&&Number.isFinite(data.x)&&Number.isFinite(data.z)&&canOccupyBody(data.x,data.z,0,0);
 [player.x,player.z]=!legacy&&safe?[data.x,data.z]:fallback;
 player.cover=false;player.stance=0;
 return true;
}
function makeScar(i,n){
 if(scars[i][n])return;
 const w=i?breach2:breach1;
 const mark=add('exposed mortar',w.x+(i?n*.14:-.16),1.18+n*.15,w.z+(i?.16:n*.14),
  i?.065:.045,.37,i?.045:.065,[.08,.10,.10],mat.concrete);
 mark.tilt=n===1?.42:-.60;mark.yaw=i?0:Math.PI/2;
 scars[i][n]=mark;
}
function faces(target){
 const offset=nrm(target.x-player.x,target.z-player.z);
 return Math.cos(cam.yaw)*offset.x+Math.sin(cam.yaw)*offset.z>.20;
}
function strikePanel(i,wall,seam){
 if(gameTime<nextImpact)return;
 nextImpact=gameTime+.28;player.attackUntil=gameTime+.23;
 wallHits[i]++;record('wall.'+i+'.'+wallHits[i]);
 if(wallHits[i]===3){
  scars[i].forEach(mark=>mark.active=false);
  activateBreach(wall,seam);progress(i?3:1);say(i?'exit':'breach1');playCue('breach',wall);
 }else{
  makeScar(i,wallHits[i]);playCue(wallHits[i]===1?'knock':'hit',wall);
  popup(wallHits[i]===1?'Hollow impact. A hairline crack opens.':'Mortar falls from the split.');
 }
 noiseEvents.push({x:wall.x,z:wall.z,power:wallHits[i]===3?9:4,life:.65});
 save();
}
function reset(){try{localStorage.removeItem(STORAGE)}catch(e){} location.reload()}
function activateBreach(wall,crack,scatter=true){
 if(!wall.active)return;
 wall.active=false;crack.active=false;
 for(let i=0;i<16;i++){
  const across=(i*1.73%3.3)-1.65,depth=((i%3)-1)*.26;
  rubble.push(add('permanent broken masonry',wall.x+(wall===breach2?across:depth),
    .085+(i%3)*.045,wall.z+(wall===breach2?depth:across),.26+(i%4)*.11,.12+(i%3)*.08,.22+(i%2)*.1,
    i%3===0?color.rust:color.wall,0));
 }
 add('broken wall upright A',wall.x+(wall===breach2?-1.64:0),1.25,wall.z+(wall===breach2?0:-1.64),.28,2.5,.15,color.wall,0);
 add('broken wall upright B',wall.x+(wall===breach2?1.64:0),1.25,wall.z+(wall===breach2?0:1.64),.28,2.5,.15,color.wall,0);
 if(!scatter)return;
 for(let i=0;i<36;i++)particles.push({x:wall.x+(Math.random()-.5)*(wall===breach2?2.8:.6),
   y:.45+Math.random()*2.1,z:wall.z+(Math.random()-.5)*(wall===breach2?.6:2.8),
   vx:(Math.random()-.5)*3.0,vy:1.2+Math.random()*2.2,
   vz:(Math.random()-.5)*3.5,life:.8+Math.random()*1.4,
   c:i%4===0?color.rust:color.wall});
}
function popup(text,t=2.4){message={text,until:gameTime+t};}
function say(id){const d=window.MGDialogue?.[id];if(!d)return;dialogueQueue.push({id,...d});}
function displayNextDialogue(dt){
 if(message&&gameTime>message.until)message=null;
 const line=dialogueQueue[0];
 if(!line){$('subtitle').style.opacity='0';return}
 if(line.remaining===undefined){
  if(line.id.startsWith('boss'))duel.voice=gameTime+Math.min(1.7,line.duration);
  line.remaining=line.duration;
  if(['bigBoss','schneider','continueMission','tagReply'].includes(line.id))playCue('radio');
  const synth=window.speechSynthesis;
  if($('voiceToggle').checked&&synth&&window.SpeechSynthesisUtterance){
   const voice=synth.getVoices().find(v=>v.localService&&v.lang.toLowerCase().startsWith('en'));
   if(voice){
    const speech=new SpeechSynthesisUtterance(line.text);
    speech.voice=voice;speech.rate=.97;
    speech.onstart=()=>{line.speaking=true};
    speech.onend=speech.onerror=()=>{line.speaking=false};
    synth.speak(speech);
   }
  }
 }
 line.remaining-=dt;
 $('subtitle').textContent=line.speaker+' · '+line.text;
 $('subtitle').style.opacity='1';
 if(line.remaining<=0&&(!line.speaking||line.remaining < -12))dialogueQueue.shift();
}
function progress(s){if(s!==stage+1)return false;stage=s;elapsedStage=0;quietSince=gameTime;syncEquipment();syncBoss();record('stage.'+s);save();return true}
function SFX(){if(!context)return;try{if(context.state==='suspended')context.resume();}catch(e){}}
function initAudio(){if(audioReady)return;try{
 context=new (window.AudioContext||window.webkitAudioContext)();audioReady=true;
 const master=context.createGain();master.gain.value=.14;master.connect(context.destination);
 function drone(freq,type){const osc=context.createOscillator(),gain=context.createGain();osc.type=type;
  osc.frequency.value=freq;gain.gain.value=0;osc.connect(gain);gain.connect(master);osc.start();return gain;}
 audioBed={master,patrol:drone(52,'sine'),tension:drone(81,'triangle'),alert:drone(108,'sawtooth')};
} catch(e){audioReady=false;audioBed=null}}
function noiseBurst(duration=.09,volume=.04,cutoff=900,origin=null){
 if(!context)return;
 const length=Math.ceil(context.sampleRate*duration),buffer=context.createBuffer(1,length,context.sampleRate),data=buffer.getChannelData(0);
 for(let i=0;i<length;i++)data[i]=(Math.random()*2-1)*(1-i/length);
 const source=context.createBufferSource(),filter=context.createBiquadFilter(),gain=context.createGain();
 source.buffer=buffer;filter.type='lowpass';filter.frequency.value=cutoff;
 gain.gain.value=volume;source.connect(filter);filter.connect(gain);if(origin&&context.createStereoPanner){
  const offset=nrm(origin.x-player.x,origin.z-player.z);
  const panner=context.createStereoPanner();
  panner.pan.value=clamp(-Math.sin(cam.yaw)*offset.x+Math.cos(cam.yaw)*offset.z,-1,1);
  gain.gain.value/=1+.10*dist(origin,player);
  gain.connect(panner);panner.connect(context.destination);
 }else gain.connect(context.destination);
 source.start();
}
function tone(freq,dur,vol,type='sine',slide=1,origin=null,delay=0){if(!context)return;const now=context.currentTime+delay,o=context.createOscillator(),g=context.createGain();o.type=type;o.frequency.setValueAtTime(freq,now);o.frequency.exponentialRampToValueAtTime(Math.max(20,freq*slide),now+dur);g.gain.setValueAtTime(.0001,now);g.gain.exponentialRampToValueAtTime(vol/(origin?1+.20*dist(origin,player):1),now+.012);g.gain.exponentialRampToValueAtTime(.0001,now+dur);o.connect(g);
 if(origin&&context.createStereoPanner){
  const dx=origin.x-player.x,dz=origin.z-player.z,pan=context.createStereoPanner();
  pan.pan.value=clamp((-Math.sin(cam.yaw)*dx+Math.cos(cam.yaw)*dz)/Math.max(1,Math.hypot(dx,dz)),-1,1);
  g.connect(pan);pan.connect(context.destination);
 }else g.connect(context.destination);
 o.start(now);o.stop(now+dur+.03)}
function playCue(name,origin=null){if(!audioReady)return;SFX();switch(name){case 'propertyPing':tone(760,.075,.028,'sine',1,origin);tone(980,.09,.027,'sine',1,origin,.14);break;case 'bossStep':noiseBurst(.115,.073,410,origin);break;case 'card':noiseBurst(.09,.045,3100,origin);tone(640,.10,.025,'triangle');break;case 'unlock':tone(510,.12,.045);tone(765,.19,.035);noiseBurst(.74,.10,490,origin);break;case 'rack':noiseBurst(.19,.065,2600,origin);tone(302,.11,.04,'triangle',.62);break;case 'seal':tone(210,.48,.12,'sawtooth',.47);noiseBurst(.8,.12,360);setTimeout(()=>{tone(310,.13,.048);tone(480,.24,.044)},370);break;case 'denied':tone(142,.19,.06,'square',.64);break;case 'select':tone(620,.095,.042);tone(910,.12,.035);break;case 'menu':tone(510,.08,.027,'triangle');break;case 'breach':tone(95,.6,.23,'sawtooth',.35);noiseBurst(.46,.12,600,origin);tone(162,.34,.10,'triangle',.55);break;case 'step':noiseBurst(.085,.035,460);tone(64,.065,.021,'triangle',.54);break;case 'radio':tone(860,.10,.032,'square');tone(460,.13,.022,'sine');break;case 'gunshot':noiseBurst(.13,.15,2000,origin);tone(120,.18,.09,'sawtooth',.42);break;case 'alert':tone(820,.35,.14,'sawtooth',.86);tone(1100,.32,.10,'square',.68);break;case 'hit':noiseBurst(.13,.06,1500);tone(125,.2,.10,'sawtooth',.52);break;case 'whoosh':noiseBurst(.12,.025,1900);tone(235,.14,.035,'triangle',.55);break;case 'knock':tone(175,.11,.11,'triangle',.38);noiseBurst(.11,.078,570,origin);setTimeout(()=>{tone(147,.10,.065,'triangle',.42);noiseBurst(.08,.05,440,origin)},105);break;case 'win':for(let i=0;i<3;i++)setTimeout(()=>tone([350,470,620][i],.23,.09),i*150);break;}}
function ambience(dt){if(!audioReady||paused||!started)return;
 if(audioBed){
  const now=context.currentTime,attack=.19;
  audioBed.patrol.gain.setTargetAtTime(awareness===0?.21:.045,now,attack);
  audioBed.tension.gain.setTargetAtTime(awareness>=1&&awareness<3?.27:.015,now,attack);
  audioBed.alert.gain.setTargetAtTime(awareness===3?.25:.001,now,attack);
 }
 beatTime+=dt;
 const spacing=stage===4&&player.x>15?1.12:awareness>=3?.78:awareness>=1?1.6:6.5;
 if(beatTime>=spacing){beatTime=0;
  if(stage===4&&player.x>15){tone(55,.35,.042,'triangle');tone(82,.26,.019,'sine');}
  else if(awareness>=3){tone(104,.20,.035,'sawtooth');tone(208,.12,.017,'triangle');}
  else if(awareness>=1)tone(87,.29,.017,'triangle');
  else noiseBurst(.24,.006,130);
 }
}
let framePad=null, padIssue='', lastPadIdentity='', lastInputIsPad=false;
const padSnapshots=new Map();
function pressed(button){return typeof button==='number'?button>.45:!!(button?.pressed||button?.value>.45);}
function pollGamepad(){
 let devices=[];
 try{
  if(typeof navigator.getGamepads==='function')devices=Array.from(navigator.getGamepads()||[]);
  else padIssue='Gamepad API unavailable. Open the extracted game in desktop Edge or Chrome.';
 }catch(error){
  padIssue='Controller access blocked. Open the game in a standalone browser tab.';
 }
 const pads=devices.filter(p=>p&&p.connected!==false);
 let selected=pads.find(p=>p.index===activePadIndex), changed=null;
 for(const p of pads){
  const input=[...p.buttons.map(b=>pressed(b)?1:0),...p.axes.map(v=>Math.abs(v)>.18?Math.round(v*8):0)];
  const key=p.index+'|'+p.id, prior=padSnapshots.get(key);
  if(input.some((v,i)=>v!==0&&v!==(prior?.[i]||0)))changed=p;
  padSnapshots.set(key,input);
 }
 for(const key of padSnapshots.keys())if(!pads.some(p=>p.index+'|'+p.id===key))padSnapshots.delete(key);
 selected=changed||selected||pads.find(p=>p.mapping==='standard')||pads[0]||null;
 if(changed)lastInputIsPad=true;
 const identity=selected?selected.index+'|'+selected.id:'';
 if(identity!==lastPadIdentity){
  padPrev=[];if(equipOpen&&lastInputIsPad){equipOpen=null;drawEquipment();}
  lastPadIdentity=identity;
 }
 framePad=selected;activePadIndex=selected?selected.index:-1;
 if(selected)padIssue='';
 return selected;
}
function activeGamepad(){return framePad;}
function updatePadStatus(p){
 let state=padIssue||'Focus this window and press a button on your Xbox controller.';
 if(p){
  const held=p.buttons.map((b,i)=>pressed(b)?i:null).filter(i=>i!==null);
  state=p.id+' · '+(p.mapping||'unmapped')+' · buttons '+(held.join(', ')||'idle')+
   ' · sticks '+Array.from(p.axes).slice(0,4).map(v=>v.toFixed(2)).join(' / ');
 }
 $('padReadout').textContent=state;
 $('pausePadReadout').textContent=state;
 $('controller').textContent=p?(lastInputIsPad?'GAMEPAD ACTIVE':'GAMEPAD CONNECTED'):(padIssue?'CONTROLLER ACCESS UNAVAILABLE':'KEYBOARD + MOUSE');
}
function inputDown(k){return !!key[k]}
function action(name){if(!started){if(name==='start')start(false);return;}if(paused&&name!=='pause'&&name!=='intel')return;
 if(name==='interact'){
   if(equipOpen){
    if(equipOpen==='items'&&inventories.items[eqIndex]===tagSlot){
     equipOpen=null;setDownTag();eqIndex=0;drawEquipment();
    }
    return;
   }
   const exitAction=exitInteraction();
   if(exitAction==='debrief'){
    const report={carried:'The processing tag was still attached when Snake crossed the shutter.',
     placed:'Snake left the processing tag behind.',spent:'Shoot Gunner followed the signal and found the tag on the floor.'};
    $('propertyOutcome').textContent=report[tag.state]||'';
    setPaused(true,'complete');return;
   }
   if(exitAction==='take'){
     progress(6);playCue('card');popup('Card 3 recovered.');return;
   }
   if(exitAction==='scan'){
     progress(7);playCue('unlock',exitReader);popup('Card accepted. The shutter is rising.');return;
   }
   if(exitAction==='locked'){playCue('denied');popup('Reader requires Card 3.');return;}

   if(stage===-2&&dist(player,informant)<2.2){
     progress(-1);say('informant');return;
   }
   if(stage===-1&&dist(player,post)<2.1){
     if(player.aim){popup('Lower your weapon to surrender.');return}
     sealedWeapon=equippedWeapon;equippedWeapon='BARE HANDS';
     player.x=-10.6;player.z=0;player.cover=false;player.stance=0;cam.yaw=0;
     window.speechSynthesis?.cancel();dialogueQueue=[];document.getElementById('gl').animate?.([{filter:'brightness(.40)'},{filter:'brightness(1)'}],380);progress(0);say('capture');playCue('seal');return;
   }
   if(stage===3&&dist(player,gearLocker)<gearLocker.r&&
      !lineBlocked(player,{x:gearLocker.x,z:gearLocker.z+(player.z<gearLocker.z?-.44:.44)})){
     recoveredGear=true;equippedWeapon=sealedWeapon;armPropertyTag();progress(4);
     popup('Personal effects returned · '+stunAmmo+' charges');playCue('select');say('equipment');return;
   }
   if(stage===0&&faces(breach1)&&dist(player,breach1)<2.1&&player.x<breach1.x){strikePanel(0,breach1,seam1);return}
   if(stage===1&&dist(player,fox)<2.1&&!lineBlocked(player,fox)){
     fox.mode=escortMode?'following':'resting';progress(2);
     say('rescue1');say('rescue2');say('rescue3');playCue('select');return;
   }
   if(stage===2&&faces(breach2)&&dist(player,breach2)<2.1&&player.z>breach2.z){strikePanel(1,breach2,seam2);return}
   // A / E also becomes a contextual cover action when no mission object
   // is interactable. Never replace a wall breach, rescue, or gear recovery.
   if(player.cover){player.cover=false;player.coverNormal=null;popup('LEFT WALL COVER',1.4);playCue('select');return;}
   const wallFace=coverWall(player);
   if(wallFace&&player.stance!==2){
     player.cover=true;player.coverNormal=wallFace;
     player.yaw=Math.atan2(-wallFace.z,-wallFace.x);
     popup('WALL COVER — X / F KNOCK · MOVE ALONG WALL',2.6);playCue('select');return;
   }
   popup('NO ACTION — APPROACH AND FACE A TARGET');playCue('denied');
 }else if(name==='crouch'){
  const next=player.stance===1?0:1;
  if(setPlayerStance(next)){
   discovery.crouched=true;
   popup(player.stance===1?'CROUCHED — FOOTSTEPS QUIETER':'STANDING — NORMAL HEIGHT',1.7);
   playCue('select');
  }
 }
 else if(name==='prone'){
  const next=player.stance===2?1:2;
  if(setPlayerStance(next)){
   popup(player.stance===2?'PRONE — LOW PROFILE / EXTENDED BODY':'CROUCHED',1.8);
   playCue('select');
  }
 }
 else if(name==='cover'){
   if(player.cover){player.cover=false;player.coverNormal=null;popup('LEFT WALL COVER');playCue('select');}
   else {const surface=coverWall(player);if(surface&&player.stance!==2){player.cover=true;player.coverNormal=surface;player.yaw=Math.atan2(-surface.z,-surface.x);popup('WALL COVER — X / F KNOCK',2.4);playCue('select');}else{popup('NO SUITABLE WALL WITHIN REACH');playCue('denied');}}
 }
 else if(name==='knock'||name==='melee'&&player.cover){
   const wall=coverWall(player);
   if(!player.cover||!wall){player.cover=false;player.coverNormal=null;popup('NEED WALL COVER TO KNOCK');playCue('denied');return;}
   if(player.stance===2){popup('CANNOT KNOCK WHILE PRONE');playCue('denied');return;}
   // A measured action cannot be spammed into a permanent free lure.
   if((player.knockCooldown||0)>gameTime)return;
   player.knockCooldown=gameTime+.75;
   player.knockUntil=gameTime+.25;
   player.yaw=Math.atan2(-wall.z,-wall.x);
   const point=wall.surface||{x:player.x-wall.x*.36,z:player.z-wall.z*.36};
   const event={id:++nextNoiseId,x:point.x,z:point.z,source:'knock',power:7.2,life:.55};
   noiseEvents.push(event);
   playCue('knock');popup('KNOCK — GUARDS MAY INVESTIGATE',2.3);
 }
 else if(name==='melee'){if(player.attackUntil>gameTime)return;player.attackUntil=gameTime+.28;if(player.stance===2){popup('CANNOT STRIKE WHILE PRONE');playCue('denied');return;}const g=guards.find(g=>{const d=dist(g,player),toward={x:(g.x-player.x)/(d||1),z:(g.z-player.z)/(d||1)};return !g.knocked&&d<1.5&&Math.cos(player.yaw)*toward.x+Math.sin(player.yaw)*toward.z>.48&&!lineBlocked(player,g)});if(g){g.hp-=35;g.aimTime=0;g.state=3;g.alert=8;g.last={x:player.x,z:player.z};if(g.hp<=0){g.knocked=true;g.state=0;popup(g.boss?'SHOOT GUNNER DOWN':'GUARD INCAPACITATED');}else popup('MELEE IMPACT — GUARD ALERTED');playCue('hit');noiseEvents.push({x:player.x,z:player.z,power:5,life:.8});}else{popup('PUNCH — NO TARGET',1);playCue('whoosh');}}
 else if(name==='fire'){
  if(!kitAvailable()){popup('WEAPON CONFISCATED — UNARMED');playCue('denied');}
  else if(equippedWeapon!=='STUN PISTOL'){popup('SELECT STUN PISTOL IN RIGHT-HAND WINDOW');playCue('denied');}
  else if(stunAmmo<=0){popup('NO STUN CHARGES');playCue('denied');}
  else if(!player.aim){popup('HOLD AIM BEFORE FIRING');playCue('denied');}
  else{
    stunAmmo--;noiseEvents.push({x:player.x,z:player.z,power:8,life:.75});playCue('gunshot');
    const aim={x:Math.cos(cam.yaw),z:Math.sin(cam.yaw)};
    const target=guards.filter(g=>!g.knocked&&dist(g,player)<11&&!lineBlockedSight(player,g,player.stance===2?.55:player.stance===1?1.0:1.5,1.35))
      .map(g=>({g,range:dist(g,player),dot:aim.x*(g.x-player.x)+aim.z*(g.z-player.z)}))
      .filter(v=>v.dot>0&&v.dot/v.range>.93).sort((a,b)=>a.range-b.range)[0];
    if(target){const g=target.g;g.hp=Math.max(0,g.hp-100);g.knocked=g.hp===0;g.state=g.knocked?0:3;g.suspicion=g.knocked?0:1;g.aimTime=0;g.shotVisual=0;popup(g.knocked?'TARGET DOWN':'SHOOT GUNNER STAGGERS',2);shots.push({x:player.x,z:player.z,y:1.2,toX:g.x,toZ:g.z,toY:1.3,life:.18});}
    else popup('STUN FIRED — NO TARGET',1.7);
    if(stage<0)save();
  }
}
 else if(name==='pause'){setPaused(!paused,'pause');}
 else if(name==='intel'){if(!paused){setPaused(true,'intel')}else if($('intel').style.display==='flex'){setPaused(false,'intel')}else{showIntel();}}
 else if(name==='codec'){
   record('radio');
   if(stage===4&&tag.state==='carried'&&tag.flash>=0&&!dialogueQueue.some(line=>line.id==='tagReply')){
    say('tagQuestion');say('tagReply');return;
   }
   say(stage<0?'schneider':stage===0?'bigBoss':'continueMission');
 }
 else if(name==='equipitem'||name==='equipweapon'){equipOpen=name==='equipitem'?'items':'weapons';eqIndex=0;discovery.openedEquip=true;drawEquipment();playCue('menu');}
 else if(name==='equipclose'){if(equipOpen){const slot=inventories[equipOpen][eqIndex];if(slot.usable&&equipOpen==='weapons')equippedWeapon=slot.name;popup(slot.usable?slot.name+' SELECTED':'EQUIPMENT CONFISCATED');playCue(slot.usable?'select':'denied');equipOpen=null;drawEquipment();}}
 else if(name==='next'||name==='previous'){if(equipOpen){const slots=inventories[equipOpen];eqIndex=(eqIndex+(name==='next'?1:-1)+slots.length)%slots.length;drawEquipment();playCue('menu')}}
 else if(name==='tips'){discovery.showTips=!discovery.showTips;tipToggle.checked=discovery.showTips;popup(discovery.showTips?'TRAINING PROMPTS ON':'TRAINING PROMPTS OFF')}
}
function lineBlocked(a,b,ignoreWeak=false){
 // Analytic segment/AABB intersection: stepped rays can skip 28cm walls.
 for(const o of obstacles){
  if(!o.active||o.name==='concrete floor'||(ignoreWeak&&o===breach1))continue;
  let near=0,far=1,hit=true;
  for(const axis of ['x','z']){
   const span=b[axis]-a[axis],lo=o[axis]-o[axis==='x'?'w':'d']/2,hi=o[axis]+o[axis==='x'?'w':'d']/2;
   if(Math.abs(span)<1e-7){if(a[axis]<lo||a[axis]>hi){hit=false;break}}
   else {let t1=(lo-a[axis])/span,t2=(hi-a[axis])/span;if(t1>t2)[t1,t2]=[t2,t1];near=Math.max(near,t1);far=Math.min(far,t2);if(near>far){hit=false;break}}
  }
  if(hit&&near<=1&&far>=0)return true;
 }
 return false;
}
// 3D awareness: low objects shelter a crouching head but do not magically
// conceal a standing head. This must agree with the visible obstacle heights.
// Canonical 3D line-of-sight slab test. Rendering and AI MUST use the same
// occlusion result, so radar never draws a cone through a solid wall.
// Return the first intersected ray fraction, or Infinity if clear.
function firstSightBlockFraction(a,b,fromY=1.58,toY=player.stance===2?.48:player.stance===1?.92:1.52){
 let nearest=Infinity;
 for(const o of obstacles){
  if(!o.active||o.name==='concrete floor')continue;
  let near=0,far=1,hit=true;
  for(const axis of ['x','z']){
   const delta=b[axis]-a[axis],half=(axis==='x'?o.w:o.d)*.5;
   const lo=o[axis]-half,hi=o[axis]+half;
   if(Math.abs(delta)<1e-7){if(a[axis]<lo||a[axis]>hi){hit=false;break;}}
   else{
    let t1=(lo-a[axis])/delta,t2=(hi-a[axis])/delta;
    if(t1>t2)[t1,t2]=[t2,t1];near=Math.max(near,t1);far=Math.min(far,t2);
    if(near>far){hit=false;break;}
   }
  }
  if(!hit||near>1||far<0)continue;
  const lo=o.y-o.h/2,hi=o.y+o.h/2,dy=toY-fromY;
  if(Math.abs(dy)<1e-7){if(fromY<lo||fromY>hi)continue;}
  else{
   let t1=(lo-fromY)/dy,t2=(hi-fromY)/dy;
   if(t1>t2)[t1,t2]=[t2,t1];near=Math.max(near,t1);far=Math.min(far,t2);
   if(near>far)continue;
  }
  if(near<=1&&far>=0)nearest=Math.min(nearest,Math.max(0,near));
 }
 return nearest;
}
function lineBlockedSight(a,b,fromY=1.58,toY=player.stance===2?.48:player.stance===1?.92:1.52){
 return firstSightBlockFraction(a,b,fromY,toY)<=1;
}
// The same height-dependent occlusion is used to plot Soliton vision wedges.
// This is what a STANDING, CROUCHED or PRONE player can currently expose.
const RADAR_CONE_HALF_ANGLE=.87; // updateGuards' exact player vision threshold
const RADAR_VISION_RANGE=10;     // updateGuards' exact player vision range
const RADAR_NEAR_RADIUS=1.65;    // updateGuards' near-field 360-degree detection
function radarVisionDistance(g,angle,maxRange=RADAR_VISION_RANGE){
 if(g.knocked)return 0;
 const end={x:g.x+Math.cos(angle)*maxRange,z:g.z+Math.sin(angle)*maxRange};
 const targetHeight=player.stance===2?.48:player.stance===1?.92:1.52;
 const obstruction=firstSightBlockFraction(g,end,1.58,targetHeight);
 // Leave a small padding so colour never implies vision through the wall.
 return Math.min(maxRange,Math.max(0,obstruction*maxRange-.035));
}
function drawRadarVision(ctx,coord,g){
 if(g.knocked)return;
 const center=coord(g.x,g.z),steps=42;
 const cautious=g.state>0;
 // Soliton radar renders cone before structural outlines and character markers.
 ctx.beginPath();ctx.moveTo(center[0],center[1]);
 for(let i=0;i<=steps;i++){
  const angle=g.yaw-RADAR_CONE_HALF_ANGLE+(2*RADAR_CONE_HALF_ANGLE)*i/steps;
  const range=radarVisionDistance(g,angle);
  const c=coord(g.x+Math.cos(angle)*range,g.z+Math.sin(angle)*range);
  ctx.lineTo(c[0],c[1]);
 }
 ctx.closePath();ctx.fillStyle=cautious?'rgba(236,172,94,.31)':'rgba(143,215,164,.25)';ctx.fill();
 ctx.strokeStyle=cautious?'rgba(245,196,105,.77)':'rgba(148,219,174,.65)';
 ctx.lineWidth=.75;ctx.stroke();
 // Nearly touching guards is risky even from behind; occlusion still applies.
 ctx.beginPath();let drawing=false;
 for(let i=0;i<=48;i++){
  const a=Math.PI*2*i/48;
  const r=radarVisionDistance(g,a,RADAR_NEAR_RADIUS);
  const c=coord(g.x+Math.cos(a)*r,g.z+Math.sin(a)*r);
  if(!drawing){ctx.moveTo(c[0],c[1]);drawing=true;}else ctx.lineTo(c[0],c[1]);
 }
 ctx.closePath();ctx.strokeStyle=cautious?'#e7a4697b':'#7bbf9877';
 ctx.setLineDash([2,3]);ctx.lineWidth=.8;ctx.stroke();ctx.setLineDash([]);
 // An explicit heading arrow makes orientation readable even on a small HUD.
 const nose=coord(g.x+Math.cos(g.yaw)*.78,g.z+Math.sin(g.yaw)*.78);
 const left=coord(g.x+Math.cos(g.yaw+2.4)*.19,g.z+Math.sin(g.yaw+2.4)*.19);
 const right=coord(g.x+Math.cos(g.yaw-2.4)*.19,g.z+Math.sin(g.yaw-2.4)*.19);
 ctx.beginPath();ctx.moveTo(nose[0],nose[1]);ctx.lineTo(left[0],left[1]);
 ctx.lineTo(right[0],right[1]);ctx.closePath();ctx.fillStyle=cautious?'#ffe1a1':'#c5f7d5';ctx.fill();
}
// Body collision is independent of the character rendering. A prone soldier
// occupies a low, LONG capsule aligned with his facing, not the upright disc.
// The numbers are provisional human-scale tuning values in metres, NOT
// dimensions derived from the Connect2Media original edition.
const bodyShapes=Object.freeze({
  0:Object.freeze({height:1.78,radius:.33,halfSegment:0,pose:'standing'}),
  1:Object.freeze({height:1.14,radius:.34,halfSegment:0,pose:'crouched'}),
  2:Object.freeze({height:.67,radius:.31,halfSegment:.88,pose:'prone'})
});
function bodyShape(stance=player.stance){return bodyShapes[stance]||bodyShapes[0]}
function bodySamples(x,z,stance,yaw){
 const b=bodyShape(stance),c=Math.cos(yaw),s=Math.sin(yaw);
 const offsets=b.halfSegment?[-1,-.5,0,.5,1]:[0];
 return offsets.map(n=>({x:x+c*n*b.halfSegment,z:z+s*n*b.halfSegment,r:b.radius}));
}
function canOccupyBody(x,z,stance=player.stance,yaw=player.yaw){
 const b=bodyShape(stance);
 for(const sample of bodySamples(x,z,stance,yaw)){
  if(sample.x-sample.r<-32.2||sample.x+sample.r>29.1||
     sample.z-sample.r<-4.77||sample.z+sample.r>4.77)return false;
  for(const o of obstacles){
   if(!o.active||o.name==='concrete floor')continue;
   // Low obstacles block only if their actual vertical extent overlaps the pose.
   if(o.y-o.h/2>=b.height-.03||o.y+o.h/2<=.03)continue;
   const dx=Math.max(Math.abs(sample.x-o.x)-o.w/2,0);
   const dz=Math.max(Math.abs(sample.z-o.z)-o.d/2,0);
   if(dx*dx+dz*dz<sample.r*sample.r)return false;
  }
 }
 return true;
}
function setPlayerStance(next){
 if(next===player.stance)return true;
 if(!canOccupyBody(player.x,player.z,next,player.yaw)){
  popup(next===2?'NOT ENOUGH ROOM TO LIE PRONE':'NOT ENOUGH CLEARANCE TO RISE',2);
  playCue('denied');return false;
 }
 player.stance=next;
 if(next===2){player.cover=false;player.coverNormal=null}
 return true;
}
function movePlayer(vx,vz,dt){
 // Substep fast motion to avoid tunnelling through thin walls/doors.
 const dx=vx*dt,dz=vz*dt,steps=Math.max(1,Math.ceil(Math.hypot(dx,dz)/.13));
 for(let i=0;i<steps;i++){
  const x=player.x+dx/steps,z=player.z+dz/steps;
  if(canOccupyBody(x,player.z))player.x=x;
  if(canOccupyBody(player.x,z))player.z=z;
 }
}
// Guards still use upright circular capsules (separate from Snake's prone shape).
function collides(x,z,rad=.31){if(x<-32.2||x>29.1||z<-4.77||z>4.77)return true;return obstacles.some(o=>o.active&&o.name!=='concrete floor'&&x>o.x-o.w/2-rad&&x<o.x+o.w/2+rad&&z>o.z-o.d/2-rad&&z<o.z+o.d/2+rad)}
function moveEntity(o,vx,vz,dt,rad=.31){const xx=o.x+vx*dt,zz=o.z+vz*dt;if(!collides(xx,o.z,rad))o.x=xx;if(!collides(o.x,zz,rad))o.z=zz;}
function coverWall(o){
 // Find an actual vertical surface rather than using a blind collision probe.
 // Return the outward normal (wall to Snake) and acoustic impact point.
 // Low barriers can conceal a crouched soldier but cannot support standing
 // wall-press or knock animations.
 const needed=o.stance===1?.93:1.20;
 let best=null,nearest=Infinity;
 for(const wall of obstacles){
  if(!wall.active||wall.name==='concrete floor'||wall.y+wall.h*.5<needed)continue;
  const hw=wall.w*.5,hd=wall.d*.5;
  const qx=clamp(o.x,wall.x-hw,wall.x+hw),qz=clamp(o.z,wall.z-hd,wall.z+hd);
  const vx=o.x-qx,vz=o.z-qz,d=Math.hypot(vx,vz);
  if(d<.14||d>.98||d>=nearest)continue;
  // Reject shallow contact with horizontal/low props that aren't cover-height.
  const normal={x:vx/d,z:vz/d};
  nearest=d;
  best={...normal,surface:{x:qx,z:qz},object:wall.name};
 }
 return best;
}
// Both guards and Fox use solid-wall A*. Fox additionally penalises routes
// through visible firing lanes; he does not blindly sprint toward Snake.
const cell=.72;
function guardSeesPosition(g,pos,eyeHeight=.94){
 if(g.knocked)return false;
 const d=dist(g,pos);if(d>10.5)return false;
 const toward=Math.atan2(pos.z-g.z,pos.x-g.x);
 const view=Math.abs(Math.atan2(Math.sin(toward-g.yaw),Math.cos(toward-g.yaw)))<.96;
 return (view||d<1.55)&&!lineBlockedSight(g,pos,1.48,eyeHeight);
}
function foxRouteRisk(pos){
 let risk=0;
 for(const g of guards){
  if(g.knocked)continue;
  const d=dist(g,pos);
  if(d>11.5)continue;
  // Even if a sentry is facing away, being almost within arm's reach is risky.
  if(d<1.4)risk+=11;
  if(guardSeesPosition(g,pos))risk+=(g.state===3?21:15)*(1+Math.max(0,6-d)*.06);
 }
 return risk;
}
function pathfind(a,b,options={}){
 const avoidGuards=!!options.avoidGuards;
 const sx=Math.round(a.x/cell),sz=Math.round(a.z/cell),ex=Math.round(b.x/cell),ez=Math.round(b.z/cell);
 const frontier=[{x:sx,z:sz,k:0}],came=new Map(),seen=new Map(),riskCache=new Map();
 const start=sx+','+sz;seen.set(start,0);
 function danger(x,z){if(!avoidGuards)return 0;const key=x+','+z;
  if(!riskCache.has(key))riskCache.set(key,foxRouteRisk({x:x*cell,z:z*cell}));
  return riskCache.get(key);
 }
 for(let n=0;n<1700&&frontier.length;n++){
  frontier.sort((a,b)=>a.k-b.k);
  const c=frontier.shift(),id=c.x+','+c.z;
  if(c.x===ex&&c.z===ez){let out=[{x:c.x*cell,z:c.z*cell}],j=id;
   while(came.has(j)){const p=came.get(j);out.push({x:p.x*cell,z:p.z*cell});j=p.x+','+p.z;}
   return out.reverse().slice(1);
  }
  for(const [ax,az] of [[1,0],[-1,0],[0,1],[0,-1]]){
   const x=c.x+ax,z=c.z+az,k=x+','+z;
   if(collides(x*cell,z*cell,.32))continue;
   const g=seen.get(id)+1+danger(x,z);
   if(g>=(seen.get(k)??Infinity))continue;
   seen.set(k,g);came.set(k,{x:c.x,z:c.z});
   frontier.push({x,z,k:g+Math.abs(ex-x)+Math.abs(ez-z)});
  }
 }
 return [];
}
// Retain a distinct persistent escort character. He walks through the same
// collision + A* navigation system as guards rather than teleporting to Snake.
// A captive has no gun: self-preservation means crouching, evasion and cover.
function foxCoverSpot(){
 if(stage<2||!fox.alive)return null;
 const exposedNow=foxRouteRisk(fox);
 const trailing={x:player.x-Math.cos(player.yaw)*1.25,z:player.z-Math.sin(player.yaw)*1.25};
 // Evaluate the *next* position as well as his current location, including
 // patrolling guards who have not been alerted. Fox prepares before exposure.
 if(exposedNow<2&&foxRouteRisk(trailing)<2)return null;
 let best=null,score=Infinity;
 for(const c of coverAssets){
  for(const g of guards){
   if(g.knocked||dist(g,c)>12)continue;
   const away=nrm(c.x-g.x,c.z-g.z);
   if(Math.abs(away.x)+Math.abs(away.z)<.01)continue;
   const radius=Math.max(c.w,c.d)*.5+.65;
   const candidate={x:c.x+away.x*radius,z:c.z+away.z*radius};
   if(collides(candidate.x,candidate.z,.33))continue;
   const risk=foxRouteRisk(candidate);
   if(risk>1.5)continue;
   const distance=dist(candidate,fox);
   const cost=distance+Math.max(0,dist(candidate,player)-5.5)*.55;
   if(cost<score){best=candidate;score=cost;}
  }
 }
 return best;
}
function updateFox(dt){
 if(!fox.alive||missionFailed)return;
 if(stage<2||!escortMode){
  if(stage>=1&&dist(fox,player)<3.4&&!lineBlocked(fox,player))
   fox.yaw=lerpAngle(fox.yaw,Math.atan2(player.z-fox.z,player.x-fox.x),1-Math.exp(-dt*2.8));
  return;
 }
 fox.hitTimer=Math.max(0,fox.hitTimer-dt);
 const trailing={x:player.x-Math.cos(player.yaw)*1.25,
                 z:player.z-Math.sin(player.yaw)*1.25};
 const exposed=foxRouteRisk(fox)>2;
 const imminentExposure=foxRouteRisk(trailing)>2;
 const threat=exposed||imminentExposure;
 const cover=threat?foxCoverSpot():null;
 // Fox is independently stealthy. He won't start an ALERT; when Snake has
 // already triggered one, a guard with real line of sight can still shoot him.
 fox.stance=threat?1:0;
 const d=hypot(fox.x-player.x,fox.z-player.z);
 const target=cover||trailing;
 fox.mode=cover?'seeking_cover':(d>5?'regrouping':d>1.75?'following':'holding');
 if(fox.mode==='holding'||fox.hitTimer>.13){fox.moveSpeed=0;
   fox.yaw=lerpAngle(fox.yaw,Math.atan2(player.z-fox.z,player.x-fox.x),dt*2.5);return;}
 fox.pathTime-=dt;
 if(fox.pathTime<=0||!fox.waypoints.length||hypot(target.x-(fox.target?.x||0),target.z-(fox.target?.z||0))>.9){
   fox.waypoints=pathfind(fox,target,{avoidGuards:true});fox.target={...target};fox.pathTime=.58;
 }
 const next=fox.waypoints[0]||target;
 if(hypot(fox.x-next.x,fox.z-next.z)<.32)fox.waypoints.shift();
 else{
   const toward=nrm(next.x-fox.x,next.z-fox.z);
   const nextStep={x:fox.x+toward.x*.55,z:fox.z+toward.z*.55};
   // Do not leave a sheltered position directly into a guard's active sightline.
   // Hold briefly, crouched, then retry as the patrol turns or Snake intervenes.
   // It is not a mandatory player stealth gate: Snake may neutralise/distract
   // guards and Fox will resume the escort automatically.
   if(foxRouteRisk(nextStep)>8&&foxRouteRisk(fox)<3){
      fox.mode='waiting_for_clear_path';fox.stance=1;fox.moveSpeed=0;
      fox.pathTime=Math.min(fox.pathTime,.24);
      if(gameTime-fox.calloutAt>9){fox.calloutAt=gameTime;
        popup('GRAY FOX: HOLDING — ENEMY SIGHTLINE AHEAD',1.5);}
      return;
   }
   const speed=fox.mode==='regrouping'?3.65:fox.mode==='seeking_cover'?2.7:2.55;
   const ox=fox.x,oz=fox.z;
   moveEntity(fox,toward.x*speed,toward.z*speed,dt,.29);
   fox.moveSpeed=hypot(fox.x-ox,fox.z-oz)/(dt||1);
   if(fox.moveSpeed>.03)fox.yaw=lerpAngle(fox.yaw,Math.atan2(toward.z,toward.x),dt*8);
   // Fox's movement is audible to the player, but is not a stealth penalty
   // applied to Snake. This companion must never initiate guard suspicion.
   if(fox.moveSpeed>.14&&gameTime-fox.footstepAt>(fox.stance?.72:.48)){
      fox.footstepAt=gameTime;
      noiseEvents.push({x:fox.x,z:fox.z,source:'fox',power:.9,life:.20});
      if(hypot(fox.x-player.x,fox.z-player.z)<4.8)playCue('step');
   }
 }
 if(fox.mode==='seeking_cover'&&gameTime-fox.calloutAt>8){fox.calloutAt=gameTime;
    popup('GRAY FOX TAKING COVER',1.1);playCue('radio');}
}
function damageFox(amount){
 if(stage<2||stage===4||!fox.alive||missionFailed)return false;
 fox.hp=Math.max(0,fox.hp-Math.max(0,amount));fox.hitTimer=.46;
 fox.stance=1;playCue('hit');
 if(fox.hp<=0){fox.alive=false;fox.mode='down';missionFailed=true;
   say('escortFail');setPaused(true,'failed');
   $('failStats').textContent='GRAY FOX KIA — ESCORT FAILED';return true;}
 popup('GRAY FOX HIT — '+Math.ceil(fox.hp)+'% HEALTH · FIND COVER',2.2);
 return true;
}
// Shoot Gunner runs the property hall. He stands his ground while addressing
// Snake, commits to each shot, then changes position between marked stations.
const dutyStations=[{x:24.8,z:.6},{x:23.5,z:3.4},{x:20.1,z:-3.55}];
const duel={phase:'duty',age:0,station:0,barks:new Set(),remembered:null,
 lastHp:240,recoil:0,voice:0,heelTravel:0,staggerReady:0};
function bossBeat(phase){
 duel.phase=phase;duel.age=0;boss.pathTime=0;boss.waypoints=[];
}
function bossLine(id){
 if(duel.barks.has(id))return;
 duel.barks.add(id);say(id);
}
function bossWalk(target,speed,dt){
 boss.pathTime-=dt;
 if(boss.pathTime<=0){
  boss.waypoints=pathfind(boss,target);
  boss.pathTime=1.1;
 }
 while(boss.waypoints.length&&dist(boss,boss.waypoints[0])<.22)boss.waypoints.shift();
 const way=boss.waypoints[0]||target, remaining=dist(boss,way);
 if(remaining<.08)return;
 const direction=nrm(way.x-boss.x,way.z-boss.z);
 const pace=Math.min(speed,remaining/Math.max(.001,dt));
 const ox=boss.x,oz=boss.z;
 moveEntity(boss,direction.x*pace,direction.z*pace,dt,.32);
 duel.heelTravel+=Math.hypot(boss.x-ox,boss.z-oz);
 if(duel.heelTravel>.73){playCue('bossStep',boss);duel.heelTravel%=.73;}
 boss.yaw=lerpAngle(boss.yaw,Math.atan2(direction.z,direction.x),1-Math.exp(-5*dt));
}
function updateShootGunner(dt,pulse){
 if(stage!==4)return;
 duel.age+=dt;boss.shotVisual=Math.max(0,boss.shotVisual-dt);
 duel.recoil=Math.max(0,duel.recoil-dt*3.5);
 const inRoom=player.x>14.8&&player.x<27.3;
 const angle=Math.atan2(player.z-boss.z,player.x-boss.x);
 const distance=dist(boss,player);
 const facing=Math.cos(angle-boss.yaw);
 const sees=inRoom&&(facing>.12||distance<1.7)&&!lineBlockedSight(boss,player);
 if(sees)duel.remembered={x:player.x,z:player.z};
 const wounded=boss.hp<duel.lastHp;
 duel.lastHp=boss.hp;
 if(wounded){
  duel.recoil=1;
  if(gameTime>=duel.staggerReady){bossBeat('stagger');duel.staggerReady=gameTime+2.3;}
  if(boss.hp<=120)bossLine('bossWounded');
 }
 // A pulse is a single observed location. Between pulses he has no updates.
 if(pulse&&!sees&&!wounded&&['duty','search','cross','trace'].includes(duel.phase)&&
    pulse.x>14.8&&pulse.x<27.2&&dist(boss,pulse)<10.5){
  duel.signal={...pulse};
  bossBeat('trace');bossLine('bossTag');
 }
 const phase=duel.phase;
 boss.aimTime=phase==='shoulder'?duel.age:0;
 boss.state=phase==='duty'?0:phase==='search'?2:3;
 if(phase==='duty'){
  if(sees){bossLine('bossEntry');bossBeat('address');}
  return;
 }
 if(phase==='address'){
  if(sees)boss.yaw=lerpAngle(boss.yaw,angle,1-Math.exp(-3*dt));
  if(duel.age>=1.9)bossBeat(sees?'shoulder':'search');
 }else if(phase==='shoulder'){
  if(!sees){bossBeat('search');return;}
  boss.yaw=lerpAngle(boss.yaw,angle,1-Math.exp(-5*dt));
  if(duel.age>=1.05&&facing>.95){
   const y=player.stance===2?.48:player.stance===1?.92:1.52;
   shots.push({x:boss.x+.72*Math.cos(boss.yaw),z:boss.z+.72*Math.sin(boss.yaw),y:1.36,
    toX:player.x,toZ:player.z,toY:y,life:.17});
   boss.shotVisual=.18;duel.recoil=1;playCue('gunshot',boss);
   health=Math.max(0,health-25);popup('Under fire from the property hall.',1.1);
   bossBeat('recover');
  }
 }else if(phase==='recover'||phase==='stagger'){
  if(duel.age>=(phase==='stagger'?.60:1.20)){
   duel.station=(duel.station+1)%dutyStations.length;bossBeat('cross');
   if(phase==='recover')playCue('rack',boss);
  }
 }else if(phase==='cross'){
  const station=dutyStations[duel.station];
  bossWalk(station,boss.hp<=120?2.05:1.65,dt);
  if(dist(boss,station)<.45||duel.age>3.6)bossBeat(sees?'shoulder':'search');
 }else if(phase==='trace'){
  if(sees){
   if(duel.barks.has('bossEntry'))bossBeat('shoulder');
   else{bossLine('bossEntry');bossBeat('address');}
   return;
  }
  bossWalk(duel.signal,1.55,dt);
  if(dist(boss,duel.signal)<.78||duel.age>4.5){
   if(tag.state==='placed'&&dist(boss,tag.point)<.8){
    bossBeat('inspect');return;
   }
   bossBeat('search');
  }
 }else if(phase==='inspect'){
  if(sees){bossBeat('shoulder');return;}
  if(duel.age>=.95){
   tag.state='spent';save();bossLine('bossEmpty');bossBeat('search');
  }
 }else if(phase==='search'){
  if(sees){bossBeat('shoulder');return;}
  if(duel.remembered&&duel.age<2){
   const last=duel.remembered;
   boss.yaw=lerpAngle(boss.yaw,Math.atan2(last.z-boss.z,last.x-boss.x),1-Math.exp(-3*dt));
  }else{
   boss.yaw+=Math.sin(duel.age*1.6)*dt*1.3;
   if(inRoom&&duel.age>2.4)bossLine('bossSearch');
   if(duel.age>4){duel.station=(duel.station+1)%dutyStations.length;bossBeat('cross');}
  }
 }
}

function updateGuards(dt){const pulse=updatePropertyTag();let max=0;const triggered=[];for(const g of guards){if(g.knocked)continue;if(g.boss){updateShootGunner(dt,pulse);max=Math.max(max,g.state);continue;}const d=dist(g,player),direction=Math.atan2(player.z-g.z,player.x-g.x),view=Math.abs(Math.atan2(Math.sin(direction-g.yaw),Math.cos(direction-g.yaw)))<.87;const vision=d<10&&(view||d<1.65)&&!lineBlockedSight(g,player);
 const dFox=escortMode&&stage>=2&&fox.alive?dist(g,fox):Infinity;
 const toFox=Math.atan2(fox.z-g.z,fox.x-g.x);
 const foxFacing=Math.abs(Math.atan2(Math.sin(toFox-g.yaw),Math.cos(toFox-g.yaw)))<.94;
 // The companion never creates suspicion or starts an alert. Guards can
 // engage him only if Snake has already caused a genuine active alert.
 const foxVision=g.state===3&&dFox<9&&(foxFacing||dFox<1.7)&&
   !lineBlockedSight(g,fox,1.48,fox.stance===1?.86:1.47);
 const spotted=foxVision&&(!vision||dFox+1.1<d);
 g.aimSubject=spotted?'fox':'snake'; // 'fox' only while already ALERT
 const seenTarget=spotted?fox:player;
 const stanceFactor=player.stance===2?.21:player.stance===1?.48:1;const moving=inputDown('w')||inputDown('a')||inputDown('s')||inputDown('d')||padUsing;const footstepsNoise=moving?(player.stance===2?.35:player.stance===1?1.15:inputDown('shift')?6.3:2.4):0;
 const heard=d<footstepsNoise+1.3&&!lineBlocked(g,player);
 let eventNoise=null;
 for(const n of noiseEvents){
   if(n.source==='fox')continue;
   const acousticDistance=hypot(g.x-n.x,g.z-n.z);
   // Structure attenuates sound; no global wall-knock telepathy.
   const audibleRange=n.power*(lineBlocked(g,n)?.56:1);
   if(acousticDistance<audibleRange){eventNoise=n;break;}
 }
 let print=footsteps.find(f=>f.life>0&&hypot(g.x-f.x,g.z-f.z)<1.5);
 // This suspicion branch is strictly Snake-driven. Seeing Fox during an
 // existing alert may change a firing target, never start or extend alert.
 if(vision){const range=d;
   g.suspicion=clamp(g.suspicion+dt*(range<3?2.1:.78)*stanceFactor,0,1);
   g.last={x:player.x,z:player.z};
 }else if(heard){g.suspicion=clamp(g.suspicion+dt*.23,0,1);g.last={x:player.x,z:player.z}}else if(eventNoise){
    if(eventNoise.source==='knock'){
      if(g.state<2&&g.lastKnockId!==eventNoise.id){
        g.lastKnockId=eventNoise.id;
        g.suspicion=Math.max(g.suspicion,.46);
        g.last={x:eventNoise.x,z:eventNoise.z};
        g.investigateUntil=gameTime+9.5;
        g.scanYaw=null;
        g.pathTime=0;
        playCue('radio');
      }
    }else if(g.state<3){g.suspicion=Math.max(g.suspicion,.45);g.last={x:eventNoise.x,z:eventNoise.z}}
  }else if(print&&g.state<3){g.suspicion=Math.max(g.suspicion,.3);g.last={x:print.x,z:print.z}}else g.suspicion=Math.max(0,g.suspicion-dt*.075);
 if(g.postGuard&&stage<0&&g.hp===100&&!player.aim){g.suspicion=Math.min(g.suspicion,.4);g.state=Math.min(g.state,1)}
 if(g.suspicion>=.98){if(g.state!==3){playCue('alert');say('detected');triggered.push(g)}g.state=3;g.alert=7.7;g.radioTimer=1.8;}
 if(g.state===3&&!vision){g.alert-=dt;if(g.alert<=0){g.state=2;g.search=7;say('searched')}}
 else if(g.state===2){g.search-=dt;if(g.search<=0){g.state=1;g.caution=5}}
 else if(g.state===1&&g.suspicion<.25){g.caution-=dt;if(g.caution<=0){g.state=0;say('clear')}}
 else if(g.state===0&&g.suspicion>.22)g.state=1;
 // Local acoustic investigation is NOT full combat alert: patrol -> noise
 // location -> deliberate head scan -> return to post. A sighting can still
 // escalate normally. Search times are tunable, not hard-coded level gates.
 const investigating=g.state===1&&g.investigateUntil>gameTime;
 if(g.state===1&&g.investigateUntil&&g.investigateUntil<=gameTime){
   g.investigateUntil=0;g.suspicion=Math.min(g.suspicion,.13);g.state=0;
   g.pathTime=0;g.scanYaw=null;
 }
 const base=g.postGuard?post:g.yard?{x:-17.5,z:-3}:g.patrol===0?{x:8.0,z:3.15}:{x:11.8,z:-3.05};const alternate=g.postGuard?post:g.yard?{x:-20,z:3}:g.patrol===0?{x:11.7,z:3.0}:{x:7.1,z:-3.05};let target=g.state>=2||investigating?g.last:(g.patrol%2?alternate:base);if(g.state===0&&dist(g,target)<.55){g.patrol=1-g.patrol;target=g.patrol?alternate:base}
 g.pathTime-=dt;if(g.pathTime<=0||!g.waypoints.length||dist(target,g.target||{x:0,z:0})>1){g.waypoints=pathfind(g,target);g.target={...target};g.pathTime=.9}
 const next=g.waypoints[0]||target;
 if(investigating&&dist(g,target)<.82){
   if(g.scanYaw==null)g.scanYaw=g.yaw;
   g.yaw=lerpAngle(g.yaw,g.scanYaw+Math.sin(gameTime*2.4)*.72,dt*4.8);
 }else if(dist(g,next)<.4)g.waypoints.shift();else{
   g.scanYaw=null;
   const dir=nrm(next.x-g.x,next.z-g.z);const speed=g.state===3?2.3:g.state===2?1.7:1.07;
   const ox=g.x,oz=g.z;moveEntity(g,dir.x*speed*(g.aimTime>0?.35:1),dir.z*speed*(g.aimTime>0?.35:1),dt,.3);
   if(hypot(g.x-ox,g.z-oz)>.002)g.yaw=lerpAngle(g.yaw,Math.atan2(dir.z,dir.x),dt*5);
 }
 // The armed guard must see the player, face them, and maintain an aim
 // for 0.48s before firing. Crates, cabinets and walls interrupt the shot.
 g.fireCooldown=Math.max(0,g.fireCooldown-dt);
 g.shotVisual=Math.max(0,g.shotVisual-dt);
 if(g.state===3 && (vision||foxVision) && hypot(g.x-seenTarget.x,g.z-seenTarget.z)<9.7){
  const toPlayer=Math.atan2(seenTarget.z-g.z,seenTarget.x-g.x);
  g.yaw=lerpAngle(g.yaw,toPlayer,dt*5.5);
  const aligned=Math.abs(Math.atan2(Math.sin(toPlayer-g.yaw),Math.cos(toPlayer-g.yaw)))<.25;
  g.aimTime=aligned?Math.min(.8,g.aimTime+dt):0;
  if(g.aimTime>=.48 && g.fireCooldown<=0){
    g.warned=false;g.fireCooldown=1.35+(g.patrol===1?.28:0);
    g.aimTime=0;
    g.shotVisual=.18;
    shots.push({x:g.x+Math.cos(g.yaw)*.77,z:g.z+Math.sin(g.yaw)*.77,y:1.19,
      toX:seenTarget.x,toZ:seenTarget.z,toY:spotted?(fox.stance===1?.84:1.28):(player.stance===2?.48:player.stance===1?.84:1.33),life:.17});
    playCue('gunshot',g);
    noiseEvents.push({x:g.x,z:g.z,power:9,life:.5});
    // Revalidate on firing frame: no bullets through a closed wall or cover.
    const blocked=lineBlockedSight(g,seenTarget,1.48,spotted?(fox.stance===1?.84:1.28):(player.stance===2?.48:player.stance===1?.84:1.33));
    if(!blocked){
      if(spotted)damageFox(12);
      else{health=Math.max(0,health-12);popup('UNDER FIRE — BREAK SIGHTLINE',1.3);playCue('hit');}
    }else popup('ENEMY ROUND STOPPED BY COVER',1.1);
  }
 }else g.aimTime=Math.max(0,g.aimTime-dt*4);
 g.radioTimer-=dt;max=Math.max(max,g.state);
 }
 for(const g of triggered){for(const other of guards){if(g===other||other.knocked||other.boss)continue;if(hypot(g.x-other.x,g.z-other.z)<14){other.suspicion=Math.max(other.suspicion,.55);other.last={...g.last};other.state=Math.max(other.state,1)}}}
 awareness=max;
 if(health<=0){location.hash='continue';location.reload();return}
 if(stage===4&&boss.knocked){
   const landing={x:boss.x+.48*Math.sin(boss.yaw),z:boss.z-.48*Math.cos(boss.yaw)};
   cardDrop=validCardPosition(landing)?landing:validCardPosition(boss)?{x:boss.x,z:boss.z}:{...cardHome};
   dropTime=gameTime;
   dialogueQueue=dialogueQueue.filter(line=>!line.id.startsWith('boss'));
   progress(5);playCue('card',cardDrop);popup('A card falls from Shoot Gunner’s belt.');
 }

}
function lerpAngle(a,b,t){return a+Math.atan2(Math.sin(b-a),Math.cos(b-a))*clamp(t,0,1)}
function updatePlayer(dt){let mF=(inputDown('w')?1:0)-(inputDown('s')?1:0),mR=(inputDown('d')?1:0)-(inputDown('a')?1:0);const pad=activeGamepad();if(pad){const dz=v=>Math.abs(v)>.16?v:0;mF+=-dz(pad.axes[1]||0);mR+=dz(pad.axes[0]||0)}const m=nrm(mF,mR),moving=hypot(mF,mR)>.08;
 player.sprint=player.stance===0&&(inputDown('shift')||pressed(pad?.buttons[10]));player.aim=inputDown('mouse2')||pressed(pad?.buttons[6]);
 if(moving){let speed=player.stance===2?.95:player.stance===1?1.7:player.sprint?5.6:3.45;if(player.aim)speed*=.63;speed*=Math.min(1,Math.hypot(mF,mR));const f={x:Math.cos(cam.yaw),z:Math.sin(cam.yaw)},r={x:-f.z,z:f.x};let vx=(f.x*m.x+r.x*m.z)*speed,vz=(f.z*m.x+r.z*m.z)*speed;
 if(player.cover){
   const tangent={x:-player.coverNormal.z,z:player.coverNormal.x},along=vx*tangent.x+vz*tangent.z;
   vx=tangent.x*along;vz=tangent.z*along;
   const adjacent=coverWall(player);
   if(!adjacent){player.cover=false;player.coverNormal=null;}
   else player.coverNormal=adjacent;
 }
 // Prone transitions use the actual horizontal body footprint for both
 // translation and yaw. Stop rotation when a crate or wall blocks the legs.
 const wantedYaw=player.cover?Math.atan2(-player.coverNormal.z,-player.coverNormal.x):
  player.aim?lerpAngle(player.yaw,cam.yaw,dt*12):
  lerpAngle(player.yaw,Math.atan2(vz,vx),dt*11);
 if(player.stance!==2||canOccupyBody(player.x,player.z,2,wantedYaw))
  player.yaw=wantedYaw;
 movePlayer(vx,vz,dt);
 discovery.moved=true;
 if(gameTime-lastStep>(player.stance===2?.86:player.stance===1?.6:player.sprint?.28:.41)){lastStep=gameTime;footsteps.push({x:player.x,z:player.z,life:10});playCue('step')}
 }
 if(player.cover&&!coverWall(player)){player.cover=false;player.coverNormal=null;}
 if(player.cover&&player.coverNormal)player.yaw=lerpAngle(player.yaw,Math.atan2(-player.coverNormal.z,-player.coverNormal.x),dt*10);
 updateShutter(dt);
 if(stage===7&&!finalGate.active&&player.x>27.85&&Math.abs(player.z)<1.4){
   progress(8);discovery.finished=true;popup('Checkpoint secured. Locate Dr. Pettrovich.',5);
 }

}
// Segment vs. sphere-expanded 3D AABB. Exact entry time avoids skipped thin walls.
function cameraHitFraction(a,b,radius=.24){
 let earliest=1;
 const floorY=.29; // floor top y=0 + camera sphere + safety margin
 if(b.y<floorY&&a.y>floorY){earliest=Math.min(earliest,(a.y-floorY)/(a.y-b.y));}
 const ceilingY=3.36-radius-.06;
 if(b.y>ceilingY&&a.y<ceilingY){earliest=Math.min(earliest,(ceilingY-a.y)/(b.y-a.y));}

 for(const o of [...obstacles,...cameraOnlySolids]){
  if(!o.active||o.name==='concrete floor')continue;
  let lo=0,hi=1,valid=true;
  for(const [axis,half] of [['x','w'],['y','h'],['z','d']]){
   const min=o[axis]-o[half]*.5-radius,max=o[axis]+o[half]*.5+radius;
   const delta=b[axis]-a[axis];
   if(Math.abs(delta)<1e-9){if(a[axis]<min||a[axis]>max){valid=false;break}}
   else{
    let t1=(min-a[axis])/delta,t2=(max-a[axis])/delta;
    if(t1>t2)[t1,t2]=[t2,t1];lo=Math.max(lo,t1);hi=Math.min(hi,t2);
    if(lo>hi){valid=false;break}
   }
  }
  if(valid&&lo<=1&&hi>=0)earliest=Math.min(earliest,Math.max(0,lo));
 }
 return earliest;
}
function cameraSafePosition(focus,eye,radius=.24){
 const a={x:focus[0],y:focus[1],z:focus[2]},b={x:eye[0],y:eye[1],z:eye[2]};
 const length=Math.hypot(b.x-a.x,b.y-a.y,b.z-a.z);
 // Leave a few centimetres ahead of collision, including for high FOV camera.
 const fraction=Math.max(0,cameraHitFraction(a,b,radius)-Math.min(.09/Math.max(length,1e-3),.25));
 return [a.x+(b.x-a.x)*fraction,a.y+(b.y-a.y)*fraction,a.z+(b.z-a.z)*fraction];
}
function cameraPositionPenetrates(eye,radius=.24){
 if(eye[1]<.28||eye[1]>3.36-radius+.012)return true;
 return [...obstacles,...cameraOnlySolids].some(o=>o.active&&o.name!=='concrete floor'
  && Math.abs(eye[0]-o.x)<o.w*.5+radius
  && Math.abs(eye[1]-o.y)<o.h*.5+radius
  && Math.abs(eye[2]-o.z)<o.d*.5+radius);
}
function updateCamera(dt){const pad=activeGamepad();if(pad&&started&&!paused){const dz=v=>Math.abs(v)>.12?v:0;const rx=dz(pad.axes[2]||0),ry=dz(pad.axes[3]||0);if(rx||ry){cam.yaw+=rx*dt*2.4;cam.pitch=clamp(cam.pitch-ry*dt*1.65,-.75,.78);discovery.looked=true;}}
 const height=player.stance===2?.65:player.stance===1?1.14:1.56;
 cam.focusY=lerp(cam.focusY,height,clamp(dt*12,0,1));
 const wantedDistance=player.aim?2.7:player.stance===2?3.25:4.25;
 cam.dist=lerp(cam.dist,wantedDistance,clamp(dt*8,0,1));
 const dir={x:Math.cos(cam.yaw),z:Math.sin(cam.yaw)};
 const shoulder=cam.shoulder*(player.aim?.63:.45),h=cam.focusY;
 const focus=[player.x,h,player.z];
 const desired=[player.x-dir.x*cam.dist*Math.cos(cam.pitch)-dir.z*shoulder,
  h+1.1+Math.sin(cam.pitch)*cam.dist,
  player.z-dir.z*cam.dist*Math.cos(cam.pitch)+dir.x*shoulder];
 // First resolve the desired position against physical world solids.
 const safeDesired=cameraSafePosition(focus,desired);
 // Then smooth the actual camera, and resolve collision AGAIN. Smoothing by
 // itself can cross a wall on rapid turns, crouch or a checkpoint load.
 const alpha=clamp(dt*15,0,1);
 const smoothed=cam.eye.map((v,i)=>lerp(v,safeDesired[i],alpha));
 cam.eye=cameraSafePosition(focus,smoothed);
 // If a previous frame or checkpoint left the camera outside, snap to a safe
 // position instead of interpolating through a surface to return indoors.
 if(cameraPositionPenetrates(cam.eye))cam.eye=safeDesired;
 cam.eye[1]=clamp(cam.eye[1],.30,3.36-.24-.04);
 return focus;
}
let drawn=0;
function drawPrimitive(shape,pos,scale,tint,yaw=0,material=0,tilt=0){
 useMesh(shape);gl.uniform3f(loc.pos,...pos);gl.uniform3f(loc.scale,...scale);
 gl.uniform1f(loc.yaw,yaw);gl.uniform1f(loc.tilt,tilt);
 gl.uniform3f(loc.tint,...tint);gl.uniform1i(loc.mat,material);
 gl.drawArrays(gl.TRIANGLES,0,meshLibrary[shape].count);drawn++;
}
function drawCube(pos,scale,tint,yaw=0,material=0){drawPrimitive('cube',pos,scale,tint,yaw,material)}
function drawRound(pos,scale,tint,yaw=0,material=0,tilt=0){drawPrimitive('sphere',pos,scale,tint,yaw,material,tilt)}
function drawCylinder(pos,scale,tint,yaw=0,material=0,tilt=0){drawPrimitive('cylinder',pos,scale,tint,yaw,material,tilt)}
// Purpose-built low-poly original character prototypes, NOT imported Konami art.
// Characters share a common proportioned humanoid skeleton. Animated segments
// follow movement, stance and alert, and use rounded geometry, not block cubes.
const fieldActors=[];
let renderFields=null;
try {
 if(window.createFieldRenderer) renderFields=window.createFieldRenderer(gl,shader);
} catch(error) {
 console.warn('Field rig unavailable',error);
 $('rigStatus').textContent='SIMPLIFIED CHARACTERS ACTIVE';
}
function queueFieldActor(actor,kind,time,dt){
 const isSnake=kind==='player',isFox=kind==='fox', stance=actor.stance||0;
 const prior=actor.fieldPose||(actor.fieldPose={x:actor.x,z:actor.z,phase:0,stride:0,crouch:0,prone:0,down:actor.knocked||actor.alive===false?1:0,captive:isFox&&stage<2?1:0});
 const travel=Math.hypot(actor.x-prior.x,actor.z-prior.z);
 prior.x=actor.x;prior.z=actor.z;
 const step=dt>0&&travel<6?travel:0, blend=1-Math.exp(-dt*13);
 prior.phase+=step*(actor.sprint?4.1:5.5);
 prior.stride=lerp(prior.stride,clamp(step/Math.max(dt,.001)/2.6,0,1),blend);
 prior.crouch=lerp(prior.crouch,stance===1||actor.boss&&duel.phase==='inspect'?1:0,blend);
 prior.prone=lerp(prior.prone,stance===2?1:0,blend);
 if(actor.boss){
  const raising=duel.phase==='shoulder'?clamp(duel.age/.78,0,1):0;
  prior.focus=lerp(prior.focus||0,raising,1-Math.exp(-dt*12));
 }
 prior.captive=lerp(prior.captive,isFox&&stage<2?1:0,1-Math.exp(-dt*2.5));
 prior.down=lerp(prior.down,actor.knocked||actor.alive===false?1:0,blend);
 const strike=isSnake?Math.sin(Math.PI*clamp(((actor.attackUntil||0)-time)/.28,0,1)):0;
 const tap=isSnake?Math.sin(Math.PI*clamp(((actor.knockUntil||0)-time)/.25,0,1)):0;
 fieldActors.push({role:isSnake?'snake':isFox?'fox':actor.boss?'shoot_gunner':'guard',actor,gear:isSnake&&kitAvailable(),
  state:{...prior,time,strike,boss:!!actor.boss,
   order:actor.boss&&duel.phase==='address'?Math.sin(Math.PI*clamp(duel.age/1.9,0,1)):0,
   mouth:actor.boss&&duel.voice>time&&!actor.knocked?.45+.45*Math.sin(time*17):0,
   recoil:actor.boss?duel.recoil:0,hit:actor.hitTimer>0?1:0,
   aim:actor.boss?(prior.focus||0)*.8:actor.aim||actor.state===3?1:0,cover:actor.cover?.54+.46*tap:0}});
 const flat=Math.max(prior.prone,prior.down);
 gl.enable(gl.BLEND);gl.blendFunc(gl.SRC_ALPHA,gl.ONE_MINUS_SRC_ALPHA);gl.depthMask(false);
 drawPrimitive('shadowPlane',[actor.x,.012,actor.z],[.86+1.35*flat,.002,.74+.15*flat],[1,1,1],actor.yaw,5);
 gl.depthMask(true);gl.disable(gl.BLEND);
 if(actor.shotVisual>0){
  const muzzle=actor.yaw, intensity=actor.shotVisual/.18;
  drawRound([actor.x+.81*Math.cos(muzzle),1.25,actor.z+.81*Math.sin(muzzle)],
   [.18*intensity,.10*intensity,.10*intensity],[1,.74,.22],muzzle,3);
 }
 if(actor.state===3&&actor.aimTime>.10)
  drawRound([actor.x,1.99,actor.z],[.12,.09,.12],[.93,.57,.20],0,3);
}
function human(actor,kind,t,dt){
 if(renderFields&&actor!==informant){
  queueFieldActor(actor,kind,t,dt);
  return;
 }
 const isPlayer=kind==='player',isGuard=kind==='guard',alive=isGuard?!actor.knocked:actor.alive!==false;
 const x=actor.x,z=actor.z,yaw=actor.yaw||0,c=Math.cos(yaw),s=Math.sin(yaw);
 // Gait phase advances with ACTUAL displacement, not wall-clock time.
 // Blocking a character against a wall must stop the legs immediately.
 const travel=actor.renderLastX===undefined?0:Math.hypot(x-actor.renderLastX,z-actor.renderLastZ);
 actor.renderLastX=x;actor.renderLastZ=z;
 const actualSpeed=dt>0?clamp(travel/dt,0,6):0;
 const strideTarget=clamp(actualSpeed/(isPlayer&&player.sprint?5.3:kind==='guard'?1.9:2.7),0,1);
 const lerpAlpha=1-Math.exp(-Math.max(0,dt)/.12);
 actor.renderStride=lerp(actor.renderStride||0,strideTarget,lerpAlpha);
 actor.gaitPhase=(actor.gaitPhase||0)+travel*(isPlayer&&player.sprint?2.6:4.2);
 actor.renderCrouch=lerp(actor.renderCrouch||0,(isPlayer&&player.stance===1||kind==='fox'&&actor.stance===1)?1:0,lerpAlpha);
 actor.renderProne=lerp(actor.renderProne||0,isPlayer&&player.stance===2?1:0,lerpAlpha);
 const velocity=actor.renderStride,phase=actor.gaitPhase||0,stride=Math.sin(phase)*velocity;
 const stance=isPlayer?player.stance:0,prone=stance===2,crouch=stance===1;
 const uniform=actor===informant?[.60,.51,.35]:actor.boss?[.44,.31,.20]:isPlayer?[.24,.35,.29]:isGuard?[.27,.32,.25]:actor.hitTimer>0?[.60,.35,.29]:[.41,.39,.31];
 const uniformDark=isPlayer?[.16,.25,.23]:isGuard?[.18,.23,.19]:[.33,.30,.26];
 const equipment=[.13,.18,.17],leather=[.18,.17,.15],flesh=[.62,.47,.35];
 const armor=isPlayer?[.22,.28,.25]:isGuard?[.34,.35,.27]:uniform;
 const hair=isPlayer?[.14,.13,.12]:isGuard?[.12,.14,.13]:[.24,.20,.16];
 // Positions are local to actor; all attachments rotate around the same yaw.
 drawRound([x,.013,z],[alive?.84:1.26,.018,alive?.63:.53],[.105,.13,.12],yaw,mat.grate);
 const P=(a,b,d)=>[x+c*a-s*d,b,z+s*a+c*d];
 const oval=(px,py,pz,sx,sy,sz,clr,mat=0,tilt=0)=>drawRound(P(px,py,pz),[sx,sy,sz],clr,yaw,mat,tilt);
 const block=(px,py,pz,sx,sy,sz,clr,mat=2)=>drawCube(P(px,py,pz),[sx,sy,sz],clr,yaw,mat);
 const tube=(ax,ay,az,bx,by,bz,r,clr)=>{
  const mx=(ax+bx)/2,my=(ay+by)/2,mz=(az+bz)/2;
  const length=Math.hypot(bx-ax,by-ay,bz-az);
  const pitch=Math.atan2(bx-ax,by-ay);
  drawCylinder(P(mx,my,mz),[r*2,length,r*2],clr,yaw,2,pitch);
  oval(ax,ay,az,r*2,r*2,r*2,clr);
 };
 if(!alive){
  oval(0,.14,0,1.20,.30,.48,uniform,0,Math.PI/2);
  oval(.64,.16,0,.31,.27,.31,flesh);
  return;
 }
 // The visible torso CYLINDER physically tips from upright to prone as
 // the collider changes from a vertical footprint to a low longitudinal one.
 // Blend over ~0.2s so the action reads clearly instead of snapping poses.
 if((isPlayer&&actor.renderProne>.008)||prone){
  const p=clamp(actor.renderProne,0,1);
  const cy=lerp(1.04,.43,p),tilt=p*Math.PI/2;
  const trunkLength=lerp(.78,1.09,p);
  drawCylinder(P(.02,cy,0),[lerp(.59,.52,p),trunkLength,lerp(.48,.59,p)],uniform,yaw,2,tilt);
  oval(lerp(.04,.40,p),lerp(1.28,.47,p),0,
       lerp(.52,.58,p),lerp(.35,.27,p),lerp(.58,.65,p),armor,2,tilt);
  oval(lerp(.03,.75,p),lerp(1.66,.50,p),0,
       lerp(.40,.34,p),lerp(.46,.30,p),.36,flesh);
  oval(lerp(-.055,.82,p),lerp(1.83,.62,p),0,
       .35,lerp(.17,.12,p),.37,hair);
  for(const side of [-1,1]){
   const hip=[lerp(-.05,-.40,p),lerp(.88,.35,p),side*.19];
   const knee=[lerp(.08,-.83,p),lerp(.50,.31,p),side*.20];
   const ankle=[lerp(-.08,-1.02,p),lerp(.16,.25,p),side*.20];
   tube(...hip,...knee,.125,uniformDark);
   tube(...knee,...ankle,.115,uniformDark);
   oval(lerp(.04,-1.08,p),lerp(.11,.25,p),side*.20,.35,.18,.22,leather);
   const shoulder=[lerp(-.01,.34,p),lerp(1.42,.38,p),side*.29];
   const elbow=[lerp(-.05,.60,p),lerp(.99,.31,p),side*.37];
   const hand=[lerp(.10,.93,p),lerp(.87,.19,p),side*.41];
   tube(...shoulder,...elbow,.12,uniform);
   tube(...elbow,...hand,.10,uniformDark);
   oval(...hand,.18,.15,.18,equipment);
  }
  return;
 }
 const bend=isPlayer||kind==='fox'?actor.renderCrouch:0;
 const hips=lerp(.92,.64,bend),shoulder=lerp(1.36,1.07,bend),head=lerp(1.68,1.30,bend);
 const bounce=velocity*Math.abs(Math.sin(phase))*.026;
 const strideAmount=lerp(.39,.20,bend);
 const armSwing=stride*.24*(isGuard&&actor.state>=2?.26:1);
 const strike=isPlayer&&actor.attackUntil>t?Math.sin(Math.PI*(1-(actor.attackUntil-t)/.28)):0;
 const flinch=kind==='fox'&&actor.hitTimer>0?Math.sin(actor.hitTimer*17)*.13:0;
 const aimPose=(isPlayer&&actor.aim)||(isGuard&&actor.state>=3);
 oval(0,hips,0,.55,.43,.48,uniformDark);
 oval(-.015,shoulder-.13+bounce,0,.67,.77,.46,uniform,2);
 oval(.06,shoulder+.07+bounce,0,.53,.38,.58,armor,2);
 // Shoulder seams, vest straps and identifiable olive 1995 tactical silhouette.
 for(const side of [-1,1]){
  block(.03,shoulder+.11,side*.21,.55,.064,.07,equipment);
  block(.00,hips-.06,side*.25,.27,.23,.22,equipment);
  const hip=[-.035,hips-.07,side*.18];
  const legPhase=phase+(side>0?Math.PI:0);
  const legSwing=Math.sin(legPhase)*velocity;
  const footLift=Math.max(0,Math.cos(legPhase))*velocity*.16;
  const knee=[.08+legSwing*strideAmount*.42,lerp(.50,.37,bend)+footLift*.65,side*.17];
  const ankle=[-.08+legSwing*strideAmount,lerp(.16,.17,bend)+footLift,side*.19];
  tube(...hip,...knee,lerp(.15,.13,bend),uniform);
  oval(...knee,.23,.23,.24,uniformDark);
  tube(...knee,...ankle,.125,uniformDark);
  oval(ankle[0]+.11,.12,ankle[2],.38,.23,.25,leather);
  const sh=[-.01,shoulder+.16,side*.37];
  const wallBrace=isPlayer&&actor.cover&&side>0;
  const wallTap=wallBrace&&actor.knockUntil>t?Math.sin(Math.PI*clamp((actor.knockUntil-t)/.25,0,1))*.23:0;
  const elbow=wallBrace?[.19,shoulder-.15,side*.40]:
    [-.055+armSwing*(-side)+strike*(side>0?.29:0)+(aimPose?.11:0)-flinch,
     shoulder-.24+(aimPose?.16:0)-Math.abs(flinch),side*.45];
  const hand=wallBrace?[.46+wallTap,shoulder-.21,side*.37]:
    [.10+armSwing*1.4*(-side)+strike*(side>0?.52:0)+(aimPose?.46:0),
     hips-.01+strike*(side>0?.32:0)+(aimPose?.32:0),side*.43];
  tube(...sh,...elbow,.13,uniform);
  tube(...elbow,...hand,.105,uniformDark);
  oval(...hand,.18,.22,.18,equipment);
 }
 // Face is separate from hair/helmet. Guard headgear is utilitarian, not MGS2 gear.
 tube(.00,shoulder+.30,0,.03,head-.12,0,.10,flesh);
 oval(.03,head,0,.39,.47,.37,flesh);
 if(isGuard){
  oval(-.035,head+.18,0,.48,.23,.45,hair);
  block(.05,head+.10,0,.46,.07,.48,armor);
  oval(.24,head-.01,0,.10,.12,.33,equipment);
 }else{
  oval(-.055,head+.21,0,.41,.17,.41,hair);
  block(-.02,head+.10,0,.41,.065,.415,isPlayer?[.12,.19,.18]:uniformDark);
  if(isPlayer){
   const flutter=.045*Math.sin(t*5+phase)*(.25+velocity);
   tube(-.18,head+.12,-.09,-.39,head-.01+flutter,-.11,.036,[.14,.21,.18]);
   tube(-.19,head+.11,-.07,-.43,head+.02-flutter,-.04,.033,[.14,.21,.18]);
   block(-.20,head-.005,.21,.12,.16,.14,equipment); // communications earpiece
   block(-.19,head-.09,.31,.16,.035,.035,[.23,.30,.27]);
  }
 }
 // Minimal face geometry at the local forward (+X), distinct brow and nose.
 oval(.24,head+.015,0,.20,.17,.18,flesh);
 block(.25,head+.07,0,.045,.045,.22,hair);
 if(isPlayer&&kitAvailable()){
  block(-.18,shoulder-.14,0,.20,.50,.50,equipment); // strap/utility pack
  block(.18,hips-.12,.35,.16,.25,.13,equipment);
  block(.18,hips-.12,-.35,.16,.25,.13,equipment);
 }else if(isGuard){
  block(-.20,shoulder-.12,0,.22,.50,.45,equipment);
  block(.18,shoulder-.18,0,.11,.11,.37,leather); // radio pouch
  // Distinct 1995-appropriate generic carbine silhouette. Not an extracted game model.
  block(.32,shoulder-.34,.22,.54,.17,.14,[.13,.16,.17]); // receiver
  block(.70,shoulder-.31,.22,.40,.085,.085,[.09,.13,.13]); // barrel
  block(.13,shoulder-.48,.22,.12,.31,.12,leather); // grip
  block(-.02,shoulder-.34,.22,.24,.13,.15,leather); // stock
  if(actor.shotVisual>0){
    const flash=actor.shotVisual/.18;
    oval(1.01,shoulder-.31,.22,.22*flash,.16*flash,.16*flash,[1,.74,.22],3);
  }
  if(actor.aimTime>.10 && actor.state===3){
    oval(.48,head+.43,0,.12,.09,.12,[.93,.57,.20],3);
  }
 }else{
  block(.23,hips+.08,0,.06,.20,.36,uniformDark); // captive garment seam
 }
}
// The stripe and three raised bars identify the item in muted light.
function drawPropertyTag(x,y,z,t){
 const lit=tag.flash<=t&&t-tag.flash<.4&&tag.state!=='spent';
 drawPrimitive('bevel',[x,y,z],[.14,.044,.095],[.38,.45,.39],0,mat.steel);
 drawCube([x+.033,y+.024,z],[.041,.007,.031],lit?[.98,.68,.20]:[.18,.20,.15],0,lit?mat.light:mat.steel);
}
function drawAccessCard(x,y,z,yaw,flat){
 const c=Math.cos(yaw),s=Math.sin(yaw);
 function piece(forward,up,side,depth,height,width,colour){
  const dx=flat?up:forward,dy=flat?forward:up;
  drawPrimitive('bevel',[x+c*dx-s*side,y+dy,z+s*dx+c*side],
   flat?[height,depth,width]:[depth,height,width],colour,yaw,mat.steel);
 }
 piece(0,0,0,.016,.205,.15,[.84,.80,.60]);
 piece(.009,.052,0,.006,.045,.141,[.20,.39,.39]);
 for(const side of [-.038,0,.038])piece(.012,-.025,side,.007,.048,.012,[.18,.19,.17]);
}
function cameraPlanes(m){
 const planes=[];
 for(let axis=0;axis<3;axis++)for(const sign of [-1,1]){
  const p=[0,1,2,3].map(col=>m[col*4+3]+sign*m[col*4+axis]);
  const length=Math.hypot(p[0],p[1],p[2]);planes.push(p.map(v=>v/length));
 }
 return planes;
}
function drawFrame(t,dt){let w=canvas.clientWidth,h=canvas.clientHeight,scale=window.devicePixelRatio>1?Math.min(1.5,window.devicePixelRatio):1;let ww=Math.max(2,Math.floor(w*scale)),hh=Math.max(2,Math.floor(h*scale));if(canvas.width!==ww||canvas.height!==hh){canvas.width=ww;canvas.height=hh;gl.viewport(0,0,ww,hh)}const target=updateCamera(dt);const pv=m4.mul(m4.persp(Math.PI/(player.aim?3.6:3.1),ww/hh,.075,120),m4.look(cam.eye,target));gl.clear(gl.COLOR_BUFFER_BIT|gl.DEPTH_BUFFER_BIT);gl.useProgram(program);gl.uniformMatrix4fv(loc.pv,false,new Float32Array(pv));gl.uniform3f(loc.eye,...cam.eye);gl.uniform1f(loc.time,t);drawn=0;
 const planes=cameraPlanes(pv);
 for(const o of geometry)if(o.active){
 const radius=o.boundRadius??(o.boundRadius=.5*Math.hypot(o.w,o.h,o.d));
 if(planes.some(p=>p[0]*o.x+p[1]*o.y+p[2]*o.z+p[3]<-radius))continue;
 const tint=(awareness>=3 && o.name==='lamp glow') ?
   ((Math.floor(t*3.2)%2)===0?[.9,.28,.16]:o.color):o.color;
 drawPrimitive(o.mesh,[o.x,o.y,o.z],[o.w,o.h,o.d],tint,o.yaw,o.material,o.tilt);
}
 if(tag.state==='carried'&&player.stance!==2){
  const c=Math.cos(player.yaw),s=Math.sin(player.yaw);
  drawPropertyTag(player.x-.17*c-.10*s,player.stance===1?.64:.94,player.z-.17*s+.10*c,t);
 }
 if(stage<0)drawPropertyTag(-14.55,1.032,-1.12,t);
 if(tag.point)drawPropertyTag(tag.point.x,.029,tag.point.z,t);
 if(stage<0){
   human(informant,'fox',t,dt);

 }
 if(stage>=4&&!boss.knocked){
   const reach=.66, angle=boss.yaw;
   drawAccessCard(boss.x+.25*Math.cos(angle)+.17*Math.sin(angle),.94,
    boss.z+.25*Math.sin(angle)-.17*Math.cos(angle),angle,false);
   if(!renderFields)drawCube([boss.x+reach*Math.cos(angle),boss.aimTime>.1?1.46:1.18,boss.z+reach*Math.sin(angle)],
     [.83,.13,.13],[.15,.17,.16],angle,2);
 }
 if(stage===5){
  const fall=clamp((t-dropTime)/.38,0,1);
  drawAccessCard(cardDrop.x,.035+.84*(1-fall*fall),cardDrop.z,.36,fall>.85);
 }
 for(const f of footsteps)drawCube([f.x,.007,f.z],[.28,.009,.16],[.24,.22,.16]);
 for(const p of particles)drawCube([p.x,p.y,p.z],[.08,.07,.08],p.c);
 if(Math.hypot(cam.eye[0]-player.x,cam.eye[1]-(player.stance===2?.65:player.stance===1?1.14:1.56),cam.eye[2]-player.z)>.88)(!window.__MG_RIG_READY || (player.stance===2&&!window.__MG_RIG_PRONE) || (player.stance===1&&!window.__MG_RIG_CROUCH))&&human(player,'player',t,dt);
 // Same persistent Gray Fox actor before and after rescue. No visual duplicate,
 // teleport, or proxy left behind when the player leaves the prison.
 if(!window.__MG_RIG_READY || (fox.stance===1&&!window.__MG_RIG_CROUCH))human(fox,'fox',t,dt);
 if(escortMode&&stage>=2&&fox.alive){
   // World-space health indicator ties the HUD to Fox's physical position.
   const remaining=clamp(fox.hp/100,0,1);
   drawCube([fox.x,2.07,fox.z],[1.10,.065,.11],[.07,.14,.16],0,2);
   if(remaining>.001)drawCube([fox.x-.52+remaining*.52,2.07,fox.z-.064],
     [remaining*1.04,.047,.075],remaining<.36?[.94,.35,.28]:[.42,.83,.90],0,3);
 }
 for(const g of guards)if(g.boss||!window.__MG_RIG_READY)human(g,'guard',t,dt);
 // Brief visible tracers communicate the firing direction. Damage logic uses
 // lineBlockedSight, not these visual elements.
 for(const shot of shots){
   const dx=shot.toX-shot.x,dz=shot.toZ-shot.z,dy=shot.toY-shot.y;
   const planar=Math.hypot(dx,dz),length=Math.hypot(planar,dy);
   const yaw=Math.atan2(dz,dx),tilt=Math.atan2(planar,dy);
   drawCylinder([(shot.x+shot.toX)*.5,(shot.y+shot.toY)*.5,(shot.z+shot.toZ)*.5],
     [.025,length,.025],[.91,.73,.32],yaw,3,tilt);
 }
 if(fieldActors.length){
  renderFields(fieldActors,new Float32Array(pv));fieldActors.length=0;
  gl.useProgram(program);currentMesh='';
 }
 // Render actual GLB skinned models in an optional transparent Three.js pass.
 // The original renderer and all gameplay systems remain authoritative.
 if(window.__MG_RIG_READY){
   const a=[{id:'snake',kind:'player',x:player.x,z:player.z,yaw:player.yaw||0,
     stance:player.stance,cover:player.cover,knocking:player.knockUntil>t,hit:false,down:false,visible:Math.hypot(cam.eye[0]-player.x,cam.eye[2]-player.z)>.75},
     {id:'fox',kind:'fox',x:fox.x,z:fox.z,yaw:fox.yaw||0,
       stance:fox.stance||0,hit:fox.hitTimer>0,down:!fox.alive,visible:true}];
   for(let i=0;i<guards.length;i++){
     const g=guards[i];if(g.boss)continue;a.push({id:'guard'+i,kind:'guard',x:g.x,z:g.z,yaw:g.yaw||0,
       stance:0,hit:!!g.hitTimer,down:!!g.knocked,visible:true});
   }
   const solids=[...obstacles.filter(o=>o.active),
    ...cameraOnlySolids.filter(o=>o.active)];
   window.dispatchEvent(new CustomEvent('mg:rig-frame',{detail:{
     dt,camera:{eye:[...cam.eye],target:[...target],fov:Math.PI/(player.aim?3.6:3.1)},
     actors:a,solids:solids.map((o,i)=>({id:o.name+'-'+i,x:o.x,y:o.y,z:o.z,
       w:o.w,h:o.h,d:o.d,solid:obstacles.includes(o),cameraSolid:cameraOnlySolids.includes(o),active:o.active,yaw:o.yaw||0}))
   }}));
 }
}
const chapters=[
 ['INFILTRATION','Speak to the prisoner under the yard light.'],
 ['THE APPROACH','Reach the processing post. Lower your weapon to surrender.'],
 ['SOLITARY','Search your cell for a way out.'],
 ['CONTACT','Find the prisoner in the neighbouring cell.'],
 ['GRAY FOX','Find a way from solitary into the guard passage.'],
 ['PERSONAL EFFECTS','Retrieve your kit from the property locker.'],
 ['SHOOT GUNNER','Cross the shutter and overcome Shoot Gunner.'],
 ['AFTERMATH','Recover the access card beside Shoot Gunner.'],
 ['CARD 3','Use Card 3 at the reader beside the east shutter.'],
 ['EXIT CLEAR','Leave by the east shutter when ready.'],
 ['CHECKPOINT SECURED','Next objective · Locate Dr. Pettrovich.']
];
function updateHUD(dt){
 const [phase,objective]=chapters[stage+2];$('phase').textContent=phase;$('obj').textContent=objective;
 const sample=[player.x,player.z,cam.yaw,cam.pitch];
 if(!movementSample||sample.some((v,i)=>Math.abs(v-movementSample[i])>(i<2?.05:.025))){
  movementSample=sample;quietSince=gameTime;
 }
 const silence=gameTime-quietSince;
 const hintEligible=elapsedStage>=28&&(silence>=6||elapsedStage>=100);
 $('hint').textContent=stage>=0&&stage<3&&awareness<2&&discovery.showTips&&hintEligible?
   window.MGTutorial.getHint({...discovery,stage,elapsedStage:elapsedStage>=75&&(silence>=10||elapsedStage>=150)?75:28}):'';
 const pulseCaption=$('propertyCaption');
 const audible=gameTime-tag.flash<1.55;
 pulseCaption.textContent=audible&&stage===-1?'Two chirps · processing desk':
  audible&&stage===4&&tag.state==='carried'?'Two chirps · inside your kit':
  audible&&stage===4&&tag.state==='placed'&&dist(player,tag.point)<6?'Two chirps · loose tag':'';
 let prompt='';
 if(stage===-2&&dist(player,informant)<2.2)prompt='E / A · TALK';
 if(stage===-1&&dist(player,post)<2.1)prompt=arrestGuard.knocked?'E / A · CALL THE DUTY GUARD AND SURRENDER':'E / A · SURRENDER';
 if(stage===1&&dist(player,fox)<2.1)prompt='E / A · SPEAK TO FOX';
 if(stage===3&&dist(player,gearLocker)<2)prompt='E / A · RECOVER KIT';
 const accessPrompt=exitInteraction();
 if(accessPrompt==='debrief')prompt='E / A · REVIEW MISSION';
 if(accessPrompt==='take')prompt='E / A · TAKE CARD 3';
 if(accessPrompt==='scan')prompt='E / A · PRESENT CARD 3';
 if(accessPrompt==='locked')prompt='E / A · INSPECT READER';
 $('prompt').textContent=prompt;$('prompt').style.opacity=prompt?'1':'0';
 $('tutorial').style.opacity='0';
 const alertState=awareness>=3?'ALERT':awareness===2?'EVASION':awareness===1?'CAUTION':'NORMAL';$('alert').textContent=awareness>=3?'ENEMY ALERT — SEEK COVER':awareness===2?'EVASION — LAST KNOWN LOCATION':'CAUTION — GUARDS INVESTIGATING';$('alert').style.opacity=awareness>=1?'1':'0';$('alert').style.color=awareness>=3?'#ff6963':'#ecca7b';$('radarTitle').textContent=awareness>=3?'RADAR JAMMED / ALERT':awareness===2?'RADAR OFFLINE / SEARCH':'FIELD MAP / '+alertState;
 $('reticle').style.opacity=player.aim?'1':'0';
 const threat=guards.filter(g=>!g.knocked&&g.state===3&&g.aimTime>.1);
 const coverHint=$('coverFeedback');
 if(coverHint){
  const nearby=guards.filter(g=>!g.knocked&&g.state>=2&&dist(g,player)<10);
  const shield=nearby.length>0&&nearby.every(g=>lineBlockedSight(g,player));
  coverHint.textContent=awareness>=1 ? (shield?'COVER EFFECTIVE · ENEMY SIGHT OBSTRUCTED':
   (threat.length?'ENEMY AIMING · MOVE BEHIND COVER':'EXPOSED · FIND SOLID COVER')) :
   (stage===3?'STEALTH ROUTE · TALL CABINETS + CROUCH COVER':'');
  if(stage===4&&player.x>14.8&&duel.phase==='shoulder')
   coverHint.textContent='SHOOT GUNNER RAISES HIS WEAPON';
  coverHint.dataset.safe=shield?'yes':'no';
 }
 const healthGauge=$('lifeFill');if(healthGauge)healthGauge.style.width=clamp(health,0,100)+'%';if($('lifeHP'))$('lifeHP').textContent=Math.ceil(health)+' / 100';
 const foxHUD=$('foxBox');if(foxHUD){foxHUD.style.display=escortMode&&stage>=2?'block':'none';
  $('foxFill').style.width=clamp(fox.hp,0,100)+'%';
  $('foxHP').textContent=Math.ceil(fox.hp)+' / 100';
  $('foxTactics').textContent=fox.mode.replaceAll('_',' ').toUpperCase()+(stage===3?' · '+Math.round(dist(player,fox)*10)/10+'M FROM SNAKE':'');
  foxHUD.dataset.danger=fox.hp<=35?'yes':'no';}
$('status').textContent=`LIFE ${Math.ceil(health)} / 100   |   ${player.stance===2?'PRONE':player.stance===1?'CROUCHED':'STANDING'}${player.cover?' / COVER':''}\nEQUIPMENT: ${kitAvailable()?equippedWeapon+' / '+stunAmmo+' CHARGES':'CONFISCATED'}   |   ${alertState}${message?'\n'+message.text:''}`;
 $('controls').textContent=lastInputIsPad&&framePad?'A Interact   ·   B Crouch   ·   Y Prone   ·   Menu Pause':'E Interact   ·   Ctrl Crouch   ·   Z Prone   ·   Esc Pause';
 if(!paused)displayNextDialogue(dt);
 const canvas2=$('radar'),ctx=canvas2.getContext('2d');ctx.clearRect(0,0,182,140);ctx.fillStyle='#0e2424';ctx.fillRect(0,0,182,140);if(awareness>=2){ctx.fillStyle=awareness>=3?'#c25e51':'#c7a36b';ctx.font='bold 18px monospace';ctx.textAlign='center';ctx.fillText(awareness>=3?'ALERT':'SEARCH',91,65);ctx.font='9px monospace';ctx.fillText('SIGNAL UNAVAILABLE',91,89);return;}
 const coord=(x,z)=>[7+(x+28.6)/58.1*168,8+(z+5.1)/10.2*124];
 ctx.save();ctx.beginPath();ctx.rect(6,7,169,125);ctx.clip();
 // World space guard cones are drawn before walls, so opaque structure is
 // legible and the player can see where solid cover interrupts guard vision.
 for(const g of guards)drawRadarVision(ctx,coord,g);
 ctx.strokeStyle='#587469';ctx.lineWidth=1;ctx.strokeRect(6,7,169,125);
 for(const o of geometry){if(!o.active||!o.name.includes('boundary')&&!o.name.includes('divider')&&!o.name.includes('crate')&&!o.name.includes('partition')&&!o.name.includes('damaged'))continue;const [x,z]=coord(o.x-o.w/2,o.z-o.d/2);ctx.fillStyle='#3c665c';ctx.fillRect(x,z,o.w/58.1*168,Math.max(1,o.d/10.2*124))}
 const p=coord(player.x,player.z);
 // Player heading arrow, with a high-contrast dot distinguishing Fox (blue).
 const playerForward=coord(player.x+Math.cos(player.yaw)*.72,player.z+Math.sin(player.yaw)*.72);
 ctx.strokeStyle='#b1dfb4';ctx.lineWidth=1.5;ctx.beginPath();ctx.moveTo(p[0],p[1]);ctx.lineTo(playerForward[0],playerForward[1]);ctx.stroke();
 ctx.fillStyle='#b1dfb4';ctx.beginPath();ctx.arc(p[0],p[1],4,0,Math.PI*2);ctx.fill();
 if(escortMode&&stage>=2&&fox.alive){const q=coord(fox.x,fox.z);ctx.fillStyle='#7fc5e3';ctx.beginPath();ctx.arc(q[0],q[1],3.6,0,Math.PI*2);ctx.fill();}
 for(const g of guards){if(g.knocked)continue;const c=coord(g.x,g.z);ctx.fillStyle=g.state>=1?'#f7b96b':'#e58a79';ctx.beginPath();ctx.arc(c[0],c[1],2.6,0,Math.PI*2);ctx.fill()}
 ctx.restore();
}
function drawEquipment(){const panel=$('equip');panel.style.display=equipOpen?'block':'none';if(!equipOpen)return;const slots=inventories[equipOpen];panel.innerHTML=`<div class="kicker">${equipOpen==='items'?'LEFT / ITEMS':'RIGHT / WEAPONS'} &nbsp;|&nbsp; TIME ×0.2</div>`+slots.map((v,i)=>`<div class="slot ${i===eqIndex?'picked ':''}${v.usable?'':'locked'}">${i===eqIndex?'► ':''}${v.name}${v.usable?'':' · LOCKED'}</div>`).join('')+(slots[eqIndex]===tagSlot?'<small>Stamped at processing. E / A · Set down</small>':'')+'<small>Hold LB / RB or Tab / G. D-pad left/right or [ ] selects. Release to close.</small>'}
function setPaused(value,type){paused=!!value;$('pause').style.display='none';$('intel').style.display='none';$('complete').style.display='none';$('missionFail').style.display='none';if(value){if(type==='complete'){$('complete').style.display='flex';paused=true}else if(type==='failed'){$('missionFail').style.display='flex';paused=true}else if(type==='intel'){showIntel()}else{$('pause').style.display='flex'}if(document.pointerLockElement)document.exitPointerLock?.()}else{if(canvas.requestPointerLock)Promise.resolve(canvas.requestPointerLock()).catch(()=>{})}}
function showIntel(){
 paused=true;$('intel').style.display='flex';$('pause').style.display='none';
 $('intelText').textContent=stage>=2?['rescue1','rescue2','rescue3'].map(id=>window.MGDialogue[id].speaker+' — '+window.MGDialogue[id].text).join('  '):
 stage>=-1?window.MGDialogue.informant.text:'Find a prisoner who knows where Fox is held.';
}
function start(resume){
 $('intro').style.display='none';showIntro=false;started=true;initAudio();
 if(!resume||!load()){
  const cellStart=new URLSearchParams(location.search).has('cell');
  stage=cellStart?0:-2;player.x=cellStart?-10.6:-27;player.z=0;elapsedStage=0;
  stunAmmo=6;equippedWeapon=cellStart?'BARE HANDS':'STUN PISTOL';syncEquipment();
  if(rehearsal){
   stage=4;player.x=15.75;player.z=.5;recoveredGear=true;
   equippedWeapon=sealedWeapon='STUN PISTOL';cam.yaw=0;armPropertyTag();
   activateBreach(breach1,seam1,false);activateBreach(breach2,seam2,false);
   syncEquipment();syncBoss();
  }else if(cellStart)say('capture');
  save();
 }
 last=performance.now();cam.eye=cameraSafePosition([player.x,player.stance?1.14:1.56,player.z],[player.x-2.8,2.2,player.z+1.0]);if(canvas.requestPointerLock)Promise.resolve(canvas.requestPointerLock()).catch(()=>{});}
$('rehearse').onclick=()=>{location.search='?boss'};$('start').onclick=()=>start(false);$('resume').onclick=()=>start(true);$('brief').onclick=()=>$('briefing').style.display='flex';$('backBrief').onclick=()=>$('briefing').style.display='none';$('unpause').onclick=()=>setPaused(false);$('pauseIntel').onclick=()=>action('intel');$('restart').onclick=reset;$('retry').onclick=reset;$('retryEscort').onclick=()=>location.reload();$('finishIntel').onclick=()=>setPaused(true,'intel');$('returnToGame').onclick=()=>setPaused(false);$('closeIntel').onclick=()=>setPaused(false);$('tipsToggle').onchange=()=>{discovery.showTips=!!tipToggle.checked;save()};
canvas.onclick=()=>{if(started&&!paused&&canvas.requestPointerLock)canvas.requestPointerLock().catch(()=>{})};
canvas.addEventListener('contextmenu',e=>e.preventDefault());
document.addEventListener('mousemove',e=>{if(document.pointerLockElement===canvas&&!paused){cam.yaw-=e.movementX*.0023;cam.pitch=clamp(cam.pitch+e.movementY*.002,-.74,.78);discovery.looked=true}});
document.addEventListener('mousedown',e=>{lastInputIsPad=false;if(!started||paused)return;if(e.button===2)key.mouse2=true;if(e.button===0)action('fire')});document.addEventListener('mouseup',e=>{if(e.button===2)key.mouse2=false});
document.addEventListener('keydown',e=>{lastInputIsPad=false;const k=e.key.toLowerCase();if(['tab',' ','arrowup','arrowdown','arrowleft','arrowright'].includes(k))e.preventDefault();if(key[k])return;key[k]=true;if(!started){if(k==='enter')start(false);return}if(k==='escape'){action('pause');return}if(paused){if(k==='j'||k==='c')setPaused(false);return}const actions={e:'interact',control:'crouch',z:'prone',v:'cover',f:'melee',n:'knock',j:'intel',c:'codec',t:'tips','[':'previous',']':'next'};if(actions[k])action(actions[k]);if(k==='tab')action('equipitem');if(k==='g')action('equipweapon');if(k==='r')reset();if(k==='q')cam.shoulder*=-1;});document.addEventListener('keyup',e=>{const k=e.key.toLowerCase();key[k]=false;if(k==='tab'||k==='g')action('equipclose')});window.addEventListener('blur',()=>{for(const k of Object.keys(key))key[k]=false;if(equipOpen)action('equipclose')});
function readGamepad(){const p=pollGamepad();updatePadStatus(p);padUsing=!!p&&(Math.abs(p.axes[0]||0)>.16||Math.abs(p.axes[1]||0)>.16);if(!p){padPrev=[];return;}const now=p.buttons.map(pressed);const previous=padPrev;const down=i=>now[i]&&!previous[i];const up=i=>!now[i]&&previous[i];padPrev=now;if(!started){if(down(0)||down(9))start(false);return;}if(down(9)){action('pause');return}if(paused){if(down(1)||down(0)){setPaused(false)}return}if(down(0))action('interact');if(down(1)){action('crouch');}if(down(2))action('melee');if(down(3))action('prone');if(down(12))action('intel');if(down(8))action('codec');if(down(11))cam.shoulder*=-1;if(down(4))action('equipitem');if(down(5))action('equipweapon');if(up(4)||up(5))action('equipclose');if(down(14))action('previous');if(down(15))action('next');if(down(7)){player.aim=inputDown('mouse2')||pressed(p.buttons[6]);action('fire');}}
function frame(now){requestAnimationFrame(frame);const real=clamp((now-last)/1000,0,.06);last=now;readGamepad();const dt=started&&!paused?real*(equipOpen?.2:1):0;if(started&&!paused){gameTime+=dt;levelTime+=dt;elapsedStage+=dt;updatePlayer(dt);updateFox(dt);updateGuards(dt);ambience(dt);for(const f of footsteps)f.life-=dt;footsteps=footsteps.filter(f=>f.life>0);shots.forEach(s=>s.life-=dt);shots=shots.filter(s=>s.life>0);for(const p of particles){p.x+=p.vx*dt;p.y+=p.vy*dt;p.z+=p.vz*dt;p.vy-=4.4*dt;p.life-=dt}particles=particles.filter(p=>p.life>0);noiseEvents.forEach(n=>n.life-=dt);noiseEvents=noiseEvents.filter(n=>n.life>0);if(awareness===0&&stage!==4&&Math.floor(gameTime/12)!==Math.floor((gameTime-dt)/12))save();if(awareness!==prevAlert)record('awareness.'+awareness);if(awareness===3&&prevAlert!==3)achievement.push({time:gameTime});prevAlert=awareness;}drawFrame(gameTime,dt);if(started)updateHUD(dt);frameCount++;if(now-frameStart>1000){$('fps').textContent=Math.round(frameCount*1000/(now-frameStart))+' FPS';frameStart=now;frameCount=0}}
// Console contract for deterministic browser tests, never elevated to source fidelity.
window.__mgGameDebug={get state(){return {stage,tagState:tag.state,tagPoint:tag.point&&{...tag.point},signal:duel.signal&&{...duel.signal},bossBeat:duel.phase,bossLocation:{x:boss.x,z:boss.z},cardDrop:{...cardDrop},hasCard3:inventories.items[2].usable,shutterOpen:!finalGate.active,accessPrompt:exitInteraction(),wallHits:[...wallHits],bossHP:boss.hp,health,recoveredGear,stunAmmo,equippedWeapon,sectorSecured,safeLift:{...safeLift},player:{...player},bodyCollider:{...bodyShape(),orientation:player.stance===2?'horizontal':'vertical',samples:bodySamples(player.x,player.z,player.stance,player.yaw)},awareness,paused,equipOpen,noiseEvents:noiseEvents.map(n=>({...n})),guards:guards.map(g=>({state:g.state,x:g.x,z:g.z,yaw:g.yaw,last:{...g.last},investigateUntil:g.investigateUntil||0,lastKnockId:g.lastKnockId||0,knocked:g.knocked,aimSubject:g.aimSubject,aimTime:g.aimTime,fireCooldown:g.fireCooldown,shotVisual:g.shotVisual})),fox:{x:fox.x,z:fox.z,hp:fox.hp,alive:fox.alive,mode:fox.mode,stance:fox.stance,moveSpeed:fox.moveSpeed},missionFailed,coverAssets:coverAssets.map(c=>({...c})),activeTracers:shots.length,originalMapVerified:false,renderedFrames:drawn,visualVersion:'original_humanoid_mk4_escort_armed_guards',meshKinds:Object.keys(meshLibrary),materialAtlas:{...materialAtlas},environmentModelCounts:{bedParts:sceneDetail.filter(o=>o.name.startsWith('cot ')||o.name.startsWith('blanket ')||o.name.startsWith('pillow ')||o.name==='folded wool blanket').length,lockerDetails:sceneDetail.filter(o=>o.name.startsWith('locker ')).length,cabinetDetails:sceneDetail.filter(o=>o.name.includes('front trim')||o.name.includes('bolt head')).length,ceilingsBlockCamera:cameraOnlySolids.some(o=>o.name==='ceiling'&&o.active)},camEye:[...cam.eye],cameraBlocked:cameraPositionPenetrates(cam.eye),cameraBelowFloor:cam.eye[1]<.28,activePadIndex,padPrev:[...padPrev],started}},readPad:()=>readGamepad(),start:()=>start(false),act:action,teleport:(x,z)=>{player.x=x;player.z=z},setCamera:(yaw,pitch=0)=>{cam.yaw=yaw;cam.pitch=pitch},stepCamera:(dt=.016)=>updateCamera(dt),canOccupy:(x,z,stance,yaw=player.yaw)=>canOccupyBody(x,z,stance,yaw),moveBody:(vx,vz,dt)=>movePlayer(vx,vz,dt),step:(dt)=>{gameTime+=dt;updatePlayer(dt);updateFox(dt);updateGuards(dt)},damageFox:(n)=>damageFox(n),stepFox:(dt)=>updateFox(dt),sightBlocked:(a,b,fromY=1.58,toY=1.52)=>lineBlockedSight(a,b,fromY,toY),radarVisionDistance:(index,angle,range=10)=>radarVisionDistance(guards[index],angle,range),findPath:(a,b)=>pathfind(a,b),foxRouteRisk:(p)=>foxRouteRisk(p),guardSeesPosition:(index,pos)=>guardSeesPosition(guards[index],pos),reset};
// Deterministic development-only combat hook (absent unless tests opt in).
if(window.__MG_TEST__){
 window.__mgGameDebug.testSetGuard=(index,settings)=>{
   const g=guards[index];if(!g)return false;
   Object.assign(g,settings);g.waypoints=[];g.pathTime=0;return true;
 };
 window.__mgGameDebug.testSetFox=(settings)=>{
   Object.assign(fox,settings);fox.waypoints=[];fox.pathTime=0;return true;
 };
 window.__mgGameDebug.testClearTransient=()=>{
   noiseEvents=[];footsteps=[];shots=[];return true;
 };
 window.__mgGameDebug.testFoxNoise=()=>{
   noiseEvents.push({x:fox.x,z:fox.z,source:'fox',power:100,life:100});return true;
 };
 window.__mgGameDebug.testConfigureFoxThreat=()=>{
   if(stage<2)return false;
   player.x=-8;player.z=4;player.yaw=0;
   fox.x=6;fox.z=0;fox.hp=100;fox.alive=true;fox.stance=0;fox.mode='holding';fox.hitTimer=0;
   const g=guards[0];g.x=7.4;g.z=0;g.yaw=Math.PI;g.knocked=false;g.suspicion=1;
   g.state=3;g.alert=7;g.aimTime=0;g.fireCooldown=0;g.shotVisual=0;
   g.waypoints=[];g.pathTime=0;g.last={x:fox.x,z:fox.z};
   guards[1].knocked=true;return true;
 };
 window.__mgGameDebug.testStepGuards=(dt)=>{gameTime+=dt;updateGuards(dt)};
 window.__mgGameDebug.testResetFoxAfterShot=()=>{fox.hp=100;fox.alive=true;fox.hitTimer=0;for(const g of guards)g.knocked=true;awareness=0;};
}
try{$('resume').disabled=!localStorage.getItem(STORAGE)}catch(e){$('resume').disabled=true}drawEquipment();if(location.hash==='#continue'){history.replaceState(null,'',location.pathname+location.search);start(true)}else if(rehearsal)start(false);
requestAnimationFrame(frame);
})();

