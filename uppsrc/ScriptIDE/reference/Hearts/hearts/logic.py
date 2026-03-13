import random

SUITS = ['clubs', 'diamonds', 'hearts', 'spades']
RANKS = ['2', '3', '4', '5', '6', '7', '8', '9', '10', 'jack', 'queen', 'king', 'ace']

class Card:
    def __init__(self, suit, rank):
        self.suit = suit
        self.rank = rank
        self.id = str(suit) + "_" + str(rank)

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

def rank_index(rank):
    for i in range(len(RANKS)):
        if RANKS[i] == rank:
            return i
    return -1

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
        self.last_trick_winner = -1
        self.last_trick_points = 0
        self.last_round_scores = [0, 0, 0, 0]
        self.last_round_moon_shooter = -1
        self.game_over = False
        self.trick_pending = False
        self.pending_trick_winner = -1
        self.pending_trick_points = 0

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
        self.last_trick_winner = -1
        self.last_trick_points = 0
        self.last_round_scores = [0, 0, 0, 0]
        self.last_round_moon_shooter = -1
        self.game_over = False
        self.trick_pending = False
        self.pending_trick_winner = -1
        self.pending_trick_points = 0
        
        for i in range(52):
            self.players[i % 4].append(deck[i])
            
        pass_dir = self.round_number % 4
        if pass_dir == 0:
            self.phase = 'PLAYING'
            self.start_play_phase()
        else:
            self.phase = 'PASSING'
            self.log("Round " + str(self.round_number) + ": Passing phase. Select 3 cards.")

    def select_pass(self, player_index, cards):
        if self.phase != 'PASSING': return False
        if len(cards) != 3: return False
        
        self.passed_cards[player_index] = cards
        
        everyone_selected = True
        for p in self.passed_cards:
            if len(p) != 3:
                everyone_selected = False
                break
        if everyone_selected:
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
                    self.log("Player " + str(i) + " has 2 of Clubs and starts.")
                    return

    def validate_play(self, player_index, card):
        if self.phase != 'PLAYING':
            return False, "Not in playing phase"
        if self.trick_pending:
            return False, "Waiting for trick resolution"
        if player_index != self.turn:
            return False, "Not your turn"

        total_cards_in_hands = 0
        for p in self.players:
            total_cards_in_hands += len(p)
        first_trick = total_cards_in_hands == 52
        first_lead = first_trick and not self.leading_suit

        if first_lead:
            if card.suit != 'clubs' or card.rank != '2':
                return False, "Must lead 2 of Clubs"
        
        # Must follow suit
        if self.leading_suit:
            has_suit = False
            for c in self.players[player_index]:
                if c.suit == self.leading_suit:
                    has_suit = True
                    break
            if has_suit and card.suit != self.leading_suit:
                return False, "Must follow suit: " + str(self.leading_suit)
        
        # Hearts breaking rule
        if card.suit == 'hearts' and not self.hearts_broken:
            has_other = False
            for c in self.players[player_index]:
                if c.suit != 'hearts':
                    has_other = True
                    break
            if has_other and not self.leading_suit:
                return False, "Hearts not broken yet"

        # First trick special rule: no points on first trick
        if first_trick and card.get_points() > 0:
            has_safe_card = False
            for c in self.players[player_index]:
                if c.get_points() == 0:
                    has_safe_card = True
                    break
            if has_safe_card:
                return False, "No point cards allowed on first trick"

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
            winner_index, points = self.get_trick_result()
            self.trick_pending = True
            self.pending_trick_winner = winner_index
            self.pending_trick_points = points
        else:
            self.turn = (self.turn + 1) % 4
            
        return True, "OK"

    def get_trick_result(self):
        winner_index = 0
        highest_rank_idx = -1
        
        for trick_item in self.trick:
            p_idx, card = trick_item
            if card.suit == self.leading_suit:
                r_idx = rank_index(card.rank)
                if r_idx > highest_rank_idx:
                    highest_rank_idx = r_idx
                    winner_index = p_idx
        
        # Award points
        points = 0
        for trick_item in self.trick:
            _, c = trick_item
            points += c.get_points()
        return winner_index, points

    def resolve_trick(self):
        if self.trick_pending:
            winner_index = self.pending_trick_winner
            points = self.pending_trick_points
        else:
            winner_index, points = self.get_trick_result()

        self.round_scores[winner_index] += points
        self.last_trick_winner = winner_index
        self.last_trick_points = points
        self.trick_pending = False
        self.pending_trick_winner = -1
        self.pending_trick_points = 0
        
        self.log("Trick resolved. Player " + str(winner_index) + " wins " + str(points) + " points.")
        
        # Clean up
        self.trick = []
        self.leading_suit = None
        self.turn = winner_index
        
        # Check round end
        round_done = True
        for p in self.players:
            if len(p) != 0:
                round_done = False
                break
        if round_done:
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
            self.log("PLAYER " + str(moon_shooter) + " SHOT THE MOON!")
            for i in range(4):
                if i == moon_shooter:
                    self.round_scores[i] = 0
                else:
                    self.round_scores[i] = 26

        self.last_round_scores = list(self.round_scores)
        self.last_round_moon_shooter = moon_shooter
        
        for i in range(4):
            self.scores[i] += self.round_scores[i]
            self.log("Player " + str(i) + " total score: " + str(self.scores[i]))
            
        # Check Game Over (100 pts)
        game_over = False
        for s in self.scores:
            if s >= 100:
                game_over = True
                break
        self.game_over = game_over
        if game_over:
            self.phase = 'GAME_OVER'
            self.log("GAME OVER!")
        else:
            self.phase = 'ROUND_END'

    def begin_next_round(self):
        if self.game_over:
            return
        self.deal()
