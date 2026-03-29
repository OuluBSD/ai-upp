# Engine Function Catalog

**Analysis Date**: 2026-03-29  
**Files Analyzed**: Game.esc (1563 lines), CarverTest.esc, Demo.esc, C8_Intro.esc, C8_Part1.esc (1726 lines), C8_Part2.esc (1453 lines)

---

## Summary

- **Total unique functions**: 47
- **Most used**: `say_line` (50+ times), `break_time` (40+ times), `put_at` (30+ times)
- **Files with most calls**: Game.esc (primary game logic), C8_Part1.esc/C8_Part2.esc (PICO-8 ports)

---

## Core Functions

### Room Management
- **`change_room(room, fade_type)`** - Switch to a different room with optional fade transition
  - Used: Game.esc (8x), Demo.esc (2x), C8_Intro.esc (2x), C8_Part1.esc (4x), C8_Part2.esc (3x)
  - Parameters: `room` (room object), `fade_type` (1 = iris fade, -1 = fade out)
  - Example: `change_room(:rm_outside, 1)`

- **`reload(offset1, offset2, size)`** - Reload graphics data from memory
  - Used: Game.esc (2x), C8_Intro.esc (2x)
  - Parameters: Memory offsets and size for graphics loading

### Camera Control
- **`camera_follow(actor)`** - Make camera follow a specific actor
  - Used: Game.esc (10x), Demo.esc (3x), C8_Part1.esc (2x), C8_Part2.esc (2x)
  - Parameters: `actor` (actor object reference)
  - Example: `camera_follow(:selected_actor)`

- **`camera_at(position)`** - Set camera to a specific position
  - Used: Game.esc (3x), C8_Part2.esc (2x)
  - Parameters: `position` (x coordinate or actor)
  - Example: `camera_at(144)` or `camera_at(ben_actor)`

- **`camera_pan_to(target)`** - Pan camera smoothly to target position
  - Used: Game.esc (2x), C8_Part2.esc (1x)
  - Parameters: `target` (actor or coordinate)
  - Example: `camera_pan_to(:selected_actor)`

- **`camera_pan_to_coord(x, y)`** - Pan camera to specific coordinates
  - Used: Game.esc (1x)
  - Parameters: `x`, `y` (coordinates)
  - Example: `camera_pan_to_coord(212,60)`

- **`wait_for_camera()`** - Wait for camera movement to complete
  - Used: Game.esc (3x), C8_Part2.esc (1x)
  - No parameters

### Actor Movement & Positioning
- **`put_at(object, x, y, room)`** - Place an object/actor at specific coordinates
  - Used: Game.esc (15x), Demo.esc (3x), C8_Part1.esc (8x), C8_Part2.esc (6x)
  - Parameters: `object` (object/actor reference), `x`, `y` (coordinates), `room` (optional room)
  - Example: `put_at(:selected_actor, 30, 55, :rm_outside)`

- **`walk_to(actor, x, y)`** - Make actor walk to coordinates
  - Used: Game.esc (6x), C8_Part1.esc (2x), C8_Part2.esc (2x)
  - Parameters: `actor`, `x`, `y` (destination)
  - Example: `walk_to(:selected_actor, :purp_tentacle.x-8, :purp_tentacle.y)`

- **`stop_actor(actor)`** - Stop actor movement
  - Used: Game.esc (2x), C8_Part1.esc (1x), C8_Part2.esc (1x)
  - Parameters: `actor`
  - Example: `stop_actor(:selected_actor)`

- **`wait_for_actor(actor)`** - Wait for actor to finish walking
  - Used: Game.esc (1x), C8_Part2.esc (1x)
  - Parameters: `actor` (optional, defaults to selected_actor)

- **`proximity(actor1, actor2)`** - Get distance between two actors/objects
  - Used: Game.esc (1x), C8_Part1.esc (1x)
  - Parameters: Two actors/objects
  - Returns: Distance value
  - Example: `proximity(:main_actor, obj_back_door)`

