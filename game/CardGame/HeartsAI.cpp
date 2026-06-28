#include "CardGame.h"

NAMESPACE_UPP

Vector<Card> HeartsAI::ChoosePassCards(const Vector<Card>& hand) {
	struct ScoredCard : public Moveable<ScoredCard> {
		int score;
		Card card;
	};
	Vector<ScoredCard> scored_hand;
	for (const Card& c : hand) {
		int score = c.GetPoints() * 10;
		if (c.suit == "spades") {
			if (c.rank == "queen" || c.rank == "king" || c.rank == "ace") {
				score += 15;
			}
		}
		if (c.rank == "ace" || c.rank == "king" || c.rank == "queen") {
			score += 5;
		}
		ScoredCard sc;
		sc.score = score;
		sc.card = c;
		scored_hand.Add(sc);
	}

	for (int i = 0; i < scored_hand.GetCount(); i++) {
		int best = i;
		for (int j = i + 1; j < scored_hand.GetCount(); j++) {
			if (scored_hand[j].score > scored_hand[best].score) {
				best = j;
			}
		}
		if (best != i) {
			Swap(scored_hand[i], scored_hand[best]);
		}
	}

	Vector<Card> result;
	int count = scored_hand.GetCount() < 3 ? scored_hand.GetCount() : 3;
	for (int i = 0; i < count; i++) {
		result.Add(scored_hand[i].card);
	}
	return result;
}

Card HeartsAI::ChooseCard(int player_index, const Vector<Card>& hand, const Vector<TrickItem>& trick, const String& leading_suit, bool hearts_broken) {
	if (hand.IsEmpty()) {
		return Card();
	}

	if (leading_suit.IsEmpty()) {
		// We are leading the trick
		Vector<Card> valid_leads;
		for (const Card& c : hand) {
			if (c.suit == "hearts" && !hearts_broken) {
				bool only_hearts = true;
				for (const Card& card : hand) {
					if (card.suit != "hearts") {
						only_hearts = false;
						break;
					}
				}
				if (only_hearts) {
					valid_leads.Add(c);
				}
			} else {
				valid_leads.Add(c);
			}
		}

		bool has_spade_queen = false;
		for (const Card& card2 : hand) {
			if (card2.suit == "spades" && card2.rank == "queen") {
				has_spade_queen = true;
				break;
			}
		}

		// Sort by rank ascending
		for (int i = 0; i < valid_leads.GetCount(); i++) {
			int best = i;
			for (int j = i + 1; j < valid_leads.GetCount(); j++) {
				if (RankIndex(valid_leads[j].rank) < RankIndex(valid_leads[best].rank)) {
					best = j;
				}
			}
			if (best != i) {
				Swap(valid_leads[i], valid_leads[best]);
			}
		}

		if (has_spade_queen) {
			Vector<Card> non_spades;
			for (const Card& c : valid_leads) {
				if (c.suit != "spades") {
					non_spades.Add(c);
				}
			}
			if (!non_spades.IsEmpty()) {
				return non_spades[0];
			}
		}

		return valid_leads[0];
	}

	// We are following
	Vector<Card> follow_suit;
	for (const Card& c : hand) {
		if (c.suit == leading_suit) {
			follow_suit.Add(c);
		}
	}

	if (!follow_suit.IsEmpty()) {
		// Find highest card currently in trick of the leading suit
		int current_winner_rank = -1;
		for (const TrickItem& item : trick) {
			if (item.card.suit == leading_suit) {
				int card_rank = RankIndex(item.card.rank);
				if (card_rank > current_winner_rank) {
					current_winner_rank = card_rank;
				}
			}
		}

		// Play the highest card that is LOWER than the current winner
		Vector<Card> safe_cards;
		for (const Card& c : follow_suit) {
			if (RankIndex(c.rank) < current_winner_rank) {
				safe_cards.Add(c);
			}
		}
		if (!safe_cards.IsEmpty()) {
			// Sort descending by rank
			for (int i = 0; i < safe_cards.GetCount(); i++) {
				int best = i;
				for (int j = i + 1; j < safe_cards.GetCount(); j++) {
					if (RankIndex(safe_cards[j].rank) > RankIndex(safe_cards[best].rank)) {
						best = j;
					}
				}
				if (best != i) {
					Swap(safe_cards[i], safe_cards[best]);
				}
			}
			return safe_cards[0];
		}

		// If we must win, play the lowest card to save high cards
		for (int i = 0; i < follow_suit.GetCount(); i++) {
			int best = i;
			for (int j = i + 1; j < follow_suit.GetCount(); j++) {
				if (RankIndex(follow_suit[j].rank) < RankIndex(follow_suit[best].rank)) {
					best = j;
				}
			}
			if (best != i) {
				Swap(follow_suit[i], follow_suit[best]);
			}
		}
		return follow_suit[0];
	} else {
		// Void! Dump points.
		// Search for Queen of Spades
		for (const Card& c : hand) {
			if (c.suit == "spades" && c.rank == "queen") {
				return c;
			}
		}

		// Search for high hearts
		Vector<Card> hearts;
		for (const Card& c : hand) {
			if (c.suit == "hearts") {
				hearts.Add(c);
			}
		}
		if (!hearts.IsEmpty()) {
			for (int i = 0; i < hearts.GetCount(); i++) {
				int best = i;
				for (int j = i + 1; j < hearts.GetCount(); j++) {
					if (RankIndex(hearts[j].rank) > RankIndex(hearts[best].rank)) {
						best = j;
					}
				}
				if (best != i) {
					Swap(hearts[i], hearts[best]);
				}
			}
			return hearts[0];
		}

		// Dump highest card
		Vector<Card> sorted_hand;
		sorted_hand <<= hand;
		for (int i = 0; i < sorted_hand.GetCount(); i++) {
			int best = i;
			for (int j = i + 1; j < sorted_hand.GetCount(); j++) {
				if (RankIndex(sorted_hand[j].rank) > RankIndex(sorted_hand[best].rank)) {
					best = j;
				}
			}
			if (best != i) {
				Swap(sorted_hand[i], sorted_hand[best]);
			}
		}
		return sorted_hand[0];
	}
}

END_UPP_NAMESPACE
