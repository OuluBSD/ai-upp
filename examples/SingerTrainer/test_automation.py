# SingerTrainer Automation Script

def on_frame_update(dt):
    # This is called 60 times per second
    pass

def on_start_training():
    print("Training started from script!")

# Demonstrate using the exposed VocalMode constants
print("Vocal modes available:")
print("FRY:", FRY)
print("MODAL:", MODAL)

# Simulate clicking a button that we will register
simulate_click("start_button")
