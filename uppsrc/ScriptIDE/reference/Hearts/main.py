import sys
import hearts_view
from hearts.logic import GameState

state = None
asset_base = "../../../../share/imgs/cards/default/" # Resolved relative to .gamestate location in C++
PLAYER_NAMES = ["You", "West", "North", "East"]

selected_cards = []
autoplay_enabled = False
autoplay_finished = False
pending_pass_player = -1
started = False
rendered_round_number = -1
rendered_trick_ids = []
rendered_hidden_counts = {"opp_left": 0, "opp_top": 0, "opp_right": 0}
rendered_hand_ids = []
current_hand_ids_for_assert = []
current_trick_ids_for_assert = []
collecting_trick = False
collecting_winner = -1
collecting_points = 0
pass_animating = False

AI_ACTION_DELAY_MS = 500
TRICK_RESOLVE_DELAY_MS = 900
TRICK_COLLECT_DELAY_MS = 700
PASS_ANIMATION_DELAY_MS = 700
ROUND_SUMMARY_DELAY_MS = 2500

def ui_log(msg):
    hearts_view.log(msg)

def assert_state_invariants(where):
    total_cards = len(state.trick)
    ids = []
    for i in range(4):
        total_cards = total_cards + len(state.players[i])
        for c in state.players[i]:
            # DONT REMOVE: None cards or cards without ids have repeatedly shown up as
            # impossible rendered sprites (for example sprite id "None") during pass/play transitions.
            assert c is not None, where + ": player " + str(i) + " has None card"
            cid = str(c.id)
            assert cid != "" and cid != "None", where + ": player " + str(i) + " has card with invalid id"
            ids.append(c.id)
    for trick_item in state.trick:
        _, c = trick_item
        assert c is not None, where + ": trick has None card"
        cid = str(c.id)
        assert cid != "" and cid != "None", where + ": trick has card with invalid id"
        ids.append(c.id)

    assert total_cards >= 0 and total_cards <= 52, where + ": total cards = " + str(total_cards)
    assert (total_cards % 4) == 0, where + ": total cards not divisible by 4: " + str(total_cards)

    unique = []
    for card_id in ids:
        seen = False
        for old_id in unique:
            if old_id == card_id:
                seen = True
                break
        assert not seen, where + ": duplicate card " + str(card_id)
        unique.append(card_id)

    if state.phase == 'PASSING':
        selected_total = 0
        for i in range(4):
            selected_total = selected_total + len(state.passed_cards[i])
            assert len(state.passed_cards[i]) <= 3, where + ": player " + str(i) + " passed > 3 cards"
        assert selected_total <= 12, where + ": selected_total = " + str(selected_total)

    if state.phase == 'PLAYING':
        assert state.turn >= 0 and state.turn < 4, where + ": invalid turn " + str(state.turn)
        assert len(state.trick) <= 4, where + ": trick size = " + str(len(state.trick))
        if state.trick_pending:
            assert len(state.trick) == 4, where + ": trick_pending with trick size " + str(len(state.trick))

def assert_render_invariants(where, current_face_card_ids):
    # DONT REMOVE: these assertions catch stale visible hand/trick sprites that are
    # otherwise easy to miss because the logical game state can already be correct.
    assert len(current_hand_ids_for_assert) == len(state.players[0]), where + ": rendered hand ids = " + str(len(current_hand_ids_for_assert)) + ", expected " + str(len(state.players[0]))
    expected_face = len(state.players[0]) + len(state.trick)
    assert len(current_face_card_ids) == expected_face, where + ": rendered face cards = " + str(len(current_face_card_ids)) + ", expected " + str(expected_face)

    unique = []
    for card_id in current_face_card_ids:
        seen = False
        for old_id in unique:
            if old_id == card_id:
                seen = True
                break
        assert not seen, where + ": duplicate rendered face card " + str(card_id)
        unique.append(card_id)

    for hand_id in current_hand_ids_for_assert:
        for trick_id in current_trick_ids_for_assert:
            assert hand_id != trick_id, where + ": card rendered both in hand and trick: " + str(hand_id)

    assert rendered_hidden_counts["opp_left"] == len(state.players[1]), where + ": left hidden count mismatch"
    assert rendered_hidden_counts["opp_top"] == len(state.players[2]), where + ": top hidden count mismatch"
    assert rendered_hidden_counts["opp_right"] == len(state.players[3]), where + ": right hidden count mismatch"

