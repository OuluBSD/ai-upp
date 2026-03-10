import hearts_view
from hearts.logic import GameState

state = GameState()
asset_base = "uppsrc/ScriptIDE/reference/Hearts/assets/"

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
    x = 50
    for card in state.players[0]:
        hearts_view.set_card(card.id, asset_base + card.id + ".png", x, 450)
        x += 30
    
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
