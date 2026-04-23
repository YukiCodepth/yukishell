import socket
import threading
import time
import sys

# Aesthetic colors
CYAN = "\033[1;36m"
GREEN = "\033[1;32m"
DIM = "\033[90m"
RESET = "\033[0m"

print(f"\n{CYAN}━━━ Yuki Network Topographer ━━━{RESET}\n")

# Try to automatically find the local IP prefix (e.g., 192.168.1. or 10.0.0.)
def get_local_prefix():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(('8.8.8.8', 80))
        ip = s.getsockname()[0]
        return '.'.join(ip.split('.')[:-1]) + '.'
    except Exception:
        return '192.168.1.' # Fallback
    finally:
        s.close()

prefix = get_local_prefix()
print(f"{DIM}[*] Sweeping Subnet: {prefix}0/24{RESET}\n")

active_hosts = []
lock = threading.Lock()

def scan_ip(ip):
    # We check if port 80 (Web) or 22 (SSH) is open to confirm a device is alive
    for port in [80, 22, 443]:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(0.3) # 300ms timeout for blazing fast scans
        try:
            result = s.connect_ex((ip, port))
            if result == 0:
                with lock:
                    if ip not in active_hosts:
                        active_hosts.append(ip)
                        print(f"  {GREEN}[+] Host Alive:{RESET} {ip:<15} {DIM}(Port {port} Open){RESET}")
                s.close()
                return
        except:
            pass
        s.close()

# Spin up 255 threads to scan the whole network in under 1 second
threads = []
for i in range(1, 255):
    ip = f"{prefix}{i}"
    t = threading.Thread(target=scan_ip, args=(ip,))
    threads.append(t)
    t.start()

for t in threads:
    t.join()

print(f"\n{CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{RESET}\n")
if not active_hosts:
    print(f"{DIM}[-] No open ports detected on local subnet.{RESET}\n")