def start():
    global state
    global autoplay_enabled
    global autoplay_finished
    global pending_pass_player
    global selected_cards
    global started
    global rendered_round_number
    global rendered_trick_ids
    global rendered_hidden_counts
    global rendered_hand_ids
    global current_hand_ids_for_assert
    global current_trick_ids_for_assert
    global collecting_trick
    global collecting_winner
    global collecting_points
    global pass_animating
    if started:
        return
    started = True
    state = GameState()
    state.log_callback = ui_log
    selected_cards = []
    autoplay_finished = False
    pending_pass_player = -1
    rendered_round_number = -1
    rendered_trick_ids = []
    rendered_hidden_counts = {"opp_left": 0, "opp_top": 0, "opp_right": 0}
    rendered_hand_ids = []
    current_hand_ids_for_assert = []
    current_trick_ids_for_assert = []
    collecting_trick = False
    collecting_winner = -1
    collecting_points = 0
    pass_animating = False
    hearts_view.log("Hearts game starting...")
    state.deal()
    assert_state_invariants("start.deal")
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
            name + "  T:" + str(state.scores[i]) + "  R:+" + str(state.round_scores[i]) + \
            "  C:" + str(len(state.players[i]))
        )
        hearts_view.set_highlight(hand_zone_ids[i], state.phase == 'PLAYING' and state.turn == i)

    if state.phase == 'ROUND_END':
        summary_lines = ["Hand complete"]
        for i in range(4):
            summary_lines = summary_lines + [
                PLAYER_NAMES[i] + "  +" + str(state.last_round_scores[i]) + "  T:" + str(state.scores[i])
            ]
        if state.last_round_moon_shooter >= 0:
            summary_lines = summary_lines + ["Moon: " + PLAYER_NAMES[state.last_round_moon_shooter]]
        hearts_view.set_label("trick_area", "\n".join(summary_lines))
    elif state.game_over:
        hearts_view.set_label("trick_area", "Game over")
    else:
        hearts_view.set_label("trick_area", "")

    if state.game_over:
        score_lines = []
        for i in range(4):
            score_lines = score_lines + [
                PLAYER_NAMES[i] + "  T:" + str(state.scores[i]) + "  R:+" + str(state.round_scores[i])
            ]
        if state.last_round_scores[0] != 0 or state.last_round_scores[1] != 0 or state.last_round_scores[2] != 0 or state.last_round_scores[3] != 0:
            score_lines = score_lines + [""]
            score_lines = score_lines + ["Last hand:"]
            for i in range(4):
                score_lines = score_lines + [
                    PLAYER_NAMES[i] + "  +" + str(state.last_round_scores[i])
                ]
            if state.last_round_moon_shooter >= 0:
                score_lines = score_lines + ["Moon: " + PLAYER_NAMES[state.last_round_moon_shooter]]
        hearts_view.set_label("score_board", "\n".join(score_lines))
    else:
        hearts_view.set_label("score_board", "")

    in_passing = state.phase == 'PASSING' and pass_direction_text() != "hold"
    show_pass_controls = in_passing and pending_pass_player < 0
    if show_pass_controls:
        hearts_view.set_button("button_clear", "Clear", len(selected_cards) > 0)
        hearts_view.set_button("button_pass", "Pass cards", len(selected_cards) == 3)
    else:
        hearts_view.set_button("button_clear", "", False)
        hearts_view.set_button("button_pass", "", False)

    trick_count = len(state.trick)
    if state.phase == 'PASSING':
        selected = len(selected_cards)
        if pass_direction_text() == "hold":
            status = "Round " + str(state.round_number) + ": no passing. Starting play."
        else:
            status = "Round " + str(state.round_number) + ": pass 3 cards " + \
                     pass_direction_text() + " (" + str(selected) + "/3 selected)"
    elif state.phase == 'ROUND_END':
        status = "Round " + str(state.round_number) + " complete. Next hand starting soon."
    elif state.phase == 'PLAYING':
        actor = PLAYER_NAMES[state.turn]
        prefix = "Your turn"
        if state.turn != 0:
            prefix = "Waiting for " + actor
        if state.trick_pending:
            prefix = "Resolving trick"
        if collecting_trick and collecting_winner >= 0:
            prefix = "Collecting trick for " + PLAYER_NAMES[collecting_winner]
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

    if state.game_over:
        winner = 0
        for i in range(1, 4):
            if state.scores[i] < state.scores[winner]:
                winner = i
        status = "Game over. Winner: " + PLAYER_NAMES[winner] + " with " + str(state.scores[winner]) + " points."

    hearts_view.set_status(status)

