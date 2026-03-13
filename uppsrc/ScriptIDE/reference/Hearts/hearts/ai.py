import random

def rank_index(rank_order, rank):
    for i in range(len(rank_order)):
        if rank_order[i] == rank:
            return i
    return -1

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

    for i in range(len(scored_hand)):
        best = i
        for j in range(i + 1, len(scored_hand)):
            if scored_hand[j][0] > scored_hand[best][0]:
                best = j
        if best != i:
            tmp = scored_hand[i]
            scored_hand[i] = scored_hand[best]
            scored_hand[best] = tmp

    result = []
    count = 3
    if len(scored_hand) < count:
        count = len(scored_hand)
    for i in range(count):
        result.append(scored_hand[i][1])
    return result

def choose_card(player_index, hand, trick, leading_suit, hearts_broken):
    rank_order = ['2','3','4','5','6','7','8','9','10','jack','queen','king','ace']

    if not leading_suit:
        # We are leading the trick
        valid_leads = []
        for c in hand:
            if c.suit == 'hearts' and not hearts_broken:
                only_hearts = True
                for card in hand:
                    if card.suit != 'hearts':
                        only_hearts = False
                        break
                if only_hearts:
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
        for i in range(len(valid_leads)):
            best = i
            for j in range(i + 1, len(valid_leads)):
                if rank_index(rank_order, valid_leads[j].rank) < rank_index(rank_order, valid_leads[best].rank):
                    best = j
            if best != i:
                tmp = valid_leads[i]
                valid_leads[i] = valid_leads[best]
                valid_leads[best] = tmp

        if has_spade_queen:
            # Try to lead something other than spades
            non_spades = []
            for c in valid_leads:
                if c.suit != 'spades':
                    non_spades.append(c)
            if non_spades: return non_spades[0]

        return valid_leads[0]

    # We are following
    follow_suit = []
    for c in hand:
        if c.suit == leading_suit:
            follow_suit.append(c)

    if follow_suit:
        # Find highest card currently in trick of the leading suit
        current_winner_rank = -1
        for trick_item in trick:
            p_idx, c = trick_item
            if c.suit == leading_suit:
                card_rank = rank_index(rank_order, c.rank)
                if card_rank > current_winner_rank:
                    current_winner_rank = card_rank

        # Heuristic: play the highest card that is LOWER than the current winner
        safe_cards = []
        for c in follow_suit:
            if rank_index(rank_order, c.rank) < current_winner_rank:
                safe_cards.append(c)
        if safe_cards:
            for i in range(len(safe_cards)):
                best = i
                for j in range(i + 1, len(safe_cards)):
                    if rank_index(rank_order, safe_cards[j].rank) > rank_index(rank_order, safe_cards[best].rank):
                        best = j
                if best != i:
                    tmp = safe_cards[i]
                    safe_cards[i] = safe_cards[best]
                    safe_cards[best] = tmp
            return safe_cards[0]

        # If we must win, play the lowest card to "save" high cards for later
        for i in range(len(follow_suit)):
            best = i
            for j in range(i + 1, len(follow_suit)):
                if rank_index(rank_order, follow_suit[j].rank) < rank_index(rank_order, follow_suit[best].rank):
                    best = j
            if best != i:
                tmp = follow_suit[i]
                follow_suit[i] = follow_suit[best]
                follow_suit[best] = tmp
        return follow_suit[0]
    else:
        # We are void! DUMP POINTS.
        # Priority: Queen of Spades > High Hearts > High Cards

        # Search for Queen of Spades
        spade_queen = None
        for c in hand:
            if c.suit == 'spades' and c.rank == 'queen':
                spade_queen = c
                break
        if spade_queen: return spade_queen

        # Search for high hearts
        hearts = []
        for c in hand:
            if c.suit == 'hearts':
                hearts.append(c)
        if hearts:
            for i in range(len(hearts)):
                best = i
                for j in range(i + 1, len(hearts)):
                    if rank_index(rank_order, hearts[j].rank) > rank_index(rank_order, hearts[best].rank):
                        best = j
                if best != i:
                    tmp = hearts[i]
                    hearts[i] = hearts[best]
                    hearts[best] = tmp
            return hearts[0]

        # Just dump the highest card
        for i in range(len(hand)):
            best = i
            for j in range(i + 1, len(hand)):
                if rank_index(rank_order, hand[j].rank) > rank_index(rank_order, hand[best].rank):
                    best = j
            if best != i:
                tmp = hand[i]
                hand[i] = hand[best]
                hand[best] = tmp
        return hand[0]
