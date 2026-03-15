import cardgame_view
import solitaire_bridge

asset_base = "../../../../share/imgs/cards/default/"

deal_index = 0
move_count = 0
last_action = "Click the stock or drag a visible card."

rank_names = ["ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king"]
rank_values = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13]
suit_names = ["clubs", "diamonds", "hearts", "spades"]
suit_colors = ["black", "red", "red", "black"]

foundation_names = ["foundation_1", "foundation_2", "foundation_3", "foundation_4"]
tableau_names = ["tableau_1", "tableau_2", "tableau_3", "tableau_4", "tableau_5", "tableau_6", "tableau_7"]

card_defs = []
stock = []
waste = []
foundations = [[], [], [], []]
tableau = [[], [], [], [], [], [], []]
tableau_up = [0, 0, 0, 0, 0, 0, 0]

sprite_ids = []
sprite_zones = []
sprite_indexes = []


def build_card_defs():
    global card_defs
    if len(card_defs) > 0:
        return
    i = 0
    while i < len(suit_names):
        j = 0
        while j < len(rank_names):
            card_defs.append([suit_names[i] + "_" + rank_names[j], rank_values[j], suit_names[i], suit_colors[i]])
            j = j + 1
        i = i + 1


def card_asset(card_name):
    return asset_base + card_name + ".png"


def find_card_def(card_name):
    i = 0
    while i < len(card_defs):
        if card_defs[i][0] == card_name:
            return card_defs[i]
        i = i + 1
    return ["", 0, "", ""]


def card_rank(card_name):
    return find_card_def(card_name)[1]


def card_suit(card_name):
    return find_card_def(card_name)[2]


def card_color(card_name):
    return find_card_def(card_name)[3]


def take_first(items):
    if len(items) == 0:
        return ""
    item = items[0]
    items.remove(item)
    return item


def take_last(items):
    if len(items) == 0:
        return ""
    item = items[len(items) - 1]
    items.remove(item)
    return item


def clear_sprite_registry():
    sprite_ids.clear()
    sprite_zones.clear()
    sprite_indexes.clear()


def register_sprite(sprite_id, zone_id, index):
    sprite_ids.append(sprite_id)
    sprite_zones.append(zone_id)
    sprite_indexes.append(index)


def lookup_sprite(sprite_id):
    i = 0
    while i < len(sprite_ids):
        if sprite_ids[i] == sprite_id:
            return [sprite_zones[i], sprite_indexes[i]]
        i = i + 1
    return ["", -1]


def zone_index(zone_names, zone_id):
    i = 0
    while i < len(zone_names):
        if zone_names[i] == zone_id:
            return i
        i = i + 1
    return -1


def make_deck(offset):
    deck = []
    i = 0
    while i < len(card_defs):
        deck_index = (offset + i) % len(card_defs)
        deck.append(card_defs[deck_index][0])
        i = i + 1
    return deck


def append_cards(dst, cards):
    i = 0
    while i < len(cards):
        dst.append(cards[i])
        i = i + 1


def reset_game():
    global move_count
    global last_action
    stock.clear()
    waste.clear()

    i = 0
    while i < len(foundations):
        foundations[i].clear()
        i = i + 1

    i = 0
    while i < len(tableau):
        tableau[i].clear()
        tableau_up[i] = 0
        i = i + 1

    deck = make_deck((deal_index * 7) % len(card_defs))

    column = 0
    while column < len(tableau):
        count = column + 1
        j = 0
        while j < count:
            tableau[column].append(take_first(deck))
            j = j + 1
        tableau_up[column] = 1
        column = column + 1

    while len(deck) > 0:
        stock.append(take_first(deck))

    move_count = 0
    last_action = "Deal " + str(deal_index + 1) + " ready."


def draw_card(zone_id, sprite_id, card_name, x, y, index):
    cardgame_view.set_card(sprite_id, card_asset(card_name), int(x), int(y), 0)
    register_sprite(sprite_id, zone_id, index)


