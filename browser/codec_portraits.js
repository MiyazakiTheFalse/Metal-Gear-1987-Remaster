/* Original 1995 radio-portrait performance layer.
 * Character features are based on Konami's Japanese 1987 MSX2 manual,
 * pages 10-11. Original vector illustrations; no extracted sprites,
 * no pasted celebrity portraits or ripped game assets.
 *
 * Portraits are source-quality, expressive 2D illustrations; final
 * physically-based 3D likeness renders / VO-driven phoneme alignment
 * still require character production and approval.
 */
(function(root){
'use strict';
const PEOPLE=Object.freeze({
 snake:{label:'SOLID SNAKE',role:'FOXHOUND / ROOKIE',h:'#453f33',skin:'#c8a17f',cloth:'#315d57',variant:'swept',age:'young'},
 bigBoss:{label:'BIG BOSS',role:'FOXHOUND / COMMAND',h:'#777166',skin:'#bc9576',cloth:'#2e5958',variant:'boss',age:'mature'},
 schneider:{label:'KYLE SCHNEIDER',role:'RESISTANCE / ENGINEER',h:'#897963',skin:'#c09d83',cloth:'#454f4d',variant:'short',age:'adult'},
 diane:{label:'DIANE',role:'RESISTANCE / INTELLIGENCE',h:'#b59c7d',skin:'#d4af96',cloth:'#40444a',variant:'punk',age:'young'},
 jennifer:{label:'JENNIFER',role:'RESISTANCE / MEDICAL',h:'#65544b',skin:'#d2af94',cloth:'#697c70',variant:'cap',age:'young'},
 fox:{label:'GRAY FOX',role:'FOXHOUND / CAPTIVE',h:'#756856',skin:'#aa8a70',cloth:'#686e66',variant:'bandage',age:'adult'},
 pettrovich:{label:'DR. PETTROVICH',role:'ENGINEER / CAPTIVE',h:'#9e9582',skin:'#d1af8c',cloth:'#c1c9bf',variant:'bald',age:'elder'},
 ellen:{label:'ELLEN PETTROVICH',role:'CIVILIAN / CAPTIVE',h:'#4f403c',skin:'#cba28c',cloth:'#6a7269',variant:'long',age:'young'}
});
const REMOTE_IDS=new Set(['bigBoss','schneider','diane','jennifer','pettrovich','ellen']);
function speakerId(s){
 const t=String(s||'').toUpperCase();
 if(t.includes('BIG BOSS'))return 'bigBoss';
 if(t.includes('SCHNEIDER'))return 'schneider';
 if(t.includes('DIANE'))return 'diane';
 if(t.includes('JENNIFER'))return 'jennifer';
 if(t.includes('GRAY FOX')||t.includes('GREY FOX'))return 'fox';
 if(t.includes('PETTROVICH')||t.includes('PETROVICH'))return t.includes('ELLEN')||t.includes('ELEN')?'ellen':'pettrovich';
 if(t.includes('SNAKE'))return 'snake';
 return null;
}
function hair(p){
 switch(p.variant){
 case 'swept':return '<path d="M47 107 Q31 72 60 42 Q79 23 102 39 Q116 21 144 37 Q161 48 153 80 Q143 66 129 65 Q115 42 96 57 Q67 63 64 96 Z" fill="'+p.h+'"/><path d="M56 72 Q75 29 121 49 Q98 49 82 76 Q74 90 55 91Z" fill="#a79b82" opacity=".44"/><path d="M140 48 Q169 71 151 108 L140 128 137 93Z" fill="'+p.h+'"/>';
 case 'boss':return '<path d="M45 100 Q30 43 76 30 Q108 19 139 38 Q171 58 152 104 L146 78 Q128 58 110 64 Q76 50 61 91Z" fill="'+p.h+'"/><path d="M62 52 Q94 30 130 49" stroke="#cbc0a1" stroke-width="9" fill="none" opacity=".35"/><path d="M48 111 Q41 155 69 182 Q92 208 128 183 Q158 151 149 112 L144 157 123 185 Q99 193 70 173 56 145 55 111Z" fill="#544c42"/>';
 case 'short':return '<path d="M47 102 Q40 50 68 39 Q85 17 101 36 L118 27 137 43 148 30 155 64 147 98 Q136 64 115 67 Q81 57 63 91Z" fill="'+p.h+'"/><path d="M49 67 L72 33 65 77Z M94 49 L101 27 112 65Z M133 49 L153 34 140 84Z" fill="#b9a485" opacity=".45"/>';
 case 'punk':return '<path d="M45 99 Q35 61 66 45 L78 17 90 42 113 24 121 47 145 35 158 79 143 111 Q130 69 100 73 Q70 68 55 110Z" fill="'+p.h+'"/><path d="M48 110 Q23 98 43 67 L54 57 57 115Z M151 107 Q174 84 150 61L140 85Z" fill="#766e61"/>';
 case 'cap':return '<path d="M51 101 Q38 53 75 42 Q103 28 140 47 Q161 76 151 106 L148 161 130 193 72 186 53 162Z" fill="'+p.h+'"/><path d="M40 69 Q47 27 103 28 Q151 30 159 72 Q111 86 47 78Z" fill="#626f65"/><path d="M34 75 Q87 88 159 73 L176 79 Q101 105 34 81Z" fill="#83968a" stroke="#273e39" stroke-width="3"/>';
 case 'bandage':return '<path d="M46 108 Q38 47 88 37 Q132 28 150 72 L151 112 135 70 Q89 56 58 101Z" fill="'+p.h+'"/><path d="M43 58 Q94 38 156 60 L150 82 Q95 63 48 81Z" fill="#bbb7a6" stroke="#77776d" stroke-width="2"/><path d="M52 45 L137 73 M79 39 L157 63 M49 78 L124 54" stroke="#ede8d4" stroke-opacity=".50" stroke-width="7"/>';
 case 'bald':return '<path d="M47 117 Q35 48 100 35 Q165 49 153 118 L144 77 Q117 36 80 57 Q58 75 56 116Z" fill="#8f8170" opacity=".40"/><path d="M47 119 Q41 159 69 191L62 158 57 116Z M153 118 Q159 160 132 188L144 160Z" fill="'+p.h+'"/>';
 case 'long':return '<path d="M47 101 Q37 40 78 35 Q123 19 154 48 L162 204 143 207 139 83 Q107 52 102 62 Q70 69 58 100 L60 209 41 210Z" fill="'+p.h+'"/><path d="M100 49 L96 94" stroke="#9f8d7c" stroke-width="4"/>';
 default:return '';
 }
}
function accents(p){
 let out='';
 if(p.variant==='boss'){
   out+='<path d="M70 147 Q83 173 99 176 Q115 176 133 144 L129 181 Q112 199 94 193 L68 178Z" fill="#5a5046"/><path d="M72 148L90 157 119 158 132 147" stroke="#93887a" stroke-opacity=".6" stroke-width="5"/>';
   // Big Boss loses his right eye (viewer left). Historical manual depicts
   // a patch; the strap crosses the forehead, not both eyes.
   out+='<path d="M55 82 L129 69" stroke="#353c3a" stroke-width="8" fill="none"/><path d="M61 93 Q72 85 88 91 L86 122 Q74 130 64 116Z" fill="#171f20" stroke="#546561" stroke-width="2"/>';
 }
 if(p.variant==='bandage'){
   out+='<path d="M55 95 L137 111" stroke="#c5c1b2" stroke-width="13" stroke-opacity=".85"/><path d="M66 94L127 106" stroke="#f0ecdd" stroke-width="3" opacity=".4"/><path d="M68 159 Q87 167 107 164 L126 157" stroke="#6b5147" stroke-width="2"/>';
 }
 if(p.variant==='bald'){
   out+='<g fill="none" stroke="#40514d" stroke-width="3"><ellipse cx="80" cy="111" rx="18" ry="13"/><ellipse cx="122" cy="111" rx="18" ry="13"/><path d="M98 111L104 111M62 108L51 103M140 106L151 103"/></g><path d="M87 158 Q103 168 116 154" stroke="#837363" stroke-width="4" fill="none"/>';
 }
 if(p.variant==='punk')
   out+='<path d="M146 143 l13 14 -13 8Z" fill="#bdd0c6"/>';
 return out;
}
function faceSvg(id){
 const p=PEOPLE[id]||PEOPLE.snake;
 const key=id.replace(/[^a-zA-Z]/g,'');
 return `<svg class="mg-codec-portrait-art" viewBox="0 0 200 236" role="img" aria-label="${p.label}, original interpretation of the 1987 manual">
 <defs><linearGradient id="bg_${key}" x2="1" y2="1"><stop stop-color="#183835"/><stop offset=".47" stop-color="#0b1e20"/><stop offset="1" stop-color="#020d12"/></linearGradient>
 <linearGradient id="skin_${key}" x1=".05" y1=".2" x2=".97" y2=".8"><stop stop-color="#ead6af"/><stop offset=".33" stop-color="${p.skin}"/><stop offset=".65" stop-color="${p.skin}"/><stop offset="1" stop-color="#594536"/></linearGradient>
 <linearGradient id="cloth_${key}" x1="0" y1="0" x2="1" y2="1"><stop stop-color="#7a9484"/><stop offset=".55" stop-color="${p.cloth}"/><stop offset="1" stop-color="#1a2e2c"/></linearGradient>
 <radialGradient id="shine_${key}"><stop stop-color="#91c9bd" stop-opacity=".32"/><stop offset="1" stop-color="#91c9bd" stop-opacity="0"/></radialGradient>
 <clipPath id="cut_${key}"><rect width="200" height="236" rx="3"/></clipPath></defs>
 <g clip-path="url(#cut_${key})"><rect width="200" height="236" fill="url(#bg_${key})"/>
 <ellipse cx="85" cy="72" rx="124" ry="115" fill="url(#shine_${key})"/>
 <g stroke="#91d5b7" stroke-opacity=".09" stroke-width=".5">
 ${Array.from({length:14},(_,i)=>'<path d="M0 '+(i*18+7)+' H200"/>').join('')}</g>
 <path d="M-5 236 Q15 196 66 186 L84 167 116 168 138 186 Q194 194 207 236Z" fill="url(#cloth_${key})"/>
 <path d="M86 169 L82 205 101 230 123 204 117 169" fill="#a18870" opacity=".83"/>
 <path d="M82 176 L101 204 117 177 131 193 101 228 69 192Z" fill="#d4ddce" opacity=".55"/>
 <path d="M65 95 Q57 82 52 100 Q49 121 67 137 L69 165 Q83 187 102 188 Q127 187 140 162 L145 137 Q162 121 153 98 Q150 91 145 95 Q146 54 104 50 Q56 47 65 95Z" fill="url(#skin_${key})" stroke="#493f36" stroke-width="2"/>
 <path d="M70 89Q96 80 111 84 L122 95 137 92" stroke="#fff4d6" stroke-opacity=".13" fill="none" stroke-width="5"/>
 <path d="M66 108 Q80 96 94 109 M109 108 Q123 97 138 108" stroke="#382f2a" stroke-width="4" fill="none"/>
 <path d="M68 111 Q81 104 94 114 M109 114 Q122 104 136 111" stroke="#d6c6a6" stroke-width="2" fill="none"/>
 <ellipse cx="83" cy="111" rx="3.4" ry="5.4" fill="#28352f"/><ellipse cx="121" cy="111" rx="3.4" ry="5.4" fill="#28352f"/>
 <g class="mg-codec-eyelids"><path d="M68 108 Q80 117 95 108" stroke="${p.skin}" stroke-width="12" fill="none"/><path d="M107 108 Q121 117 138 108" stroke="${p.skin}" stroke-width="12" fill="none"/></g>
 <path d="M103 111 L99 141 Q101 146 110 144" stroke="#725949" stroke-width="3" fill="none" opacity=".72"/>
 <path d="M82 154 Q100 146 119 153" stroke="#704b42" stroke-width="2" fill="none"/>
 <g class="mg-codec-mouth"><ellipse cx="102" cy="155" rx="18" ry="3" fill="#432e2d"/><path d="M87 153 Q102 148 117 153" stroke="#d29a85" stroke-width="2" fill="none"/></g>
 <path d="M61 125Q66 143 75 152 M141 125Q138 148 128 158" stroke="#58493e" stroke-opacity=".27" stroke-width="4" fill="none"/>
 ${hair(p)}${accents(p)}
 <path d="M0 225 L200 225" stroke="#79b2a3" stroke-opacity=".48" stroke-width="1"/>
 <path d="M11 0V236 M189 0V236" stroke="#7ac8b1" stroke-opacity=".12"/>
 </g></svg>`;
}
let view=null,current='';
function ensure(){
 if(view)return view;
 if(typeof document==='undefined'||typeof document.createElement!=='function')return null;
 const css=document.createElement('style');
 css.textContent=`#mgCodecHeads{position:absolute;bottom:215px;right:28px;z-index:5;width:min(430px,47vw);background:linear-gradient(135deg,#061a1feD,#030b12f0);border:1px solid #597e75;border-top:2px solid #83b69e;box-shadow:0 12px 30px #000c;opacity:0;transform:translateY(12px);transition:opacity .17s,transform .17s;pointer-events:none;padding:9px 12px 11px;color:#d9e8dd}
 #mgCodecHeads.visible{opacity:1;transform:translateY(0)}#mgCodecHeads .mg-heads-row{display:flex;gap:12px;align-items:stretch}
 #mgCodecHeads .mg-head{width:88px;flex-shrink:0;overflow:hidden;border:1px solid #4d7968;background:#0d2527;position:relative}
 #mgCodecHeads .mg-head.remote{width:108px}#mgCodecHeads .mg-codec-portrait-art{display:block;width:100%;height:120px;filter:contrast(1.11) saturate(.72)}
 #mgCodecHeads .mg-head:not(.speaking) .mg-codec-portrait-art{filter:contrast(1.08) saturate(.45) brightness(.78)}
 #mgCodecHeads .mg-label{font:700 10px ui-monospace,monospace;color:#d8dfcb;letter-spacing:.07em}
 #mgCodecHeads .mg-sub{font:9px ui-monospace,monospace;color:#83c5aa;margin-top:5px}
 #mgCodecHeads .mg-meter{height:7px;margin-top:9px;background:repeating-linear-gradient(90deg,#294c42 0 4px,transparent 4px 8px)}
 #mgCodecHeads .mg-head.speaking .mg-codec-mouth{animation:mg-viseme .19s steps(2,end) infinite;transform-origin:102px 155px}
 #mgCodecHeads .mg-codec-eyelids{animation:mg-blink 5.9s linear infinite;transform-origin:100px 110px;transform:scaleY(0)}
 #mgCodecHeads .mg-head .mg-codec-portrait-art{animation:mg-live-face 6s ease-in-out infinite}
 #mgCodecHeads .mg-head.speaking .mg-meter{background:repeating-linear-gradient(90deg,#7fd6b1 0 4px,transparent 4px 8px)}
 @keyframes mg-viseme{0%,100%{transform:scaleY(1)}35%{transform:scaleY(2.5)}67%{transform:scaleY(1.25)}}
 @keyframes mg-blink{0%,19%,20%,21%,72%,73%,74%,100%{transform:scaleY(0)}19.5%,73.5%{transform:scaleY(1)}}
 @keyframes mg-live-face{0%,100%{transform:translateY(0)}50%{transform:translateY(1px)}}
 #mgCodecHeads .mg-detail{min-width:0;flex:1;padding-top:4px}
 #mgCodecHeads .mg-freq{font:700 22px ui-monospace,monospace;color:#9be0b7;margin-top:12px}
 #mgCodecHeads .mg-mode{font:9px ui-monospace,monospace;letter-spacing:.13em;color:#97b5a9;margin-top:8px}
 @media(max-width:740px){#mgCodecHeads{right:10px;bottom:230px;width:min(305px,86vw);padding:7px}#mgCodecHeads .mg-head{width:70px}#mgCodecHeads .mg-head.remote{width:82px}#mgCodecHeads .mg-codec-portrait-art{height:88px}#mgCodecHeads .mg-freq{font-size:17px}}`;
 document.head.appendChild(css);
 view=document.createElement('section');
 view.id='mgCodecHeads';view.setAttribute('aria-hidden','true');
 const host=document.getElementById('hud')||document.body;
 host.appendChild(view);
 return view;
}
const frequencies={bigBoss:'120.85',schneider:'120.79',diane:'120.33',jennifer:'120.48'};
function show(line){
 const id=speakerId(line?.speaker);
 if(!id || (id==='snake'&&!['tagQuestion'].includes(line.id)) ||
    (id==='fox'&&!['radioFox'].includes(line.id))){hide();return;}
 const panel=ensure();if(!panel)return;
 const remote=id==='snake'?'bigBoss':id;
 const key=id+'|'+remote;
 if(key!==current){
  current=key;
  const left=PEOPLE.snake,right=PEOPLE[remote]||PEOPLE.bigBoss;
  panel.innerHTML=`<div class="mg-heads-row"><div class="mg-head ${id==='snake'?'speaking':''}" id="mgCodecLeft">${faceSvg('snake')}</div>
    <div class="mg-detail"><div class="mg-label" id="mgCodecName">${right.label}</div>
    <div class="mg-sub">${right.role}</div><div class="mg-freq">${frequencies[remote]||'—'}</div>
    <div class="mg-mode">FOXHOUND RADIO / 1995</div><div class="mg-meter"></div></div>
    <div class="mg-head remote ${id!=='snake'?'speaking':''}" id="mgCodecRight">${faceSvg(remote)}</div></div>`;
 }
 panel.classList.add('visible');
 const left=document.getElementById('mgCodecLeft'),right=document.getElementById('mgCodecRight');
 if(left)left.classList.toggle('speaking',id==='snake');
 if(right)right.classList.toggle('speaking',id!=='snake');
}
function hide(){if(view)view.classList.remove('visible')}
root.MGCodecPortraits={people:PEOPLE,faceSvg,speakerId,show,hide,
 reference:'Konami 1987 Japanese MSX2 manual character plates pages 10–11'};
})(typeof window==='undefined'?globalThis:window);
