import hearts_view
from hearts.logic import GameState

state = GameState()
asset_base = "uppsrc/ScriptIDE/reference/Hearts/assets/"

def start():
    print("Hearts game starting...")
    state.deal()
    refresh_ui()

def refresh_ui():
    # Show human cards (player 0)
    hearts_view.clear_sprites()
    
    x = 50
    for card in state.players[0]:
        hearts_view.set_card(card.id, asset_base + card.id + ".png", x, 450)
        x += 30
    
    # Show trick area
    tx = 300
    for p_idx, card in state.trick:
        hearts_view.set_card(f"trick_{p_idx}", asset_base + card.id + ".png", tx, 200)
        tx += 80

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
        refresh_ui()

if __name__ == "__main__":
    start()
