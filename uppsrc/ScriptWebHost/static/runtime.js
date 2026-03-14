const bootstrapDump = document.getElementById('bootstrap');
const logDump = document.getElementById('log');
const statusLine = document.getElementById('status');
const table = document.getElementById('table');
const spriteLayer = document.getElementById('sprite-layer');
const stageTitle = document.getElementById('stage-title');

const runtime = {
  mode: 'loading',
  bootstrap: null,
  zones: new Map(),
  zoneTexts: new Map(),
  sprites: new Map(),
  buttons: new Map(),
  callbacks: {},
  timeouts: new Set(),
  currentModule: null,
};

function syncRuntimeState() {
  document.body.dataset.runtime = runtime.mode;
  document.body.dataset.spriteCount = String(runtime.sprites.size);
}

function setRuntimeMode(mode) {
  runtime.mode = mode;
  document.body.dataset.runtime = mode;
}

function logLine(msg) {
  const line = document.createElement('div');
  line.textContent = String(msg);
  logDump.appendChild(line);
  logDump.scrollTop = logDump.scrollHeight;
}

function queryArgv() {
  const argv = [];
  const params = new URLSearchParams(window.location.search);
  if (params.get('autoplay') === '1' || params.get('autoplay') === 'true')
    argv.push('--autoplay');
  return argv;
}

function setStatus(msg) {
  statusLine.textContent = String(msg || '');
}

function normalizePath(path) {
  const out = [];
  for (const part of String(path || '').split('/')) {
    if (!part || part === '.')
      continue;
    if (part === '..') {
      if (out.length)
        out.pop();
      continue;
    }
    out.push(part);
  }
  return '/' + out.join('/');
}

function resolveRepoUrl(path) {
  if (!runtime.bootstrap)
    return path;
  const root = String(runtime.bootstrap.server_root_path || '');
  const gameDir = String(runtime.bootstrap.gamestate_dir || '');
  let abs = String(path || '');
  if (!abs.startsWith('/'))
    abs = normalizePath(gameDir + '/' + abs);
  if (root && abs.startsWith(root)) {
    const rel = abs.slice(root.length).replace(/^\/+/, '');
    return '/fs/' + rel.split('/').map(encodeURIComponent).join('/');
  }
  return path;
}

function setBox(el, box) {
  if (box.left !== undefined) el.style.left = (box.left * 100) + '%';
  if (box.right !== undefined) el.style.right = (box.right * 100) + '%';
  if (box.top !== undefined) el.style.top = (box.top * 100) + '%';
  if (box.bottom !== undefined) el.style.bottom = (box.bottom * 100) + '%';
  if (box.width !== undefined) el.style.width = (box.width * 100) + '%';
  if (box.height !== undefined) el.style.height = (box.height * 100) + '%';
  const tx = (box.transform_x || 0) * 100;
  const ty = (box.transform_y || 0) * 100;
  el.style.transform = (tx || ty) ? `translate(${tx}%, ${ty}%)` : '';
}

function classify(obj) {
  const role = String(obj.user_class || obj.type || 'zone').toLowerCase();
  if (role === 'button') return 'button';
  if (role === 'label') return 'label';
  if (role === 'hand') return 'zone hand';
  if (role === 'trick') return 'zone trick';
  if (role === 'container') return 'zone container';
  return 'zone';
}

function textFor(obj) {
  if (obj.type === 'Button')
    return obj.label || obj.id || 'button';
  if (obj.label)
    return obj.label;
  return '';
}

function getZoneElement(id) {
  return runtime.zones.get(id) || null;
}

function getZoneRect(id) {
  const el = getZoneElement(id);
  if (!el)
    return null;
  const a = el.getBoundingClientRect();
  const b = table.getBoundingClientRect();
  return {
    x: a.left - b.left,
    y: a.top - b.top,
    w: a.width,
    h: a.height,
  };
}

function ensureSprite(id) {
  let img = runtime.sprites.get(id);
  if (img)
    return img;
  img = document.createElement('img');
  img.className = 'sprite';
  img.dataset.id = id;
  img.draggable = false;
  img.addEventListener('click', () => {
    if (runtime.currentModule && typeof runtime.currentModule.on_click === 'function')
      runtime.currentModule.on_click(id);
  });
  spriteLayer.appendChild(img);
  runtime.sprites.set(id, img);
  syncRuntimeState();
  return img;
}

function spriteLayerBase(layer) {
  if (layer === 'moving')
    return 30000;
  if (layer === 'trick')
    return 20000;
  return 10000;
}

function spriteZIndex(layer, x, y) {
  if (layer === 'moving')
    return spriteLayerBase(layer) + Math.round(x) + Math.round(y);
  if (layer === 'trick')
    return spriteLayerBase(layer) + Math.round(x);
  return spriteLayerBase(layer) + Math.round(x);
}

function inferZoneLayer(zoneId) {
  const zone = String(zoneId || '');
  if (zone.startsWith('trick_'))
    return 'trick';
  return 'hand';
}

