import cardgame_view
import solitaire.bridge

asset_base = "../../../../share/imgs/cards/default/"

deal_index = 0

deals = [
    [
        ["clubs_ace"],
        ["back9", "diamonds_7"],
        ["back9", "back9", "hearts_jack"],
        ["back9", "back9", "back9", "spades_4"],
        ["back9", "back9", "back9", "back9", "clubs_9"],
        ["back9", "back9", "back9", "back9", "back9", "diamonds_king"],
        ["back9", "back9", "back9", "back9", "back9", "back9", "spades_ace"],
    ],
    [
        ["hearts_3"],
        ["back9", "clubs_10"],
        ["back9", "back9", "diamonds_queen"],
        ["back9", "back9", "back9", "hearts_8"],
        ["back9", "back9", "back9", "back9", "spades_jack"],
        ["back9", "back9", "back9", "back9", "back9", "clubs_4"],
        ["back9", "back9", "back9", "back9", "back9", "back9", "diamonds_ace"],
    ],
]

foundations = ["", "", "", ""]
waste_cards = ["spades_king", "hearts_2", "clubs_5"]

def card_asset(card_name):
    return asset_base + card_name + ".png"

def draw_stack(zone_id, cards, vertical_step):
    zone = cardgame_view.get_zone_rect(zone_id)
    if not zone:
        return
    x = zone['x'] + (zone['w'] - 72) / 2
    y = zone['y']
    i = 0
    while i < len(cards):
        card_name = cards[i]
        sprite_id = zone_id + "_" + str(i)
        cardgame_view.set_card(sprite_id, card_asset(card_name), int(x), int(y + (i * vertical_step)), 0)
        i = i + 1

def draw_top_card(zone_id, card_name):
    zone = cardgame_view.get_zone_rect(zone_id)
    if not zone or card_name == "":
        return
    x = zone['x'] + (zone['w'] - 72) / 2
    y = zone['y'] + (zone['h'] - 96) / 2
    cardgame_view.set_card(zone_id + "_card", card_asset(card_name), int(x), int(y), 0)

def refresh_ui():
    global deal_index
    cardgame_view.begin_sprite_frame()
    cardgame_view.clear_sprites()
    cardgame_view.set_status(solitaire.bridge.deal_label(deal_index))
    cardgame_view.set_label("label_title", solitaire.bridge.title_text())
    cardgame_view.set_label("status_line", solitaire.bridge.status_text())
    cardgame_view.set_button("button_new", "Next deal", True)

    draw_top_card("stock", "back9")
    draw_top_card("waste", waste_cards[deal_index % len(waste_cards)])

    i = 0
    while i < 4:
        foundation_card = foundations[i]
        if foundation_card == "":
            cardgame_view.set_label("foundation_" + str(i + 1), "A")
        else:
            cardgame_view.set_label("foundation_" + str(i + 1), "")
            draw_top_card("foundation_" + str(i + 1), foundation_card)
        i = i + 1

    columns = deals[deal_index]
    i = 0
    while i < len(columns):
        cardgame_view.set_label("tableau_" + str(i + 1), "")
        draw_stack("tableau_" + str(i + 1), columns[i], 28)
        i = i + 1

def start():
    refresh_ui()

def on_button(button_id):
    global deal_index
    if button_id != "button_new":
        return
    deal_index = (deal_index + 1) % len(deals)
    refresh_ui()

def on_click(card_id):
    cardgame_view.log("Clicked: " + str(card_id))