def draw_hidden_hand(zone_id, player_index, sprite_prefix, vertical):
    global rendered_hidden_counts
    hand_rect = hearts_view.get_zone_rect(zone_id)
    if not hand_rect:
        return

    card_count = len(state.players[player_index])
    old_count = rendered_hidden_counts[sprite_prefix]

    if vertical:
        available_height = hand_rect['h'] - 96
        step = available_height / max(1, card_count - 1)
        if step > 18:
            step = 18
        total_height = step * max(0, card_count - 1) + 96
        start_y = hand_rect['y'] + (hand_rect['h'] - total_height) / 2
        x = hand_rect['x']
        for i in range(card_count):
            hearts_view.set_card(
                sprite_prefix + "_" + str(i),
                asset_base + "back9.png",
                x,
                int(start_y + i * step),
                90 if zone_id == "hand_left" else 270
            )
    else:
        available_width = hand_rect['w'] - 72
        step = available_width / max(1, card_count - 1)
        if step > 24:
            step = 24
        total_width = step * max(0, card_count - 1) + 72
        start_x = hand_rect['x'] + (hand_rect['w'] - total_width) / 2
        y = hand_rect['y']
        for i in range(card_count):
            hearts_view.set_card(
                sprite_prefix + "_" + str(i),
                asset_base + "back9.png",
                int(start_x + i * step),
                y,
                0
            )

    for i in range(card_count, old_count):
        hearts_view.remove_sprite(sprite_prefix + "_" + str(i))
    rendered_hidden_counts[sprite_prefix] = card_count