function setSpritePosition(img, x, y, rotation, animate, layer) {
  const maxX = Math.max(0, table.clientWidth - 72);
  const maxY = Math.max(0, table.clientHeight - 96);
  x = Math.max(0, Math.min(maxX, x));
  y = Math.max(0, Math.min(maxY, y));
  if (animate)
    img.classList.add('animating');
  else
    img.classList.remove('animating');
  img.style.left = `${x}px`;
  img.style.top = `${y}px`;
  img.style.transform = `rotate(${rotation || 0}deg)`;
  img.dataset.layer = layer || img.dataset.layer || 'hand';
  img.style.zIndex = String(spriteZIndex(img.dataset.layer, x, y));
}

function setCard(id, asset, x, y, rotation) {
  const img = ensureSprite(id);
  img.src = resolveRepoUrl(asset);
  img.alt = id;
  setSpritePosition(img, x, y, rotation, false, img.dataset.layer || 'hand');
}

function removeSprite(id) {
  const img = runtime.sprites.get(id);
  if (!img)
    return;
  img.remove();
  runtime.sprites.delete(id);
  syncRuntimeState();
}

function clearSprites() {
  for (const id of Array.from(runtime.sprites.keys()))
    removeSprite(id);
  syncRuntimeState();
}

function moveCard(id, zoneId, offset, animate) {
  const img = ensureSprite(id);
  const rect = getZoneRect(zoneId);
  if (!rect)
    return;
  const x = rect.x + (rect.w - 72) / 2 + (offset || 0);
  const y = rect.y + (rect.h - 96) / 2;
  const targetLayer = inferZoneLayer(zoneId);
  if (!animate) {
    setSpritePosition(img, x, y, 0, false, targetLayer);
    return;
  }
  img.classList.remove('animating');
  img.dataset.layer = 'moving';
  img.style.zIndex = String(spriteZIndex('moving', x, y));
  img.getBoundingClientRect();
  window.requestAnimationFrame(() => {
    setSpritePosition(img, x, y, 0, true, targetLayer);
  });
}

function setLabel(id, text) {
  const el = getZoneElement(id);
  if (!el)
    return;
  const textNode = runtime.zoneTexts.get(id);
  if (textNode) {
    textNode.textContent = String(text || '');
    return;
  }
  el.textContent = String(text || '');
}

function setHighlight(id, on) {
  const el = getZoneElement(id);
  if (!el)
    return;
  el.classList.toggle('highlight', !!on);
}

function setButton(id, text, enabled) {
  const el = getZoneElement(id);
  if (!el)
    return;
  el.textContent = String(text || '');
  el.disabled = !enabled;
  el.classList.toggle('disabled', !enabled);
  el.style.display = text ? '' : 'none';
}

function setTimeoutCallback(delayMs, name) {
  const token = window.setTimeout(() => {
    runtime.timeouts.delete(token);
    const cb = runtime.currentModule && runtime.currentModule[name];
    if (typeof cb === 'function')
      cb();
  }, Number(delayMs) || 0);
  runtime.timeouts.add(token);
}

function beginSpriteFrame() {
}

function setExpectedSpriteCount(zoneId, count) {
}

function renderBaseLayout(data) {
  runtime.bootstrap = data;
  bootstrapDump.textContent = JSON.stringify(data, null, 2);
  stageTitle.textContent = data.gamestate_path || 'ScriptWebHost';
  table.innerHTML = '';
  table.appendChild(spriteLayer);
  runtime.zones.clear();
  runtime.zoneTexts.clear();
  runtime.buttons.clear();

  const form = data.form || {};
  const meta = form.meta || {};
  if (meta.background)
    table.style.background = `rgb(${meta.background})`;

  const objects = form.objects || [];
  const byId = new Map();
  for (const obj of objects) {
    const isButton = obj.type === 'Button';
    const el = document.createElement(isButton ? 'button' : 'div');
    el.className = 'obj ' + classify(obj);
    el.dataset.id = obj.id || '';
    el.title = obj.anchor || '';
    const txt = textFor(obj);
    if (isButton) {
      if (txt)
        el.textContent = txt;
    }
    else {
      const textNode = document.createElement('div');
      textNode.className = 'obj-text';
      if (txt)
        textNode.textContent = txt;
      else if (obj.id && !String(el.className).includes('zone'))
        textNode.textContent = obj.id;
      el.appendChild(textNode);
      if (obj.id)
        runtime.zoneTexts.set(obj.id, textNode);
    }
    if (isButton) {
      el.addEventListener('click', () => {
        if (runtime.currentModule && typeof runtime.currentModule.on_button === 'function')
          runtime.currentModule.on_button(obj.id);
      });
      runtime.buttons.set(obj.id, el);
    }
    if (obj.id)
      byId.set(obj.id, el);
    obj.__el = el;
  }

  for (const obj of objects) {
    const layout = obj.layout || {};
    const local = (layout.local_to_parent || {}).browser || null;
    const root = layout.browser || {};
    const parent = obj.parent && byId.get(obj.parent);
    setBox(obj.__el, parent && local ? local : root);
    (parent || table).appendChild(obj.__el);
    if (obj.id)
      runtime.zones.set(obj.id, obj.__el);
  }
  table.appendChild(spriteLayer);
}

