import sys
import os
import subprocess
import warnings
from dotenv import load_dotenv

# Suppress annoying LangChain/LangGraph deprecation warnings to keep the terminal clean
warnings.filterwarnings("ignore")

load_dotenv()

from langchain_openai import ChatOpenAI
from langchain_google_genai import ChatGoogleGenerativeAI
from langchain_anthropic import ChatAnthropic
from langchain_core.messages import HumanMessage, SystemMessage
from langchain_community.tools.tavily_search import TavilySearchResults

from langgraph.prebuilt import create_react_agent
from langchain_core.tools import tool

@tool
def terminal_executor(command: str) -> str:
    """Executes a bash command in the Linux terminal and returns the output. Use this to navigate, read files, or modify the system."""
    print(f"\x1b[31m\x1b[1m[⚙️ Agent Executing]:\x1b[0m {command}")
    try:
        result = subprocess.run(command, shell=True, capture_output=True, text=True, timeout=10)
        output = result.stdout + result.stderr
        return output[:2000] if output else "Command successful. No output."
    except subprocess.TimeoutExpired:
        return "Error: Command timed out. Never run interactive commands like nano, vim, or top."
    except Exception as e:
        return f"Error: {e}"

def main():
    if len(sys.argv) < 3:
        print("\033[31mError: Missing arguments from C Shell.\033[0m")
        sys.exit(1)

    provider = sys.argv[1].lower()
    prompt = sys.argv[2]
    
    gemini_version = os.environ.get("GEMINI_MODEL", "gemini-2.5-flash")
    
    if provider not in ["exec", "auto"]:
        print(f"\n\x1b[35m\x1b[1m[ Yuki AI | Engine: {provider.upper()} ]\x1b[0m Thinking...\n")

    try:
        # --- Phase 2: Autonomous Agent Mode ---
        if provider == "auto":
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.2)
            tools = [terminal_executor]
            
            system_instruction = "You are an autonomous Linux AI agent. You have full terminal access. Fulfill the user's request by running commands. Analyze the output and take the next step. If a command fails, try to fix it. When the goal is complete, summarize what you did."
            
            agent_executor = create_react_agent(
                model=llm, 
                tools=tools, 
                prompt=system_instruction
            )

            try:
                response = agent_executor.invoke({"messages": [("user", prompt)]})
                
                # Cleanly extract the final AI message (Handles complex dictionaries)
                raw_content = response["messages"][-1].content
                if isinstance(raw_content, list):
                    final_output = "".join([item.get("text", "") if isinstance(item, dict) else str(item) for item in raw_content])
                else:
                    final_output = str(raw_content)
                    
                print(f"\n\033[32m\x1b[1m[Agent Finished]:\033[0m {final_output.strip()}\n")
                
            except KeyboardInterrupt:
                print("\n\x1b[31m\x1b[1m[!] Override Authorized. Revoking AI Terminal Access.\x1b[0m\n")
                sys.exit(1)

        # --- Phase 1: Smart Execute ---
        elif provider == "exec":
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.0)
            sys_msg = "You are a Linux terminal expert. Output ONLY the raw, valid bash command. No markdown, no backticks, no text."
            response = llm.invoke([SystemMessage(content=sys_msg), HumanMessage(content=prompt)])
            print(response.content.strip().replace("```bash", "").replace("```", "").strip())
            
        # --- Standard Search ---
        elif provider == "search":
            tavily = TavilySearchResults(max_results=3)
            search_context = tavily.invoke({"query": prompt})
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.7)
            augmented_prompt = f"Answer based ONLY on these search results.\n\nResults:\n{search_context}\n\nQuestion: {prompt}"
            response = llm.invoke([HumanMessage(content=augmented_prompt)])
            print(f"\033[32mResponse:\033[0m {response.content}\n")
            
        # --- Standard Chat ---
        else: 
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.7)
            response = llm.invoke([HumanMessage(content=prompt)])
            print(f"\033[32mResponse:\033[0m {response.content}\n")

    except Exception as e:
        if provider == "exec":
            print(f"echo 'AI Exec Error: {e}'") 
        else:
            print(f"\033[31m[!] API Error:\033[0m {e}")

    sys.stdout.flush()

if __name__ == "__main__":
    main()