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
      runtime_api.log(String(msg));
  }

  deal() {
    this.round_number += 1;
    const deck = create_deck();
    runtime_api.shuffleArray(deck);
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

const __scriptwebhost_module__ = {
  Card: function(suit, rank) { return new Card(suit, rank); },
  GameState: function() { return new GameState(); },
  SUITS,
  RANKS,
  create_deck,
  rank_index,
};