def draw_top_card(zone_id, cards):
    zone = cardgame_view.get_zone_rect(zone_id)
    if not zone or len(cards) == 0:
        return
    x = zone['x'] + (zone['w'] - 72) / 2
    y = zone['y'] + (zone['h'] - 96) / 2
    top_index = len(cards) - 1
    draw_card(zone_id, zone_id + "_card", cards[top_index], x, y, top_index)


def draw_stock():
    if len(stock) == 0:
        cardgame_view.set_label("stock", "")
        return
    zone = cardgame_view.get_zone_rect("stock")
    if not zone:
        return
    x = zone['x'] + (zone['w'] - 72) / 2
    y = zone['y'] + (zone['h'] - 96) / 2
    draw_card("stock", "stock_card", "back9", x, y, len(stock) - 1)


def draw_waste():
    cardgame_view.set_label("waste", "")
    if len(waste) == 0:
        return
    draw_top_card("waste", waste)


def draw_foundations():
    i = 0
    while i < len(foundations):
        zone_id = foundation_names[i]
        if len(foundations[i]) == 0:
            cardgame_view.set_label(zone_id, "A")
        else:
            cardgame_view.set_label(zone_id, "")
            draw_top_card(zone_id, foundations[i])
        i = i + 1


def draw_tableau():
    i = 0
    while i < len(tableau):
        zone_id = tableau_names[i]
        zone = cardgame_view.get_zone_rect(zone_id)
        if zone:
            x = zone['x'] + (zone['w'] - 72) / 2
            y = zone['y']
            hidden = len(tableau[i]) - tableau_up[i]
            j = 0
            while j < len(tableau[i]):
                card_name = tableau[i][j]
                if j < hidden:
                    card_name = "back9"
                draw_card(zone_id, zone_id + "_" + str(j), card_name, x, y + (j * 28), j)
                j = j + 1
        cardgame_view.set_label(zone_id, "")
        i = i + 1


def refresh_ui():
    status = "Deal " + str(deal_index + 1) + "  Moves " + str(move_count)
    cardgame_view.begin_sprite_frame()
    cardgame_view.clear_sprites()
    clear_sprite_registry()

    cardgame_view.set_status(status)
    cardgame_view.set_label("label_title", solitaire_bridge.title_text())
    cardgame_view.set_label("status_line", last_action)
    cardgame_view.set_button("button_new", "Next deal", True)

    draw_stock()
    draw_waste()
    draw_foundations()
    draw_tableau()


def reveal_tableau_if_needed(column_index):
    if len(tableau[column_index]) == 0:
        tableau_up[column_index] = 0
        return
    if tableau_up[column_index] < 1:
        tableau_up[column_index] = 1


def recycle_waste():
    while len(waste) > 0:
        stock.append(take_last(waste))


def draw_from_stock():
    global move_count
    global last_action
    if len(stock) == 0:
        if len(waste) == 0:
            last_action = "No more cards to draw."
            return
        recycle_waste()
        last_action = "Recycled waste back to stock."
        move_count = move_count + 1
        return
    waste.append(take_last(stock))
    move_count = move_count + 1
    last_action = "Drew a card from stock."


def can_place_on_foundation(card_name, foundation_index):
    pile = foundations[foundation_index]
    if len(pile) == 0:
        return card_rank(card_name) == 1
    top_card = pile[len(pile) - 1]
    return card_suit(card_name) == card_suit(top_card) and card_rank(card_name) == card_rank(top_card) + 1


def can_place_on_tableau(card_name, tableau_index):
    pile = tableau[tableau_index]
    if len(pile) == 0:
        return card_rank(card_name) == 13
    top_card = pile[len(pile) - 1]
    return card_color(card_name) != card_color(top_card) and card_rank(card_name) == card_rank(top_card) - 1


def get_tableau_drag_cards(tableau_index, start_index):
    cards = []
    hidden = len(tableau[tableau_index]) - tableau_up[tableau_index]
    if start_index < hidden or start_index >= len(tableau[tableau_index]):
        return cards
    i = start_index
    while i < len(tableau[tableau_index]):
        cards.append(tableau[tableau_index][i])
        i = i + 1
    return cards


