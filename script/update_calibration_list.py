#!/usr/bin/env python3
import json
import re
from pathlib import Path

def parse_ids_from_name(name: str):
    m = re.fullmatch(r"([0-9a-fA-F]{4})_([0-9a-fA-F]{4})", name)
    if not m:
        return None
    return int(m.group(1), 16), int(m.group(2), 16)

def parse_ids_from_json(obj):
    vid = obj.get("vendor_id")
    pid = obj.get("product_id")
    if vid is None or pid is None:
        return None
    try:
        return int(vid), int(pid)
    except Exception:
        try:
            return int(str(vid), 16), int(str(pid), 16)
        except Exception:
            return None

def main():
    root = Path(__file__).resolve().parents[1]
    calib_root = root / "share" / "calibration"
    list_path = calib_root / "list.json"

    entries = []
    skipped = []

    for stcal in calib_root.rglob("*.stcal"):
        rel = stcal.relative_to(root).as_posix()
        folder = stcal.parent.name

        ids = None
        # Try folder naming convention
        ids = parse_ids_from_name(folder)

        # Try project.json metadata if present
        if ids is None:
            project = stcal.parent / "project.json"
            if project.exists():
                try:
                    data = json.loads(project.read_text())
                    state = data.get("state", {}) if isinstance(data, dict) else {}
                    ids = parse_ids_from_json(state) or parse_ids_from_json(data)
                except Exception:
                    pass

        # Try stcal JSON metadata if present
        if ids is None:
            try:
                data = json.loads(stcal.read_text())
                if isinstance(data, dict):
                    ids = parse_ids_from_json(data)
            except Exception:
                pass

        if ids is None:
            skipped.append(rel)
            continue

        vendor_id, product_id = ids
        entry = {
            "vendor_id": f"0x{vendor_id:04x}",
            "product_id": f"0x{product_id:04x}",
            "path": rel,
        }
        entries.append(entry)

    entries.sort(key=lambda e: (e["vendor_id"], e["product_id"], e["path"]))
    list_path.write_text(json.dumps(entries, indent=2) + "\n")

    if skipped:
        print("Skipped (no vendor/product id):")
        for rel in skipped:
            print("  -", rel)

if __name__ == "__main__":
    main()
