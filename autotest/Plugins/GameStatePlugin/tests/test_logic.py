def test_score():
    score = get_game_score()
    if score != 42:
        return 1
    return 0

test_score()
