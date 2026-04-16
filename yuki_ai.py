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

# --- Tools ---
@tool
def hardware_vision(query: str) -> str:
    """Captures a static image from the webcam."""
    pass 

@tool
def web_search(query: str) -> str:
    """Searches the live internet."""
    console.print(f"[bold blue]🔍 AI Searching:[/bold blue] {query}")
    try:
        return str(TavilySearchResults(max_results=3).invoke({"query": query}))
    except Exception as e: return f"Search failed: {e}"

@tool
def read_file(filepath: str) -> str:
    """Reads local files."""
    try:
        with open(filepath, 'r') as f: return f.read()[:3000]
    except Exception as e: return f"Error reading file: {e}"

@tool
def terminal_executor(command: str) -> str:
    """Executes a bash command."""
    try:
        result = subprocess.run(command, shell=True, capture_output=True, text=True, timeout=15)
        return (result.stdout + result.stderr)[:2000] or "Success."
    except Exception as e: return f"Error: {e}"

def display_ai_response(content):
    if console:
        print("\n")
        console.print("[bold magenta]━━━ Yuki AI Response ━━━[/bold magenta]")
        console.print(Markdown(content))
        console.print("[bold magenta]━━━━━━━━━━━━━━━━━━━━━━━━[/bold magenta]\n")
    else: print(f"\n[Agent]: {content}\n")

