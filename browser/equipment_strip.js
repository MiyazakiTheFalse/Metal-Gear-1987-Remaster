/* Original MGS-inspired hold/scroll/release inventory presentation.
   Pure deterministic view model. Never owns ammo, unlocks, mission or saving.
   Five visible cards: -2,-1,current,+1,+2, with wrap and stable selection. */
(function(root){
  'use strict';
  const ICONS={
    'BARE HANDS':'✕', 'NO ITEM':'Ø', 'HANDGUN':'⌁','SUPPRESSOR':'⌁',
    'RATION':'+','CARD 1':'01','CARD 2':'02','CARD 3':'03',
    'BINOCULARS':'◎','CIGARETTES':'≋','UNLISTED PROPERTY TAG':'!','PARACHUTE':'▽',
    'FREIGHT CARTON [TEMPORARY]':'□','CARDBOARD BOX [F3 / CARD 2]':'□'
  };
  function wrapped(index, count){
    if(count<=0)return 0;
    return ((Math.trunc(index)%count)+count)%count;
  }
  function windowSlots(slots, selected, quantities={}){
    if(!Array.isArray(slots)||!slots.length)return [];
    const result=[-2,-1,0,1,2].map(offset=>{
      const index=wrapped(selected+offset,slots.length),item=slots[index];
      return {
        index, offset, centered:offset===0, name:item.name,
        icon:ICONS[item.name]||'▧',
        available:item.usable===true, blank:false,
        count:Object.prototype.hasOwnProperty.call(quantities,item.name)?
          quantities[item.name]:null
      };
    });
    // A three-slot weapon inventory must NOT draw duplicated HANDGUN and
    // SUPPRESSOR cards on both ends of a five-position presentation.
    // Center, nearest neighbors, then outer cards take precedence.
    const seen=new Set();
    for(const position of [2,1,3,0,4]){
      const item=result[position];
      item.blank=seen.has(item.index);
      seen.add(item.index);
    }
    return result;
  }
  function inventoryLabel(entry){return entry.blank?'':entry.available?entry.name:'UNAVAILABLE';}
  const api={windowSlots,wrapped,inventoryLabel};
  if(typeof module!=='undefined'&&module.exports)module.exports=api;
  if(root)root.MGEquipmentStrip=api;
})(typeof window!=='undefined'?window:undefined);
