#!/usr/bin/env node
/* Deterministic, dependency-free export of hand-authored original animated
 * browser codec portraits into 3 actual SVG art frames per character.
 * SVG is editable source art, not photoreal final renders.
 *
 * Run: node tools/export_codec_portraits.cjs
 * For Unreal, rasterize exported SVGs to PNG and import the resulting PNGs.
 */
'use strict';
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm');
const root=path.resolve(__dirname,'..');
const source=fs.readFileSync(path.join(root,'browser/codec_portraits.js'),'utf8');
const sandbox={window:{}};
vm.runInNewContext(source,sandbox,{filename:'codec_portraits.js'});
const portraits=sandbox.window.MGCodecPortraits;
if(!portraits)throw Error('Portrait renderer missing');
const target=path.join(root,'assets/source/codec');
fs.mkdirSync(target,{recursive:true});
let count=0;
for(const id of Object.keys(portraits.people)){
 const base=portraits.faceSvg(id);
 if(!base.includes('mg-codec-mouth')||!base.includes('mg-codec-eyelids'))
  throw Error('Missing facial motion tracks for '+id);
 for(const expression of ['Neutral','Talk','Blink']){
  let svg=base.replace('<svg ', '<svg xmlns="http://www.w3.org/2000/svg" ');
  if(expression==='Neutral')
   svg=svg.replace('<svg xmlns=','<svg xmlns=').replace(
      '<defs>', '<style>.mg-codec-eyelids{transform-origin:100px 110px;transform:scaleY(0)}</style><defs>');
  if(expression==='Talk'){
   svg=svg.replace('rx="18" ry="3"','rx="16" ry="8"');
   svg=svg.replace('<defs>', '<style>.mg-codec-eyelids{transform-origin:100px 110px;transform:scaleY(0)}</style><defs>');
  }
  // For Blink, leave closed eyelid artwork intact. The actor's iris and
  // mouth remain otherwise unchanged, so frames can switch seamlessly.
  const name='T_Codec_'+id+'_'+expression+'.svg';
  fs.writeFileSync(path.join(target,name),svg+'\n','utf8');
  count++;
 }
}
console.log('Exported',count,'original editable SVG frames to',target);
