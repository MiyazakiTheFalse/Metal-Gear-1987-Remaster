// Run: node browser/tests/test_equipment_strip.cjs
const assert = require('node:assert/strict');
const strip = require('../equipment_strip.js');
const inventory = [
 {name:'NO ITEM',usable:true},
 {name:'RATION',usable:true},
 {name:'BINOCULARS',usable:false},
 {name:'CIGARETTES',usable:true},
 {name:'CARD 1',usable:true}
];
assert.equal(strip.wrapped(-1,5),4);
assert.equal(strip.wrapped(5,5),0);
assert.equal(strip.wrapped(8,5),3);
assert.equal(strip.wrapped(-11,5),4);
assert.deepEqual(strip.windowSlots(inventory,0).map(x=>x.name),
 ['CIGARETTES','CARD 1','NO ITEM','RATION','BINOCULARS']);
const model=strip.windowSlots(inventory,1,{'RATION':2,'CIGARETTES':'∞'});
assert.equal(model.length,5);
assert.equal(model[2].centered,true);
assert.equal(model[2].name,'RATION');
assert.equal(model[2].count,2);
assert.equal(model[3].available,false);
assert.equal(strip.inventoryLabel(model[3]),'UNAVAILABLE');
assert.equal(strip.windowSlots([],0).length,0);
const one=strip.windowSlots([{name:'HANDGUN',usable:true}],4);
assert.equal(one.length,5);
assert.equal(one.filter(x=>!x.blank).length,1,
 'one weapon should not render five visible duplicate weapons');
const three=strip.windowSlots([
 {name:'BARE HANDS',usable:true},
 {name:'HANDGUN',usable:true},
 {name:'SUPPRESSOR',usable:false}
],0);
assert.equal(three.filter(x=>!x.blank).length,3);
assert.equal(three[2].blank,false);
assert.equal(three[3].name,'HANDGUN');
assert.equal(three[0].blank,true);
assert.equal(strip.inventoryLabel(three[0]),'');
assert.equal(strip.windowSlots(inventory,0)[2].icon,'Ø');
console.log('PASS: five-card hold-scroll-release strip, wrapping, counters, empty/unowned and centered selection.');