### Animation
- **`do_anim(object, animation, params)`** - Play an animation on an object/actor
  - Used: Game.esc (8x), Demo.esc (3x), C8_Part2.esc (1x)
  - Parameters: `object`, `animation` (animation name or array), `params` (optional)
  - Example: `do_anim(:selected_actor, "face_towards", "face_front")`
  - Special: Can take `"anim_face"` with target object for automatic facing

---

## UI Functions

### Dialog System
- **`say_line(text)`** / **`say_line(actor, text, wait, duration)`** - Display dialog line
  - Used: Game.esc (25x), Demo.esc (10x), C8_Part1.esc (8x), C8_Part2.esc (12x)
  - Parameters: `text` (string with `:` for line breaks), optional `actor`, `wait`, `duration`
  - Example: `say_line("it's an old bucket")` or `say_line_actor(me, "what do you want?", 0, 0)`

- **`say_line_actor(actor, text, wait, duration)`** - Display dialog with specific actor
  - Used: Game.esc (8x), C8_Part2.esc (4x)
  - Parameters: `actor`, `text`, `wait` (0 = immediate), `duration`
  - Example: `say_line_actor(:purp_tentacle, "stop!:come back here!", true, 0)`

- **`stop_talking()`** - Stop current dialog
  - Used: Game.esc (2x), Demo.esc (1x)
  - No parameters

- **`dialog_set(options_array)`** - Set up dialog options
  - Used: Game.esc (2x), Demo.esc (1x), C8_Part2.esc (1x)
  - Parameters: Array of dialog option strings
  - Example: `dialog_set({ "where am i?", "who are you?", "nevermind" })`

- **`dialog_start(color, highlight_color)`** - Show dialog options
  - Used: Game.esc (2x), Demo.esc (1x), C8_Part2.esc (1x)
  - Parameters: `color`, `highlight_color`
  - Example: `dialog_start(:selected_actor.col, 7)`

- **`dialog_hide()`** - Hide dialog options
  - Used: Game.esc (2x), Demo.esc (1x), C8_Part2.esc (1x)
  - No parameters

- **`dialog_clear()`** - Clear dialog options
  - Used: Game.esc (2x), Demo.esc (1x), C8_Part2.esc (1x)
  - No parameters

- **`dialog_end()`** - End dialog mode
  - Used: Game.esc (1x), C8_Part2.esc (1x)
  - No parameters

### Text Display
- **`print_line(text, x, y, color, shadow, centered, width, outline)`** - Print text at position
  - Used: Game.esc (5x), C8_Intro.esc (2x), C8_Part2.esc (2x)
  - Parameters: `text`, `x`, `y`, `color`, `shadow`, `centered`, `width`, `outline`
  - Example: `print_line("congratulations!:you've completed the game!", 64, 45, 8, 1, true, 8, false)`

---

## Game Logic Functions

### Cutscene System
- **`cutscene(type, setup_fn, cleanup_fn)`** - Start a cutscene
  - Used: Game.esc (10x), Demo.esc (3x), C8_Part1.esc (2x), C8_Part2.esc (4x)
  - Parameters: 
    - `type` (1 = no verbs, 2 = quick-cut, 3 = no verbs & no follow)
    - `setup_fn` (lambda/function to execute)
    - `cleanup_fn` (optional cleanup function, can be `void`)
  - Example:
    ```esc
    cutscene(
        1, // no verbs
        @(me) {
            say_line("dialog here");
        },
        void
    )
    ```

### Script Control
- **`start_script(script_fn, background)`** - Start a script
  - Used: Game.esc (6x), Demo.esc (2x), C8_Part1.esc (2x)
  - Parameters: `script_fn` (function reference), `background` (true = continues on room change)
  - Example: `start_script(me.scripts.anim_clock, true)`

- **`stop_script(script_fn)`** - Stop a running script
  - Used: Game.esc (6x), Demo.esc (1x), C8_Part1.esc (1x)
  - Parameters: `script_fn` (function reference)
  - Example: `stop_script(me.scripts.anim_clock)`

- **`script_running(script_fn)`** - Check if script is running
  - Used: Game.esc (1x), Demo.esc (1x)
  - Parameters: `script_fn`
  - Returns: Boolean
  - Example: `script_running(room_curr.scripts.spin_top)`

