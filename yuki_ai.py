import sys
import os
import subprocess
import warnings
import time
import threading
import base64
import math
from collections import deque
from dotenv import load_dotenv

try:
    from rich.console import Console
    from rich.markdown import Markdown
    console = Console()
except ImportError:
    console = None

warnings.filterwarnings("ignore")
load_dotenv()

from langchain_google_genai import ChatGoogleGenerativeAI
from langchain_core.messages import HumanMessage, SystemMessage
from langchain_community.tools.tavily_search import TavilySearchResults
from langgraph.prebuilt import create_react_agent
from langchain_core.tools import tool

def display_ai_response(content):
    if console:
        print("\n")
        console.print("[bold magenta]━━━ Yuki AI Response ━━━[/bold magenta]")
        console.print(Markdown(content))
        console.print("[bold magenta]━━━━━━━━━━━━━━━━━━━━━━━━[/bold magenta]\n")
    else: print(f"\n[Agent]: {content}\n")

def main():
    if len(sys.argv) < 3: sys.exit(1)
    provider = sys.argv[1].lower().replace("--", "") # Strips dashes so '--commit' becomes 'commit'
    prompt = sys.argv[2]
    gemini_version = os.environ.get("GEMINI_MODEL", "gemini-2.0-flash")
    
    try:
        # --- V18.1: AI GIT ASSISTANT ---
        if provider == "commit":
            # 1. Silently grab the staged git changes
            diff_result = subprocess.run("git diff --staged", shell=True, capture_output=True, text=True)
            diff_text = diff_result.stdout.strip()
            
            # If nothing is staged, fallback to grabbing unstaged changes
            if not diff_text:
                diff_result = subprocess.run("git diff", shell=True, capture_output=True, text=True)
                diff_text = diff_result.stdout.strip()
                
            if not diff_text:
                console.print("\n[bold red][!] No code changes detected. Modify some files first![/bold red]\n")
                sys.exit(0)
                
            console.print("\n[dim magenta]⚡ Analyzing Git Diff...[/dim magenta]\n", end="\r")
            
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.2)
            sys_msg = (
                "You are an expert DevOps engineer. Analyze the provided git diff and write a clean, professional commit message. "
                "Use the Conventional Commits format (e.g., feat:, fix:, refactor:, docs:). "
                "Keep it concise. Output ONLY the commit message, no markdown formatting, no explanations."
            )
            
            response = llm.invoke([SystemMessage(content=sys_msg), HumanMessage(content=f"Diff:\n{diff_text}")])
            
            print("\033[1;32m" + response.content.strip() + "\033[0m\n")
            
        # --- V17.1: AUTO-DATASHEET RETRIEVER ---
        elif provider == "chip":
            import cv2
            console.print("\n[bold yellow]🔍 Yuki Datasheet Scanner Initiated.[/bold yellow]")
            os.environ["QT_LOGGING_RULES"] = "*=false" 
            os.environ["OPENCV_LOG_LEVEL"] = "FATAL"
            cap = cv2.VideoCapture(0)
            if not cap.isOpened(): sys.exit(1)
            frame_to_send = None
            print("\033[38;2;166;227;161m  -> Press [SPACE] to scan the chip.\033[0m")
            print("\033[38;2;243;139;168m  -> Press [Q] to cancel.\033[0m\n")
            while True:
                ret, frame = cap.read()
                if not ret: break
                height, width = frame.shape[:2]
                cx, cy = width // 2, height // 2
                cv2.rectangle(frame, (cx - 100, cy - 50), (cx + 100, cy + 50), (0, 255, 0), 2)
                cv2.putText(frame, "ALIGN CHIP TEXT HERE", (cx - 90, cy - 60), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)
                cv2.imshow('Yuki Chip Scanner', frame)
                key = cv2.waitKey(1) & 0xFF
                if key == 32: 
                    frame_to_send = frame
                    console.print("[bold green]📸 Scan complete! Processing topography...[/bold green]")
                    break
                elif key == ord('q') or key == 27: break
            cap.release()
            cv2.destroyAllWindows()
            if frame_to_send is None: return
            _, buffer = cv2.imencode('.jpg', frame_to_send)
            img_b64 = base64.b64encode(buffer).decode('utf-8')
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.1) 
            sys_msg = "You are an expert ECE analyzer. Read the chip part number. Respond ONLY with a markdown datasheet containing: 1. Component Name. 2. A clean ASCII Art Pinout Diagram (ENCLOSED IN A ```text CODE BLOCK). 3. Key Specs."
            user_msg = HumanMessage(content=[{"type": "text", "text": sys_msg}, {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{img_b64}"}}])
            display_ai_response(llm.invoke([user_msg]).content)

        # --- V17.0: TERMINAL OSCILLOSCOPE ---
        elif provider == "plot":
            import serial
            port, baud_rate, width, height = prompt.strip(), 115200, 70, 15
            data = deque([0] * width, maxlen=width)
            console.print(f"\n[bold green]🟢 Yuki Telemetry Initiated.[/bold green]")
            try:
                ser = serial.Serial(port, baud_rate, timeout=1)
                time.sleep(1.5) 
                print("\n" * (height + 4))
                sys.stdout.write("\033[?25l")
                while True:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if not line: continue
                    try: data.append(float(line))
                    except ValueError: continue 
                    min_val, max_val = min(data), max(data)
                    val_range = max_val - min_val if max_val != min_val else 1
                    sys.stdout.write(f"\033[{height + 4}A")
                    print(f"\033[1;36mYuki Oscilloscope ❯\033[0m Port: {port} | \033[38;2;166;227;161mCurrent: {data[-1]:.2f}\033[0m | Min: {min_val:.2f} | Max: {max_val:.2f}\033[K\n\033[K", end="")
                    for y in range(height, -1, -1):
                        row_str = "│ "
                        threshold = min_val + (y / height) * val_range
                        for x in data: row_str += "\033[38;2;180;190;254m█\033[0m" if x >= threshold else " "
                        print(row_str + "\033[K")
                    print("└" + "─" * (width + 1) + "\033[K\n\033[90mPress Ctrl+C to terminate telemetry.\033[0m\033[K")
                    time.sleep(0.03) 
            except serial.SerialException: console.print(f"[bold red]Error: Could not connect to {port}.[/bold red]")
            except KeyboardInterrupt:
                sys.stdout.write("\033[?25h")
                console.print("\n[bold red]🔴 Telemetry Terminated.[/bold red]")
                if 'ser' in locals(): ser.close()

        # --- V16.0: INTERACTIVE VISUAL TUTOR ---
        elif provider == "live":
            import cv2
            console.print("\n[bold green]🟢 Yuki Visual Tutor Initiated.[/bold green]")
            os.environ["QT_LOGGING_RULES"] = "*=false" 
            os.environ["OPENCV_LOG_LEVEL"] = "FATAL"
            frame_lock, latest_frame, running = threading.Lock(), [None], [True]
            def capture_loop():
                cap = cv2.VideoCapture(0)
                if not cap.isOpened(): running[0] = False; return
                while running[0]:
                    ret, frame = cap.read()
                    if ret:
                        intensity = int(((math.sin(time.time() * 3.0) + 1) / 2) * 205 + 50)
                        cv2.putText(frame, "yuki ai live", (15, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 0, intensity), 1, cv2.LINE_AA)
                        with frame_lock: latest_frame[0] = frame.copy()
                        cv2.imshow('Yuki Live [Interactive Stream]', frame)
                    if cv2.waitKey(1) & 0xFF in [27, ord('q')]: running[0] = False; break
                cap.release(); cv2.destroyAllWindows()
            threading.Thread(target=capture_loop, daemon=True).start()
            time.sleep(1.5)
            if not running[0]: sys.exit(1)
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.6)
            chat_history = [SystemMessage(content="You are Yuki, an interactive visual tutor.")]
            try:
                while running[0]:
                    user_input = input("\033[1;36mAman ❯ \033[0m")
                    if user_input.lower() in ['exit', 'quit', 'q']: running[0] = False; break
                    if not user_input.strip(): continue
                    with frame_lock: frame_to_analyze = latest_frame[0].copy()
                    _, buffer = cv2.imencode('.jpg', frame_to_analyze)
                    img_b64 = base64.b64encode(buffer).decode('utf-8')
                    user_msg = HumanMessage(content=[{"type": "text", "text": user_input}, {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{img_b64}"}}])
                    display_ai_response(llm.invoke(chat_history + [user_msg]).content)
                    chat_history.extend([user_msg, llm.invoke(chat_history + [user_msg])])
                    if len(chat_history) > 7: chat_history = [chat_history[0]] + chat_history[-6:]
            except KeyboardInterrupt: running[0] = False
            finally: os._exit(0) 

        elif provider == "exec":
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.0)
            print(llm.invoke([SystemMessage(content="Output ONLY raw bash command."), HumanMessage(content=prompt)]).content.replace("```bash", "").replace("```sh", "").replace("```", "").strip())
        
        # --- V18.0: THE TYPEWRITER STREAMING UI (WITH ON-THE-FLY MARKDOWN PARSING) ---
        else: 
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.7)
            status_messages = [
                "[bold magenta]⚡ Pinging Neural Core...[/bold magenta]",
                "[bold cyan]🧠 Analyzing Context Matrix...[/bold cyan]",
                "[bold green]📝 Formatting Output Stream...[/bold green]",
                "[bold yellow]📡 Establishing Uplink...[/bold yellow]"
            ]
            is_streaming = [False]

            def update_status(status_obj):
                idx = 0
                while not is_streaming[0]:
                    status_obj.update(status_messages[idx % len(status_messages)])
                    idx += 1
                    time.sleep(0.4)

            with console.status(status_messages[0], spinner="dots12") as status:
                t = threading.Thread(target=update_status, args=(status,))
                t.daemon = True
                t.start()

                try:
                    response_stream = llm.stream([HumanMessage(content=prompt)])
                    first_chunk = True
                    bold_toggle = False
                    
                    for chunk in response_stream:
                        if first_chunk:
                            is_streaming[0] = True
                            status.stop() 
                            print("\n\033[1m\033[38;2;180;190;254m━━━ Yuki AI Response ━━━\033[0m\n")
                            first_chunk = False
                        
                        text = chunk.content
                        while "**" in text:
                            if not bold_toggle:
                                text = text.replace("**", "\033[1m\033[38;2;180;190;254m", 1)
                                bold_toggle = True
                            else:
                                text = text.replace("**", "\033[0m", 1)
                                bold_toggle = False
                                
                        print(text, end="", flush=True)
                    
                    print("\n\n\033[1m\033[38;2;180;190;254m━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n")
                except Exception as e:
                    is_streaming[0] = True
                    status.stop()
                    console.print(f"\n[bold red][!] Stream Error:[/bold red] {e}")

    except Exception as e:
        console.print(f"\n[bold red][!] API Error:[/bold red] {e}")

if __name__ == "__main__":
    main()
