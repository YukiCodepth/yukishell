import sys
import os
import subprocess
import warnings
from dotenv import load_dotenv

# UI Library for Claude-style formatting
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
        # Creating a subtle border/separator like Claude
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
        if provider == "auto":
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.1)
            tools = [terminal_executor, web_search, read_file]
            system_instruction = "You are Yuki-Agent. Use Markdown for all responses (headers, bold, lists). Be structured like Claude."
            
            agent_executor = create_react_agent(model=llm, tools=tools, prompt=system_instruction)
            response = agent_executor.invoke({"messages": [("user", prompt)]})
            
            content = response["messages"][-1].content
            if isinstance(content, list):
                content = " ".join([item.get("text", "") if isinstance(item, dict) else str(item) for item in content])
            
            display_ai_response(str(content))

        elif provider == "exec":
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.0)
            response = llm.invoke([SystemMessage(content="Output ONLY raw bash."), HumanMessage(content=prompt)])
            print(response.content.strip())
            
        else: 
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.7)
            console.print(f"\n[bold magenta]🤔 Thinking...[/bold magenta]")
            response = llm.invoke([HumanMessage(content=prompt)])
            display_ai_response(response.content)

    except Exception as e:
        console.print(f"[bold red][!] API Error:[/bold red] {e}")

if __name__ == "__main__":
    main()