- **`break_time(frames)`** - Wait for specified number of frames
  - Used: Game.esc (20x), Demo.esc (8x), C8_Part1.esc (6x), C8_Part2.esc (10x)
  - Parameters: `frames` (can be `void` for 1 frame)
  - Example: `break_time(10)` or `break_time(void)`

### Object Interaction
- **`pickup_obj(object, actor)`** - Add object to actor's inventory
  - Used: Game.esc (4x), Demo.esc (2x), C8_Part1.esc (2x), C8_Part2.esc (3x)
  - Parameters: `object`, `actor` (optional, defaults to selected_actor)
  - Example: `pickup_obj(me)` or `pickup_obj(obj_bucket, :main_actor)`

- **`open_door(door, paired_door)`** - Open a door
  - Used: Game.esc (4x), C8_Part2.esc (1x)
  - Parameters: `door`, `paired_door` (optional, for double doors)
  - Example: `open_door(me, obj_front_door_inside)`

- **`close_door(door, paired_door)`** - Close a door
  - Used: Game.esc (3x), C8_Part2.esc (1x)
  - Parameters: `door`, `paired_door` (optional)
  - Example: `close_door(door1, door2)`

- **`come_out_door(door, target_door, fade)`** - Move actor through door to another room
  - Used: Game.esc (5x), C8_Part1.esc (4x), C8_Part2.esc (2x)
  - Parameters: `door`, `target_door`, `fade` (optional fade type)
  - Example: `come_out_door(me, obj_landing_exit_hall)`

### Inventory & State
- **`has_flag(classes, flag)`** - Check if object has a specific class/flag
  - Used: Game.esc (1x), C8_Part2.esc (1x)
  - Parameters: `classes` (array), `flag` (string)
  - Returns: Boolean
  - Example: `has_flag(bu.classes, "class_actor")`

### Game State
- **`reset()`** - Reset game state
  - Used: Game.esc (1x), Demo.esc (1x)
  - No parameters

- **`reset_ui()`** - Reset UI colors and settings
  - Used: Game.esc (2x), Demo.esc (1x)
  - No parameters

- **`reload()`** - Reload game (restore defaults)
  - Used: Game.esc (1x)
  - No parameters

---

## Drawing Functions

### Map & Graphics
- **`map(src_x, src_y, dest_x, dest_y, width, height, flags)`** - Draw map tiles
  - Used: Game.esc (6x), C8_Part1.esc (1x)
  - Parameters: Source coordinates, destination coordinates, dimensions, optional flags
  - Example: `map(56,23, 136,60, 6,1)` or `map(58,16, 40,28, 6,4, 0x80)`

- **`set_trans_col(color, enable)`** - Set transparent color
  - Used: Game.esc (4x), C8_Part1.esc (2x), C8_Part2.esc (1x)
  - Parameters: `color` (color index), `enable` (boolean)
  - Example: `set_trans_col(8, true)`

- **`rectfill(x1, y1, x2, y2, color)`** - Draw filled rectangle
  - Used: Game.esc (1x), C8_Part2.esc (1x)
  - Parameters: Coordinates and color
  - Example: `rectfill(35, 20, 43, 56, 0)`

- **`line(x1, y1, x2, y2, color)`** - Draw line
  - Used: Game.esc (1x), C8_Part1.esc (2x), C8_Part2.esc (1x)
  - Parameters: Start/end coordinates and color
  - Example: `line(me.x, me.y, obj_pendulum.bobx, obj_pendulum.boby, 9)`

- **`circfill(x, y, radius, color)`** - Draw filled circle
  - Used: Game.esc (1x)
  - Parameters: Center coordinates, radius, color
  - Example: `circfill(obj_pendulum.bobx, obj_pendulum.boby, 2)`

- **`spr(sprite_num, x, y, w, h, flip_x, flip_y)`** - Draw sprite
  - Used: C8_Part1.esc (2x), C8_Part2.esc (2x)
  - Parameters: Sprite number, position, dimensions, flip flags
  - Example: `spr(15, me.x, me.y+16, 1, 1)`

- **`sspr(src_x, src_y, src_w, src_h, dest_x, dest_y, dest_w, dest_h, flip_x, flip_y)`** - Draw sprite region
  - Used: C8_Intro.esc (1x)
  - Parameters: Source and destination rectangles, flip flags

