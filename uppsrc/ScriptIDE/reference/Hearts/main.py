import sys
import hearts_view
from hearts.logic import GameState

state = GameState()
asset_base = "assets/" # Resolved relative to .gamestate location in C++

selected_cards = []

def ui_log(msg):
    hearts_view.log(msg)

state.log_callback = ui_log

def start():
    hearts_view.log("Hearts game starting...")
    state.deal()
    refresh_ui()
    if "--autoplay" in sys.argv:
        autoplay_loop()

def refresh_ui():
    hearts_view.clear_sprites()

    # Human hand (Player 0)
    hand_rect = hearts_view.get_zone_rect("hand_self")
    if hand_rect:
        card_count = len(state.players[0])
        if card_count > 0:
            start_x = hand_rect['x']
            available_width = hand_rect['w'] - 72
            step_x = available_width / max(1, card_count - 1)
            if step_x > 30: step_x = 30

            total_width = step_x * (card_count - 1) + 72
            start_x += (hand_rect['w'] - total_width) / 2

            # Sort hand for easier viewing (by suit then rank)
            sorted_hand = sorted(state.players[0], key=lambda c: (c.suit, c.get_points(), c.rank))
            # Sync sorted hand back so logic matches visual index if needed, though we find by id
            state.players[0] = sorted_hand

            for i, card in enumerate(state.players[0]):
                cx = start_x + (i * step_x)
                cy = hand_rect['y']

                # Pop up selected cards
                if card in selected_cards:
                    cy -= 20

                hearts_view.set_card(card.id, asset_base + card.id + ".png", int(cx), cy)

    # Trick area
    trick_zones = ["trick_bottom", "trick_left", "trick_top", "trick_right"]
    for p_idx, card in state.trick:
        hearts_view.move_card(card.id, trick_zones[p_idx], 0, True)

def process_ai_pass():
    """Have all AI players submit their pass selections."""
    from hearts.ai import choose_pass_cards
    for i in range(4):
        if len(state.passed_cards[i]) == 0:
            state.select_pass(i, choose_pass_cards(state.players[i]))
    refresh_ui()

def process_ai_turns():
    """Advance the game while the current player is an AI (turn != 0 in human mode)."""
    from hearts.ai import choose_card
    while state.turn != 0 and state.phase == 'PLAYING':
        p_idx = state.turn
        hand = state.players[p_idx]
        if not hand: break
        card = choose_card(p_idx, hand, state.trick, state.leading_suit, state.hearts_broken)
        state.play_card(p_idx, card)
        refresh_ui()

def autoplay_loop():
    """Drive all 4 players with AI until the game ends (someone reaches 100 pts)."""
    from hearts.ai import choose_card
    hearts_view.log("Autoplay mode: 4 AI players")
    max_tricks = 2000  # safety cap
    trick_count = 0
    while trick_count < max_tricks:
        if state.phase == 'PASSING':
            process_ai_pass()
        elif state.phase == 'PLAYING':
            # Play one card for the current player (AI for all 4)
            p_idx = state.turn
            hand = state.players[p_idx]
            if not hand: break
            card = choose_card(p_idx, hand, state.trick, state.leading_suit, state.hearts_broken)
            state.play_card(p_idx, card)
            refresh_ui()
            trick_count += 1
        else:
            break
        if any(s >= 100 for s in state.scores):
            break
    hearts_view.log("--- Final Scores ---")
    for i in range(4):
        hearts_view.log(f"  Player {i}: {state.scores[i]} points")
    winner = 0
    for i in range(1, 4):
        if state.scores[i] < state.scores[winner]:
            winner = i
    hearts_view.log(f"Winner: Player {winner} with {state.scores[winner]} points!")

def on_click(card_id):
    card = next((c for c in state.players[0] if c.id == card_id), None)
    if not card: return

    if state.phase == 'PASSING':
        if card in selected_cards:
            selected_cards.remove(card)
        elif len(selected_cards) < 3:
            selected_cards.append(card)
        refresh_ui()

        if len(selected_cards) == 3:
            # Pass cards for human player, then AI
            state.select_pass(0, list(selected_cards))
            selected_cards.clear()
            process_ai_pass()
            process_ai_turns()
    else:
        success, msg = state.play_card(0, card)
        if success:
            refresh_ui()
            process_ai_turns()
        else:
            hearts_view.log(f"Invalid move: {msg}")