def main():
    if len(sys.argv) < 3: sys.exit(1)
    provider = sys.argv[1].lower()
    prompt = sys.argv[2]
    gemini_version = os.environ.get("GEMINI_MODEL", "gemini-2.0-flash")
    
    try:
        # --- V17.1: AUTO-DATASHEET RETRIEVER ---
        if provider == "chip":
            import cv2
            
            console.print("\n[bold yellow]🔍 Yuki Datasheet Scanner Initiated.[/bold yellow]")
            console.print("[dim]Hold an IC/Microchip to the camera. Ensure the laser-etched text is well lit.[/dim]")
            
            os.environ["QT_LOGGING_RULES"] = "*=false" 
            os.environ["OPENCV_LOG_LEVEL"] = "FATAL"
            
            cap = cv2.VideoCapture(0)
            if not cap.isOpened():
                console.print("[bold red]Fatal: Could not access the webcam.[/bold red]")
                sys.exit(1)

            frame_to_send = None
            print("\033[38;2;166;227;161m  -> Press [SPACE] to scan the chip.\033[0m")
            print("\033[38;2;243;139;168m  -> Press [Q] or [ESC] to cancel.\033[0m\n")

            while True:
                ret, frame = cap.read()
                if not ret: break
                
                height, width = frame.shape[:2]
                cx, cy = width // 2, height // 2
                cv2.rectangle(frame, (cx - 100, cy - 50), (cx + 100, cy + 50), (0, 255, 0), 2)
                cv2.putText(frame, "ALIGN CHIP TEXT HERE", (cx - 90, cy - 60), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)

                cv2.imshow('Yuki Chip Scanner [Press Space]', frame)
                key = cv2.waitKey(1) & 0xFF
                if key == 32: 
                    frame_to_send = frame
                    console.print("[bold green]📸 Scan complete! Processing silicon topography...[/bold green]")
                    break
                elif key == ord('q') or key == 27: 
                    break
                    
            cap.release()
            cv2.destroyAllWindows()

            if frame_to_send is None: return

            _, buffer = cv2.imencode('.jpg', frame_to_send)
            img_b64 = base64.b64encode(buffer).decode('utf-8')
            
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.1)
            
            sys_msg = (
                "You are an expert ECE component analyzer. Look at the provided image of an electronic component or microchip. "
                "Read the text written on it to identify the exact part number (e.g., NE555, LM324, ATmega328). "
                "Respond ONLY with a highly structured markdown datasheet containing: "
                "1. **Component Name & Main Function**. "
                "2. **A clean, beautifully formatted ASCII Art Pinout Diagram** of the chip (YOU MUST ENCLOSE THE ASCII ART IN A ```text CODE BLOCK SO IT DOES NOT WORD WRAP). "
                "3. **Key Specs** (Operating voltage, max current). "
                "If you cannot read the text, state that the image is too blurry."
            )
            
            user_msg = HumanMessage(content=[
                {"type": "text", "text": sys_msg},
                {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{img_b64}"}}
            ])
            
            console.print("[dim magenta]⚡ Accessing global component databases...[/dim magenta]", end="\r")
            response = llm.invoke([user_msg])
            print("\033[K", end="") 
            display_ai_response(response.content)

        # --- V17.0: TERMINAL OSCILLOSCOPE ---
        elif provider == "plot":
            import serial
            port = prompt.strip()
            baud_rate = 115200
            width = 70
            height = 15
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
                    try: val = float(line)
                    except ValueError: continue 
                    data.append(val)    
                    
                    min_val, max_val = min(data), max(data)
                    val_range = max_val - min_val if max_val != min_val else 1
                    
                    sys.stdout.write(f"\033[{height + 4}A")
                    print(f"\033[1;36mYuki Oscilloscope ❯\033[0m Port: {port} | \033[38;2;166;227;161mCurrent: {val:.2f}\033[0m | Min: {min_val:.2f} | Max: {max_val:.2f}\033[K\n\033[K", end="")
                    
                    for y in range(height, -1, -1):
                        row_str = "│ "
                        threshold = min_val + (y / height) * val_range
                        for x in data: row_str += "\033[38;2;180;190;254m█\033[0m" if x >= threshold else " "
                        print(row_str + "\033[K")
                    
                    print("└" + "─" * (width + 1) + "\033[K")
                    print("\033[90mPress Ctrl+C to terminate telemetry.\033[0m\033[K")
                    time.sleep(0.03) 
                    
            except serial.SerialException as e:
                console.print(f"[bold red]Hardware Error: Could not connect to {port}.[/bold red]")
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
            frame_lock = threading.Lock()
            latest_frame = [None]
            running = [True]

            def capture_loop():
                cap = cv2.VideoCapture(0)
                if not cap.isOpened():
                    running[0] = False
                    return
                while running[0]:
                    ret, frame = cap.read()
                    if ret:
                        pulse = (math.sin(time.time() * 3.0) + 1) / 2
                        intensity = int(pulse * 205 + 50)
                        cv2.putText(frame, "yuki ai live", (15, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 0, intensity), 1, cv2.LINE_AA)
                        with frame_lock: latest_frame[0] = frame.copy()
                        cv2.imshow('Yuki Live [Interactive Stream]', frame)
                    if cv2.waitKey(1) & 0xFF in [27, ord('q')]:
                        running[0] = False
                        break
                cap.release()
                cv2.destroyAllWindows()

            threading.Thread(target=capture_loop, daemon=True).start()
            time.sleep(1.5) 
            if not running[0]: sys.exit(1)

            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.6)
            chat_history = [SystemMessage(content="You are Yuki, an interactive visual tutor.")]

            try:
                while running[0]:
                    user_input = input("\033[1;36mAman ❯ \033[0m")
                    if user_input.lower() in ['exit', 'quit', 'q']:
                        running[0] = False
                        break
                    if not user_input.strip(): continue
                    with frame_lock: frame_to_analyze = latest_frame[0].copy()
                    _, buffer = cv2.imencode('.jpg', frame_to_analyze)
                    img_b64 = base64.b64encode(buffer).decode('utf-8')
                    user_msg = HumanMessage(content=[{"type": "text", "text": user_input}, {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{img_b64}"}}])
                    response = llm.invoke(chat_history + [user_msg])
                    display_ai_response(response.content)
                    chat_history.extend([user_msg, response])
                    if len(chat_history) > 7: chat_history = [chat_history[0]] + chat_history[-6:]
            except KeyboardInterrupt: running[0] = False
            finally: os._exit(0) 

        # --- OTHER MODES ---
        elif provider == "auto": pass
        elif provider == "exec":
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.0)
            response = llm.invoke([SystemMessage(content="Output ONLY the raw bash command."), HumanMessage(content=prompt)])
            print(response.content.replace("```bash", "").replace("```sh", "").replace("```", "").strip())
            
        # --- PHASE 19.2: PAC-MAN LOADER, FLUID UI & LIVE MARKDOWN ---
        else: 
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.7)
            
            # 1. The Threaded Pac-Man Spinner Function
            def animated_thinking(stop_event):
                import itertools
                # Pac-Man (Yellow) eating dots (Green) with exact spacing to prevent jitter
                frames = [
                    "\033[38;2;249;226;175mᗧ\033[0m \033[38;2;166;227;161m• • •\033[0m  ",
                    "\033[38;2;249;226;175mO\033[0m \033[38;2;166;227;161m • •\033[0m   ",
                    "\033[38;2;249;226;175mᗧ\033[0m \033[38;2;166;227;161m  •\033[0m    ",
                    "\033[38;2;249;226;175mO\033[0m        "
                ]
                statuses = ["Pinging Gemini Core...", "Analyzing Context...", "Allocating Memory...", "Formatting Output..."]
                frame_cycle = itertools.cycle(frames)
                status_cycle = itertools.cycle(statuses)
                idx = 0
                while not stop_event.is_set():
                    if idx % 15 == 0: current_status = next(status_cycle)
                    sys.stdout.write(f"\r  {next(frame_cycle)} \033[38;2;180;190;254m[ {current_status} ]\033[0m\033[K")
                    sys.stdout.flush()
                    time.sleep(0.12) # Slightly slower for the perfect munching speed
                    idx += 1

            stop_event = threading.Event()
            spinner_thread = threading.Thread(target=animated_thinking, args=(stop_event,))
            print("\n")
            spinner_thread.start()

            try:
                response_stream = llm.stream([HumanMessage(content=prompt)])
                first_chunk = next(response_stream)
                stop_event.set()
                spinner_thread.join()
                
                sys.stdout.write("\r\033[K")
                if console: console.print("[bold magenta]━━━ Yuki AI Response ━━━[/bold magenta]\n")
                else: print("\033[1;35m━━━ Yuki AI Response ━━━\033[0m\n")
                
                buffer = first_chunk.content
                is_bold, is_code = False, False
                
                def flush_buffer(buf, bold_state, code_state):
                    while len(buf) > 0:
                        if buf.startswith("**"):
                            bold_state = not bold_state
                            sys.stdout.write("\033[1m" if bold_state else "\033[0m")
                            buf = buf[2:]
                            continue
                        if buf == "*": break 
                        if buf.startswith("* "):
                            sys.stdout.write("\033[38;2;137;180;250m•\033[0m ") 
                            buf = buf[2:]
                            continue
                        if buf == "`": break 
                        if buf.startswith("`"):
                            code_state = not code_state
                            sys.stdout.write("\033[33m" if code_state else "\033[0m")
                            buf = buf[1:]
                            continue
                        
                        sys.stdout.write(buf[0])
                        sys.stdout.flush()
                        buf = buf[1:]
                        time.sleep(0.003) 
                    return buf, bold_state, code_state

                buffer, is_bold, is_code = flush_buffer(buffer, is_bold, is_code)
                for chunk in response_stream:
                    buffer += chunk.content
                    buffer, is_bold, is_code = flush_buffer(buffer, is_bold, is_code)
                
                if buffer:
                    sys.stdout.write(buffer.replace("**", "").replace("*", ""))
                    sys.stdout.flush()
                
                print("\n\n")
                if console: console.print("[bold magenta]━━━━━━━━━━━━━━━━━━━━━━━━[/bold magenta]\n")
                else: print("\033[1;35m━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n")

            except StopIteration:
                stop_event.set()
                sys.stdout.write("\r\033[K\033[31m[!] No response generated.\033[0m\n")
            except Exception as e:
                stop_event.set()
                spinner_thread.join()
                sys.stdout.write(f"\r\033[K\033[31m[!] API Error: {e}\033[0m\n")

    except Exception as e:
        if console: console.print(f"[bold red][!] API Error:[/bold red] {e}")
        else: print(f"\033[31m[!] API Error: {e}\033[0m")

if __name__ == "__main__":
    main()
