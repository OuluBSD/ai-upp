#!/bin/bash

# Task 3: The "Non-Stop" X11 Integration Loop

# 1. Environment Setup
export DISPLAY=:0
echo "Environment: DISPLAY set to $DISPLAY"

# 2. Cleanup
if [ -f capture.png ]; then
    echo "Cleanup: Removing old capture.png..."
    rm capture.png
fi

# 3. Build
echo "Step 1: Building OpenVRDemoRoom..."
python3 script/build.py -j14 upptst/OpenVRDemoRoom
BUILD_STATUS=$?

if [ $BUILD_STATUS -ne 0 ]; then
    echo "FAILURE: Build failed with exit code $BUILD_STATUS"
    exit $BUILD_STATUS
fi
echo "Build: SUCCESS"

# 4. Run (Emulation Mode)
# Mapping "--emulatedevice" requirement to the actual positional arguments:
# Test 0, Method 0, Emulate=1 (true), Verbose=0 (false)
echo "Step 2: Running OpenVRDemoRoom (Emulation Mode)..."
timeout 30 ./bin/OpenVRDemoRoom 0 0 1 0
RUN_STATUS=$?

if [ $RUN_STATUS -ne 0 ]; then
    if [ -s capture.png ]; then
        echo "WARNING: Application run exited with code $RUN_STATUS, but 'capture.png' exists. Proceeding..."
    else
        echo "FAILURE: Application run failed with exit code $RUN_STATUS and no screenshot was captured."
        exit $RUN_STATUS
    fi
fi

if [ ! -s capture.png ]; then
    echo "FAILURE: Application exited successfully but 'capture.png' is missing or empty."
    exit 1
fi
echo "Run: SUCCESS (Screenshot captured)"

# 5. Validate (Vision API)
echo "Step 3: Validating Visual Output..."
python3 script/vision.py capture.png \
    --prompt "Analyze this image and determine if the following elements are present: 1. A 3D environment/skybox with castle ruins. 2. Vegetation (trees/plants) in the environment. 3. A silver or metallic floor/ground. 4. White overlay text OR a distinct white/red cube marker." \
    --require ruins vegetation silver_floor white_text_or_cube

VISION_STATUS=$?

if [ $VISION_STATUS -ne 0 ]; then
    echo "FAILURE: Vision validation failed."
    exit $VISION_STATUS
fi

echo "---------------------------------------------------"
echo "SUCCESS: Code-to-Vision pipeline verified."
echo "---------------------------------------------------"
exit 0
