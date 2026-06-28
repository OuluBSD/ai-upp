#ifndef _game_TexasHoldem_Evaluator_h_
#define _game_TexasHoldem_Evaluator_h_

uint16_t rank_hand(const Vector<Card>& hole_cards, const Vector<Card>& board);
Vector<int> find_winners(const Vector<PlayerState>& players, const Vector<Card>& board);
Vector<int> find_winners(const Vector<PlayerState>& players, const Vector<int>& eligible_idxs, const Vector<Card>& board);

#endif
