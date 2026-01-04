#!/usr/bin/env python3
"""Analyze average color of an image."""

import sys
from PIL import Image
import numpy as np

def analyze_image(path):
    """Calculate average color of image."""
    try:
        img = Image.open(path)
        img_array = np.array(img)

        # Get image dimensions and channels
        h, w = img_array.shape[:2]
        channels = img_array.shape[2] if len(img_array.shape) > 2 else 1

        print(f"Image: {w}x{h}, {channels} channels")

        # Calculate average per channel
        if channels >= 3:
            avg_r = np.mean(img_array[:, :, 0])
            avg_g = np.mean(img_array[:, :, 1])
            avg_b = np.mean(img_array[:, :, 2])

            if channels == 4:
                avg_a = np.mean(img_array[:, :, 3])
                print(f"Average color: R={avg_r:.2f}, G={avg_g:.2f}, B={avg_b:.2f}, A={avg_a:.2f}")
            else:
                print(f"Average color: R={avg_r:.2f}, G={avg_g:.2f}, B={avg_b:.2f}")

            # Overall brightness (0-255)
            brightness = (avg_r + avg_g + avg_b) / 3
            print(f"Brightness: {brightness:.2f}/255 ({brightness/255*100:.1f}%)")

        else:
            avg = np.mean(img_array)
            print(f"Average grayscale: {avg:.2f}/255")

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1

    return 0

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: analyze_image.py <image_path>")
        sys.exit(1)

    sys.exit(analyze_image(sys.argv[1]))
