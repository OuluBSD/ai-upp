#include "ScriptWebHost.h"

NAMESPACE_UPP

static const ScriptWebHostApp& GetScriptWebHostApp(const Http& http)
{
	return static_cast<const ScriptWebHostApp&>(http.App());
}

SKYLARK(HomePage, "")
{
	const ScriptWebHostApp& app = GetScriptWebHostApp(http);
	http.ContentType("text/html; charset=utf-8")
	    << "<!DOCTYPE html><html><head><title>ScriptWebHost</title>"
	    << "<style>"
	       ":root{--chrome:#17261c;--panel:#f3efe3;--table:#289c43;--ink:#101612;--line:rgba(16,22,18,.18);"
	       "--zone:rgba(255,255,255,.14);--zone-line:rgba(255,255,255,.35);--button:#f4e8b2;--button-ink:#2d2b1a}"
	       "*{box-sizing:border-box}body{margin:0;font-family:'Iosevka Aile',ui-sans-serif,sans-serif;"
	       "background:linear-gradient(180deg,#ebe6da,#d8d0bf);color:var(--ink)}"
	       ".shell{display:grid;grid-template-columns:minmax(320px,1fr) 360px;min-height:100vh}"
	       ".stage-wrap{padding:24px}.stage-head{display:flex;justify-content:space-between;gap:16px;align-items:end;margin-bottom:16px}"
	       ".title{font-size:28px;font-weight:700;letter-spacing:.02em}.meta{font-size:13px;opacity:.75}"
	       ".table-frame{background:#0d1710;padding:18px;border-radius:24px;box-shadow:0 18px 60px rgba(0,0,0,.24)}"
	       ".table{position:relative;aspect-ratio:4/3;width:min(100%,1000px);background:var(--table);border-radius:18px;"
	       "overflow:hidden;box-shadow:inset 0 0 0 1px rgba(255,255,255,.18)}"
	       ".obj{position:absolute;border-radius:12px;overflow:hidden}"
	       ".obj.zone{background:var(--zone);border:1px dashed var(--zone-line)}"
	       ".obj.label{display:flex;align-items:center;justify-content:center;padding:6px 10px;"
	       "font-size:15px;color:#f7f4eb;text-shadow:0 1px 0 rgba(0,0,0,.25)}"
	       ".obj.button{display:flex;align-items:center;justify-content:center;padding:6px 12px;background:var(--button);"
	       "color:var(--button-ink);border:1px solid rgba(0,0,0,.16);font-weight:700;box-shadow:0 2px 8px rgba(0,0,0,.12)}"
	       ".obj.button.disabled{opacity:.45}"
	       ".obj.hand::after,.obj.trick::after,.obj.container::after{content:attr(data-id);position:absolute;left:8px;top:6px;"
	       "font-size:11px;letter-spacing:.04em;text-transform:uppercase;color:rgba(255,255,255,.72)}"
	       ".side{border-left:1px solid var(--line);background:rgba(255,255,255,.45);backdrop-filter:blur(8px);padding:24px;overflow:auto}"
	       ".side h2{margin:0 0 12px;font-size:15px;text-transform:uppercase;letter-spacing:.08em}.side pre{margin:0;background:#f8f5ed;"
	       "border:1px solid var(--line);padding:14px;border-radius:12px;overflow:auto;font:12px/1.35 'Iosevka Term',ui-monospace,monospace}"
	       ".legend{display:flex;flex-wrap:wrap;gap:8px;margin:12px 0 18px}.chip{padding:6px 10px;border-radius:999px;"
	       "font-size:12px;background:rgba(23,38,28,.08);border:1px solid rgba(23,38,28,.12)}"
	       "@media (max-width:1100px){.shell{grid-template-columns:1fr}.side{border-left:0;border-top:1px solid var(--line)}}"
	    << "</style></head><body>"
	    << "<div class='shell'>"
	    << "<section class='stage-wrap'>"
	    << "<div class='stage-head'>"
	    << "<div><div class='title'>ScriptWebHost</div><div class='meta'>Session <code>" << app.GetSessionId() << "</code></div></div>"
	    << "<div class='meta'>GameState <code>" << app.GetGamestatePath() << "</code></div>"
	    << "</div>"
	    << "<div class='table-frame'><div id='table' class='table'></div></div>"
	    << "</section>"
	    << "<aside class='side'>"
	    << "<h2>Bootstrap</h2>"
	    << "<div class='legend'>"
	    << "<span class='chip'><a href='/api/status'>/api/status</a></span>"
	    << "<span class='chip'><a href='/api/bootstrap'>/api/bootstrap</a></span>"
	    << "<span class='chip'>DOM renderer</span>"
	    << "<span class='chip'>No canvas</span>"
	    << "</div>"
	    << "<pre id='bootstrap'>loading...</pre>"
	    << "</aside>"
	    << "</div>"
	    << "<script>"
	       "const table=document.getElementById('table');"
	       "const dump=document.getElementById('bootstrap');"
	       "function setBox(el,box){"
	         "if(box.left!==undefined)el.style.left=(box.left*100)+'%';"
	         "if(box.right!==undefined)el.style.right=(box.right*100)+'%';"
	         "if(box.top!==undefined)el.style.top=(box.top*100)+'%';"
	         "if(box.bottom!==undefined)el.style.bottom=(box.bottom*100)+'%';"
	         "if(box.width!==undefined)el.style.width=(box.width*100)+'%';"
	         "if(box.height!==undefined)el.style.height=(box.height*100)+'%';"
	         "const tx=(box.transform_x||0)*100;"
	         "const ty=(box.transform_y||0)*100;"
	         "el.style.transform=(tx||ty)?`translate(${tx}%,${ty}%)`:'';"
	       "}"
	       "function classify(obj){"
	         "const role=(obj.user_class||obj.type||'zone').toLowerCase();"
	         "if(role==='button')return 'button';"
	         "if(role==='label')return 'label';"
	         "if(role==='hand')return 'zone hand';"
	         "if(role==='trick')return 'zone trick';"
	         "if(role==='container')return 'zone container';"
	         "return 'zone';"
	       "}"
	       "function textFor(obj){"
	         "if(obj.type==='Button') return obj.label||obj.id||'button';"
	         "if(obj.label) return obj.label;"
	         "return '';"
	       "}"
	       "function render(data){"
	         "dump.textContent=JSON.stringify(data,null,2);"
	         "table.innerHTML='';"
	         "const form=data.form||{};"
	         "const meta=form.meta||{};"
	         "if(meta.background){table.style.background=`rgb(${meta.background})`;}"
	         "const objects=(form.objects||[]);"
	         "const byId=new Map();"
	         "for(const obj of objects){"
	           "const el=document.createElement(obj.type==='Button'?'button':'div');"
	           "el.className='obj '+classify(obj);"
	           "el.dataset.id=obj.id||'';"
	           "el.title=obj.anchor||'';"
	           "const txt=textFor(obj);"
	           "if(txt)el.textContent=txt;"
	           "if(obj.type!=='Button' && !txt && obj.id && !String(el.className).includes('zone')) el.textContent=obj.id;"
	           "if(obj.type==='Button') el.classList.toggle('disabled', false);"
	           "if(obj.id) byId.set(obj.id, el);"
	           "obj.__el=el;"
	         "}"
	         "for(const obj of objects){"
	           "const layout=(obj.layout||{});"
	           "const local=((layout.local_to_parent||{}).browser)||null;"
	           "const root=layout.browser||{};"
	           "const parent=obj.parent && byId.get(obj.parent);"
	           "setBox(obj.__el, parent && local ? local : root);"
	           "(parent||table).appendChild(obj.__el);"
	         "}"
	       "}"
	       "fetch('/api/bootstrap').then(r=>r.json()).then(render).catch(e=>{dump.textContent=String(e);});"
	    << "</script></body></html>";
}

SKYLARK(Status, "api/status")
{
	const ScriptWebHostApp& app = GetScriptWebHostApp(http);
	http.ContentType("application/json; charset=utf-8") << app.GetStatusJson();
}

SKYLARK(Bootstrap, "api/bootstrap")
{
	const ScriptWebHostApp& app = GetScriptWebHostApp(http);
	http.ContentType("application/json; charset=utf-8") << app.GetBootstrapJson();
}

SKYLARK(TranspileEntryJs, "api/transpile-entry.js")
{
	const ScriptWebHostApp& app = GetScriptWebHostApp(http);
	PyToJsResult tr = app.GetEntryTranspile();
	http.ContentType("text/javascript; charset=utf-8");
	if(tr.ok)
		http << tr.javascript;
	else {
		http << "// transpilation failed\n";
		for(int i = 0; i < tr.errors.GetCount(); i++)
			http << "// error: " << tr.errors[i] << "\n";
	}
}

SKYLARK(CatchAll, "**")
{
	http.Redirect(HomePage);
}

END_UPP_NAMESPACE
