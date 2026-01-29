#!/usr/bin/env python3
import os
import sys
import subprocess
import shutil
import getpass
import re

def run_cmd(cmd, silent=False):
    if silent:
        return subprocess.call(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return subprocess.call(cmd, shell=True)

def check_deps():
    deps = ["sudo", "lspci", "xrandr", "aplay"]
    missing = []
    for d in deps:
        if subprocess.call(["which", d], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) != 0:
            missing.append(d)
    if missing:
        print(f"Error: Missing dependencies: {', '.join(missing)}")
        sys.exit(1)

def get_hmd_bus_id():
    print("\nScanning for Graphics Cards...")
    cards = []
    try:
        lspci_out = subprocess.check_output(["lspci", "-nn"], text=True)
        for line in lspci_out.splitlines():
            if "VGA compatible controller" in line or "3D controller" in line:
                parts = line.split()
                bus_id_str = parts[0]
                name = " ".join(parts[1:])
                bus_hex, dev_func = bus_id_str.split(':')
                dev_hex, func_hex = dev_func.split('.')
                bus_dec = int(bus_hex, 16)
                dev_dec = int(dev_hex, 16)
                func_dec = int(func_hex, 16)
                pci_id_xorg = f"PCI:{bus_dec}:{dev_dec}:{func_dec}"
                cards.append({'pci': bus_id_str, 'xorg_bus': pci_id_xorg, 'name': name})
    except Exception as e:
        print(f"Error scanning PCI: {e}")
        return None

    if not cards:
        print("No GPUs found.")
        return None

    print("Found GPUs:")
    for i, c in enumerate(cards):
        print(f"  {i}: {c['pci']} ({c['xorg_bus']}) - {c['name']}")
    
    print("Which one is the HMD/Secondary GPU? (Enter number)")
    try:
        idx = int(input("> "))
        if 0 <= idx < len(cards):
            return cards[idx]
    except:
        pass
    print("Invalid selection.")
    return None

def get_hmd_audio_card():
    print("\nScanning for Audio Cards (aplay -l)...")
    cards = []
    try:
        output = subprocess.check_output(["aplay", "-l"], text=True)
        current_card = None
        for line in output.splitlines():
            if line.startswith("card"):
                m = re.match(r'card (\d+): (.*?) \[(.*)\]', line)
                if m:
                    card_num = m.group(1)
                    card_name = m.group(2)
                    m_dev = re.search(r'device (\d+):', line)
                    dev_num = m_dev.group(1) if m_dev else '0'
                    desc = m.group(3) if m.group(3) else card_name
                    cards.append({'id': card_num, 'dev': dev_num, 'name': f"{card_name} - {desc}"})
    except:
        pass

    if not cards:
        print("No audio cards found.")
        return None

    for i, c in enumerate(cards):
        print(f"  {i}: Card {c['id']}, Dev {c['dev']} - {c['name']}")
    
    print("Which one is the HMD Audio output? (Enter number, usually an HDMI/DP device)")
    try:
        idx = int(input("> "))
        if 0 <= idx < len(cards):
            return cards[idx]
    except:
        pass
    return None

def get_hmd_output_name():
    print("\nScanning for HMD Output (xrandr)...")
    try:
        raw = subprocess.check_output(["xrandr"], text=True)
        candidates = []
        curr = None
        for line in raw.splitlines():
            if not line.startswith(" "):
                parts = line.split()
                if len(parts) > 1:
                    curr = parts[0]
                    # connected?
                    if "connected" in parts:
                        # Check if it supports VR res?
                        pass
            elif curr and "2880x1440" in line:
                candidates.append(curr)
                curr = None # Avoid adding same twice
        
        if candidates:
            print(f"Found candidate HMD outputs: {', '.join(candidates)}")
            return candidates[0] # Pick first
            
    except Exception as e:
        print(f"Error scanning xrandr: {e}")
        
    print("Could not auto-detect HMD output (2880x1440).")
    name = input("Enter HMD Output Name (e.g. HDMI-A-1) or press Enter to skip: ").strip()
    return name if name else None

def install_system_files(gpu_info):
    print("\nInstalling /etc/X11/xorg.conf.d/ files...")
    sep_conf = "/etc/X11/xorg.conf.d/01-gpu_separation.conf"
    sep_content = """Section \"ServerFlags\"\n    Option \"AutoAddGPU\" \"off\"\n    Option \"AutoBindGPU\" \"off\"\n    Option \"SingleCard\" \"true\"\nEndSection\n"""
    if run_cmd(f"echo '{sep_content}' | sudo tee {sep_conf}") != 0:
        print("Failed to write gpu_separation.conf")

    key_conf = "/etc/X11/xorg.conf.d/00-keylayouts.conf"
    key_content = """Section \"InputClass\"\n    Identifier \"keyboard-all\"\n    MatchIsKeyboard \"on\"\n    Option \"XkbLayout\" \"us\"\nEndSection\n"""
    if not os.path.exists(key_conf):
        run_cmd(f"echo '{key_content}' | sudo tee {key_conf}")

    hmd_xorg = "/etc/X11/xorg.conf.hmd"
    hmd_content = f"""
Section \"Device\"\n    Identifier \"HMD_GPU\"\n    Driver \"amdgpu\"\n    BusID \"{gpu_info['xorg_bus']}\"\nEndSection\n\nSection \"Screen\"\n    Identifier \"HMD_Screen\"\n    Device \"HMD_GPU\"\nEndSection\n"""
    driver = "modesetting"
    hmd_content = hmd_content.replace('"amdgpu"', f'"{driver}"')
    
    run_cmd(f"echo '{hmd_content}' | sudo tee {hmd_xorg}")
    print(f"Created {hmd_xorg} using driver '{driver}'")

def install_audio_files(audio_info):
    if not audio_info:
        return
        
    print("\nConfiguring Audio...")
    asound_hmd = "/etc/asound.hmd"
    content = f"""
pcm.hmd {{
    type hw
    card {audio_info['id']}
    device {audio_info['dev']}
}}
ctl.hmd {{
    type hw
    card {audio_info['id']}
}}
"""
    run_cmd(f"echo '{content}' | sudo tee {asound_hmd}")
    
    pa_file = "/etc/pulse/hmd_seat.pa"
    pa_content = f"""
#!/usr/bin/pulseaudio -nF
.include /etc/pulse/default.pa
load-module module-alsa-sink device=hw:{audio_info['id']},{audio_info['dev']} sink_name=hmd_output
set-default-sink hmd_output
"""
    run_cmd(f"echo '{pa_content}' | sudo tee {pa_file}")

def install_user_scripts(gpu_info, display_num=":1", single_gpu=False, hmd_output_name=None):
    home = os.path.expanduser("~")
    base_dir = os.path.join(home, ".ai-upp")
    bin_dir = os.path.join(base_dir, "bin")
    os.makedirs(bin_dir, exist_ok=True)
    
    start_script = os.path.join(bin_dir, "start_hmd_x.py")
    stop_script = os.path.join(bin_dir, "stop_hmd_x.sh")
    conf_file = os.path.join(base_dir, "hmd_seat.conf")
    
    # Fallback/Default
    if not hmd_output_name:
        hmd_output_name = "HDMI-A-1"

    if single_gpu:
        current_display = os.environ.get("DISPLAY", ":0")
        print(f"Single GPU detected. Configuring fallback to attach HMD to current display ({current_display}).")
        
        with open(conf_file, "w") as f:
            f.write(f"HMD_DISPLAY={current_display}\n")
            f.write(f"HMD_OUTPUT={hmd_output_name}\n")
    
        with open(start_script, "w") as f:
            f.write("#!/usr/bin/env python3\n")
            f.write("import os\n")
            f.write("import sys\n")
            f.write("import subprocess\n")
            f.write("import time\n")
            f.write("\n")
            f.write(f"HMD_DEV = '{hmd_output_name}'\n")
            f.write("\n")
            f.write("def main():\n")
            f.write("    print(f'Enabling HMD {HMD_DEV} on current display...')\n")
            f.write("    # Get primary\n")
            f.write("    primary = None\n")
            f.write("    try:\n")
            f.write("        out = subprocess.check_output(['xrandr'], text=True)\n")
            f.write("        for line in out.splitlines():\n")
            f.write("            if ' primary ' in line:\n")
            f.write("                primary = line.split()[0]\n")
            f.write("    except: pass\n")
            f.write("    \n")
                f.write("    cmd = ['xrandr', '--output', HMD_DEV, '--mode', '2880x1440', '--set', 'non-desktop', '0']\n")
                f.write("    if primary:\n")
                f.write("        cmd.extend(['--right-of', primary])\n")
                f.write("    else:\n")
                f.write("        cmd.append('--auto')\n")
                f.write("    \n")
                f.write("    # Ensure HMD is NOT primary\n")
                f.write("    if primary:\n")
                f.write("        subprocess.call(['xrandr', '--output', primary, '--primary'])\n")
                f.write("    \n")
                f.write("    print(' '.join(cmd))\n")
                f.write("    ret = subprocess.call(cmd)\n")
                f.write("    sys.exit(ret)\n")            f.write("\n")
            f.write("if __name__ == '__main__':\n")
            f.write("    main()\n")
        
        with open(stop_script, "w") as f:
            f.write("#!/bin/bash\n")
            f.write(f"xrandr --output {hmd_output_name} --off || true\n")

    else:
        with open(conf_file, "w") as f:
            f.write(f"HMD_DISPLAY={display_num}\n")
            f.write(f"HMD_X_CONFIG=/etc/X11/xorg.conf.hmd\n")
            f.write(f"HMD_OUTPUT={hmd_output_name}\n")
        
        with open(start_script, "w") as f:
            f.write("#!/usr/bin/env python3\n")
            f.write("import os\n")
            f.write("import sys\n")
            f.write("import subprocess\n")
            f.write("import time\n")
            f.write("\n")
            f.write(f"DISPLAY_NUM = '{display_num}'\n")
            f.write(f"HMD_DEV = '{hmd_output_name}'\n")
            f.write("CONF = '/etc/X11/xorg.conf.hmd'\n")
            f.write("\n")
            f.write("def main():\n")
            f.write("    lock_file = f'/tmp/.X{DISPLAY_NUM[1:]}-lock'\n")
            f.write("    if os.path.exists(lock_file):\n")
            f.write("        print(f'X server already running on {DISPLAY_NUM}')\n")
            f.write("        sys.exit(0)\n")
            f.write("\n")
            f.write("    print(f'Found VR HMD on {HMD_DEV}. Starting X server on {DISPLAY_NUM}...')\n")
            f.write("    subprocess.call(['sudo', 'xrandr', '--output', HMD_DEV, '--off'])\n")
            f.write("    cmd = f'sudo X {DISPLAY_NUM} -config {CONF} -ac -nolisten tcp -extension GLX -sharevts &'\n")
            f.write("    os.system(cmd)\n")
            f.write("    time.sleep(2)\n")
            f.write("    print(f'Configuring {HMD_DEV} on {DISPLAY_NUM}...')\n")
            f.write("    env = os.environ.copy()\n")
            f.write("    env['DISPLAY'] = DISPLAY_NUM\n")
            f.write("    ret = subprocess.call(['xrandr', '--output', HMD_DEV, '--mode', '2880x1440', '--primary'], env=env)\n")
            f.write("    sys.exit(ret)\n")
            f.write("\n")
            f.write("if __name__ == '__main__':\n")
            f.write("    main()\n")
    
        with open(stop_script, "w") as f:
            f.write("#!/bin/bash\n")
            f.write(f"DISPLAY_NUM={display_num}\n")
            f.write("LOCK_FILE=/tmp/.X${DISPLAY_NUM:1}-lock\n")
            f.write("if [ -f $LOCK_FILE ]; then\n")
            f.write("    PID=$(cat $LOCK_FILE)\n")
            f.write("    sudo kill $PID\n")
            f.write("fi\n")

    os.chmod(start_script, 0o755)
    os.chmod(stop_script, 0o755)
    print(f"Scripts installed to {bin_dir}")

def update_sudoers():
    if subprocess.call(["sudo", "test", "-d", "/etc/sudoers.d"]) != 0:
        print("Creating /etc/sudoers.d...")
        run_cmd("sudo mkdir -p /etc/sudoers.d")
        run_cmd("sudo chmod 0755 /etc/sudoers.d")

    try:
        content = subprocess.check_output(["sudo", "cat", "/etc/sudoers"], text=True)
        if "@includedir /etc/sudoers.d" not in content and "#includedir /etc/sudoers.d" not in content:
             print("\nWARNING: /etc/sudoers does not seem to include /etc/sudoers.d!")
             print("You must fix this manually using 'sudo visudo'.")
    except:
        pass

    user = getpass.getuser()
    fpath = f"/etc/sudoers.d/ai-upp-{user}"
    cmd = f"echo '{user} ALL=(ALL) NOPASSWD: /usr/bin/X, /usr/bin/xrandr, /bin/kill, /usr/bin/kill, /usr/bin/tee' | sudo tee {fpath}"
    
    print(f"\nAdding NOPASSWD rules to {fpath}...")
    if run_cmd(cmd) == 0:
        run_cmd(f"sudo chmod 0440 {fpath}")
        print("Done.")
    else:
        print("Failed.")

def main():
    check_deps()
    print("Gentoo HMD Multiseat Setup (Advanced)")
    
    force_single = "--single-gpu" in sys.argv
    
    pci_out = subprocess.check_output(["lspci"], text=True)
    gpu_count = pci_out.count("VGA compatible controller") + pci_out.count("3D controller")
    
    if force_single:
        gpu_count = 1
        print("Forcing single-GPU mode.")
    
    gpu = get_hmd_bus_id()
    if not gpu:
        print("GPU detection failed or aborted.")
        sys.exit(1)
        
    audio = get_hmd_audio_card()
    hmd_out = get_hmd_output_name()
    
    if input("\nInstall system configuration files? (requires sudo) (y/N) ").lower() == 'y':
        if gpu_count > 1 and gpu:
            install_system_files(gpu)
        else:
            print("Single GPU detected (or selection skipped). Skipping xorg.conf generation for multiseat.")
            
        install_audio_files(audio)
        update_sudoers()
        install_user_scripts(gpu, single_gpu=(gpu_count <= 1), hmd_output_name=hmd_out)
        print("\nSetup Complete. Reboot might be required for GPU separation to take full effect.")
    else:
        print("Aborted.")

if __name__ == "__main__":
    main()