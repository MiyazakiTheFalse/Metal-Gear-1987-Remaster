// One interleaved static vertex buffer per costume layer. The same depth buffer
// as the prison geometry handles characters behind walls, bars and doorways.
window.createFieldRenderer = function(gl, compile) {
 const assets=window.FieldRigs, count=assets.skeleton('shoot_gunner').length;
 const vertex=`attribute vec3 position, normal, color;
 attribute vec3 binding;
 uniform vec4 palette[${count}];
 uniform mat4 vp;
 uniform vec4 actor;
 varying vec3 shade;varying vec3 rest;varying float sheen;
 varying float distanceW;
 vec2 spin(vec2 p, vec2 cs){return vec2(cs.x*p.x-cs.y*p.y,cs.y*p.x+cs.x*p.y);}
 void main(){
  vec4 a=palette[int(binding.x)],b=palette[int(binding.y)];
  vec4 j=mix(a,b,binding.z);
  vec3 local=vec3(spin(position.xy,j.xy)+j.zw,position.z);
  vec2 xz=spin(local.xz,actor.zw)+actor.xy;
  vec3 n=vec3(spin(normal.xy,j.xy),normal.z);
  n.xz=spin(n.xz,actor.zw); n=normalize(n);
  float lit=.54+.51*max(0.,dot(n,normalize(vec3(-.4,.9,.5))));
  vec3 warm=vec3(22.,3.05,.3)-vec3(xz.x,local.y,xz.y);
  float pool=max(0.,dot(n,normalize(warm)))/(1.+dot(warm,warm)*.08);
  shade=color*(lit+vec3(.61,.43,.25)*pool);
  shade+=vec3(.043,.075,.086)*max(0.,dot(n,normalize(vec3(.52,.46,-.7))));
  sheen=pow(max(dot(n,normalize(vec3(-.46,.72,.39))),0.),13.);
  rest=position;gl_Position=vp*vec4(xz.x,local.y,xz.y,1.);distanceW=gl_Position.w;
 }`;
 const frag=`precision mediump float;
 varying vec3 shade;varying vec3 rest;varying float sheen;varying float distanceW;
 void main(){
  float cloth=step(shade.r*.94,shade.g);
  float weave=sin(rest.y*530.)*sin((rest.x+rest.z)*420.);
  float fade=1.-smoothstep(1.6,4.2,distanceW);
  vec3 material=shade*(1.+cloth*.075*weave*fade)+vec3(.069,.068,.059)*sheen;
  gl_FragColor=vec4(mix(material,vec3(.04,.09,.09),clamp((distanceW-9.)/48.,0.,.68)),1.);}`;
 const prog=gl.createProgram();
 gl.attachShader(prog,compile(vertex,gl.VERTEX_SHADER));gl.attachShader(prog,compile(frag,gl.FRAGMENT_SHADER));
 gl.linkProgram(prog);
 if(!gl.getProgramParameter(prog,gl.LINK_STATUS))throw Error(gl.getProgramInfoLog(prog));
 const attrs=['position','normal','color','binding'].map(k=>gl.getAttribLocation(prog,k));
 const uniforms=['vp','actor','palette[0]'].map(k=>gl.getUniformLocation(prog,k));
 const meshes={};
 for(const role of ['snake','fox','guard','shoot_gunner']){
  const bones=assets.skeleton(role);
  meshes[role]=assets.model(role).map(part=>{
   if(!part.length)return null;
   const data=new Float32Array(part.length*12);
   part.forEach((vertex,i)=>{
    data.set(vertex.slice(0,10),i*12);
    data[i*12+10]=Math.max(0,bones[vertex[9]][1]);data[i*12+11]=vertex[10];
   });
   const buffer=gl.createBuffer();gl.bindBuffer(gl.ARRAY_BUFFER,buffer);
   gl.bufferData(gl.ARRAY_BUFFER,data,gl.STATIC_DRAW);return {buffer,count:part.length};
  });
 }
 return function draw(actors,vp){
  gl.useProgram(prog);gl.uniformMatrix4fv(uniforms[0],false,vp);
  attrs.forEach(k=>gl.enableVertexAttribArray(k));
  for(const {role,actor,state,gear} of actors){
   const {rot,offset}=assets.pose(state), p=new Float32Array(count*4);
   const bones=assets.skeleton(role);
   for(let j=0;j<bones.length;j++){
    const [,parent,x,y]=bones[j],pi=parent*4;
    const pc=parent<0?1:p[pi], ps=parent<0?0:p[pi+1];
    const c=Math.cos(rot[j]),s=Math.sin(rot[j]);
    const ac=pc*c-ps*s,as=ps*c+pc*s;
    p.set([ac,as,(parent<0?offset[0]:p[pi+2])+x*(pc-ac)-y*(ps-as),
     (parent<0?offset[1]:p[pi+3])+x*(ps-as)+y*(pc-ac)],j*4);
   }
   gl.uniform4fv(uniforms[2],p);
   gl.uniform4f(uniforms[1],actor.x,actor.z,Math.cos(actor.yaw),Math.sin(actor.yaw));
   for(const [i,part] of meshes[role].entries()){
    if(!part||(i===1&&!gear))continue;
    gl.bindBuffer(gl.ARRAY_BUFFER,part.buffer);
    attrs.forEach((k,n)=>gl.vertexAttribPointer(k,3,gl.FLOAT,false,48,n*12));
    gl.drawArrays(gl.TRIANGLES,0,part.count);
   }
  }
  attrs.forEach(k=>gl.disableVertexAttribArray(k));
 };
};
