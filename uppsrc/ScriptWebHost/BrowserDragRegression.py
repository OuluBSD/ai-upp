#!/usr/bin/env python3
import json
import sys
import time

from selenium import webdriver
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

        result = driver.execute_script(
            """
            const api = window.ScriptWebHostRuntime;
            if (!api)
              return { error: 'runtime api missing' };
            const hand = api.getZoneRect('hand_self');
            if (!hand)
              return { error: 'hand_self zone missing' };
            const sprites = Array.from(document.querySelectorAll('#sprite-layer img.sprite'));
            const sprite = sprites.find(s => {
              const left = parseFloat(s.style.left) || 0;
              const top = parseFloat(s.style.top) || 0;
              const cx = left + 36;
              const cy = top + 48;
              return cx >= hand.x && cx <= hand.x + hand.w && cy >= hand.y && cy <= hand.y + hand.h;
            });
            if (!sprite)
              return { error: 'no hand sprite found' };

            const before = {
              left: parseFloat(sprite.style.left) || 0,
              top: parseFloat(sprite.style.top) || 0,
              className: sprite.className,
            };
            const rect = sprite.getBoundingClientRect();
            const downX = rect.left + 12;
            const downY = rect.top + 12;
            const moveX = downX + 48;
            const moveY = downY - 32;

            function fire(target, type, x, y) {
              target.dispatchEvent(new PointerEvent(type, {
                pointerId: 1,
                pointerType: 'mouse',
                clientX: x,
                clientY: y,
                bubbles: true,
              }));
            }

            fire(sprite, 'pointerdown', downX, downY);
            fire(window, 'pointermove', moveX, moveY);

            const during = {
              left: parseFloat(sprite.style.left) || 0,
              top: parseFloat(sprite.style.top) || 0,
              className: sprite.className,
              drag_active: !!api.runtime.drag,
            };

            fire(window, 'pointerup', moveX, moveY);

            const after = {
              left: parseFloat(sprite.style.left) || 0,
              top: parseFloat(sprite.style.top) || 0,
              className: sprite.className,
              drag_active: !!api.runtime.drag,
            };

            return { before, during, after };
            """
        )

        if "error" in result:
            fail(f"browser drag regression: {result['error']}")

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
