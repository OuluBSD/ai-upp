import unittest
from hearts.logic import GameState, Card

class TestHeartsLogic(unittest.TestCase):
    def setUp(self):
        self.state = GameState()
        self.state.deal()
        
        # Override the random deal to a deterministic state for testing
        self.state.players = [
            [Card('clubs', '2'), Card('hearts', '3'), Card('spades', '4')],
            [Card('clubs', '5'), Card('diamonds', '2'), Card('hearts', '10')],
            [Card('clubs', 'jack'), Card('hearts', 'ace'), Card('spades', 'queen')],
            [Card('clubs', 'ace'), Card('diamonds', 'ace'), Card('spades', 'ace')]
        ]
        self.state.turn = 0 # Player 0 has 2 of clubs
        self.state.phase = 'PLAYING'
        self.state.hearts_broken = False
        self.state.trick = []

    def test_suit_following(self):
        c2 = self.state.players[0][0]
        self.assertTrue(self.state.play_card(0, c2)[0])
        
        # Player 1 must follow suit (clubs)
        d2 = self.state.players[1][1]
        self.assertFalse(self.state.play_card(1, d2)[0]) # Invalid
        
        c5 = self.state.players[1][0]
        self.assertTrue(self.state.play_card(1, c5)[0]) # Valid

    def test_hearts_breaking(self):
        # Can't lead hearts if not broken
        self.state.turn = 0
        self.state.leading_suit = None
        h3 = self.state.players[0][1]
        self.assertFalse(self.state.play_card(0, h3)[0])
        
        # Can lead if broken
        self.state.hearts_broken = True
        self.assertTrue(self.state.play_card(0, h3)[0])

    def test_trick_resolution(self):
        self.state.play_card(0, self.state.players[0][0]) # C2
        self.state.play_card(1, self.state.players[1][0]) # C5
        self.state.play_card(2, self.state.players[2][0]) # CJ
        self.state.play_card(3, self.state.players[3][0]) # CA
        
        # Player 3 played highest club, should win
        self.assertEqual(self.state.turn, 3)

    def test_points_calculation(self):
        # Fake a trick with points
        self.state.trick = [
            (0, Card('clubs', '2')),
            (1, Card('hearts', '5')), # 1 pt
            (2, Card('spades', 'queen')), # 13 pts
            (3, Card('clubs', 'ace'))
        ]
        self.state.leading_suit = 'clubs'
        self.state.resolve_trick()
        
        self.assertEqual(self.state.round_scores[3], 14)

if __name__ == '__main__':
    unittest.main()
