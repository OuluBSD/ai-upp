import random

def choose_card(player_index, hand, trick, leading_suit, hearts_broken):
    if not leading_suit:
        # We are leading
        # Try to avoid leading points if hearts not broken
        valid_leads = []
        for c in hand:
            if c.suit == 'hearts' and not hearts_broken:
                if all(card.suit == 'hearts' for card in hand):
                    valid_leads.append(c)
            else:
                valid_leads.append(c)
        
        # Heuristic: lead low cards to stay safe
        valid_leads.sort(key=lambda x: ['2','3','4','5','6','7','8','9','10','jack','queen','king','ace'].index(x.rank))
        return valid_leads[0]
    
    # We are following
    follow_suit = [c for c in hand if c.suit == leading_suit]
    if follow_suit:
        # Heuristic: if we can't win the trick with points, play high. 
        # If there are points, play just below the current highest card if possible.
        # For simplicity: play highest valid card that doesn't win the trick, 
        # or lowest if we must win.
        follow_suit.sort(key=lambda x: ['2','3','4','5','6','7','8','9','10','jack','queen','king','ace'].index(x.rank), reverse=True)
        return follow_suit[0] # Aggressive AI: play highest
    else:
        # We are void! Dump points.
        hand.sort(key=lambda x: x.get_points(), reverse=True)
        return hand[0]
