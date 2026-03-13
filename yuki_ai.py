import sys
import os
from dotenv import load_dotenv

load_dotenv()

from langchain_openai import ChatOpenAI
from langchain_google_genai import ChatGoogleGenerativeAI
from langchain_anthropic import ChatAnthropic
from langchain_core.messages import HumanMessage
from langchain_community.tools.tavily_search import TavilySearchResults

def main():
    if len(sys.argv) < 3:
        print("\033[31mError: Missing arguments from C Shell.\033[0m")
        sys.exit(1)

    provider = sys.argv[1].lower()
    prompt = sys.argv[2]
    
    # 1. Dynamically load the models from your .env file!
    # (If the .env variable is missing, it falls back to a safe default)
    gemini_version = os.environ.get("GEMINI_MODEL", "gemini-2.5-flash")
    openai_version = os.environ.get("OPENAI_MODEL", "gpt-4o-mini")
    claude_version = os.environ.get("CLAUDE_MODEL", "claude-3-haiku-20240307")

    print(f"\n\x1b[35m\x1b[1m[ Yuki AI | Engine: {provider.upper()} | Model: {gemini_version if provider in ['gemini', 'search'] else (openai_version if provider == 'openai' else claude_version)} ]\x1b[0m Thinking...\n")

    try:
        if provider == "search":
            if not os.environ.get("TAVILY_API_KEY"):
                print("\033[31m[!] Error: TAVILY_API_KEY missing in .env\033[0m")
                sys.exit(1)
            
            print("\x1b[33m[*] Searching the web via Tavily...\x1b[0m")
            tavily = TavilySearchResults(max_results=3)
            search_context = tavily.invoke({"query": prompt})
            
            # Feed the search results to Gemini
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.7)
            augmented_prompt = f"Answer the user's question based ONLY on these web search results.\n\nSearch Results:\n{search_context}\n\nUser Question: {prompt}"
            message = HumanMessage(content=augmented_prompt)
            response = llm.invoke([message])
            
        elif provider == "openai":
            llm = ChatOpenAI(model=openai_version, temperature=0.7)
            message = HumanMessage(content=prompt)
            response = llm.invoke([message])
            
        elif provider == "claude":
            llm = ChatAnthropic(model=claude_version, temperature=0.7)
            message = HumanMessage(content=prompt)
            response = llm.invoke([message])
            
        else: 
            # Default to Gemini
            llm = ChatGoogleGenerativeAI(model=gemini_version, temperature=0.7)
            message = HumanMessage(content=prompt)
            response = llm.invoke([message])

        print(f"\033[32mResponse:\033[0m {response.content}\n")

    except Exception as e:
        # Custom Error Catching for easier debugging
        if "insufficient_quota" in str(e) or "429" in str(e):
            print(f"\033[31m[!] API Quota Error:\033[0m Your {provider.upper()} account is out of credits. Try using --gemini instead.")
        elif "404" in str(e) or "NOT_FOUND" in str(e):
            print(f"\033[31m[!] Model Not Found:\033[0m Google/OpenAI rejected the model name. Check the spelling in your .env file!")
        else:
            print(f"\033[31m[!] API Error:\033[0m {e}")

    sys.stdout.flush()

if __name__ == "__main__":
    main()
