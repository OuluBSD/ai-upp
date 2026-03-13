import sys
import hearts_view
from hearts.logic import GameState

state = None
asset_base = "assets/" # Resolved relative to .gamestate location in C++
PLAYER_NAMES = ["You", "West", "North", "East"]

selected_cards = []
autoplay_enabled = False
autoplay_finished = False
pending_pass_player = -1
started = False

AI_ACTION_DELAY_MS = 500
TRICK_RESOLVE_DELAY_MS = 900

def ui_log(msg):
    hearts_view.log(msg)

def start():
    global state
    global autoplay_enabled
    global autoplay_finished
    global pending_pass_player
    global selected_cards
    global started
    if started:
        hearts_view.log("Duplicate start() ignored.")
        return
    started = True
    state = GameState()
    state.log_callback = ui_log
    selected_cards = []
    autoplay_finished = False
    pending_pass_player = -1
    hearts_view.log("Hearts game starting...")
    state.deal()
    autoplay_enabled = "--autoplay" in sys.argv
    refresh_ui()
    if autoplay_enabled:
        hearts_view.log("Autoplay mode: 4 AI players")
        schedule_ai_step(AI_ACTION_DELAY_MS)

def pass_direction_text():
    pass_dir = state.round_number % 4
    if pass_dir == 0:
        return "hold"
    if pass_dir == 1:
        return "left"
    if pass_dir == 2:
        return "right"
    return "across"

def has_game_over_score():
    for score in state.scores:
        if score >= 100:
            return True
    return False

def suit_order_value(suit):
    if suit == "clubs":
        return 0
    if suit == "diamonds":
        return 1
    if suit == "hearts":
        return 2
    if suit == "spades":
        return 3
    return 99

def rank_order_value(rank):
    order = ["2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king", "ace"]
    for i in range(len(order)):
        if order[i] == rank:
            return i
    return 99

def sort_cards_for_hand(cards):
    result = []
    for card in cards:
        result.append(card)

    for i in range(len(result)):
        best = i
        for j in range(i + 1, len(result)):
            best_card = result[best]
            card = result[j]
            best_suit = suit_order_value(best_card.suit)
            suit = suit_order_value(card.suit)
            best_points = best_card.get_points()
            points = card.get_points()
            best_rank = rank_order_value(best_card.rank)
            rank = rank_order_value(card.rank)

            before = False
            if suit < best_suit:
                before = True
            elif suit == best_suit:
                if points < best_points:
                    before = True
                elif points == best_points and rank < best_rank:
                    before = True

            if before:
                best = j

        if best != i:
            tmp = result[i]
            result[i] = result[best]
            result[best] = tmp
    return result

def ai_rank_index(rank_order, rank):
    for i in range(len(rank_order)):
        if rank_order[i] == rank:
            return i
    return -1

def ai_choose_pass_cards(hand):
    scored_hand = []
    for c in hand:
        score = c.get_points() * 10
        if c.suit == 'spades':
            if c.rank == 'queen' or c.rank == 'king' or c.rank == 'ace':
                score += 15
        if c.rank == 'ace' or c.rank == 'king' or c.rank == 'queen':
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

