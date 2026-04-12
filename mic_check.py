import speech_recognition as sr

print("\n\033[1m\033[38;2;137;180;250m[System Audio Diagnostics]\033[0m")
print("Scanning hardware for available microphones...\n")

try:
    mics = sr.Microphone.list_microphone_names()
    for index, name in enumerate(mics):
        print(f"  Device ID {index}: \033[32m{name}\033[0m")
    print("\n\033[33mPlease paste this entire output back to me!\033[0m\n")
except Exception as e:
    print(f"Fatal Error: {e}")
