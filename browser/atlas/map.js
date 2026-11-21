/* Outer Heaven 1995 architecture viewer.
   All coordinates and physical dimensions are PROPOSED design hypotheses.
   No imported assets, licensed graphics, map API or outside JS dependencies.
*/
'use strict';
const svgNS='http://www.w3.org/2000/svg';
const $=id=>document.getElementById(id);
let atlas=null;
let selected='';
const state={floor:'SITE',mission:'',query:'',ghost:false,connections:true};
const esc=s=>String(s??'').replace(/[&<>"']/g,ch=>({
  '&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[ch]));
function svg(tag,props={},parent=$('map')){
 const el=document.createElementNS(svgNS,tag);
 for(const [k,v] of Object.entries(props))el.setAttribute(k,String(v));
 parent.appendChild(el);return el;
}
function title(parent,string){
 const t=svg('title',{},parent);t.textContent=string;return t;
}
function rect(x,y,w,h,fill,stroke,cls=''){
 return svg('rect',{x,y,width:w,height:h,fill,stroke,class:cls});
}
function line(x1,y1,x2,y2,style){
 return svg('line',{x1,y1,x2,y2,...style});
}
function text(x,y,str,cl=''){
 const t=svg('text',{x,y,class:cl});t.textContent=str;return t;
}
function areaMatches(a){
 if(state.mission&&a.mission!==state.mission)return false;
 if(!state.query)return true;
 const term=state.query.toLowerCase();
 const hay=[a.label,a.id,a.mission,a.boss,...a.pickups,...a.gates,
  ...a.beats.flatMap(b=>[b.name,b.id])].join(' ').toLowerCase();
 return hay.includes(term);
}
function colorFor(a){return a.boss?'#e9b878':a.pickups.length?'#82bfe5':'#9ee0b5'}
function visible(a){
 return a.level===state.floor||
  (state.floor==='SITE'&&a.level==='TUNNEL'&&state.ghost);
}
function zoneFor(id){return atlas.areas.find(a=>a.id===id)}
function showArea(id){
 const a=zoneFor(id);if(!a)return;
 selected=id;state.floor=a.level;$('floor').value=a.level;
 draw();renderDirectory();
 const links=atlas.links.filter(e=>e.from===id||e.to===id);
 const beatList=a.beats.length?a.beats.map(b=>
  '<li><b>#'+esc(b.ordinal)+'</b> · '+esc(b.name)+
  ' <span class="pill">'+esc(b.status)+'</span></li>').join(''):
  '<li>Zone contract exists; no individual ledger beat allocated here</li>';
 $('detail').innerHTML='<h2>'+esc(a.label)+'</h2>'+
  '<div class="small">'+esc(a.id)+' · '+esc(a.mission)+
  ' · '+esc(atlas.levels.find(l=>l.id===a.level)?.label||a.level)+'</div>'+
  '<p class="warning">PROVISIONAL PHYSICAL PLACEMENT: '+esc(a.sourceStatus)+
  '. Original MSX2 room/screen geometry is not recovered or proven by this atlas.</p>'+
  '<div class="two"><section><h3>Progression beats ('+a.beats.length+')</h3><ol>'+beatList+
  '</ol></section><section><h3>Fictional facility function</h3>'+
  '<p>Centre: '+a.center_m.join('m, ')+'m from proposed local north-west survey origin. '+
  'Nominal room footprint: '+a.footprint_m.join(' × ')+' m.</p>'+
  (a.boss?'<h3>Boss arena</h3><p>'+esc(a.boss)+
   ' · telegraphed phases, physical cover and a non-illusory player retreat.</p>':'')+
  '<h3>Weapon / item points</h3><p>'+(
   a.pickups.length?a.pickups.map(p=>'<span class="pill">'+esc(p)+'</span>').join(''):
   'No specific pickup pinned in this workstream')+'</p>'+
  '<h3>Access dependencies</h3><p>'+(
   a.gates.length?a.gates.map(p=>'<span class="pill">'+esc(p)+'</span>').join(''):
   'Validate against original card/door graph')+'</p></section></div>'+
  '<h3>Connected areas / changes on return</h3>'+
  '<p>'+links.map(e=>{
    const other=e.from===id?e.to:e.from;
    return '<span class="pill">'+esc(other)+' · '+esc(e.connector)+
     ' · '+esc(e.gate)+'</span>';
  }).join(' ')+'</p>'+
  '<h3>Production review requirements</h3><ol>'+a.designNotes.map(x=>'<li>'+esc(x)+'</li>').join('')+'</ol>';
}
function renderDirectory(){
 const list=atlas.areas.filter(areaMatches);
 $('directory').innerHTML='';
 for(const a of list){
  const b=document.createElement('button');
  b.className='area-btn'+(a.boss?' boss':a.pickups.length?' item':'')+
    (selected===a.id?' selected':'');
  b.innerHTML='<strong>'+esc(a.label)+'</strong><small>'+
   esc(a.id)+' · '+a.beats.length+' beats'+
   (a.pickups.length?' · '+a.pickups.length+' pickups':'')+'</small>';
  b.addEventListener('click',()=>showArea(a.id));
  $('directory').appendChild(b);
 }
 if(!list.length)$('directory').textContent='No zone matches that search.';
}
function drawBackdrop(){
 rect(0,0,990,580,'#0b1f1e','#487366');
 rect(0,0,990,580,'url(#grid)','none');
 // The site footprint is deliberately around 1.0 × 0.58 km. Building spans
 // and sightlines are credible CONCEPTS, not as-built military blueprints.
 for(const f of atlas.features){
  if(!f.rect_m)continue;
  const [x,y,w,h]=f.rect_m;
  let fill='#24423e',stroke='#769b8e';
  if(f.kind==='perimeter'){fill='none';stroke='#b7bfa3'}
  if(f.kind==='road'){fill='#415047';stroke='#859481'}
  if(f.kind==='yard'){fill='#514e3f';stroke='#b6a475'}
  if(f.kind==='danger'){fill='#5d332b';stroke='#e3aa7a'}
  if(f.kind==='helipad'){fill='#294647';stroke='#abd7bd'}
  if(f.kind==='utility'){fill='#383b49';stroke='#8ba6ac'}
  if(f.kind==='building'&&state.floor!=='SITE'){
    fill=f.id===atlas.levels.find(l=>l.id===state.floor)?.building?
      '#3b6658':'#21342f';
  }
  const r=rect(x,y,w,h,fill,stroke,'facility'+(
   f.kind==='building'&&state.floor!=='SITE'&&
   f.id!==atlas.levels.find(l=>l.id===state.floor)?.building?' dim':''));
  r.setAttribute('stroke-width',f.kind==='perimeter'?'5':'2');
  if(f.kind==='road')r.setAttribute('opacity','.7');
  if(f.kind==='danger')r.setAttribute('stroke-dasharray','8 5');
  title(r,(f.label||f.kind)+' · '+(f.role||'remake exterior function')+
    ' · NOT an original map coordinate');
  if(f.kind==='building'||f.kind==='yard'||f.kind==='helipad'||f.kind==='danger'){
    text(x+8,y+18,f.label||f.kind,'feature-label');
    if(f.role)text(x+8,y+32,f.role.slice(0,34),'feature-label');
  }
  if(f.kind==='helipad'){
    const circle=svg('circle',{cx:x+w/2,cy:y+h/2,r:14,
      stroke:'#a9d5b3','stroke-width':2,fill:'none'});
    text(x+w/2-4,y+h/2+4,'H');
  }
 }
 for(const f of atlas.features.filter(f=>f.kind==='tower'||f.kind==='gate')){
  const [x,y]=f.center_m;
  const s=f.kind==='tower'?12:17;
  rect(x-s/2,y-s/2,s,s,f.kind==='tower'?'#bba987':'#9cbbaa','#182d2b');
  if(f.kind==='tower')text(x-3,y+3,'T');
 }
 // Readable 100m schematic scale; visual length corresponds to viewBox metre units.
 const sx=54,sy=555;
 line(sx,sy,sx+100,sy,{stroke:'#e3ddae','stroke-width':3});
 line(sx,sy-5,sx,sy+5,{stroke:'#e3ddae','stroke-width':2});
 line(sx+100,sy-5,sx+100,sy+5,{stroke:'#e3ddae','stroke-width':2});
 text(sx+27,sy-9,'100 metres · proposed');
 text(900,43,'N ↑');
}
function draw(){
 if(!atlas)return;
 const root=$('map');root.querySelectorAll(':scope > :not(defs)').forEach(el=>el.remove());
 drawBackdrop();
 const lvl=atlas.levels.find(l=>l.id===state.floor);
 $('mapTitle').textContent=lvl?.label||'Outer Heaven / site';
 const shown=atlas.areas.filter(a=>visible(a)&&areaMatches(a));
 const shownIds=new Set(shown.map(a=>a.id));
 $('mapCaption').textContent=state.floor==='SITE'?
   'Exterior district plan, proposed measured metre scale · choose a floor to reveal interior beats':
   'Floor elevation '+lvl.elevation_m+' m (provisional) · '+shown.length+' matching areas · original gates shown by named connector';
 if(state.connections){
  for(const e of atlas.links){
   if(!shownIds.has(e.from)||!shownIds.has(e.to))continue;
   const a=zoneFor(e.from),b=zoneFor(e.to);
   const el=line(a.center_m[0],a.center_m[1],b.center_m[0],b.center_m[1],{
    class:'link'+(e.status.includes('provisional')?' candidate':'')});
   title(el,e.from+' → '+e.to+' · '+e.connector+' / '+e.gate+
     ' · design-graph, not a proven source connection');
  }
 }
 if(state.ghost){
  for(const a of atlas.areas.filter(x=>x.level!==state.floor&&areaMatches(x))){
   const g=svg('g',{class:'ghost'});
   svg('circle',{cx:a.center_m[0],cy:a.center_m[1],r:4,
     fill:colorFor(a)},g);
  }
 }
 for(const a of shown){
  const [x,y]=a.center_m;
  const cl='zone'+(a.boss?' boss':a.pickups.length?' item':'')+
   (a.id===selected?' selected':'');
  const g=svg('g',{class:cl,role:'button',tabindex:0,
   'aria-label':a.label+', '+a.beats.length+' campaign beats'});
  svg('circle',{cx:x,cy:y,r:a.boss?10:8},g);
  if(a.boss)svg('circle',{cx:x,cy:y,r:15,stroke:'#d8a770',
     fill:'none','stroke-width':1.5,'stroke-dasharray':'4 4'},g);
  if(a.pickups.length)svg('rect',{x:x+4,y:y-13,width:7,height:7,
     fill:'#90cae8',stroke:'#142c31'},g);
  text(x+13,y-8,a.label.length>23?a.label.slice(0,22)+'…':a.label);
  text(x+13,y+5,a.mission+'  ['+a.beats.length+' BEATS]');
  title(g,a.label+' · '+a.beats.map(b=>'#'+b.ordinal+' '+b.name).join(' | ')+
   ' · '+a.pickups.join(', '));
  g.addEventListener('click',()=>showArea(a.id));
  g.addEventListener('keydown',e=>{
   if(e.key==='Enter'||e.key===' '){e.preventDefault();showArea(a.id)}
  });
 }
 $('stats').textContent=atlas.areas.length+' zones · '+
 atlas.areas.reduce((n,a)=>n+a.beats.length,0)+' story beats · '+
 atlas.areas.filter(a=>a.boss).length+' arenas · '+
 atlas.links.length+' proposed connectors · '+shown.length+' on this layer';
 renderStack();renderDirectory();
}
function renderStack(){
 $('stack').innerHTML='';
 for(const l of atlas.levels){
  const b=document.createElement('button');
  b.textContent=l.label+'  '+(l.elevation_m>=0?'+':'')+l.elevation_m+' m';
  b.className=l.id===state.floor?'selected':'';
  b.addEventListener('click',()=>{
   state.floor=l.id;$('floor').value=l.id;selected='';draw();
  });
  $('stack').appendChild(b);
 }
}
async function start(){
 try{
  const response=await fetch('atlas/outer_heaven_1995.json',{cache:'no-store'});
  if(!response.ok)throw Error('HTTP '+response.status+' for the atlas JSON');
  atlas=await response.json();
  if(atlas.areas.length!==52||
     atlas.areas.reduce((n,a)=>n+a.beats.length,0)!==114)
    throw Error('Atlas is incomplete: must include all 52 areas and 114 beats.');
  for(const l of atlas.levels){
   const opt=document.createElement('option');opt.value=l.id;opt.textContent=l.label;
   $('floor').appendChild(opt);
  }
  for(const m of atlas.missions){
   const opt=document.createElement('option');opt.value=m.id;opt.textContent=m.id+' · '+m.title;
   $('mission').appendChild(opt);
  }
  $('floor').value=state.floor;
  for(const s of atlas.sources.filter(s=>s.url)){
   const a=document.createElement('a');a.href=s.url;a.textContent=s.label;
   a.target='_blank';a.rel='noopener noreferrer';$('sourceLinks').appendChild(a);
  }
  $('floor').addEventListener('change',e=>{state.floor=e.target.value;selected='';draw()});
  $('mission').addEventListener('change',e=>{
   state.mission=e.target.value;
   if(state.mission){
    const a=atlas.areas.find(a=>a.mission===state.mission);
    if(a){state.floor=a.level;$('floor').value=a.level}
   }
   draw();
  });
  $('filter').addEventListener('input',e=>{
   state.query=e.target.value.trim();
   const existing=atlas.areas.some(a=>a.level===state.floor&&areaMatches(a));
   const first=atlas.areas.find(areaMatches);
   if(!existing&&first){state.floor=first.level;$('floor').value=first.level}
   draw();
  });
  $('ghost').addEventListener('change',e=>{state.ghost=e.target.checked;draw()});
  $('connections').addEventListener('change',e=>{state.connections=e.target.checked;draw()});
  draw();
 }catch(err){
  $('stats').textContent='ATLAS FAILED TO LOAD: '+String(err.message||err)+
   '. Use the localhost launcher, not file://.';
  $('detail').textContent='Map data is missing or incomplete: '+String(err);
 }
}
start();
