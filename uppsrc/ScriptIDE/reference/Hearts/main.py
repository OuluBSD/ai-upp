import hearts_view
from hearts.logic import GameState

state = GameState()
asset_base = "assets/" # Resolved relative to .gamestate location in C++

def ui_log(msg):
    hearts_view.log(msg)

state.log_callback = ui_log

def start():
    hearts_view.log("Hearts game starting...")
    state.deal()
    refresh_ui()

def refresh_ui():
    hearts_view.clear_sprites() 
    
    # Human hand (Player 0)
    # We use the zone to fan the cards
    hand_rect = hearts_view.get_zone_rect("hand_self")
    if hand_rect:
        card_count = len(state.players[0])
        if card_count > 0:
            # Assuming card width ~ 72, distribute them
            start_x = hand_rect['x']
            available_width = hand_rect['w'] - 72
            step_x = available_width / max(1, card_count - 1)
            if step_x > 30: step_x = 30 # Max spacing
            
            # Center the fan if fewer cards
            total_width = step_x * (card_count - 1) + 72
            start_x += (hand_rect['w'] - total_width) / 2
            
            for i, card in enumerate(state.players[0]):
                cx = start_x + (i * step_x)
                hearts_view.set_card(card.id, asset_base + card.id + ".png", int(cx), hand_rect['y'])
    
    # Trick area
    trick_zones = ["trick_bottom", "trick_left", "trick_top", "trick_right"]
    for p_idx, card in state.trick:
        hearts_view.move_card(card.id, trick_zones[p_idx], 0, True)

def on_click(card_id):
    card = next((c for c in state.players[0] if c.id == card_id), None)
    if card:
        success, msg = state.play_card(0, card)
        if success:
            refresh_ui()
            process_ai_turns()
        else:
            hearts_view.log(f"Invalid move: {msg}")

def process_ai_turns():
    from hearts.ai import choose_card
    
    while state.turn != 0:
        p_idx = state.turn
        hand = state.players[p_idx]
        if not hand: break # Round ended
        
        card = choose_card(p_idx, hand, state.trick, state.leading_suit, state.hearts_broken)
        state.play_card(p_idx, card)
        refresh_ui()

if __name__ == "__main__":
    start()
