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
    console.print(f"[bold yellow]📷 AI Vision Activating:[/bold yellow] Opening Viewfinder...")
    try:
        os.environ["QT_LOGGING_RULES"] = "*=false" 
        os.environ["OPENCV_LOG_LEVEL"] = "FATAL"
        import cv2

        cap = cv2.VideoCapture(0)
        if not cap.isOpened(): return "Vision Error: Could not access webcam."

        frame_to_send = None
        print("\n\033[38;2;137;180;250m[Yuki Viewfinder] Feed started.\033[0m")
        print("\033[38;2;166;227;161m  -> Press [SPACE] to capture the photo.\033[0m")
        print("\033[38;2;243;139;168m  -> Press [Q] or [ESC] to cancel.\033[0m\n")

        devnull = os.open(os.devnull, os.O_WRONLY)
        old_stderr = os.dup(2)
        os.dup2(devnull, 2)

        try:
            while True:
                ret, frame = cap.read()
                if not ret: break
                cv2.imshow('Yuki Hardware Vision [Press Space to Snap]', frame)
                key = cv2.waitKey(1) & 0xFF
                if key == 32: 
                    frame_to_send = frame
                    console.print("[bold green]📸 Snapshot captured![/bold green]")
                    break
                elif key == ord('q') or key == 27: break
            cap.release()
            cv2.destroyAllWindows()
            cv2.waitKey(1) 
        finally:
            os.dup2(old_stderr, 2)
            os.close(devnull)
            os.close(old_stderr)

        if frame_to_send is None: return "Vision capture cancelled."
        _, buffer = cv2.imencode('.jpg', frame_to_send)
        img_b64 = base64.b64encode(buffer).decode('utf-8')
        
        vision_llm = ChatGoogleGenerativeAI(model=os.environ.get("GEMINI_MODEL", "gemini-2.0-flash"))
        msg = HumanMessage(content=[
            {"type": "text", "text": f"Agent asks: '{query}'. Describe this webcam capture. Focus on electronics."},
            {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{img_b64}"}}
        ])
        return vision_llm.invoke([msg]).content
    except Exception as e: return f"Vision Error: {e}"

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
    console.print(f"[bold cyan]📖 AI Reading:[/bold cyan] {filepath}")
    try:
        with open(filepath, 'r') as f: return f.read()[:3000]
    except Exception as e: return f"Error reading file: {e}"

@tool
def terminal_executor(command: str) -> str:
    """Executes a bash command."""
    console.print(f"[bold red]⚙️ Agent Executing:[/bold red] {command}")
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
        # --- V17.0: TERMINAL OSCILLOSCOPE ---
        if provider == "plot":
            import serial
            
            port = prompt.strip()
            baud_rate = 115200
            
            width = 70
            height = 15
            data = deque([0] * width, maxlen=width)
            
            console.print(f"\n[bold green]🟢 Yuki Telemetry Initiated.[/bold green]")
            console.print(f"[dim]Attempting to lock onto {port} at {baud_rate} baud...[/dim]\n")
            
            try:
                ser = serial.Serial(port, baud_rate, timeout=1)
                time.sleep(1.5) 
                
                # Print blank lines to make room for the render block
                print("\n" * (height + 4))
                
                # Hide the terminal cursor for smooth rendering
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
                    
                    # Move cursor UP by the exact height of our render block
                    sys.stdout.write(f"\033[{height + 4}A")
                    
                    # \033[K clears the line to prevent ghosting from previous frames
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
                # Restore the cursor before exiting
                sys.stdout.write("\033[?25h")
                console.print("\n[bold red]🔴 Telemetry Terminated.[/bold red]")
                if 'ser' in locals(): ser.close()

        # --- V16.0: INTERACTIVE VISUAL TUTOR ---
        elif provider == "live":
            import cv2
            
            console.print("\n[bold green]🟢 Yuki Visual Tutor Initiated.[/bold green]")
            console.print("[dim]Optical sensors active. Type your questions below. Type 'exit' to close.[/dim]\n")

            os.environ["QT_LOGGING_RULES"] = "*=false" 
            os.environ["OPENCV_LOG_LEVEL"] = "FATAL"

            frame_lock = threading.Lock()
            latest_frame = [None]
            running = [True]

            def capture_loop():
                cap = cv2.VideoCapture(0)
                if not cap.isOpened():
                    console.print("[bold red]Fatal: Could not access the webcam.[/bold red]")
                    running[0] = False
                    return
                while running[0]:
                    ret, frame = cap.read()
                    if ret:
                        pulse = (math.sin(time.time() * 3.0) + 1) / 2
                        intensity = int(pulse * 205 + 50)
                        hud_color = (0, 0, intensity)
                        cv2.putText(frame, "yuki ai live", (15, 25), cv2.FONT_HERSHEY_SIMPLEX, 
                                    0.4, hud_color, 1, cv2.LINE_AA)

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
            sys_text = (
                "You are Yuki, an interactive visual tutor and engineering assistant for Aman. "
                "You are viewing a continuous live camera feed. Aman will ask you questions about what is currently on the screen. "
                "Analyze the image and answer in a helpful, educational, and conversational tone. "
                "Use markdown to format your explanations clearly. Speak as if you are looking at the object live with him."
            )
            chat_history = [SystemMessage(content=sys_text)]

            try:
                while running[0]:
                    user_input = input("\033[1;36mAman ❯ \033[0m")
                    if user_input.lower() in ['exit', 'quit', 'q']:
                        running[0] = False
                        break
                    if not user_input.strip(): continue
                    
                    with frame_lock:
                        if latest_frame[0] is None:
                            console.print("[bold red]Camera is still warming up...[/bold red]")
                            continue
                        frame_to_analyze = latest_frame[0].copy()

                    _, buffer = cv2.imencode('.jpg', frame_to_analyze)
                    img_b64 = base64.b64encode(buffer).decode('utf-8')

                    console.print("[dim magenta]⚡ Analyzing video feed...[/dim magenta]", end="\r")

                    user_msg = HumanMessage(content=[
                        {"type": "text", "text": user_input},
                        {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{img_b64}"}}
                    ])

                    response = llm.invoke(chat_history + [user_msg])

                    print("\033[K", end="") 
                    display_ai_response(response.content)

                    chat_history.extend([user_msg, response])
                    if len(chat_history) > 7:
                        chat_history = [chat_history[0]] + chat_history[-6:]

            except KeyboardInterrupt:
                running[0] = False
            finally:
                console.print("\n[bold red]🔴 Yuki Live Terminated.[/bold red]")
                os._exit(0) 

        elif provider == "auto":
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.1)
            tools = [terminal_executor, web_search, read_file, hardware_vision]
            sys_inst = "You are Yuki-Agent. You have terminal, web, file, and vision access. Use Markdown."
            agent_executor = create_react_agent(model=llm, tools=tools, prompt=sys_inst)
            response = agent_executor.invoke({"messages": [("user", prompt)]})
            content = response["messages"][-1].content
            if isinstance(content, list):
                content = " ".join([item.get("text", "") if isinstance(item, dict) else str(item) for item in content])
            display_ai_response(str(content))

        elif provider == "exec":
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.0)
            sys_msg = "Output ONLY the raw bash command. No markdown formatting, no backticks, no explanations."
            response = llm.invoke([SystemMessage(content=sys_msg), HumanMessage(content=prompt)])
            clean_cmd = response.content.replace("```bash", "").replace("```sh", "").replace("```", "").strip()
            print(clean_cmd)
            
        else: 
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.7)
            console.print(f"\n[bold magenta]🤔 Thinking...[/bold magenta]")
            display_ai_response(llm.invoke([HumanMessage(content=prompt)]).content)

    except Exception as e:
        console.print(f"[bold red][!] API Error:[/bold red] {e}")

if __name__ == "__main__":
    main()