def ai_choose_card(player_index, hand, trick, leading_suit, hearts_broken):
    rank_order = ['2','3','4','5','6','7','8','9','10','jack','queen','king','ace']

    if not leading_suit:
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

        has_spade_queen = False
        for card2 in hand:
            if card2.suit == 'spades' and card2.rank == 'queen':
                has_spade_queen = True
                break

        for i in range(len(valid_leads)):
            best = i
            for j in range(i + 1, len(valid_leads)):
                if ai_rank_index(rank_order, valid_leads[j].rank) < ai_rank_index(rank_order, valid_leads[best].rank):
                    best = j
            if best != i:
                tmp = valid_leads[i]
                valid_leads[i] = valid_leads[best]
                valid_leads[best] = tmp

        if has_spade_queen:
            non_spades = []
            for c in valid_leads:
                if c.suit != 'spades':
                    non_spades.append(c)
            if non_spades:
                return non_spades[0]

        return valid_leads[0]

    follow_suit = []
    for c in hand:
        if c.suit == leading_suit:
            follow_suit.append(c)

    if follow_suit:
        current_winner_rank = -1
        for trick_item in trick:
            p_idx, c = trick_item
            if c.suit == leading_suit:
                card_rank = ai_rank_index(rank_order, c.rank)
                if card_rank > current_winner_rank:
                    current_winner_rank = card_rank

        safe_cards = []
        for c in follow_suit:
            if ai_rank_index(rank_order, c.rank) < current_winner_rank:
                safe_cards.append(c)
        if safe_cards:
            for i in range(len(safe_cards)):
                best = i
                for j in range(i + 1, len(safe_cards)):
                    if ai_rank_index(rank_order, safe_cards[j].rank) > ai_rank_index(rank_order, safe_cards[best].rank):
                        best = j
                if best != i:
                    tmp = safe_cards[i]
                    safe_cards[i] = safe_cards[best]
                    safe_cards[best] = tmp
            return safe_cards[0]

        for i in range(len(follow_suit)):
            best = i
            for j in range(i + 1, len(follow_suit)):
                if ai_rank_index(rank_order, follow_suit[j].rank) < ai_rank_index(rank_order, follow_suit[best].rank):
                    best = j
            if best != i:
                tmp = follow_suit[i]
                follow_suit[i] = follow_suit[best]
                follow_suit[best] = tmp
        return follow_suit[0]
    else:
        spade_queen = None
        for c in hand:
            if c.suit == 'spades' and c.rank == 'queen':
                spade_queen = c
                break
        if spade_queen:
            return spade_queen

        hearts = []
        for c in hand:
            if c.suit == 'hearts':
                hearts.append(c)
        if hearts:
            for i in range(len(hearts)):
                best = i
                for j in range(i + 1, len(hearts)):
                    if ai_rank_index(rank_order, hearts[j].rank) > ai_rank_index(rank_order, hearts[best].rank):
                        best = j
                if best != i:
                    tmp = hearts[i]
                    hearts[i] = hearts[best]
                    hearts[best] = tmp
            return hearts[0]

        for i in range(len(hand)):
            best = i
            for j in range(i + 1, len(hand)):
                if ai_rank_index(rank_order, hand[j].rank) > ai_rank_index(rank_order, hand[best].rank):
                    best = j
            if best != i:
                tmp = hand[i]
                hand[i] = hand[best]
                hand[best] = tmp
        return hand[0]

def choose_simple_pass_cards(hand):
    result = []
    count = 3
    if len(hand) < count:
        count = len(hand)
    for i in range(count):
        result.append(hand[i])
    return result

def choose_simple_play_card(player_index):
    hand = state.players[player_index]
    for i in range(len(hand)):
        card = hand[i]
        ok, _ = state.validate_play(player_index, card)
        if ok:
            return card
    if hand:
        return hand[0]
    return None