### Special Effects
- **`shake(enable)`** - Shake the screen
  - Used: Game.esc (2x), C8_Part2.esc (2x)
  - Parameters: `enable` (boolean)
  - Example: `shake(true)`

- **`fades(direction, speed)`** - Fade screen in/out
  - Used: Game.esc (3x), C8_Intro.esc (2x), C8_Part2.esc (2x)
  - Parameters: `direction` (1 = out, -1 = in), `speed`
  - Example: `fades(1, 1)` // fade out

- **`ie(light_level)`** - Apply lighting effect (internal function)
  - Used: C8_Part2.esc (1x)
  - Parameters: `light_level` (0.0 to 1.0)

---

## Audio Functions

- **`sfx0()`** / **`sfx1()`** - Play sound effects
  - Used: Game.esc (2x)
  - No parameters

- **`music(track_num)`** - Play music track
  - Used: C8_Part1.esc (1x), C8_Part2.esc (2x)
  - Parameters: `track_num`
  - Example: `music(1)` or `music(6)`

---

## Object Methods (Member Functions)

These are called using `object.method()` syntax:

### Room Methods
- **`room.scripts.script_name`** - Access room script
  - Used: Game.esc (6x), Demo.esc (2x)
  - Example: `:rm_landing.scripts.door_teleport(me, obj_landing_door_room3)`

### Object Properties (read/write)
- **`object.state`** - Get/set object state
  - Used extensively throughout all files
  - Example: `me.state = "state_here"` or `obj_bucket.state = "state_open"`

- **`object.x`**, **`object.y`**, **`object.z`** - Position properties
  - Used extensively
  - Example: `obj_pendulum.bobx = obj_pendulum.x + sin(angle) * 31`

- **`object.name`** - Object name property
  - Used: Game.esc (3x)
  - Example: `me.name = "secret lock"`

- **`object.lighting`** - Lighting level for room/object
  - Used: Game.esc (5x), C8_Part1.esc (1x)
  - Example: `:rm_library.lighting = 0.25`

- **`object.flip_x`** / **`object.flip`** - Flip sprite horizontally
  - Used: Game.esc (2x), C8_Part2.esc (2x)

- **`object.owner`** - Object ownership (inventory)
  - Used: Game.esc (3x), C8_Part1.esc (1x), C8_Part2.esc (2x)
  - Example: `me.owner = :purp_tentacle`

- **`object.in_room`** - Current room of object/actor
  - Used: Game.esc (3x), Demo.esc (1x), C8_Part2.esc (2x)

### Actor Properties
- **`actor.face_dir`** - Facing direction
  - Used: Game.esc (3x), C8_Part2.esc (2x)
  - Values: `"face_front"`, `"face_left"`, `"face_back"`, `"face_right"`

- **`actor.walk_speed`** - Walking speed multiplier
  - Used: C8_Part1.esc (2x)

- **`actor.scale`** - Actor scale factor
  - Used: C8_Part1.esc (2x)

- **`actor.inventory`** / **`actor.bo`** - Actor's inventory (bag of objects)
  - Used: Game.esc (2x), C8_Part2.esc (3x)

---

## Lambda/Callback Patterns

Many functions accept lambdas (anonymous functions) as parameters:

### Lambda Syntax
```esc
@(parameters) {
    // function body
}
```

### Common Lambda Usage

1. **Cutscene setup functions**:
   ```esc
   cutscene(
       1,
       @(me) {
           say_line("dialog");
           walk_to(actor, x, y);
       },
       void
   )
   ```

2. **Room enter/exit handlers**:
   ```esc
   "enter": @(&me) {
       // code here
   }
   ```

3. **Object verb handlers**:
   ```esc
   "verbs": {
       "use": @(&me) {
           // use logic
       }
   }
   ```

4. **Script definitions**:
   ```esc
   "scripts": {
       anim_clock: @() {
           // animation loop
           while (true) {
               // code
               break_time(void);
           }
       }
   }
   ```

5. **Draw handlers**:
   ```esc
   "draw": @(&me) {
       // custom drawing code
       map(...);
   }
   ```

