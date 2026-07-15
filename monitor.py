import serial
import time
import psutil
import platform
import subprocess
import re
import cpuinfo
from datetime import datetime

OS_TYPE = platform.system()

if OS_TYPE == 'Windows':
    try:
        import WinTmp
    except ImportError:
        print("Missing WinTmp. Run: pip install WinTmp")
        exit()
    SERIAL_PORT = 'COM3'
else:
    SERIAL_PORT = '/dev/ttyUSB0'

try:
    esp32 = serial.Serial(SERIAL_PORT, 115200)
    time.sleep(2)
    print(f"Connected to ESP32. Beaming unified telemetry...")
except Exception as e:
    print(f"Connection failed: {e}")
    exit()

def shorten_cpu_name(raw_name):
    clean = raw_name.replace("(R)", "").replace("(TM)", "").replace("CPU", "").replace("Processor", "")
    m = re.search(r'(i\d-\w+)', clean)
    if m: return m.group(1)
    m = re.search(r'Ryzen (\d) (\w+)', clean)
    if m: return f"R{m.group(1)} {m.group(2)}"
    words = [w for w in clean.split() if w not in ["Intel", "AMD", "Core", "with", "Radeon", "Graphics", "@"]]
    return " ".join(words[:2])

def get_cpu_temp():
    if OS_TYPE == 'Windows':
        return int(WinTmp.CPU_Temp())
    else:
        temps = psutil.sensors_temperatures()
        for name in ['k10temp', 'coretemp', 'zenpower', 'acpitz']:
            if name in temps: return int(temps[name][0].current)
        return 0

def get_gpu_temp():
    if OS_TYPE == 'Windows':
        return int(WinTmp.GPU_Temp())
    else:
        try:
            res = subprocess.run(['nvidia-smi', '--query-gpu=temperature.gpu', '--format=csv,noheader'], capture_output=True, text=True)
            if res.stdout: return int(res.stdout.strip())
            temps = psutil.sensors_temperatures()
            if 'amdgpu' in temps: return int(temps['amdgpu'][0].current)
            if 'i915' in temps: return int(temps['i915'][0].current)
        except: pass
        return 0

def get_fan_rpm():
    if OS_TYPE == 'Windows': return 0
    else:
        fans = psutil.sensors_fans()
        for name, entries in fans.items():
            if entries: return int(entries[0].current)
        return 0

def get_battery():
    battery = psutil.sensors_battery()
    if battery is None: return (0, 0)
    return (int(battery.percent), 1 if battery.power_plugged else 0)

raw_cpu = cpuinfo.get_cpu_info()['brand_raw']
short_cpu = shorten_cpu_name(raw_cpu)

while True:
    try:
        cpu = get_cpu_temp()
        gpu = get_gpu_temp()
        ram = int(psutil.virtual_memory().percent)
        fan = get_fan_rpm()
        bat_pct, bat_chg = get_battery()

        freq = psutil.cpu_freq()
        clock_ghz = 0.0
        clock_pct = 0
        if freq:
            clock_ghz = round(freq.current / 1000, 1)
            if freq.max > 0:
                clock_pct = int((freq.current / freq.max) * 100)
            else:
                clock_pct = int((freq.current / 5000) * 100)

        clock_str = f"{clock_ghz}G"

        path = 'C:\\' if OS_TYPE == 'Windows' else '/'
        usage = psutil.disk_usage(path)
        ssd_free = usage.free // (2**30)
        ssd_pct = int(usage.percent)

        # 12-hour AM/PM format, strip leading zero
        time_str = datetime.now().strftime("%I:%M %p").lstrip("0")

        if cpu == gpu: gpu = 0

        data_string = f"{cpu}|{gpu}|{ram}|{ssd_pct}|{ssd_free}|{fan}|{bat_pct}|{bat_chg}|{time_str}|{short_cpu}|{clock_str}|{clock_pct}\n"

        esp32.write(data_string.encode('utf-8'))
        time.sleep(1)

    except Exception as e:
        print(f"Error reading sensors: {e}")
        time.sleep(2)