def update_hud():
    label_ids = ["label_self", "label_left", "label_top", "label_right"]
    hand_zone_ids = ["hand_self", "hand_left", "hand_top", "hand_right"]
    for i in range(len(label_ids)):
        zone_id = label_ids[i]
        name = PLAYER_NAMES[i]
        if state.phase == 'PLAYING' and state.turn == i:
            name = "[" + name + "]"
        hearts_view.set_label(
            zone_id,
            name + "  " + str(state.scores[i]) + "  (" + str(len(state.players[i])) + ")"
        )
        hearts_view.set_highlight(hand_zone_ids[i], state.phase == 'PLAYING' and state.turn == i)

    in_passing = state.phase == 'PASSING' and pass_direction_text() != "hold"
    hearts_view.set_button("button_clear", "Clear", in_passing and len(selected_cards) > 0)
    hearts_view.set_button("button_pass", "Pass cards", in_passing and len(selected_cards) == 3)

    trick_count = len(state.trick)
    if state.phase == 'PASSING':
        selected = len(selected_cards)
        if pass_direction_text() == "hold":
            status = "Round " + str(state.round_number) + ": no passing. Starting play."
        else:
            status = "Round " + str(state.round_number) + ": pass 3 cards " + \
                     pass_direction_text() + " (" + str(selected) + "/3 selected)"
    elif state.phase == 'PLAYING':
        actor = PLAYER_NAMES[state.turn]
        prefix = "Your turn"
        if state.turn != 0:
            prefix = "Waiting for " + actor
        if state.trick_pending:
            prefix = "Resolving trick"
        broken = "hearts broken"
        if not state.hearts_broken:
            broken = "hearts closed"
        status = "Round " + str(state.round_number) + ": " + prefix + \
                 ". Trick " + str(trick_count) + "/4, " + broken + "."
    else:
        status = "Round " + str(state.round_number) + ": " + state.phase

    if state.last_trick_winner >= 0 and len(state.trick) == 0 and state.phase == 'PLAYING':
        status = status + " Last trick: " + PLAYER_NAMES[state.last_trick_winner] + \
                 " won " + str(state.last_trick_points) + " pts."

    hearts_view.set_status(status)

def draw_hidden_hand(zone_id, player_index, sprite_prefix, vertical):
    hand_rect = hearts_view.get_zone_rect(zone_id)
    if not hand_rect:
        return

    card_count = len(state.players[player_index])
    if card_count <= 0:
        return

    if vertical:
        available_height = hand_rect['h'] - 96
        step = available_height / max(1, card_count - 1)
        if step > 18:
            step = 18
        total_height = step * (card_count - 1) + 96
        start_y = hand_rect['y'] + (hand_rect['h'] - total_height) / 2
        x = hand_rect['x']
        for i in range(card_count):
            hearts_view.set_card(
                sprite_prefix + "_" + str(i),
                asset_base + "card_back.png",
                x,
                int(start_y + i * step),
                90 if zone_id == "hand_left" else 270
            )
    else:
        available_width = hand_rect['w'] - 72
        step = available_width / max(1, card_count - 1)
        if step > 24:
            step = 24
        total_width = step * (card_count - 1) + 72
        start_x = hand_rect['x'] + (hand_rect['w'] - total_width) / 2
        y = hand_rect['y']
        for i in range(card_count):
            hearts_view.set_card(
                sprite_prefix + "_" + str(i),
                asset_base + "card_back.png",
                int(start_x + i * step),
                y,
                0
            )

def refresh_ui():
    hearts_view.clear_sprites()
    update_hud()

    draw_hidden_hand("hand_left", 1, "opp_left", True)
    draw_hidden_hand("hand_top", 2, "opp_top", False)
    draw_hidden_hand("hand_right", 3, "opp_right", True)

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
            sorted_hand = sort_cards_for_hand(state.players[0])
            # Sync sorted hand back so logic matches visual index if needed, though we find by id
            state.players[0] = sorted_hand

            for i in range(len(state.players[0])):
                card = state.players[0][i]
                cx = start_x + (i * step_x)
                cy = hand_rect['y']

                # Pop up selected cards
                if card in selected_cards:
                    cy -= 20

                hearts_view.set_card(card.id, asset_base + card.id + ".png", int(cx), cy, 0)

    # Trick area
    trick_zones = ["trick_bottom", "trick_left", "trick_top", "trick_right"]
    for i in range(len(state.trick)):
        p_idx, card = state.trick[i]
        hearts_view.move_card(card.id, trick_zones[p_idx], 0, True)

