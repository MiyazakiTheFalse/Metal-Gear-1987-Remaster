// Original Outer Heaven playtest meshes. Units are metres, +X is forward.
// Ring profiles describe garment cuts, head planes and equipment separately.
(function (root) {
'use strict';
const bones = [
 ['pelvis',-1,0,.91,0], ['chest',0,0,1.05,0], ['neck',1,0,1.48,0], ['head',2,0,1.55,0]
];
for (const side of [-1,1]) {
 const suffix=side<0?'_L':'_R', base=bones.length;
 for(const [name,parent,x,y,z] of [
  ['upper_arm',1,0,1.43,.229],['forearm',base,0,1.15,.26],['hand',base+1,.015,.90,.26],
  ['thigh',0,0,.90,.115],['shin',base+3,.018,.50,.12],['foot',base+4,-.005,.115,.12]
 ]) bones.push([name+suffix,parent,x,y,z*side]);
}
const colors={skin:[.72,.53,.40],lip:[.44,.27,.22],eye:[.78,.74,.62],ink:[.07,.076,.068],
 hair:[.14,.105,.084],ash:[.30,.28,.245],sage:[.25,.34,.27],pine:[.14,.22,.19],
 steel:[.25,.30,.27],sand:[.58,.54,.43],stitch:[.44,.42,.34],boot:[.14,.13,.115],
 belt:[.19,.21,.175],buckle:[.47,.48,.40],wrap:[.69,.65,.53],ochre:[.52,.32,.18],oxblood:[.27,.12,.10],jacket:[.32,.35,.25]};
function model(role) {
 const snake=role==='snake', guard=role==='guard', fox=role==='fox',boss=role==='shoot_gunner';
 const parts=[[],[]]; let layer=0;
 // Each ring is [height, forward radius, width radius, optional forward offset].
 function surface(joint,shade,cx,cz,rings,sides=14,blend=false){
  const color=colors[shade],offset=bones[joint][3];
  const rows=rings.map(([y,rx,rz,dx=0],row)=>Array.from({length:sides},(_,i)=>{
   const a=i*2*Math.PI/sides, ca=Math.cos(a),sa=Math.sin(a);
   const lo=rings[Math.max(0,row-1)],hi=rings[Math.min(rings.length-1,row+1)];
   const slope=(hi[1]-lo[1])*ca*ca+(hi[2]-lo[2])*sa*sa;
   const n=[ca/Math.max(.005,rx),0,sa/Math.max(.005,rz)];
   const len=Math.hypot(n[0],n[2]); n[0]/=len;n[2]/=len;
   n[1]=-slope/Math.max(.003,hi[0]-lo[0]); const m=Math.hypot(...n);
   return [cx+dx+rx*ca,y,cz+rz*sa,...n.map(v=>v/m),...color,joint,
    blend?Math.max(0,1-Math.abs(y-offset)/.09)*.48:0];
  }));
  // Close the cut ends of each garment so raised shoulders and bent knees
  // never expose the inside of a sleeve or boot.
  for(const end of [0,rows.length-1]){
   const row=rows[end], [y,rx,rz,dx=0]=rings[end];
   if(rx*rz<.000001)continue;
   const ny=end===0?-1:1, center=[cx+dx,y,cz,0,ny,0,...color,joint,row[0][10]];
   for(let i=0;i<sides;i++){
    const cap=v=>[...v.slice(0,3),0,ny,0,...v.slice(6)];
    const a=cap(row[i]),b=cap(row[(i+1)%sides]);
    parts[layer].push(center,end===0?a:b,end===0?b:a);
   }
  }
  for(let j=1;j<rows.length;j++) for(let i=0;i<sides;i++){
   const k=(i+1)%sides, a=rows[j-1],b=rows[j];
   parts[layer].push(a[i],b[i],b[k],a[i],b[k],a[k]);
  }
 }
 function oval(j,col,x,y,z,rx,ry,rz,n=12){
  surface(j,col,x,z,[-1,-.72,0,.72,1].map(h=>[y+ry*h,rx*Math.sqrt(1-h*h),rz*Math.sqrt(1-h*h)]),n);
 }
 // A tapered cloth panel following a line on the body surface.
 function panel(joint,shade,start,end,width){
  const [ax,ay,az]=start,[bx,by,bz]=end;
  const vertices=[[ax,ay,az-width],[ax,ay,az+width],[bx,by,bz+width*.60],[bx,by,bz-width*.60]]
   .map(point=>[...point,1,0,0,...colors[shade],joint,0]);
  for(const ix of [0,1,2,0,2,3])parts[layer].push(vertices[ix]);
 }
 const shirt=fox?'sand':snake?'sage':boss?'ochre':'jacket',pants=fox?'stitch':snake?'pine':'steel';
 surface(0,pants,0,0,[[.76,.09,.14],[.80,.14,.19],[.90,.145,.18],[1,.13,.16]]);
 surface(1,shirt,0,0,[[.91,.15,.18],[.97,.135,.17],[1.06,.14,.167],[1.22,.155,.196],[1.38,.16,.222],[1.44,.13,.211],[1.49,.08,.10]],18,true);
 surface(2,'skin',0,0,[[1.435,.087,.08],[1.52,.079,.076],[1.59,.083,.08]]);
 surface(3,'skin',0,0,[[1.52,.044,.049,.055],[1.55,.086,.079,.026],[1.595,.101,.100,.014],[1.65,.115,.109],[1.71,.12,.111],[1.77,.105,.095],[1.805,.046,.053]],20);
 oval(3,'skin',.118,1.646,0,.035,.046,.024);
 oval(3,'lip',.121,1.589,0,.008,.006,.034);
 for(const s of [-1,1]){
  oval(3,'skin',-.016,1.645,s*.110,.027,.044,.017);
  oval(3,'eye',.103,1.677,s*.049,.011,.009,.025);
  oval(3,'ink',.114,1.677,s*.049,.004,.008,.008);
  oval(3,fox?'ash':'hair',.103,1.694,s*.050,.012,.008,.033);
 }
 if(fox){
  panel(1,'stitch',[.083,1.467,-.079],[.158,1.32,.018],.024);
  panel(1,'stitch',[.083,1.467,.079],[.157,1.36,.007],.024);
 }else{
  for(const s of [-1,1]){
   panel(1,'pine',[.084,1.469,s*.088],[.153,1.355,s*.063],.047);
   surface(1,shirt,.145,s*.103,[[1.221,.013,.046],[1.237,.019,.050],[1.292,.016,.052]],8);
   panel(1,'pine',[.155,1.296,s*.101],[.167,1.27,s*.101],.050);
  }
 }
 if(boss){
  surface(3,'boot',-.007,0,[[1.692,.129,.119],[1.724,.136,.124]],18);
  surface(3,'oxblood',-.018,-.016,[[1.719,.144,.138],[1.75,.159,.155,-.012],
   [1.776,.14,.147,-.026],[1.797,.065,.070,-.043]],20);
  oval(3,'buckle',.118,1.744,.047,.011,.016,.013);
  for(const s of [-1,1])oval(1,'boot',-.012,1.432,s*.202,.115,.061,.078);
 }else if(guard){
  surface(3,'steel',-.008,0,[[1.674,.14,.129],[1.695,.153,.142],[1.724,.146,.14],[1.77,.12,.12],[1.811,.082,.08],[1.826,.003,.003]],20);
 }else{
  surface(3,fox?'ash':'hair',-.012,0,[[1.695,.131,.116],[1.745,.136,.12],[1.795,.108,.105],[1.824,.039,.038,-.02],[1.83,.002,.002,-.035]],18);
  for(const s of [-1,1]) oval(3,fox?'ash':'hair',-.072,1.69,s*.077,.056,.060,.038);
  // Uneven hairline, swept back from the brow and broken into locks at the nape.
  for(let i=0;i<7;i++){
   const z=(i-3)*.029, delta=Math.abs(i-3)*.005;
   surface(3,fox?'ash':'hair',.085-delta,z,[[1.683+delta,.004,.005,.024],
    [1.715,.024,.019,.012],[1.75,.030,.022,-.009],[1.78,.018,.011,-.037]],8);
  }
  if(snake){
   for(const s of [-1,1]) surface(3,'hair',-.093,s*.055,
    [[1.577,.008,.005,-.029],[1.638,.043,.036,-.016],[1.714,.050,.041]],10);
   surface(3,'pine',-.009,0,[[1.693,.135,.121],[1.722,.139,.122]],20);
   for(const s of [-1,1])surface(3,'pine',-.14,s*.03,[[1.53,.013,.008,-.05],[1.62,.024,.013,-.036],[1.711,.026,.017]],8);
  }
 }
 for(let side=0;side<2;side++){
  const s=side?1:-1,j=4+side*6;
  surface(j,shirt,0,.231*s,[[1.14,.071,.064],[1.185,.087,.074],[1.29,.088,.083],[1.38,.105,.091],[1.435,.087,.087],[1.465,.045,.042]],14,true);
  surface(j+1,fox?'skin':shirt,0,.26*s,[[.899,.048,.047],[.94,.056,.05],[1.065,.075,.061],[1.15,.072,.068],[1.18,.061,.058]],14,true);
  if(fox&&side===0)surface(j+1,'wrap',0,.26*s,[[.955,.062,.058],[1.013,.068,.063],[1.029,.066,.062]]);
  surface(j,fox?'stitch':'pine',0,.232*s,[[1.15,.078,.071],[1.166,.09,.075],[1.191,.088,.076]],12);
  if(!fox){
   surface(j+3,pants,-.008,.199*s,[[.60,.067,.023],[.63,.077,.034],[.73,.079,.036]],10);
   surface(j+3,'belt',-.006,.201*s,[[.708,.076,.034],[.742,.08,.039]],10);
   surface(j+4,'boot',-.004,.12*s,[[.17,.079,.076],[.24,.083,.078],[.25,.080,.077]],14);
  }
  oval(j+2,fox?'skin':'boot',.018,.855,.26*s,.052,.085,.048);
  surface(j+3,pants,0,.115*s,[[.465,.076,.077],[.51,.095,.091,.012],[.62,.106,.095],[.77,.124,.102],[.895,.12,.103],[.93,.09,.087]],14,true);
  surface(j+4,pants,-.004,.12*s,[[.145,.073,.069],[.20,.083,.078],[.35,.091,.084,-.015],[.445,.081,.078],[.516,.09,.083,.014]],14,true);
  surface(j+5,'boot',0,.12*s,[[.01,.132,.078,.045],[.037,.141,.081,.045],[.082,.137,.08,.039],[.126,.097,.074,.014],[.19,.076,.073],[.21,.078,.073]],14);
 }
 if(!fox){
  layer=snake?1:0;
  surface(0,'belt',0,0,[[.924,.149,.185],[.982,.148,.182]],18);
  oval(0,'buckle',.146,.95,0,.012,.026,.037);
  for(const s of [-1,1]){
   surface(1,'belt',.003,s*.125,[[1.02,.143,.027],[1.28,.154,.029],[1.426,.117,.034],[1.464,.069,.029]],10);
   oval(0,'pine',-.02,.92,s*.188,.067,.074,.04);
   oval(1,'belt',.125,1.15,s*.082,.048,.079,.060);
  }
  if(guard){
   // The carried prop follows the forward hand through the patrol cycle.
   surface(12,'ink',.20,.26,[[.79,.20,.024],[.82,.26,.026],[.86,.26,.026],[.88,.18,.022]],8);
   oval(12,'steel',.54,.852,.26,.15,.014,.014);
  }
 }
 return parts;
}
// FK uses rotations about the model's lateral axis. Animation is in place.
function pose(state) {
 const {phase=0,stride=0,crouch=0,prone=0,down=0,aim=0,cover=0,hit=0,
        strike=0,time=0,captive=0}=state;
 const rot=new Array(bones.length).fill(0);
 const lean=.055+.07*stride+.19*crouch+.12*captive;
 rot[0]=-lean-1.38*prone-1.48*down; rot[1]=-.013*Math.sin(time*2.4)-.08*hit;
 rot[2]=lean*.55+.55*prone+(-.12*captive); rot[3]=.24*prone;
 for(let side=0;side<2;side++){
  const j=4+side*6,w=Math.sin(phase+side*Math.PI)*stride;
  rot[j]=-.32*w+.06+.22*crouch+1.78*prone+.24*captive;
  rot[j+1]=.18+.26*stride+.62*prone+.90*captive;
  rot[j+3]=lean+.68*w+.97*crouch-.15*prone;
  rot[j+4]=-.10-.63*Math.max(0,w)-1.83*crouch-.26*prone;
  rot[j+5]=-(rot[0]+rot[j+3]+rot[j+4])*(1-prone)*(1-down);
  if(prone){
   // Forearms reach towards the face and clear the floor as the torso rolls forward.
   rot[j]+=prone*.65;rot[j+1]+=prone*.48;
  }
  if(aim||cover||strike){const amount=Math.max(aim,side?cover:0,side?strike:0);
   rot[j]+=amount*(1.06-rot[j]);rot[j+1]+=amount*(.50-rot[j+1]);}
 }
 // Plant the lower sole through each step. Knee flexion already shortens
 // the leg, so a second fixed crouch offset would leave both boots hovering.
 function soleHeight(hip){
  let y=bones[0][3],angle=rot[0],parent=0;
  for(const j of [hip,hip+1,hip+2]){
   const dx=bones[j][2]-bones[parent][2],dy=bones[j][3]-bones[parent][3];
   y+=Math.sin(angle)*dx+Math.cos(angle)*dy;
   angle+=rot[j];parent=j;
  }
  return y+Math.cos(angle)*(.01-bones[parent][3])
   +Math.min(Math.sin(angle)*(-.087-bones[parent][2]),Math.sin(angle)*(.177-bones[parent][2]));
 }
 const grounded=.008-Math.min(soleHeight(7),soleHeight(13));
 const offset=[0,grounded*(1-prone)*(1-down)-.74*prone-.685*down,0];
 return {rot,offset};
}
root.FieldRigs={bones,model,pose};
})(typeof module==='object'?module.exports:window);
