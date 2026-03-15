#!/usr/bin/env python3
import json
import sys
import time

from selenium import webdriver
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.by import By
from selenium.webdriver.firefox.options import Options


def fail(msg):
    print(msg, file=sys.stderr)
    raise SystemExit(1)


def main():
    url = sys.argv[1] if len(sys.argv) > 1 else "http://127.0.0.1:9247/"
    screenshot = sys.argv[2] if len(sys.argv) > 2 else ""

    opts = Options()
    opts.add_argument("-headless")
    driver = webdriver.Firefox(options=opts)
    driver.set_window_size(1400, 1000)
    try:
        driver.get(url)

        deadline = time.time() + 30.0
        ready = None
        while time.time() < deadline:
            ready = driver.execute_script(
                """
                return {
                  runtime: document.body.dataset.runtime || '',
                  sprite_count: +(document.body.dataset.spriteCount || 0),
                };
                """
            )
            if ready["runtime"] == "running" and ready["sprite_count"] >= 52:
                break
            time.sleep(0.25)

        if not ready or ready["runtime"] != "running":
            fail("browser drag regression: runtime did not reach running state")

        hand_info = driver.execute_script(
            """
            const api = window.ScriptWebHostRuntime;
            if (!api)
              return { error: 'runtime api missing' };
            const hand = api.getZoneRect('hand_self');
            if (!hand)
              return { error: 'hand_self zone missing' };
            const sprites = Array.from(document.querySelectorAll('#sprite-layer img.sprite'));
            const index = sprites.findIndex(s => {
              const left = parseFloat(s.style.left) || 0;
              const top = parseFloat(s.style.top) || 0;
              const cx = left + 36;
              const cy = top + 48;
              return cx >= hand.x && cx <= hand.x + hand.w && cy >= hand.y && cy <= hand.y + hand.h;
            });
            if (index < 0)
              return { error: 'no hand sprite found' };
            return { index, id: sprites[index].dataset.id || '' };
            """
        )
        if "error" in hand_info:
            fail(f"browser drag regression: {hand_info['error']}")

        sprite = driver.find_elements(By.CSS_SELECTOR, "#sprite-layer img.sprite")[hand_info["index"]]
        before_map = driver.execute_script(
            """
            const out = {};
            for (const s of document.querySelectorAll('#sprite-layer img.sprite')) {
              const id = s.dataset.id || '';
              out[id] = {
                left: parseFloat(s.style.left) || 0,
                top: parseFloat(s.style.top) || 0,
                className: s.className,
              };
            }
            return out;
            """
        )

        ActionChains(driver).move_to_element_with_offset(sprite, 12, 12).click_and_hold().move_by_offset(48, -32).pause(0.2).perform()

        during_info = driver.execute_script(
            """
            const api = window.ScriptWebHostRuntime;
            const dragId = api && api.runtime.drag ? api.runtime.drag.id : '';
            const s = dragId ? document.querySelector(`#sprite-layer img.sprite[data-id="${dragId}"]`) : null;
            return s ? {
              id: dragId,
              left: parseFloat(s.style.left)||0,
              top: parseFloat(s.style.top)||0,
              className: s.className,
              drag_active: !!api.runtime.drag
            } : null;
            """
        )
        if not during_info:
            fail("browser drag regression: target sprite missing during drag")
        drag_id = during_info["id"]
        before = before_map.get(drag_id)
        if not before:
            fail(f"browser drag regression: missing pre-drag snapshot for {drag_id}")
        during = {
            "left": during_info["left"],
            "top": during_info["top"],
            "className": during_info["className"],
            "drag_active": during_info["drag_active"],
        }

        ActionChains(driver).release().perform()
        time.sleep(0.2)

        after = driver.execute_script(
            """
            const id = arguments[0];
            const api = window.ScriptWebHostRuntime;
            const s = document.querySelector(`#sprite-layer img.sprite[data-id="${id}"]`);
            return s ? {left: parseFloat(s.style.left)||0, top: parseFloat(s.style.top)||0, className: s.className, drag_active: !!api.runtime.drag} : null;
            """,
            drag_id,
        )
        if not after:
            fail("browser drag regression: target sprite missing after drag")
        result = {"before": before, "during": during, "after": after}

        before = result["before"]
        during = result["during"]
        after = result["after"]

        moved_x = abs(during["left"] - before["left"])
        moved_y = abs(during["top"] - before["top"])
        if moved_x < 8 and moved_y < 8:
            fail(f"browser drag regression: sprite did not move during drag: {result}")
        if not during["drag_active"]:
            fail(f"browser drag regression: runtime drag state was not active: {result}")
        if "dragging" not in during["className"]:
            fail(f"browser drag regression: sprite missing dragging class during drag: {result}")
        if after["drag_active"]:
            fail(f"browser drag regression: runtime drag state stayed active after drop: {result}")

        if screenshot:
            driver.save_screenshot(screenshot)

        print(json.dumps(result, indent=2, sort_keys=True))
    finally:
        driver.quit()


if __name__ == "__main__":
    main()
