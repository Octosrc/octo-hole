#pragma once

const char PAGE[] PROGMEM = R"HTML(<!doctype html><html dir=rtl lang=ar><head><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1"><meta name=color-scheme content=dark>
<title>درع Octo-Hole لحجب الإعلانات</title><style>
:root{--bg:#0b1220;--surface:#111a2e;--surface2:#16213a;--border:#22304f;--text:#e6edf7;--muted:#8ba0c0;--red:#fb7185;--green:#34d399;--blue:#38bdf8;--amber:#fbbf24}
*{box-sizing:border-box}
body{margin:0;min-height:100vh;font:14px/1.65 system-ui,-apple-system,"Segoe UI",Tahoma,sans-serif;background:linear-gradient(180deg,#0e1729 0,#0b1220 320px);color:var(--text)}
header{position:sticky;top:0;z-index:20;display:flex;flex-wrap:wrap;gap:12px;align-items:center;justify-content:space-between;padding:12px 18px;background:rgba(17,26,46,.92);backdrop-filter:blur(10px);border-bottom:1px solid var(--border)}
.brand{display:flex;align-items:center;gap:11px;min-width:0}
.logo{width:38px;height:38px;flex:none;display:grid;place-items:center;font-size:19px;border-radius:12px;background:linear-gradient(135deg,#22d3ee,#3b82f6);box-shadow:0 6px 18px rgba(56,189,248,.35)}
h1{margin:0;font-size:16.5px;font-weight:700}h1 span{color:var(--blue)}
.sub{font-size:11.5px;color:var(--muted);direction:ltr}
.hactions{display:flex;align-items:center;gap:9px;flex-wrap:wrap}
.pill{display:inline-flex;align-items:center;gap:7px;padding:6px 12px;border-radius:999px;font-size:12.5px;font-weight:700;border:1px solid var(--border);background:var(--surface2);white-space:nowrap}
.pill.on{color:#6ee7b7;border-color:rgba(52,211,153,.45);background:rgba(52,211,153,.12)}
.pill.off{color:#fdba74;border-color:rgba(251,146,60,.45);background:rgba(251,146,60,.12)}
.pill .dot{width:8px;height:8px;flex:none;border-radius:50%;background:currentColor;animation:pulse 1.9s infinite}
@keyframes pulse{0%{box-shadow:0 0 0 0 rgba(52,211,153,.55)}70%{box-shadow:0 0 0 8px rgba(52,211,153,0)}100%{box-shadow:0 0 0 0 rgba(52,211,153,0)}}
.wrap{padding:18px;max-width:1080px;margin:0 auto}
.warn{background:rgba(244,63,94,.1);border:1px solid rgba(244,63,94,.45);color:#fecdd3;border-radius:12px;padding:11px 14px;margin-bottom:16px;font-size:13px}
code{background:rgba(148,163,184,.16);padding:1px 6px;border-radius:5px;font-size:12px;direction:ltr;display:inline-block}
h2{font-size:13px;color:var(--muted);margin:22px 0 10px;display:flex;align-items:center;gap:9px;font-weight:700}
.badge{background:#1b2949;border:1px solid var(--border);color:#9ecbff;border-radius:999px;padding:0 9px;font-size:11.5px;font-weight:700;direction:ltr}
.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(155px,1fr));gap:12px;margin-bottom:18px}
.card{position:relative;background:linear-gradient(180deg,#15213c,#111a2e);border:1px solid var(--border);border-radius:14px;padding:13px 14px 15px;overflow:hidden;transition:.18s}
.card:hover{transform:translateY(-3px);border-color:#2f4470;box-shadow:0 12px 26px rgba(0,0,0,.4)}
.card::after{content:"";position:absolute;inset:auto 0 0 0;height:3px;background:var(--ac,var(--blue))}
.card .ico{width:31px;height:31px;border-radius:9px;display:grid;place-items:center;font-size:15px;background:rgba(56,189,248,.14);margin-bottom:9px}
.card .v{font-size:21px;font-weight:700;direction:ltr;text-align:right}
.card .l{color:var(--muted);font-size:12px}
.card.b{--ac:var(--red)}.card.b .ico{background:rgba(251,113,133,.15)}.card.b .v{color:var(--red)}
.card.a{--ac:var(--green)}.card.a .ico{background:rgba(52,211,153,.15)}.card.a .v{color:var(--green)}
.grid2{display:grid;grid-template-columns:1fr 1fr;gap:14px}
.panel{background:var(--surface);border:1px solid var(--border);border-radius:14px;padding:16px}
.donutwrap{position:relative;width:172px;height:172px;margin:4px auto 14px}
.donut{width:100%;height:100%;transform:rotate(-90deg)}
.donut circle{fill:none;stroke-width:13;stroke-linecap:round}
.donut .bg{stroke:#1c2942}
.donutlabel{position:absolute;inset:0;display:grid;place-content:center;text-align:center}
.donutlabel b{font-size:27px;direction:ltr}
.donutlabel span{display:block;font-size:11.5px;color:var(--muted)}
.legend{display:flex;justify-content:center;gap:18px;font-size:12.5px;color:var(--muted);flex-wrap:wrap}
.li{display:inline-flex;align-items:center;gap:6px}
.li i{width:9px;height:9px;border-radius:50%}
.li b{color:var(--text);direction:ltr}
.row{display:flex;justify-content:space-between;align-items:center;gap:12px;padding:10px 0;border-bottom:1px dashed #1d2b47;font-size:13px}
.row:last-child{border-bottom:0}
.row .k{color:var(--muted)}
.row .v{font-weight:700;direction:ltr}
.bar{height:9px;border-radius:99px;background:#1c2942;overflow:hidden}
.bar i{display:block;height:100%;border-radius:99px;background:linear-gradient(90deg,#22d3ee,#34d399);transition:width .5s}
.bar.w i{background:linear-gradient(90deg,#f87171,#fbbf24)}
.tablewrap{background:var(--surface);border:1px solid var(--border);border-radius:14px;overflow-x:auto}
table{width:100%;border-collapse:collapse;font-size:13px;min-width:440px}
th,td{padding:10px 12px;text-align:right;border-bottom:1px solid #1a2540;white-space:nowrap}
th{position:sticky;top:0;background:#16213a;color:var(--muted);font-size:12px}
tbody tr:last-child td{border-bottom:0}
tbody tr:hover td{background:#141f38}
td.n{font-weight:700;direction:ltr}
.ltr{direction:ltr;unicode-bidi:isolate}
.b{color:var(--red)}.a{color:var(--green)}
button{font:inherit;font-weight:600;color:var(--text);border:1px solid var(--border);background:var(--surface2);border-radius:9px;padding:7px 14px;cursor:pointer;transition:.15s}
button:hover{background:#1e2d4c;transform:translateY(-1px)}
button:active{transform:none}
.btn-red{color:#fda4af;background:rgba(244,63,94,.14);border-color:rgba(244,63,94,.45)}
.btn-red:hover{background:rgba(244,63,94,.26)}
.btn-green{color:#6ee7b7;background:rgba(52,211,153,.13);border-color:rgba(52,211,153,.45)}
.btn-green:hover{background:rgba(52,211,153,.24)}
.btn-sm{padding:4px 11px;font-size:12px;border-radius:8px}
select,input{font:inherit;background:#0d1526;border:1px solid var(--border);color:var(--text);border-radius:9px;padding:7px 10px;outline:none}
select:focus,input:focus{border-color:var(--blue);box-shadow:0 0 0 3px rgba(56,189,248,.18)}
input::placeholder{color:#5f7396}
input[type=file]{padding:0;border:0;background:none}
input[type=file]::file-selector-button{font:inherit;font-weight:600;background:var(--surface2);border:1px solid var(--border);color:var(--text);border-radius:9px;padding:7px 13px;cursor:pointer;margin-inline-end:10px}
.tag{background:rgba(244,63,94,.16);color:#fda4af;border:1px solid rgba(244,63,94,.4);border-radius:6px;padding:1px 7px;font-size:11px;margin-inline-start:6px}
.addrow{display:flex;gap:9px;margin-bottom:11px;flex-wrap:wrap}
.addrow input{flex:1;min-width:190px}
.empty{color:var(--muted);text-align:center;padding:20px}
.hint{color:var(--muted);font-size:12px;margin:8px 0 0}
.upmsg{font-size:12.5px;color:var(--muted);margin-inline-start:8px}
.foot{color:#5f7396;font-size:11.5px;text-align:center;margin:28px 0 6px}
@media(max-width:760px){.grid2{grid-template-columns:1fr}}
@media(max-width:640px){header{padding:10px 12px}.wrap{padding:13px}h1{font-size:15px}.hactions{width:100%}}
</style></head><body>
<header>
<div class=brand><div class=logo>🛡️</div><div><h1>درع <span>Octo-Hole</span> لحجب الإعلانات</h1><div class=sub id=host></div></div></div>
<div class=hactions>
<span id=statuspill class=pill on><i class=dot></i>جارٍ التحميل…</span>
<select id=pausedur><option value=30>30 ثانية</option><option value=300 selected>5 دقائق</option><option value=1800>30 دقيقة</option><option value=0>حتى إعادة التفعيل</option></select>
<button id=pausebtn class=btn-red onclick=togglePause()>إيقاف مؤقت</button>
</div>
</header>
<div class=wrap>
<div id=credwarn class=warn style="display:none">⚠️ <b>ما زالت بيانات الدخول الافتراضية مفعّلة.</b> القيمة <code>WEB_PASS</code> في <code>secrets.h</code> هي القيمة المبدئية المنشورة في المستودع — غيّرها وأعد برمجة الجهاز قبل استخدامه على شبكة غير موثوقة.</div>

<div class=cards id=sys></div>

<div class=grid2>
<div class=panel>
<h2>توزيع الطلبات <span class=badge id=donutbadge>0</span></h2>
<div class=donutwrap>
<svg class=donut viewBox="0 0 120 120" aria-hidden=true>
<defs><linearGradient id=dg x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#fb7185"/><stop offset="1" stop-color="#f97316"/></linearGradient></defs>
<circle class=bg cx=60 cy=60 r=52/><circle class=fg id=donutfg cx=60 cy=60 r=52 stroke="url(#dg)" stroke-dasharray="0 327"/>
</svg>
<div class=donutlabel><b id=donutpct>0%</b><span>محظور</span></div>
</div>
<div class=legend>
<span class=li><i style="background:#fb7185"></i>محظور <b id=lgblocked>0</b></span>
<span class=li><i style="background:#34d399"></i>مسموح <b id=lgallowed>0</b></span>
</div>
</div>
<div class=panel>
<h2>حالة النظام</h2>
<div class=row><span class=k>الذاكرة الحرة</span><span class=v id=heapv>–</span></div>
<div class="bar w" style="margin-top:8px"><i id=heapbar style="width:0%"></i></div>
<div class=row style="margin-top:14px"><span class=k>قوة الإشارة</span><span class=v id=rssiv>–</span></div>
<div class="bar" style="margin-top:8px"><i id=rssibar style="width:0%"></i></div>
<div class=row style="margin-top:14px"><span class=k>حرارة الرقاقة</span><span class=v id=tempv>–</span></div>
<div class=row><span class=k>مدة التشغيل</span><span class=v id=uptimev>–</span></div>
</div>
</div>

<h2>الأجهزة المتصلة <span class=badge id=nbClients>0</span></h2>
<div class=tablewrap><table id=ct><thead><tr><th>الجهاز (IP)</th><th>عنوان MAC</th><th>محظور</th><th>مسموح</th><th>إجراء</th></tr></thead><tbody></tbody></table></div>

<h2>النطاقات المحظورة يدوياً <span class=badge id=nbCustom>0</span></h2>
<div class=addrow><input id=dom placeholder="ads.example.com" dir=ltr><button class=btn-green onclick=addDom()>حجب نطاق</button></div>
<div class=tablewrap><table id=cl><tbody></tbody></table></div>

<h2>قائمة الحظر — الرفع</h2>
<div class=panel>
<form id=upf><input type=file id=blf accept=.bin><button>رفع قائمة الحظر</button><span id=upmsg class=upmsg></span></form>
<p class=hint>ابنِ ملف <code>blocklist.bin</code> بواسطة <code>tools/build_blocklist.py</code> ثم ارفعه من هنا — بدون كابل USB.</p>
</div>

<h2>الشبكة (WiFi)</h2>
<div class=panel>
<p class=hint style="margin:0 0 10px">حذف بيانات الاتصال المحفوظة وإعادة التشغيل لفتح صفحة الإعداد.</p>
<button class=btn-red onclick="if(confirm('سيتم نسيان شبكة الواي فاي والدخول في وضع الإعداد. هل أنت متأكد؟'))forgetWifi()">نسيان الواي فاي</button>
</div>

<div class=foot>Octo-Hole — يعمل محلياً على شبكتك، بدون سحابة.</div>
</div><script>
function fmt(n){return Number(n||0).toLocaleString('en-US')}
function esc(s){return String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]))}
const CSRF_HDRS={'X-Requested-With':'octo-hole'}
const CIRC=326.72
let ON=true
function togglePause(){fetch(ON?'/pause?s='+pausedur.value:'/resume',{headers:CSRF_HDRS}).then(load);}
async function load(){let s=await(await fetch('/stats.json')).json();
host.textContent=s.ip;
credwarn.style.display=s.defcreds?'block':'none';
ON=s.blocking!==false;
statuspill.className='pill '+(ON?'on':'off');
statuspill.innerHTML='<i class=dot></i>'+(ON?'الحظر مفعّل':(s.resumeIn>0?'متوقف — يستأنف خلال '+s.resumeIn+' ث':'الحظر متوقف'));
pausebtn.textContent=ON?'إيقاف مؤقت':'استئناف';
pausebtn.className=ON?'btn-red':'btn-green';
pausedur.style.display=ON?'':'none';
sys.innerHTML=[
['🚫',fmt(s.blocked),'إجمالي الحظر','b'],
['✅',fmt(s.allowed),'إجمالي السماح','a'],
['📋',fmt(s.domains),'نطاقات قائمة الحظر',''],
['📡',s.clients.length,'أجهزة متصلة',''],
['🌡️',s.temp+' °C','حرارة الرقاقة',''],
['⏱️',s.uptime,'مدة التشغيل','']
].map(c=>`<div class="card ${c[3]}"><div class=ico>${c[0]}</div><div class=v>${c[1]}</div><div class=l>${c[2]}</div></div>`).join('');
let tot=s.blocked+s.allowed, pct=tot?Math.min(100,s.blocked/tot*100):0;
donutfg.style.strokeDasharray=CIRC;
donutfg.style.strokeDashoffset=(CIRC*(1-pct/100)).toFixed(1);
donutpct.textContent=(pct>0&&pct<10?pct.toFixed(1):Math.round(pct))+'%';
donutbadge.textContent=fmt(tot);
lgblocked.textContent=fmt(s.blocked);lgallowed.textContent=fmt(s.allowed);
let ht=s.heapTotal||0;
heapv.textContent=Math.round(s.heap/1024)+' / '+(ht?Math.round(ht/1024):'؟')+' KB';
heapbar.style.width=(ht?Math.min(100,s.heap/ht*100):0).toFixed(1)+'%';
rssiv.textContent=s.rssi+' dBm';
rssibar.style.width=Math.max(0,Math.min(100,(s.rssi+100)/70*100)).toFixed(0)+'%';
tempv.textContent=s.temp+' °C';
uptimev.textContent=s.uptime;
nbClients.textContent=s.clients.length;
nbCustom.textContent=s.custom.length;
ct.tBodies[0].innerHTML=s.clients.sort((a,b)=>(b.blocked+b.allowed)-(a.blocked+a.allowed)).map(c=>
`<tr><td><span class=ltr>${esc(c.ip)}</span>${c.banned?'<span class=tag>محظور</span>':''}</td>
<td><span class=ltr>${esc(c.mac)}</span></td>
<td class="n b">${fmt(c.blocked)}</td><td class="n a">${fmt(c.allowed)}</td>
<td><button class="btn-sm ${c.banned?'btn-green':'btn-red'} ban" data-ip="${esc(c.ip)}">${c.banned?'رفع الحظر':'حظر'}</button></td></tr>`).join('');
cl.tBodies[0].innerHTML=s.custom.map(d=>`<tr><td><span class=ltr>${esc(d)}</span></td>
<td><button class="btn-sm btn-red rmbtn" data-d="${esc(d)}">إزالة</button></td></tr>`).join('')||'<tr><td colspan=2 class=empty>لا توجد نطاقات مُضافة بعد</td></tr>';}
function addDom(){let d=dom.value.trim();if(d){fetch('/addblock?d='+encodeURIComponent(d),{headers:CSRF_HDRS}).then(()=>{dom.value='';load()})}}
ct.addEventListener('click',e=>{if(e.target.classList.contains('ban'))fetch('/ban?ip='+e.target.dataset.ip,{headers:CSRF_HDRS}).then(load)});
cl.addEventListener('click',e=>{if(e.target.classList.contains('rmbtn'))fetch('/unblock?d='+encodeURIComponent(e.target.dataset.d),{headers:CSRF_HDRS}).then(load)});
function forgetWifi(){fetch('/forgetwifi',{headers:CSRF_HDRS}).then(r=>r.text()).then(t=>alert(t))}
upf.onsubmit=async e=>{e.preventDefault();let f=blf.files[0];if(!f)return;
upmsg.textContent='جارٍ رفع '+(f.size/1048576).toFixed(2)+' م.ب...';
let fd=new FormData();fd.append('f',f);
try{let r=await fetch('/upload',{method:'POST',headers:CSRF_HDRS,body:fd});upmsg.textContent=r.ok?'✓ تم التحديث':'✗ مرفوض — '+await r.text();}
catch(_){upmsg.textContent='✗ فشل الرفع';}
blf.value='';setTimeout(load,600);};
load();setInterval(load,3000);
</script></body></html>)HTML";
