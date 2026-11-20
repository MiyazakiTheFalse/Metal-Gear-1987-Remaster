/* Game-owned animation intent, independent of Three.js and authoritative gameplay.
 * Never grant visibility, damage or progression from a cosmetic clip.
 * Local GLB and Unreal import use the SAME named source animation vocabulary.
 */
(function(root){
 'use strict';
 const finite=(v,d=0)=>Number.isFinite(v)?v:d;
 const clipSet=new Set(['Idle','IdleAlert','IdleLook','IdleWary','Walk','Run',
  'Crouch','Prone','Down','Aim','Cover','Melee','Reaching','Hit',
  'RadioListen','CrouchAim','ProneAim','Captive','Address','Shoulder','Recoil','Search']);
 function idleVariant(id,time){
  const label=String(id||'guard');
  let hash=0;for(let i=0;i<label.length;i++)hash=(hash*31+label.charCodeAt(i))>>>0;
  const segment=Math.floor((Math.max(0,finite(time))+(hash%13)*.48)/7.25)%7;
  return segment===0?'IdleLook':segment===4?'IdleWary':'Idle';
 }
 function select(actor,speed=0,time=0){
  if(!actor||typeof actor!=='object')return 'Idle';
  const moving=Math.max(0,finite(speed));
  const stance=actor.stance===2?2:actor.stance===1?1:0;
  // Physical state and player commitment outrank atmospheric idle fidgets.
  if(actor.down)return 'Down';
  if(actor.hit)return 'Hit';
  if(actor.kind==='shoot_gunner'){
   if(actor.firing)return 'Recoil';
   if(actor.reloading)return 'Reaching';
   if(actor.telegraph)return 'Shoulder';
   if(actor.addressing)return 'Address';
   if(actor.searching)return 'Search';
  }
  if(actor.melee||actor.knocking)return 'Melee';
  if(actor.reaching||actor.reloading)return 'Reaching';
  if(actor.captive)return 'Captive';
  if(actor.radio)return 'RadioListen';
  if(actor.aiming)return stance===2?'ProneAim':stance===1?'CrouchAim':'Aim';
  if(actor.cover)return 'Cover';
  if(stance===2)return 'Prone';
  if(stance===1)return 'Crouch';
  if(moving>3.1)return 'Run';
  if(moving>.18)return 'Walk';
  if(actor.alert>=3)return 'IdleAlert';
  if(actor.searching||actor.alert===2)return 'IdleLook';
  if(actor.alert===1)return 'IdleWary';
  return idleVariant(actor.id,time);
 }
 function looped(name){
  return !['Down','Hit','Melee','Reaching','Recoil'].includes(name);
 }
 const api=Object.freeze({select,idleVariant,looped,clipSet: Object.freeze([...clipSet])});
 root.MGAnimationIntent=api;
 if(typeof module==='object'&&module.exports)module.exports=api;
})(typeof window!=='undefined'?window:globalThis);
