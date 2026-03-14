#!/usr/bin/env python3
import json
import sys
import time

from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.firefox.options import Options


def fail(msg):
    print(msg, file=sys.stderr)
    raise SystemExit(1)


def rect_contains(rect, x, y):
    return rect["x"] <= x <= rect["x"] + rect["w"] and rect["y"] <= y <= rect["y"] + rect["h"]


def main():
    url = sys.argv[1] if len(sys.argv) > 1 else "http://127.0.0.1:9247/?autoplay=1"
    screenshot = sys.argv[2] if len(sys.argv) > 2 else ""

    opts = Options()
    opts.add_argument("-headless")
    driver = webdriver.Firefox(options=opts)
    driver.set_window_size(1400, 1000)
    try:
        driver.get(url)

        deadline = time.time() + 30.0
        stable = None
        while time.time() < deadline:
            stable = driver.execute_script(
                """
                const status = document.getElementById('status')?.textContent || '';
                const api = window.ScriptWebHostRuntime;
                const zones = {
                  trick_bottom: api ? api.getZoneRect('trick_bottom') : null,
                  trick_left: api ? api.getZoneRect('trick_left') : null,
                  trick_top: api ? api.getZoneRect('trick_top') : null,
                  trick_right: api ? api.getZoneRect('trick_right') : null,
                };
                const sprites = api ? api.sprites() : [];
                const trickSprites = sprites
                  .filter(s => s.layer === 'trick')
                  .map(s => ({
                    id: s.id,
                    left: parseFloat(String(s.left || '0').replace('px', '')) || 0,
                    top: parseFloat(String(s.top || '0').replace('px', '')) || 0,
                  }));
                return {
                  runtime: document.body.dataset.runtime || '',
                  status,
                  zones,
                  trick_sprites: trickSprites,
                };
                """
            )
            all_zones = stable["zones"]
            if stable["runtime"] == "running" and all(all_zones.values()) and stable["trick_sprites"]:
                break
            time.sleep(0.4)

        if not stable:
            fail("browser regression: no runtime state received")
        if stable["runtime"] != "running":
            fail("browser regression: runtime did not reach running state")

        for zone_id, rect in stable["zones"].items():
            if not rect or rect["w"] <= 0 or rect["h"] <= 0:
                fail(f"browser regression: invalid trick zone {zone_id}: {rect}")

        if not stable["trick_sprites"]:
            fail("browser regression: autoplay did not reach a visible trick sprite")

        zone_rects = list(stable["zones"].values())
        for sprite in stable["trick_sprites"]:
            if sprite["left"] <= 2 and sprite["top"] <= 2:
                fail(f"browser regression: trick sprite stuck near origin: {sprite}")
            center_x = sprite["left"] + 36
            center_y = sprite["top"] + 48
            if not any(rect_contains(rect, center_x, center_y) for rect in zone_rects):
                fail(f"browser regression: trick sprite outside trick zones: {sprite}")

        if screenshot:
            driver.save_screenshot(screenshot)

        print(json.dumps(stable, indent=2, sort_keys=True))
    finally:
        driver.quit()


if __name__ == "__main__":
    main()
