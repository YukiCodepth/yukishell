import sys
import os
import subprocess
import warnings
import time
from dotenv import load_dotenv

# --- UI Library for Claude-style formatting ---
try:
    from rich.console import Console
    from rich.markdown import Markdown
    console = Console()
except ImportError:
    console = None

# Suppress warnings to keep the terminal aesthetic clean
warnings.filterwarnings("ignore")
load_dotenv()

from langchain_google_genai import ChatGoogleGenerativeAI
from langchain_core.messages import HumanMessage, SystemMessage
from langchain_community.tools.tavily_search import TavilySearchResults
from langgraph.prebuilt import create_react_agent
from langchain_core.tools import tool

# --- V15: The Interactive Viewfinder Tool (Kept for --auto) ---
@tool
def hardware_vision(query: str) -> str:
    """Captures a live image from the webcam to analyze physical hardware.
    Pass a specific query about what to look for."""
    console.print(f"[bold yellow]📷 AI Vision Activating:[/bold yellow] Opening Live Viewfinder...")
    try:
        os.environ["QT_LOGGING_RULES"] = "*=false" 
        os.environ["OPENCV_LOG_LEVEL"] = "FATAL"
        
        import cv2
        import base64

        cap = cv2.VideoCapture(0)
        if not cap.isOpened():
            return "Vision Error: Could not access the webcam."

        frame_to_send = None

        print("\n\033[38;2;137;180;250m[Yuki Viewfinder] Live feed started.\033[0m")
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
                if key == 32: # Spacebar
                    frame_to_send = frame
                    console.print("[bold green]📸 Snapshot captured![/bold green]")
                    break
                elif key == ord('q') or key == 27: 
                    console.print("[bold red]❌ Vision capture cancelled by user.[/bold red]")
                    break
            
            cap.release()
            cv2.destroyAllWindows()
            cv2.waitKey(1) 
            
        finally:
            os.dup2(old_stderr, 2)
            os.close(devnull)
            os.close(old_stderr)

        if frame_to_send is None:
            return "Vision capture was cancelled by the user. Ask them what they want to do next."

        _, buffer = cv2.imencode('.jpg', frame_to_send)
        img_b64 = base64.b64encode(buffer).decode('utf-8')

        console.print(f"[bold yellow]👁️  AI Analyzing Image...[/bold yellow]")
        
        vision_llm = ChatGoogleGenerativeAI(model=os.environ.get("GEMINI_MODEL", "gemini-2.0-flash"))
        
        # Formatted safely to prevent line-wrap syntax errors
        prompt_text = (
            "You are the vision module for an ECE engineering AI. "
            f"The agent asks: '{query}'. Describe what you see in this webcam "
            "capture to answer the question. Focus strictly on electronics, "
            "microcontrollers, wiring, or screens."
        )
        
        msg = HumanMessage(content=[
            {"type": "text", "text": prompt_text},
            {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{img_b64}"}}
        ])
        
        response = vision_llm.invoke([msg])
        return response.content
        
    except ImportError:
        return "Vision Error: cv2 not installed. Run 'pip install opencv-python'"
    except Exception as e:
        return f"Vision Error: {e}"

@tool
def web_search(query: str) -> str:
    """Searches the live internet for documentation or error fixes."""
    console.print(f"[bold blue]🔍 AI Searching:[/bold blue] {query}")
    try:
        tavily = TavilySearchResults(max_results=3)
        return str(tavily.invoke({"query": query}))
    except Exception as e:
        return f"Search failed: {e}"

@tool
def read_file(filepath: str) -> str:
    """Reads the contents of a local file silently."""
    console.print(f"[bold cyan]📖 AI Reading:[/bold cyan] {filepath}")
    try:
        with open(filepath, 'r') as f:
            return f.read()[:3000]
    except Exception as e:
        return f"Error reading file: {e}"

@tool
def terminal_executor(command: str) -> str:
    """Executes a bash command in the Linux terminal."""
    console.print(f"[bold red]⚙️ Agent Executing:[/bold red] {command}")
    try:
        result = subprocess.run(command, shell=True, capture_output=True, text=True, timeout=15)
        return (result.stdout + result.stderr)[:2000] or "Success."
    except Exception as e:
        return f"Error: {e}"

def display_ai_response(content):
    """Helper to render beautiful Markdown in the terminal"""
    if console:
        print("\n")
        console.print("[bold magenta]━━━ Yuki AI Response ━━━[/bold magenta]")
        md = Markdown(content)
        console.print(md)
        console.print("[bold magenta]━━━━━━━━━━━━━━━━━━━━━━━━[/bold magenta]\n")
    else:
        print(f"\n[Agent Finished]: {content}\n")

def main():
    if len(sys.argv) < 3:
        sys.exit(1)

    provider = sys.argv[1].lower()
    prompt = sys.argv[2]
    gemini_version = os.environ.get("GEMINI_MODEL", "gemini-2.0-flash")
    
    try:
        # --- V16: YUKI LIVE ENGINE (Continuous Video Stream) ---
        if provider == "live":
            import cv2
            import base64

            console.print("\n[bold green]🟢 Yuki Live Engine Initiated.[/bold green]")
            console.print("[dim]Connecting to optical sensors... Press Ctrl+C to terminate.[/dim]\n")

            os.environ["QT_LOGGING_RULES"] = "*=false" 
            os.environ["OPENCV_LOG_LEVEL"] = "FATAL"

            cap = cv2.VideoCapture(0)
            if not cap.isOpened():
                console.print("[bold red]Fatal: Could not access the webcam.[/bold red]")
                sys.exit(1)

            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.6)
            
            # Formatted safely to prevent line-wrap syntax errors
            sys_text = (
                "You are Yuki Live, a real-time environmental awareness AI "
                "for an ECE engineer. You are receiving a continuous 3-second "
                "interval stream of webcam frames. Describe what is happening. "
                "If nothing changes, give a very brief acknowledgment. If you "
                "see a circuit, describe the components. Keep your responses short, "
                "conversational, and analytical (1-2 sentences max). "
                "Do NOT use markdown headers or bold text."
            )
            system_msg = SystemMessage(content=sys_text)
            
            chat_history = [system_msg]

            try:
                while True:
                    # Flush the video buffer to get the absolute newest frame
                    for _ in range(5): cap.read() 
                    ret, frame = cap.read()
                    
                    if not ret:
                        continue

                    _, buffer = cv2.imencode('.jpg', frame)
                    img_b64 = base64.b64encode(buffer).decode('utf-8')

                    console.print("[dim cyan]⚡ Processing optical feed...[/dim cyan]", end="\r")

                    user_msg = HumanMessage(content=[
                        {"type": "text", "text": "New frame received. What is happening right now?"},
                        {"type": "image_url", "image_url": {"url": f"data:image/jpeg;base64,{img_b64}"}}
                    ])

                    full_context = chat_history + [user_msg]
                    response = llm.invoke(full_context)

                    # Clear the loading line and print the live commentary
                    print("\033[K", end="") 
                    console.print(f"[bold magenta]Yuki Live ❯[/bold magenta] {response.content}")

                    # Append to rolling memory
                    chat_history.append(user_msg)
                    chat_history.append(response)

                    # Keep memory tight to prevent API payload bloat
                    if len(chat_history) > 5:
                        chat_history = [chat_history[0]] + chat_history[-4:]

                    time.sleep(3)

            except KeyboardInterrupt:
                console.print("\n\n[bold red]🔴 Optical Feed Terminated. Shutting down Yuki Live.[/bold red]")
            finally:
                cap.release()

        # --- V14/15: The Autonomous Agent Mode (Uses Viewfinder) ---
        elif provider == "auto":
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.1)
            tools = [terminal_executor, web_search, read_file, hardware_vision]
            
            system_instruction = (
                "You are Yuki-Agent. You have terminal, web, file, and vision access. "
                "If the user asks about physical hardware, wiring, or what is in front of the computer, "
                "use the 'hardware_vision' tool to take a picture. Use Markdown for all responses."
            )
            
            agent_executor = create_react_agent(model=llm, tools=tools, prompt=system_instruction)
            response = agent_executor.invoke({"messages": [("user", prompt)]})
            
            content = response["messages"][-1].content
            if isinstance(content, list):
                content = " ".join([item.get("text", "") if isinstance(item, dict) else str(item) for item in content])
            
            display_ai_response(str(content))

        # --- Phase 1: Smart Execute ---
        elif provider == "exec":
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.0)
            response = llm.invoke([SystemMessage(content="Output ONLY raw bash."), HumanMessage(content=prompt)])
            print(response.content.strip())
            
        # --- Standard Chat/Search ---
        else: 
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.7)
            console.print(f"\n[bold magenta]🤔 Thinking...[/bold magenta]")
            response = llm.invoke([HumanMessage(content=prompt)])
            display_ai_response(response.content)

    except Exception as e:
        console.print(f"[bold red][!] API Error:[/bold red] {e}")

if __name__ == "__main__":
    main()
