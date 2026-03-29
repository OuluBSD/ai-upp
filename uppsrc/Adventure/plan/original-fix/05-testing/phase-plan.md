# Phase 05: Testing

## Overview

Test the Python conversion to ensure the game works correctly.

## Scope

- Unit tests for individual functions
- Integration tests for game flow
- Visual verification of game behavior

## Tasks

1. [ ] **001-test-bindings** - Test all Python bindings work
2. [ ] **002-test-game-init** - Test game initialization
3. [ ] **003-test-room-transitions** - Test changing rooms
4. [ ] **004-test-actor-movement** - Test actor movement and collision
5. [ ] **005-test-verbs** - Test all verb interactions
6. [ ] **006-test-inventory** - Test pickup and use mechanics
7. [ ] **007-test-cutscenes** - Test cutscene playback
8. [ ] **008-full-playthrough** - Play through entire game

## Test Approach

### Manual Testing

1. Build Adventure package: `script/build.py Adventure`
2. Run: `bin/Adventure`
3. Verify each game feature works

### Automated Testing (if feasible)

Create `test_adventure.py` with:
```python
def test_room_loading():
    # Test that rooms load correctly
    pass

def test_actor_movement():
    # Test actor can move and collide
    pass

def test_verb_interactions():
    # Test verb actions work
    pass
```

## Expected Output

- Test results documenting what works/doesn't work
- Bug list for any issues found
- Final verification that game is playable

## Acceptance Criteria

- [ ] Game starts without errors
- [ ] Player can move around
- [ ] Verbs work correctly
- [ ] Inventory system works
- [ ] Cutscenes play correctly
- [ ] Game can be completed (or reaches intended end state)

## Dependencies

- All previous phases complete
- Working Python integration

## Time Estimate

8-16 hours (depends on bugs found)
