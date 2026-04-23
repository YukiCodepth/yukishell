import re, os
def update():
    for fn in os.listdir('.'):
        if fn.endswith('.c'):
            with open(fn, 'r') as f: content = f.read()
            if 'YUKI-SHELL' in content:
                new_menu = r'''void builtin_help() {
    printf("\n\033[1;35m┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n");
    printf("┃                YUKI-SHELL V26c | AEGIS-EDGE CORE           ┃\n");
    printf("┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\033[0m\n\n");
    printf("  \033[1;36m[ CORE & FILESYSTEM ]\033[0m\n");
    printf("   xls         Enhanced lister (Metadata & Colors)\n");
    printf("   xcat <f>    Secure file stream with E-MUX headers\n");
    printf("   cd <path>   Traverse directories (supports ~)\n");
    printf("   clear       Wipe buffer and re-render HUD\n");
    printf("\n  \033[1;32m[ SHADOW & SENTINEL ]\033[0m\n");
    printf("   jobs        Monitor background (Shadow Realm) PIDs\n");
    printf("   xnet [host] Async Port Scanner (100ms Sentinel)\n");
    printf("   dash        Launch hardware telemetry dashboard\n");
    printf("   <cmd> &     Detach process to background\n");
    printf("\n  \033[1;33m[ NEURAL LINK & VISION ]\033[0m\n");
    printf("   ask         Query the AI Engine\n");
    printf("               --plot <port>  (Yuki Oscilloscope)\n");
    printf("               --chip         (Silicon Scanner)\n");
    printf("               --live         (Visual Tutor Agent)\n");
    printf("\n\033[1;35m━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
    printf(" Developed by Aman Kumar | ECE Core | Yukino Labs\n\n");
}'''
                # Replace the old function block containing 'YUKI-SHELL'
                pattern = r'(void|int)\s+\w+\s*\([^)]*\)\s*\{[^}]*YUKI-SHELL[^}]*\}'
                fixed = re.sub(pattern, new_menu, content, flags=re.DOTALL)
                with open(fn, 'w') as f: f.write(fixed)
                return fn
    return None
target = update()
if target: print(f"✅ Patched {target}")
else: print("❌ Could not find help function")
