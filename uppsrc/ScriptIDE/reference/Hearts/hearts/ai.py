import random

def choose_pass_cards(hand):
    # Heuristic: pass high point cards and high spades
    # Spades: Queen, King, Ace are dangerous
    # Hearts: All are points

    scored_hand = []
    for c in hand:
        score = c.get_points() * 10
        if c.suit == 'spades':
            if c.rank in ['queen', 'king', 'ace']:
                score += 15
        if c.rank in ['ace', 'king', 'queen']:
            score += 5
        scored_hand.append((score, c))

    scored_hand.sort(key=lambda x: x[0], reverse=True)
    return [x[1] for x in scored_hand[:3]]

def choose_card(player_index, hand, trick, leading_suit, hearts_broken):
    rank_order = ['2','3','4','5','6','7','8','9','10','jack','queen','king','ace']

    if not leading_suit:
        # We are leading the trick
        valid_leads = []
        for c in hand:
            if c.suit == 'hearts' and not hearts_broken:
                if all(card.suit == 'hearts' for card in hand):
                    valid_leads.append(c)
            else:
                valid_leads.append(c)

        # Heuristic: Lead low to stay safe, preferably not Spades if we have the Queen
        has_spade_queen = False
        for card2 in hand:
            if card2.suit == 'spades' and card2.rank == 'queen':
                has_spade_queen = True
                break

        # Sort by rank ascending
        valid_leads.sort(key=lambda x: rank_order.index(x.rank))

        if has_spade_queen:
            # Try to lead something other than spades
            non_spades = [c for c in valid_leads if c.suit != 'spades']
            if non_spades: return non_spades[0]

        return valid_leads[0]

    # We are following
    follow_suit = [c for c in hand if c.suit == leading_suit]

    if follow_suit:
        # Find highest card currently in trick of the leading suit
        current_winner_rank = -1
        for p_idx, c in trick:
            if c.suit == leading_suit:
                current_winner_rank = max(current_winner_rank, rank_order.index(c.rank))

        # Heuristic: play the highest card that is LOWER than the current winner
        safe_cards = [c for c in follow_suit if rank_order.index(c.rank) < current_winner_rank]
        if safe_cards:
            safe_cards.sort(key=lambda x: rank_order.index(x.rank), reverse=True)
            return safe_cards[0]

        # If we must win, play the lowest card to "save" high cards for later
        follow_suit.sort(key=lambda x: rank_order.index(x.rank))
        return follow_suit[0]
    else:
        # We are void! DUMP POINTS.
        # Priority: Queen of Spades > High Hearts > High Cards

        # Search for Queen of Spades
        spade_queen = next((c for c in hand if c.suit == 'spades' and c.rank == 'queen'), None)
        if spade_queen: return spade_queen

        # Search for high hearts
        hearts = [c for c in hand if c.suit == 'hearts']
        if hearts:
            hearts.sort(key=lambda x: rank_order.index(x.rank), reverse=True)
            return hearts[0]

        # Just dump the highest card
        hand.sort(key=lambda x: rank_order.index(x.rank), reverse=True)
        return hand[0]