def refresh_ui():
    global rendered_round_number
    global rendered_trick_ids
    global rendered_hand_ids
    global current_hand_ids_for_assert
    global current_trick_ids_for_assert
    global selected_cards
    if rendered_round_number != state.round_number:
        hearts_view.clear_sprites()
        rendered_round_number = state.round_number
        rendered_trick_ids = []
        rendered_hidden_counts["opp_left"] = 0
        rendered_hidden_counts["opp_top"] = 0
        rendered_hidden_counts["opp_right"] = 0
        rendered_hand_ids = []
        current_hand_ids_for_assert = []
        current_trick_ids_for_assert = []
    if state.phase != 'PASSING' and len(selected_cards) != 0:
        selected_cards = []
    hearts_view.begin_sprite_frame()
    assert_state_invariants("refresh_ui")
    update_hud()

    draw_hidden_hand("hand_left", 1, "opp_left", True)
    draw_hidden_hand("hand_top", 2, "opp_top", False)
    draw_hidden_hand("hand_right", 3, "opp_right", True)

    # Human hand (Player 0)
    current_face_card_ids = []
    current_hand_ids = []
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

            i = 0
            while i < len(state.players[0]):
                card = state.players[0][i]
                current_face_card_ids = current_face_card_ids + [card.id]
                current_hand_ids = current_hand_ids + [card.id]
                cx = start_x + (i * step_x)
                cy = hand_rect['y']

                # Pop up selected cards
                if card in selected_cards:
                    cy -= 20

                hearts_view.set_card(card.id, asset_base + card.id + ".png", int(cx), cy, 0)
                i = i + 1
    # Trick area
    trick_zones = ["trick_bottom", "trick_left", "trick_top", "trick_right"]
    source_zones = ["hand_self", "hand_left", "hand_top", "hand_right"]
    current_trick_ids = []
    i = 0
    while i < len(state.trick):
        p_idx, card = state.trick[i]
        current_trick_ids = current_trick_ids + [card.id]
        current_face_card_ids = current_face_card_ids + [card.id]
        trick_rect = hearts_view.get_zone_rect(trick_zones[p_idx])
        if not trick_rect:
            i = i + 1
            continue

        target_x = trick_rect['x'] + (trick_rect['w'] - 72) / 2
        target_y = trick_rect['y'] + (trick_rect['h'] - 96) / 2

        if i == len(state.trick) - 1:
            already_rendered = False
            for existing_id in rendered_hand_ids:
                if existing_id == card.id:
                    already_rendered = True
                    break
            if not already_rendered:
                for existing_id in rendered_trick_ids:
                    if existing_id == card.id:
                        already_rendered = True
                        break
            if not already_rendered:
                source_rect = hearts_view.get_zone_rect(source_zones[p_idx])
                if source_rect:
                    start_x = source_rect['x'] + (source_rect['w'] - 72) / 2
                    start_y = source_rect['y'] + (source_rect['h'] - 96) / 2
                else:
                    start_x = target_x
                    start_y = target_y
                hearts_view.set_card(card.id, asset_base + card.id + ".png", int(start_x), int(start_y), 0)
            hearts_view.move_card(card.id, trick_zones[p_idx], 0, True)
        else:
            hearts_view.set_card(card.id, asset_base + card.id + ".png", int(target_x), int(target_y), 0)
            hearts_view.move_card(card.id, trick_zones[p_idx], 0, False)
        i = i + 1

    for old_id in rendered_trick_ids:
        found = False
        for current_id in current_trick_ids:
            if current_id == old_id:
                found = True
                break
        if not found:
            hearts_view.remove_sprite(old_id)
    rendered_trick_ids = current_trick_ids

    for old_id in rendered_hand_ids:
        found_in_hand = False
        for current_id in current_hand_ids:
            if current_id == old_id:
                found_in_hand = True
                break
        if found_in_hand:
            continue
        found_in_trick = False
        for trick_id in current_trick_ids:
            if trick_id == old_id:
                found_in_trick = True
                break
        if not found_in_trick:
            hearts_view.remove_sprite(old_id)
    rendered_hand_ids = current_hand_ids
    current_hand_ids_for_assert = current_hand_ids
    current_trick_ids_for_assert = current_trick_ids
    # DONT REMOVE: host-side render assertions catch stale visible sprites that the logical
    # Python card-state checks cannot see, especially after pass/trick transitions.
    hearts_view.set_expected_sprite_count("hand_self", len(state.players[0]))
    hearts_view.set_expected_sprite_count("hand_left", len(state.players[1]))
    hearts_view.set_expected_sprite_count("hand_top", len(state.players[2]))
    hearts_view.set_expected_sprite_count("hand_right", len(state.players[3]))
    assert_render_invariants("refresh_ui.render", current_face_card_ids)

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
    assert_state_invariants("commit_pass.select_pass")
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

def hand_zone_id(player_index):
    if player_index == 0:
        return "hand_self"
    if player_index == 1:
        return "hand_left"
    if player_index == 2:
        return "hand_top"
    return "hand_right"

def pass_target_index(player_index):
    pass_dir = state.round_number % 4
    if pass_dir == 1:
        return (player_index + 1) % 4
    if pass_dir == 2:
        return (player_index + 3) % 4
    return (player_index + 2) % 4

