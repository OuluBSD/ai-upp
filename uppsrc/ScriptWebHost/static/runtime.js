const bootstrapDump = document.getElementById('bootstrap');
const logDump = document.getElementById('log');
const statusLine = document.getElementById('status');
const table = document.getElementById('table');
const spriteLayer = document.getElementById('sprite-layer');
const stageTitle = document.getElementById('stage-title');

const runtime = {
  bootstrap: null,
  zones: new Map(),
  sprites: new Map(),
  buttons: new Map(),
  callbacks: {},
  timeouts: new Set(),
  currentModule: null,
};

function syncRuntimeState() {
  document.body.dataset.runtime = runtime.currentModule ? 'booting' : 'loading';
  document.body.dataset.spriteCount = String(runtime.sprites.size);
}

function setRuntimeMode(mode) {
  document.body.dataset.runtime = mode;
}

function logLine(msg) {
  const line = document.createElement('div');
  line.textContent = String(msg);
  logDump.appendChild(line);
  logDump.scrollTop = logDump.scrollHeight;
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
    if (txt)
      el.textContent = txt;
    if (obj.type !== 'Button' && !txt && obj.id && !String(el.className).includes('zone'))
      el.textContent = obj.id;
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

function makeHeartsLogicModule() {
  const SUITS = ['clubs', 'diamonds', 'hearts', 'spades'];
  const RANKS = ['2', '3', '4', '5', '6', '7', '8', '9', '10', 'jack', 'queen', 'king', 'ace'];

  class Card {
    constructor(suit, rank) {
      this.suit = suit;
      this.rank = rank;
      this.id = `${suit}_${rank}`;
    }
    toString() {
      return this.id;
    }
    get_points() {
      if (this.suit === 'hearts')
        return 1;
      if (this.suit === 'spades' && this.rank === 'queen')
        return 13;
      return 0;
    }
  }

  function create_deck() {
    const deck = [];
    for (const suit of SUITS)
      for (const rank of RANKS)
        deck.push(new Card(suit, rank));
    return deck;
  }

  function rank_index(rank) {
    return RANKS.indexOf(rank);
  }

  class GameState {
    constructor() {
      this.players = [[], [], [], []];
      this.scores = [0, 0, 0, 0];
      this.round_scores = [0, 0, 0, 0];
      this.trick = [];
      this.hearts_broken = false;
      this.turn = 0;
      this.leading_suit = null;
      this.log_callback = null;
      this.round_number = 0;
      this.phase = 'PASSING';
      this.passed_cards = [[], [], [], []];
      this.last_trick_winner = -1;
      this.last_trick_points = 0;
      this.last_round_scores = [0, 0, 0, 0];
      this.last_round_moon_shooter = -1;
      this.game_over = false;
      this.trick_pending = false;
      this.pending_trick_winner = -1;
      this.pending_trick_points = 0;
    }

    log(msg) {
      if (this.log_callback)
        this.log_callback(msg);
      else
        logLine(msg);
    }

    deal() {
      this.round_number += 1;
      const deck = create_deck();
      shuffleArray(deck);
      this.players = [[], [], [], []];
      this.round_scores = [0, 0, 0, 0];
      this.hearts_broken = false;
      this.passed_cards = [[], [], [], []];
      this.last_trick_winner = -1;
      this.last_trick_points = 0;
      this.last_round_scores = [0, 0, 0, 0];
      this.last_round_moon_shooter = -1;
      this.game_over = false;
      this.trick_pending = false;
      this.pending_trick_winner = -1;
      this.pending_trick_points = 0;
      for (let i = 0; i < 52; i++)
        this.players[i % 4].push(deck[i]);
      const passDir = this.round_number % 4;
      if (passDir === 0) {
        this.phase = 'PLAYING';
        this.start_play_phase();
      } else {
        this.phase = 'PASSING';
        this.log(`Round ${this.round_number}: Passing phase. Select 3 cards.`);
      }
    }

    select_pass(playerIndex, cards) {
      if (this.phase !== 'PASSING')
        return false;
      if (!cards || cards.length !== 3)
        return false;
      this.passed_cards[playerIndex] = cards.slice();
      let everyone = true;
      for (const hand of this.passed_cards) {
        if (hand.length !== 3) {
          everyone = false;
          break;
        }
      }
      if (everyone)
        this.execute_pass();
      return true;
    }

    execute_pass() {
      const passDir = this.round_number % 4;
      const offset = ({1: 1, 2: -1, 3: 2})[passDir];
      for (let i = 0; i < 4; i++) {
        for (const card of this.passed_cards[i]) {
          const pos = this.players[i].indexOf(card);
          if (pos >= 0)
            this.players[i].splice(pos, 1);
        }
      }
      for (let i = 0; i < 4; i++) {
        const target = (i + offset + 4) % 4;
        this.players[target].push(...this.passed_cards[i]);
      }
      this.phase = 'PLAYING';
      this.start_play_phase();
    }

    start_play_phase() {
      for (let i = 0; i < 4; i++) {
        for (const card of this.players[i]) {
          if (card.suit === 'clubs' && card.rank === '2') {
            this.turn = i;
            this.log(`Player ${i} has 2 of Clubs and starts.`);
            return;
          }
        }
      }
    }

    validate_play(playerIndex, card) {
      if (this.phase !== 'PLAYING')
        return [false, 'Not in playing phase'];
      if (this.trick_pending)
        return [false, 'Waiting for trick resolution'];
      if (playerIndex !== this.turn)
        return [false, 'Not your turn'];

      let totalCards = 0;
      for (const p of this.players)
        totalCards += p.length;
      const firstTrick = totalCards === 52;
      const firstLead = firstTrick && !this.leading_suit;
      if (firstLead && (card.suit !== 'clubs' || card.rank !== '2'))
        return [false, 'Must lead 2 of Clubs'];

      if (this.leading_suit) {
        let hasSuit = false;
        for (const c of this.players[playerIndex]) {
          if (c.suit === this.leading_suit) {
            hasSuit = true;
            break;
          }
        }
        if (hasSuit && card.suit !== this.leading_suit)
          return [false, `Must follow suit: ${this.leading_suit}`];
      }

      if (card.suit === 'hearts' && !this.hearts_broken) {
        let hasOther = false;
        for (const c of this.players[playerIndex]) {
          if (c.suit !== 'hearts') {
            hasOther = true;
            break;
          }
        }
        if (hasOther && !this.leading_suit)
          return [false, 'Hearts not broken yet'];
      }

      if (firstTrick && card.get_points() > 0) {
        let hasSafe = false;
        for (const c of this.players[playerIndex]) {
          if (c.get_points() === 0) {
            hasSafe = true;
            break;
          }
        }
        if (hasSafe)
          return [false, 'No point cards allowed on first trick'];
      }

      return [true, 'OK'];
    }

    play_card(playerIndex, card) {
      const [valid, msg] = this.validate_play(playerIndex, card);
      if (!valid)
        return [false, msg];
      const pos = this.players[playerIndex].indexOf(card);
      if (pos >= 0)
        this.players[playerIndex].splice(pos, 1);
      this.trick.push([playerIndex, card]);
      if (!this.leading_suit)
        this.leading_suit = card.suit;
      if (card.suit === 'hearts' || (card.suit === 'spades' && card.rank === 'queen'))
        this.hearts_broken = true;
      if (this.trick.length === 4) {
        const [winner, points] = this.get_trick_result();
        this.trick_pending = true;
        this.pending_trick_winner = winner;
        this.pending_trick_points = points;
      } else {
        this.turn = (this.turn + 1) % 4;
      }
      return [true, 'OK'];
    }

    get_trick_result() {
      let winner = 0;
      let highest = -1;
      for (const [playerIndex, card] of this.trick) {
        if (card.suit === this.leading_suit) {
          const r = rank_index(card.rank);
          if (r > highest) {
            highest = r;
            winner = playerIndex;
          }
        }
      }
      let points = 0;
      for (const [, card] of this.trick)
        points += card.get_points();
      return [winner, points];
    }

    resolve_trick() {
      const result = this.trick_pending ? [this.pending_trick_winner, this.pending_trick_points] : this.get_trick_result();
      const winner = result[0];
      const points = result[1];
      this.round_scores[winner] += points;
      this.last_trick_winner = winner;
      this.last_trick_points = points;
      this.trick_pending = false;
      this.pending_trick_winner = -1;
      this.pending_trick_points = 0;
      this.log(`Trick resolved. Player ${winner} wins ${points} points.`);
      this.trick = [];
      this.leading_suit = null;
      this.turn = winner;
      let roundDone = true;
      for (const p of this.players) {
        if (p.length !== 0) {
          roundDone = false;
          break;
        }
      }
      if (roundDone)
        this.resolve_round();
    }

    resolve_round() {
      this.log('Round finished. Calculating scores...');
      let moon = -1;
      for (let i = 0; i < 4; i++) {
        if (this.round_scores[i] === 26) {
          moon = i;
          break;
        }
      }
      if (moon !== -1) {
        this.log(`PLAYER ${moon} SHOT THE MOON!`);
        for (let i = 0; i < 4; i++)
          this.round_scores[i] = i === moon ? 0 : 26;
      }
      this.last_round_scores = this.round_scores.slice();
      this.last_round_moon_shooter = moon;
      for (let i = 0; i < 4; i++) {
        this.scores[i] += this.round_scores[i];
        this.log(`Player ${i} total score: ${this.scores[i]}`);
      }
      let gameOver = false;
      for (const score of this.scores) {
        if (score >= 100) {
          gameOver = true;
          break;
        }
      }
      this.game_over = gameOver;
      if (gameOver) {
        this.phase = 'GAME_OVER';
        this.log('GAME OVER!');
      } else {
        this.phase = 'ROUND_END';
      }
    }

    begin_next_round() {
      if (!this.game_over)
        this.deal();
    }
  }

  return {
    Card: function(suit, rank) { return new Card(suit, rank); },
    GameState: function() { return new GameState(); },
    SUITS,
    RANKS,
    create_deck,
    rank_index,
  };
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

async function loadAndRunGame() {
  installPythonJsHelpers();
  setRuntimeMode('loading');
  syncRuntimeState();
  const bootstrap = await fetch('/api/bootstrap').then(r => r.json());
  renderBaseLayout(bootstrap);
  const sysModule = { argv: [] };
  const modules = {
    sys: sysModule,
    cardgame_view: makeCardGameView(),
    'hearts.logic': makeHeartsLogicModule(),
  };
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
  syncRuntimeState();
  setRuntimeMode('running');
}

loadAndRunGame().catch(err => {
  console.error(err);
  logLine(err && err.stack ? err.stack : String(err));
  setStatus('Runtime error');
  setRuntimeMode('error');
});