def commit_pass():
    global selected_cards
    global pending_pass_player
    if len(selected_cards) != 3:
        hearts_view.log("Select exactly 3 cards to pass.")
        refresh_ui()
        return

    pass_cards = []
    for card in selected_cards:
        pass_cards.append(card)
    hearts_view.log("Passing selected cards...")
    state.select_pass(0, pass_cards)
    selected_cards = []
    refresh_ui()
    pending_pass_player = 1
    schedule_ai_step(AI_ACTION_DELAY_MS)

def finish_autoplay_if_needed():
    global autoplay_finished
    if not autoplay_enabled or autoplay_finished:
        return
    if not has_game_over_score():
        return
    autoplay_finished = True
    hearts_view.log("--- Final Scores ---")
    for i in range(4):
        hearts_view.log("  Player " + str(i) + ": " + str(state.scores[i]) + " points")
    winner = 0
    for i in range(1, 4):
        if state.scores[i] < state.scores[winner]:
            winner = i
    hearts_view.log("Winner: Player " + str(winner) + " with " + str(state.scores[winner]) + " points!")

def schedule_ai_step(delay_ms):
    if has_game_over_score():
        finish_autoplay_if_needed()
        return
    hearts_view.set_timeout(delay_ms, "ai_step")

def ai_step():
    global pending_pass_player

    if has_game_over_score():
        finish_autoplay_if_needed()
        return

    if state.trick_pending:
        state.resolve_trick()
        refresh_ui()
        if has_game_over_score():
            finish_autoplay_if_needed()
            return
        if autoplay_enabled or state.turn != 0 or pending_pass_player >= 0:
            schedule_ai_step(AI_ACTION_DELAY_MS)
        return

    if state.phase == 'PASSING':
        if pending_pass_player < 0:
            if autoplay_enabled:
                pending_pass_player = 0
            else:
                return
        if pending_pass_player >= 4:
            pending_pass_player = -1
            if autoplay_enabled or state.turn != 0:
                schedule_ai_step(AI_ACTION_DELAY_MS)
            return

        hand = state.players[pending_pass_player]
        pass_cards = choose_simple_pass_cards(hand)
        hearts_view.log("AI player " + str(pending_pass_player) + " passes cards")
        state.select_pass(pending_pass_player, pass_cards)
        pending_pass_player += 1
        if pending_pass_player >= 4:
            pending_pass_player = -1
        refresh_ui()
        if has_game_over_score():
            finish_autoplay_if_needed()
            return
        if state.phase == 'PASSING' or autoplay_enabled or state.turn != 0:
            schedule_ai_step(AI_ACTION_DELAY_MS)
        return

    if state.phase != 'PLAYING':
        return

    if not autoplay_enabled and state.turn == 0:
        return

    p_idx = state.turn
    card = choose_simple_play_card(p_idx)
    if not card:
        return
    state.play_card(p_idx, card)
    refresh_ui()
    if state.trick_pending:
        schedule_ai_step(TRICK_RESOLVE_DELAY_MS)
    elif autoplay_enabled or state.turn != 0:
        schedule_ai_step(AI_ACTION_DELAY_MS)

def on_click(card_id):
    card = None
    for i in range(len(state.players[0])):
        c = state.players[0][i]
        if c.id == card_id:
            card = c
            break
    if not card: return

    if state.phase == 'PASSING':
        if card in selected_cards:
            selected_cards.remove(card)
        elif len(selected_cards) < 3:
            selected_cards.append(card)
        refresh_ui()
    else:
        success, msg = state.play_card(0, card)
        if success:
            refresh_ui()
            if state.trick_pending:
                schedule_ai_step(TRICK_RESOLVE_DELAY_MS)
            elif state.turn != 0:
                schedule_ai_step(AI_ACTION_DELAY_MS)
        else:
            hearts_view.log("Invalid move: " + str(msg))

def on_button(button_id):
    if button_id == "button_clear":
        if selected_cards:
            selected_cards.clear()
            refresh_ui()
        return

    if button_id == "button_pass":
        commit_pass()