function installPythonJsHelpers() {
  if (!String.prototype.join) {
    Object.defineProperty(String.prototype, 'join', {
      value(arr) {
        return (arr || []).join(String(this));
      },
      configurable: true,
    });
  }
}

function shuffleArray(items) {
  for (let i = items.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    const t = items[i];
    items[i] = items[j];
    items[j] = t;
  }
  return items;
}

function makeCardGameView() {
  return {
    log: logLine,
    set_label: setLabel,
    set_highlight: setHighlight,
    set_button: setButton,
    set_status: setStatus,
    get_zone_rect: getZoneRect,
    set_card: setCard,
    move_card: moveCard,
    remove_sprite: removeSprite,
    clear_sprites: clearSprites,
    begin_sprite_frame: beginSpriteFrame,
    set_expected_sprite_count: setExpectedSpriteCount,
    set_timeout: setTimeoutCallback,
  };
}

function requestedGameModules(bootstrap) {
  const metadata = ((bootstrap || {}).gamestate || {}).metadata || {};
  const requested = metadata.browser_modules;
  return Array.isArray(requested) ? requested : [];
}

function makeBuiltinModuleRegistry() {
  return {
    sys: { argv: queryArgv() },
    cardgame_view: makeCardGameView(),
  };
}

async function loadJsModule(def, bootstrap) {
  const path = String((def || {}).path || '');
  if (!path)
    throw new Error(`Browser module ${String((def || {}).name || '')} is missing path`);
  const url = resolveRepoUrl(path);
  const code = await fetch(url).then(r => {
    if (!r.ok)
      throw new Error(`Browser module fetch failed: ${url}`);
    return r.text();
  });
  const runtimeApi = {
    bootstrap,
    log: logLine,
    shuffleArray,
  };
  const factory = new Function('runtime_api', code + '\nreturn typeof __scriptwebhost_module__ !== "undefined" ? __scriptwebhost_module__ : null;');
  const moduleValue = factory(runtimeApi);
  if (!moduleValue)
    throw new Error(`Browser module did not expose __scriptwebhost_module__: ${String((def || {}).name || '')}`);
  return moduleValue;
}

async function loadProjectModules(bootstrap) {
  const loaded = {};
  for (const def of requestedGameModules(bootstrap)) {
    const name = String((def || {}).name || '');
    const kind = String((def || {}).kind || 'js');
    if (!name)
      throw new Error('Browser module definition is missing name');
    if (kind !== 'js')
      throw new Error(`Unsupported browser module kind: ${kind}`);
    loaded[name] = await loadJsModule(def, bootstrap);
  }
  return loaded;
}

async function makeModuleRegistry(bootstrap) {
  const modules = {
    ...makeBuiltinModuleRegistry(),
  };
  Object.assign(modules, await loadProjectModules(bootstrap));
  return modules;
}

async function loadAndRunGame() {
  installPythonJsHelpers();
  setRuntimeMode('loading');
  syncRuntimeState();
  const bootstrap = await fetch('/api/bootstrap').then(r => r.json());
  renderBaseLayout(bootstrap);
  const modules = await makeModuleRegistry(bootstrap);
  function __py_import__(name) {
    if (!(name in modules))
      throw new Error(`Unsupported module import: ${name}`);
    return modules[name];
  }
  const code = await fetch('/api/transpile-entry.js').then(r => r.text());
  const factory = new Function('__py_import__', code + '\nreturn {' +
    'start: typeof start !== "undefined" ? start : null,' +
    'on_click: typeof on_click !== "undefined" ? on_click : null,' +
    'on_button: typeof on_button !== "undefined" ? on_button : null,' +
    'ai_step: typeof ai_step !== "undefined" ? ai_step : null,' +
    'next_round: typeof next_round !== "undefined" ? next_round : null,' +
    'commit_pass: typeof commit_pass !== "undefined" ? commit_pass : null,' +
    'finish_pass_animation: typeof finish_pass_animation !== "undefined" ? finish_pass_animation : null,' +
    'finish_trick_collect: typeof finish_trick_collect !== "undefined" ? finish_trick_collect : null' +
  '};');
  runtime.currentModule = factory(__py_import__);
  setRuntimeMode('booting');
  if (!runtime.currentModule || typeof runtime.currentModule.start !== 'function')
    throw new Error('Transpiled entry did not expose start()');
  runtime.currentModule.start();
  setStatus('Running');
  setRuntimeMode('running');
  syncRuntimeState();
}

loadAndRunGame().catch(err => {
  console.error(err);
  logLine(err && err.stack ? err.stack : String(err));
  setStatus('Runtime error');
  setRuntimeMode('error');
  syncRuntimeState();
});

window.ScriptWebHostRuntime = {
  runtime,
  getZoneRect,
  getZoneElement,
  sprites: () => Array.from(runtime.sprites.values()).map(img => ({
    id: img.dataset.id,
    layer: img.dataset.layer,
    left: img.style.left,
    top: img.style.top,
    zIndex: img.style.zIndex,
  })),
};
