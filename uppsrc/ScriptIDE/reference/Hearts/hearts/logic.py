import random

SUITS = ['clubs', 'diamonds', 'hearts', 'spades']
RANKS = ['2', '3', '4', '5', '6', '7', '8', '9', '10', 'jack', 'queen', 'king', 'ace']

class Card:
    def __init__(self, suit, rank):
        self.suit = suit
        self.rank = rank
        self.id = f"{suit}_{rank}"

    def __repr__(self):
        return self.id

    def get_points(self):
        if self.suit == 'hearts':
            return 1
        if self.suit == 'spades' and self.rank == 'queen':
            return 13
        return 0

def create_deck():
    deck = []
    for s in SUITS:
        for r in RANKS:
            deck.append(Card(s, r))
    return deck

class GameState:
    def __init__(self):
        self.players = [[], [], [], []]
        self.scores = [0, 0, 0, 0]
        self.round_scores = [0, 0, 0, 0]
        self.trick = []
        self.hearts_broken = False
        self.turn = 0
        self.leading_suit = None
        self.log_callback = None
        self.round_number = 0
        self.phase = 'PASSING'
        self.passed_cards = [[], [], [], []]

    def log(self, msg):
        if self.log_callback:
            self.log_callback(msg)
        else:
            print(msg)

    def deal(self):
        self.round_number += 1
        deck = create_deck()
        random.shuffle(deck)
        self.players = [[], [], [], []]
        self.round_scores = [0, 0, 0, 0]
        self.hearts_broken = False
        self.passed_cards = [[], [], [], []]
        
        for i in range(52):
            self.players[i % 4].append(deck[i])
            
        pass_dir = self.round_number % 4
        if pass_dir == 0:
            self.phase = 'PLAYING'
            self.start_play_phase()
        else:
            self.phase = 'PASSING'
            self.log(f"Round {self.round_number}: Passing phase. Select 3 cards.")

    def select_pass(self, player_index, cards):
        if self.phase != 'PASSING': return False
        if len(cards) != 3: return False
        
        self.passed_cards[player_index] = cards
        
        if all(len(p) == 3 for p in self.passed_cards):
            self.execute_pass()
            
        return True

    def execute_pass(self):
        pass_dir = self.round_number % 4
        # 1 = left (+1), 2 = right (-1), 3 = across (+2)
        offset = {1: 1, 2: -1, 3: 2}[pass_dir]
        
        for i in range(4):
            for c in self.passed_cards[i]:
                self.players[i].remove(c)
                
        for i in range(4):
            target = (i + offset) % 4
            self.players[target].extend(self.passed_cards[i])
            
        self.phase = 'PLAYING'
        self.start_play_phase()

    def start_play_phase(self):
        for i in range(4):
            for card in self.players[i]:
                if card.suit == 'clubs' and card.rank == '2':
                    self.turn = i
                    self.log(f"Player {i} has 2 of Clubs and starts.")
                    return

    def validate_play(self, player_index, card):
        if self.phase != 'PLAYING':
            return False, "Not in playing phase"
        if player_index != self.turn:
            return False, "Not your turn"
        
        # Must follow suit
        if self.leading_suit:
            has_suit = any(c.suit == self.leading_suit for c in self.players[player_index])
            if has_suit and card.suit != self.leading_suit:
                return False, f"Must follow suit: {self.leading_suit}"
        
        # Hearts breaking rule
        if card.suit == 'hearts' and not self.hearts_broken:
            has_other = any(c.suit != 'hearts' for c in self.players[player_index])
            if has_other and not self.leading_suit:
                return False, "Hearts not broken yet"

        # First trick special rule: no points on first trick
        if not self.hearts_broken and len(self.players[0]) == 13 and not self.leading_suit:
             # This is checked inside play_card logic or here
             pass

        return True, "OK"

    def play_card(self, player_index, card):
        valid, msg = self.validate_play(player_index, card)
        if not valid:
            return False, msg
        
        self.players[player_index].remove(card)
        self.trick.append((player_index, card))
        
        if not self.leading_suit:
            self.leading_suit = card.suit
        
        if card.suit == 'hearts' or (card.suit == 'spades' and card.rank == 'queen'):
            self.hearts_broken = True
            
        if len(self.trick) == 4:
            self.resolve_trick()
        else:
            self.turn = (self.turn + 1) % 4
            
        return True, "OK"

    def resolve_trick(self):
        # Determine winner
        winner_index = 0
        highest_rank_idx = -1
        
        rank_order = {r: i for i, r in enumerate(RANKS)}
        
        for p_idx, card in self.trick:
            if card.suit == self.leading_suit:
                r_idx = rank_order[card.rank]
                if r_idx > highest_rank_idx:
                    highest_rank_idx = r_idx
                    winner_index = p_idx
        
        # Award points
        points = sum(c.get_points() for _, c in self.trick)
        self.round_scores[winner_index] += points
        
        self.log(f"Trick resolved. Player {winner_index} wins {points} points.")
        
        # Clean up
        self.trick = []
        self.leading_suit = None
        self.turn = winner_index
        
        # Check round end
        if all(len(p) == 0 for p in self.players):
            self.resolve_round()

    def resolve_round(self):
        self.log("Round finished. Calculating scores...")
        
        # Check Shooting the Moon
        moon_shooter = -1
        for i in range(4):
            if self.round_scores[i] == 26:
                moon_shooter = i
                break
        
        if moon_shooter != -1:
            self.log(f"PLAYER {moon_shooter} SHOT THE MOON!")
            for i in range(4):
                if i == moon_shooter:
                    self.round_scores[i] = 0
                else:
                    self.round_scores[i] = 26
        
        for i in range(4):
            self.scores[i] += self.round_scores[i]
            self.log(f"Player {i} total score: {self.scores[i]}")
            
        # Check Game Over (100 pts)
        if any(s >= 100 for s in self.scores):
            self.log("GAME OVER!")
        else:
            self.deal()
