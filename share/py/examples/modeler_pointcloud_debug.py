# Example: pointcloud debug actions from ModelerApp scripting.
# Attach this script to a scene/object/directory script component.

def on_start():
    # Build synthetic inputs.
    modeler.debug_generate_pointcloud()
    modeler.debug_simulate_observation()
    modeler.debug_run_localization()

    # Controller pipeline.
    modeler.debug_simulate_controller_observations()
    modeler.debug_run_controller_localization()

    # Full sequence (optional).
    # modeler.debug_run_full_synthetic()


def on_frame(dt):
    # Per-frame hook (optional).
    pass
