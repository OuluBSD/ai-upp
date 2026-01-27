# SingerTrainer Initialization & Exercise Definition
print("SingerTrainer Scripting Engine Online")

def on_frame_update(dt):
    pass

def on_start_training():
    print("Action: Start Training triggered")
    # Define the "Lörpötys Trio" exercise: Subharmonic -> Modal -> Distortion
    plotter_clear()
    plotter_add_node(0.0, SUBHARMONIC, 110.0) # Low Subharmonic
    plotter_add_node(2.0, MODAL, 220.0)       # Mid Modal
    plotter_add_node(5.0, DISTORTION, 440.0)  # High Distortion
    plotter_start()

# Initialize the exercise map on load
on_start_training()