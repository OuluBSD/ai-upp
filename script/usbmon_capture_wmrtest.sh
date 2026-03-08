#!/usr/bin/env bash
set -euo pipefail

duration="${1:-5}"
async_buffers="${2:-4}"
timeout_ms="${3:-0}"
bus="${4:-2}"
out="${5:-/tmp/usbmon_${bus}u.log}"

sudo mount -t debugfs none /sys/kernel/debug 2>/dev/null || true
sudo modprobe usbmon

cap_timeout=$((duration + 5))
sudo timeout "${cap_timeout}" cat "/sys/kernel/debug/usb/usbmon/${bus}u" > "${out}" &
cap_pid=$!

./bin/WmrTest --test-dump "${duration}" --async-buffers "${async_buffers}" --timeout-ms "${timeout_ms}" || true

wait "${cap_pid}" || true
ls -l "${out}"
