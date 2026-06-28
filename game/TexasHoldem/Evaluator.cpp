#include "TexasHoldem.h"

NAMESPACE_UPP

uint16_t rank_hand(const Vector<Card>& hole_cards, const Vector<Card>& board) {
	static omp::HandEvaluator evaluator;
	omp::Hand hand = omp::Hand::empty();
	for (const Card& c : hole_cards) hand += omp::Hand(c.id);
	for (const Card& c : board) hand += omp::Hand(c.id);
	return evaluator.evaluate(hand);
}

Vector<int> find_winners(const Vector<PlayerState>& players, const Vector<int>& eligible_idxs, const Vector<Card>& board) {
	uint16_t best_score = 0;
	Vector<int> winners;
	static omp::HandEvaluator evaluator;

	for (int idx : eligible_idxs) {
		const PlayerState& p = players[idx];
		if (p.folded) continue;

		omp::Hand hand = omp::Hand::empty();
		for (const Card& c : p.hole_cards) hand += omp::Hand(c.id);
		for (const Card& c : board) hand += omp::Hand(c.id);

		uint16_t score = evaluator.evaluate(hand);
		if (score > best_score) {
			best_score = score;
			winners.Clear();
			winners.Add(idx);
		} else if (score == best_score) {
			winners.Add(idx);
		}
	}
	return winners;
}

Vector<int> find_winners(const Vector<PlayerState>& players, const Vector<Card>& board) {
	Vector<int> eligible;
	for (int i = 0; i < players.GetCount(); i++) {
		if (!players[i].folded) {
			eligible.Add(i);
		}
	}
	return find_winners(players, eligible, board);
}

END_UPP_NAMESPACE