def start_pass_animation():
    global pass_animating
    if pass_animating:
        return
    pass_animating = True
    for i in range(4):
        source_zone = hand_zone_id(i)
        target_zone = hand_zone_id(pass_target_index(i))
        cards = state.passed_cards[i]
        for j in range(len(cards)):
            card = cards[j]
            sprite_id = "pass_" + str(i) + "_" + str(j) + "_" + str(card.id)
            asset = asset_base + "back9.png"
            if i == 0:
                asset = asset_base + card.id + ".png"
            source_rect = hearts_view.get_zone_rect(source_zone)
            if source_rect:
                sx = source_rect['x'] + (source_rect['w'] - 72) / 2 + (j - 1) * 18
                sy = source_rect['y'] + (source_rect['h'] - 96) / 2
            else:
                sx = 0
                sy = 0
            hearts_view.set_card(sprite_id, asset, int(sx), int(sy), 0)
            hearts_view.move_card(sprite_id, target_zone, (j - 1) * 18, True)
    hearts_view.set_timeout(PASS_ANIMATION_DELAY_MS, "finish_pass_animation")

def finish_pass_animation():
    global pass_animating
    pass_animating = False
    for i in range(4):
        cards = state.passed_cards[i]
        for j in range(len(cards)):
            card = cards[j]
            hearts_view.remove_sprite("pass_" + str(i) + "_" + str(j) + "_" + str(card.id))
    refresh_ui()
    if autoplay_enabled or state.turn != 0:
        schedule_ai_step(AI_ACTION_DELAY_MS)

def start_trick_collect():
    global collecting_trick
    global collecting_winner
    global collecting_points
    if not state.trick_pending:
        return
    collecting_trick = True
    collecting_winner = state.pending_trick_winner
    collecting_points = state.pending_trick_points
    offsets = [-24, -8, 8, 24]
    zone_id = hand_zone_id(collecting_winner)
    for i in range(len(state.trick)):
        p_idx, card = state.trick[i]
        offset = 0
        if i < len(offsets):
            offset = offsets[i]
        hearts_view.move_card(card.id, zone_id, offset, True)
    update_hud()
    hearts_view.set_timeout(TRICK_COLLECT_DELAY_MS, "finish_trick_collect")

def finish_trick_collect():
    global collecting_trick
    global collecting_winner
    global collecting_points
    if not state.trick_pending:
        collecting_trick = False
        collecting_winner = -1
        collecting_points = 0
        return
    state.resolve_trick()
    collecting_trick = False
    collecting_winner = -1
    collecting_points = 0
    refresh_ui()
    if has_game_over_score():
        finish_autoplay_if_needed()
        return
    if state.phase == 'ROUND_END':
        hearts_view.set_timeout(ROUND_SUMMARY_DELAY_MS, "next_round")
        return
    if autoplay_enabled or state.turn != 0 or pending_pass_player >= 0:
        schedule_ai_step(AI_ACTION_DELAY_MS)

def next_round():
    if state.game_over:
        finish_autoplay_if_needed()
        return
    if state.phase != 'ROUND_END':
        return
    state.begin_next_round()
    assert_state_invariants("next_round.deal")
    refresh_ui()
    if autoplay_enabled:
        schedule_ai_step(AI_ACTION_DELAY_MS)

def ai_step():
    global pending_pass_player

    assert_state_invariants("ai_step.begin")

    if has_game_over_score():
        finish_autoplay_if_needed()
        return

    if pass_animating:
        return

    if state.trick_pending:
        if not collecting_trick:
            start_trick_collect()
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
        assert_state_invariants("ai_step.pass")
        pending_pass_player = pending_pass_player + 1
        if pending_pass_player >= 4:
            pending_pass_player = -1
        if state.phase == 'PLAYING':
            refresh_ui()
            start_pass_animation()
            return
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
    success, msg = state.play_card(p_idx, card)
    if not success:
        hearts_view.log("AI move failed: " + str(msg))
        schedule_ai_step(AI_ACTION_DELAY_MS)
        return
    assert_state_invariants("ai_step.play")
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
    if pass_animating:
        return

    if state.phase == 'PASSING':
        if card in selected_cards:
            selected_cards.remove(card)
        elif len(selected_cards) < 3:
            selected_cards.append(card)
        refresh_ui()
    else:
        success, msg = state.play_card(0, card)
        if success:
            assert_state_invariants("on_click.play")
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