def remove_tableau_drag_cards(tableau_index, start_index):
    cards = get_tableau_drag_cards(tableau_index, start_index)
    while len(tableau[tableau_index]) > start_index:
        tableau[tableau_index].remove(tableau[tableau_index][start_index])
    tableau_up[tableau_index] = tableau_up[tableau_index] - len(cards)
    reveal_tableau_if_needed(tableau_index)
    return cards


def get_drag_cards(source_zone, source_index):
    waste_index = zone_index(["waste"], source_zone)
    if waste_index >= 0:
        if len(waste) == 0 or source_index != len(waste) - 1:
            return []
        return [waste[len(waste) - 1]]

    foundation_index = zone_index(foundation_names, source_zone)
    if foundation_index >= 0:
        if len(foundations[foundation_index]) == 0 or source_index != len(foundations[foundation_index]) - 1:
            return []
        return [foundations[foundation_index][len(foundations[foundation_index]) - 1]]

    tableau_index = zone_index(tableau_names, source_zone)
    if tableau_index >= 0:
        return get_tableau_drag_cards(tableau_index, source_index)

    return []


def remove_drag_cards(source_zone, source_index):
    if source_zone == "waste":
        return [take_last(waste)]

    foundation_index = zone_index(foundation_names, source_zone)
    if foundation_index >= 0:
        return [take_last(foundations[foundation_index])]

    tableau_index = zone_index(tableau_names, source_zone)
    if tableau_index >= 0:
        return remove_tableau_drag_cards(tableau_index, source_index)

    return []


def try_move_cards(source_zone, source_index, target_zone):
    global move_count
    cards = get_drag_cards(source_zone, source_index)
    if len(cards) == 0:
        return False
    if source_zone == target_zone:
        return False

    foundation_index = zone_index(foundation_names, target_zone)
    if foundation_index >= 0:
        if len(cards) != 1:
            return False
        if not can_place_on_foundation(cards[0], foundation_index):
            return False
        moved = remove_drag_cards(source_zone, source_index)
        foundations[foundation_index].append(moved[0])
        move_count = move_count + 1
        return True

    tableau_index = zone_index(tableau_names, target_zone)
    if tableau_index >= 0:
        if not can_place_on_tableau(cards[0], tableau_index):
            return False
        moved = remove_drag_cards(source_zone, source_index)
        append_cards(tableau[tableau_index], moved)
        tableau_up[tableau_index] = tableau_up[tableau_index] + len(moved)
        move_count = move_count + 1
        return True

    return False


def try_auto_flip(card_id):
    global last_action
    sprite = lookup_sprite(card_id)
    source_zone = sprite[0]
    source_index = sprite[1]
    tableau_index = zone_index(tableau_names, source_zone)
    if tableau_index < 0:
        return False
    hidden = len(tableau[tableau_index]) - tableau_up[tableau_index]
    if hidden <= 0:
        return False
    if source_index != hidden - 1:
        return False
    tableau_up[tableau_index] = tableau_up[tableau_index] + 1
    last_action = "Turned over a card in " + source_zone + "."
    return True


def start():
    build_card_defs()
    reset_game()
    refresh_ui()


def on_button(button_id):
    global deal_index
    global last_action
    if button_id != "button_new":
        return
    deal_index = (deal_index + 1) % 8
    reset_game()
    last_action = "Loaded deal " + str(deal_index + 1) + "."
    refresh_ui()


def on_click(card_id):
    global last_action
    sprite = lookup_sprite(card_id)
    source_zone = sprite[0]
    if source_zone == "stock":
        draw_from_stock()
    elif not try_auto_flip(card_id):
        last_action = "Clicked " + str(card_id) + "."
    cardgame_view.log(last_action)
    refresh_ui()


def on_drag(card_id, zone_id):
    global last_action
    sprite = lookup_sprite(card_id)
    source_zone = sprite[0]
    source_index = sprite[1]
    if source_zone == "" or zone_id == "":
        last_action = "Drop target rejected."
        cardgame_view.log(last_action)
        refresh_ui()
        return

    if try_move_cards(source_zone, source_index, zone_id):
        last_action = "Moved " + str(card_id) + " to " + str(zone_id) + "."
    else:
        last_action = "Illegal move to " + str(zone_id) + "."
    cardgame_view.log(last_action)
    refresh_ui()
