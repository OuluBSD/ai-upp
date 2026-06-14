folded_seats = set()
folded_seats.add(1)
folded_seats.add(2)

cur_in_game_set = set()
cur_in_game_set.add(2)
cur_in_game_set.add(3)

reactivated = folded_seats & cur_in_game_set

assert len(reactivated) == 1
assert 2 in reactivated
