import re
import os

# Find the .c file that contains the help function
target = None
for f in os.listdir('.'):
    if f.endswith('.c'):
        with open(f, 'r', errors='ignore') as file:
            if 'void builtin_help()' in file.read():
                target = f
                break

if not target:
    print("❌ Error: Could not find any .c file with 'void builtin_help()'.")
    exit(1)

new_help = r'''void builtin_help() {
    printf("\n\033[1;35m┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
    printf("┃                YUKI-SHELL V26c | AEGIS-EDGE CORE           ┃\n");
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\033[0m\n\n");

    printf("  \033[1;36m[ CORE & FILESYSTEM ]\033[0m\n");
    printf("   xls         Enhanced directory lister (Metadata & Colors)\n");
    printf("   xcat <f>    Secure file stream with E-MUX headers\n");
    printf("   cd <path>   Traverse directories (supports ~ and ..)\n");
    printf("   clear       Wipe buffer and re-render E-MUX HUD\n");
    printf("   neofetch    Display OS logo and live hardware specs\n");

    printf("\n  \033[1;32m[ SHADOW & SENTINEL ]\033[0m\n");
    printf("   jobs        Monitor background (Shadow Realm) PIDs\n");
    printf("   xnet [host] Async Port Scanner (100ms Sentinel Engine)\n");
    printf("   dash        Launch real-time hardware telemetry dashboard\n");
    printf("   <cmd> &     Detach process to background execution\n");

    printf("\n  \033[1;33m[ NEURAL LINK & VISION ]\033[0m\n");
    printf("   ask         Query the Multi-Model AI Engine\n");
    printf("               Usage: ask [flag] \"prompt\"\n");
    printf("               --plot <port>  (Yuki Oscilloscope / Serial)\n");
    printf("               --chip         (Silicon Scanner / Datasheet)\n");
    printf("               --live         (Real-Time Visual Tutor Agent)\n");
    printf("               --voice        (Acoustic-to-Text Command)\n");

    printf("\n\033[1;35m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf(" Developed by Aman Kumar | ECE Core | Yukino Labs\n\n");
}'''

with open(target, 'r') as f:
    data = f.read()

# Replace the entire old function block
data = re.sub(r'void builtin_help\(\) \{.*?\}', new_help, data, flags=re.DOTALL)

with open(target, 'w') as f:
    f.write(data)

print(f"✅ {target} patched successfully!")
