import ipaddress
import socket
import sys
import threading

CYAN = "\033[1;36m"
GREEN = "\033[1;32m"
YELLOW = "\033[1;33m"
DIM = "\033[90m"
RESET = "\033[0m"

COMMON_PORTS = [22, 80, 443, 3000, 5000, 5173, 5432, 6379, 8000, 8080, 8443]


def get_local_prefix():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.connect(("8.8.8.8", 80))
        ip = sock.getsockname()[0]
        return ".".join(ip.split(".")[:-1]) + "."
    except Exception:
        return "192.168.1."
    finally:
        sock.close()


def is_host(value):
    try:
        ipaddress.ip_address(value)
        return True
    except ValueError:
        return False


def scan_port(host, port, timeout=0.18):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    try:
        return sock.connect_ex((host, port)) == 0
    except OSError:
        return False
    finally:
        sock.close()


def scan_host(host):
    print(f"\n{CYAN}━━━ Yuki Network Sentinel ━━━{RESET}\n")
    print(f"{DIM}[*] Target Host: {host}{RESET}\n")

    open_ports = []
    lock = threading.Lock()

    def worker(port):
        if scan_port(host, port):
            with lock:
                open_ports.append(port)
                print(f"  {GREEN}[+] Open:{RESET} {host:<15} {DIM}port {port}{RESET}")

    threads = [threading.Thread(target=worker, args=(port,)) for port in COMMON_PORTS]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()

    if not open_ports:
        print(f"  {DIM}[-] No common ports open on {host}.{RESET}")
    print(f"\n{CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{RESET}\n")


def scan_subnet(target=None):
    prefix = get_local_prefix()
    if target:
        target = target.strip()
        if target.endswith("."):
            prefix = target
        elif target.endswith(".0/24"):
            prefix = target[:-4]
        elif "/" in target:
            try:
                network = ipaddress.ip_network(target, strict=False)
                hosts = [str(ip) for ip in network.hosts()]
                return scan_hosts(hosts, target)
            except ValueError:
                print(f"{YELLOW}[!] Invalid subnet: {target}{RESET}")
                return
        elif is_host(target):
            return scan_host(target)

    hosts = [f"{prefix}{i}" for i in range(1, 255)]
    scan_hosts(hosts, f"{prefix}0/24")


def scan_hosts(hosts, label):
    print(f"\n{CYAN}━━━ Yuki Network Topographer ━━━{RESET}\n")
    print(f"{DIM}[*] Sweeping Subnet: {label}{RESET}\n")

    active_hosts = []
    lock = threading.Lock()

    def worker(host):
        for port in (80, 22, 443):
            if scan_port(host, port, timeout=0.25):
                with lock:
                    if host not in active_hosts:
                        active_hosts.append(host)
                        print(f"  {GREEN}[+] Host Alive:{RESET} {host:<15} {DIM}(Port {port} Open){RESET}")
                return

    threads = [threading.Thread(target=worker, args=(host,)) for host in hosts]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()

    print(f"\n{CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━{RESET}\n")
    if not active_hosts:
        print(f"{DIM}[-] No open ports detected on target subnet.{RESET}\n")


if __name__ == "__main__":
    scan_subnet(sys.argv[1] if len(sys.argv) > 1 else None)
