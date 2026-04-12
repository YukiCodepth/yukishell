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
    pass # (Legacy tool logic hidden for brevity, handled by new modes)

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
                
                # Add a scanning reticle to the center of the camera
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

            if frame_to_send is None: 
                return

            _, buffer = cv2.imencode('.jpg', frame_to_send)
            img_b64 = base64.b64encode(buffer).decode('utf-8')
            
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.1) # Low temp for factual accuracy
            
            # The God-Tier Prompt Engineering
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
                    try:
                        val = float(line)
                        data.append(val)
                    except ValueError:
                        continue 
                        
                    min_val = min(data)
                    max_val = max(data)
                    val_range = max_val - min_val if max_val != min_val else 1
                    
                    sys.stdout.write(f"\033[{height + 4}A")
                    print(f"\033[1;36mYuki Oscilloscope ❯\033[0m Port: {port} | \033[38;2;166;227;161mCurrent: {val:.2f}\033[0m | Min: {min_val:.2f} | Max: {max_val:.2f}\033[K\n\033[K", end="")
                    
                    for y in range(height, -1, -1):
                        row_str = "│ "
                        threshold = min_val + (y / height) * val_range
                        for x in data:
                            if x >= threshold:
                                row_str += "\033[38;2;180;190;254m█\033[0m"
                            else:
                                row_str += " "
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
                        with frame_lock:
                            latest_frame[0] = frame.copy()
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
                    with frame_lock:
                        frame_to_analyze = latest_frame[0].copy()
                    _, buffer = cv2.imencode('.jpg', frame_to_analyze)
                    img_b64 = base64.b64encode(buffer).decode('utf-8')
                    user_msg = HumanMessage(content=[{"type": "text", "text": user_input}, {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{img_b64}"}}])
                    response = llm.invoke(chat_history + [user_msg])
                    display_ai_response(response.content)
                    chat_history.extend([user_msg, response])
                    if len(chat_history) > 7: chat_history = [chat_history[0]] + chat_history[-6:]
            except KeyboardInterrupt:
                running[0] = False
            finally:
                os._exit(0) 

        # --- OTHER MODES ---
        elif provider == "auto":
            # Compacted for brevity
            pass
        elif provider == "exec":
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.0)
            response = llm.invoke([SystemMessage(content="Output ONLY the raw bash command."), HumanMessage(content=prompt)])
            print(response.content.replace("```bash", "").replace("```sh", "").replace("```", "").strip())
        else: 
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.7)
            display_ai_response(llm.invoke([HumanMessage(content=prompt)]).content)

    except Exception as e:
        console.print(f"[bold red][!] API Error:[/bold red] {e}")

if __name__ == "__main__":
    main()
