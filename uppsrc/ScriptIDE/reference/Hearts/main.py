import hearts_view
from hearts.logic import GameState

state = GameState()
asset_base = "uppsrc/ScriptIDE/reference/Hearts/assets/"

def start():
    print("Hearts game starting...")
    state.deal()
    refresh_ui()

def refresh_ui():
    # Human hand is handled by the zone 'hand_self'
    # For now, we manually offset within that zone.
    
    # We clear and redraw for simplicity in this prototype.
    hearts_view.clear_sprites() 
    
    # Refresh human hand sprites
    x = 50
    for card in state.players[0]:
        hearts_view.set_card(card.id, asset_base + card.id + ".png", x, 450)
        x += 30
    
    # Trick area zones: trick_left, trick_top, trick_right, trick_bottom
    # Player 0 is bottom, 1 is left, 2 is top, 3 is right
    trick_zones = ["trick_bottom", "trick_left", "trick_top", "trick_right"]
    
    for p_idx, card in state.trick:
        # Move card to the center of its assigned zone
        hearts_view.move_card(card.id, trick_zones[p_idx], 0, True)

def on_click(card_id):
    print(f"Clicked card: {card_id}")
    # Find card in player 0 hand
    card = next((c for c in state.players[0] if c.id == card_id), None)
    if card:
        success, msg = state.play_card(0, card)
        if success:
            refresh_ui()
            # Let AI play
            process_ai_turns()
        else:
            print(f"Invalid move: {msg}")

def process_ai_turns():
    from hearts.ai import choose_card
    
    while state.turn != 0:
        p_idx = state.turn
        hand = state.players[p_idx]
        card = choose_card(p_idx, hand, state.trick, state.leading_suit, state.hearts_broken)
        
        print(f"AI Player {p_idx} plays {card.id}")
        state.play_card(p_idx, card)
        # Refresh UI will trigger the move_card to the correct zone
        refresh_ui()

if __name__ == "__main__":
    start()
