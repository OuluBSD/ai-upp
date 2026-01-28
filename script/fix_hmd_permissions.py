#!/usr/bin/env python3
import os
import subprocess
import glob

# Target devices to fix permissions for
TARGETS = [
    {"name": "HoloLens Sensors", "vid": "045e", "pid": "0659"},
    {"name": "QHMD", "vid": "03f0", "pid": "0367"},
]

def get_file_content(path):
    try:
        with open(path, 'r') as f:
            return f.read().strip()
    except Exception:
        return ""

def fix_permissions(path):
    print(f"Found device at {path}. Fixing permissions...")
    try:
        subprocess.check_call(['sudo', 'chmod', '666', path])
        print(f"Successfully changed permissions for {path}")
    except subprocess.CalledProcessError as e:
        print(f"Error: Failed to change permissions for {path}: {e}")

def main():
    print("Scanning for HMD devices...")
    fixed_count = 0

    # 1. Fix HIDRAW devices (for sensors)
    # /sys/class/hidraw/hidraw*/device/uevent contains "HID_ID=0003:0000VID:0000PID"
    print("Checking /sys/class/hidraw...")
    for hid_path in glob.glob("/sys/class/hidraw/hidraw*"):
        uevent_path = os.path.join(hid_path, "device/uevent")
        content = get_file_content(uevent_path)
        
        for target in TARGETS:
            # HID uevent usually uses uppercase hex
            vid_hex = target["vid"].upper()
            pid_hex = target["pid"].upper()
            
            # Match strict pattern
            search_str = f":0000{vid_hex}:0000{pid_hex}"
            
            if search_str in content:
                print(f"Identified {target['name']} on {os.path.basename(hid_path)}")
                dev_node = os.path.join("/dev", os.path.basename(hid_path))
                if os.path.exists(dev_node):
                    fix_permissions(dev_node)
                    fixed_count += 1

    # 2. Fix USB devices (for libusb/camera)
    # /sys/bus/usb/devices/*/idVendor and idProduct
    print("Checking /sys/bus/usb/devices...")
    for usb_path in glob.glob("/sys/bus/usb/devices/*"):
        vid_path = os.path.join(usb_path, "idVendor")
        pid_path = os.path.join(usb_path, "idProduct")
        
        if not os.path.exists(vid_path) or not os.path.exists(pid_path):
            continue
            
        vid = get_file_content(vid_path)
        pid = get_file_content(pid_path)
        
        for target in TARGETS:
            if vid == target["vid"] and pid == target["pid"]:
                print(f"Identified {target['name']} USB device at {os.path.basename(usb_path)}")
                
                bus_path = os.path.join(usb_path, "busnum")
                dev_path = os.path.join(usb_path, "devnum")
                
                bus = get_file_content(bus_path)
                dev = get_file_content(dev_path)
                
                if bus and dev:
                    # Construct /dev/bus/usb/BBB/DDD
                    usb_dev_node = f"/dev/bus/usb/{int(bus):03d}/{int(dev):03d}"
                    if os.path.exists(usb_dev_node):
                        fix_permissions(usb_dev_node)
                        fixed_count += 1
                        
                    # Check USB speed
                    speed_path = os.path.join(usb_path, "speed")
                    speed = get_file_content(speed_path)
                    if speed:
                        print(f"Device speed: {speed} Mbps")
                        if speed != "5000" and speed != "10000" and speed != "20000":
                            print(f"WARNING: Device is not running at SuperSpeed (5000+ Mbps). Current speed: {speed} Mbps. This may cause bandwidth issues.")
                    else:
                        print("Could not determine device speed.")

    if fixed_count == 0:
        print("No target HMD devices found.")
    else:
        print(f"Finished. Fixed permissions for {fixed_count} device nodes.")

if __name__ == "__main__":
    main()