---

## Special Patterns

### Object Definition Structure
```esc
:object_name = {
    "name": "display name",
    "state": "current_state",
    "x": x_coord,
    "y": y_coord,
    "w": width,
    "h": height,
    "state_stateName": sprite_num,
    "classes": ["class_pickupable", "class_openable"],
    "verbs": {
        "use": @(&me) { /* handler */ }
    },
    "init": @(&me) { /* initialization */ },
    "draw": @(&me) { /* custom drawing */ }
}
```

### Room Definition Structure
```esc
:room_name = {
    "map": [tile_x, tile_y, tile_w, tile_h],
    "objects": [list, of, objects],
    "enter": @(&me) { /* enter handler */ },
    "exit": @(&me) { /* exit handler */ },
    "scripts": {
        script_name: @() { /* script code */ }
    }
}
```

### Actor Definition Structure
```esc
:actor_name = {
    "name": "actor name",
    "idle": [front, left, back, right],
    "talk": [frames...],
    "walk_anim_side": [frames...],
    "walk_anim_front": [frames...],
    "walk_anim_back": [frames...],
    "col": color,
    "trans_col": transparent_color,
    "walk_speed": speed,
    "frame_delay": delay,
    "classes": ["class_actor", "class_talkable"]
}
```

### Dialog Loop Pattern
```esc
while (true) {
    dialog_set({
        (condition ? "" : "option text"),
        "always shown"
    });
    dialog_start(color, highlight);
    
    while (!selected_sentence) { break_time(void); }
    
    dialog_hide();
    
    cutscene(1, @(me) {
        say_line(selected_sentence.msg);
        if (selected_sentence.num == 1) {
            // handle option 1
        }
    });
    
    dialog_clear();
}
```

---

## Function Categories by Usage Count

| Category | Functions | Total Calls |
|----------|-----------|-------------|
| Core (room/camera/actor) | 12 | ~80 |
| UI (dialog/text) | 9 | ~60 |
| Game Logic (cutscene/scripts) | 8 | ~70 |
| Drawing | 8 | ~20 |
| Audio | 2 | ~3 |
| Object Methods | 10+ | ~50 |

---

## Notes for Python Bindings

### High Priority (Most Used)
1. `change_room()` - Essential for navigation
2. `camera_follow()` / `camera_at()` - Camera control
3. `put_at()` - Object/actor positioning
4. `say_line()` / `say_line_actor()` - Dialog system
5. `cutscene()` - Cutscene system
6. `break_time()` - Timing/yielding
7. `walk_to()` - Actor movement
8. `pickup_obj()` - Inventory system
9. `start_script()` / `stop_script()` - Script control
10. `dialog_*()` functions - Dialog UI

### Medium Priority
- `do_anim()` - Animation system
- `open_door()` / `close_door()` / `come_out_door()` - Door system
- `print_line()` - Text display
- Drawing functions (`map()`, `set_trans_col()`, etc.)
- `shake()` / `fades()` - Effects

### Low Priority (Specialized)
- `proximity()` - Distance checking
- `reload()` - Graphics reloading
- `reset()` / `reset_ui()` - Game reset
- Audio functions (`sfx0()`, `music()`)

### Special Considerations
1. **Lambda support**: Python bindings must support passing Python functions as lambdas
2. **Object properties**: Need property get/set for `x`, `y`, `state`, etc.
3. **Reference handling**: Objects/rooms/actors passed by reference
4. **Void parameter**: `void` is used as a null parameter (different from `None`)
5. **String keys**: Many objects use string keys like `"state_open"`, `"face_front"`

---

## Files Analyzed

1. **Game.esc** (1563 lines) - Main Scumm-8 game, most comprehensive usage
2. **CarverTest.esc** - Minimal test file, room carving functions
3. **Demo.esc** (474 lines) - Simplified demo version
4. **C8_Intro.esc** (748 lines) - PICO-8 intro sequence
5. **C8_Part1.esc** (1726 lines) - PICO-8 game part 1
6. **C8_Part2.esc** (1453 lines) - PICO-8 game part 2

**Total lines analyzed**: ~6000 lines of ESC code
