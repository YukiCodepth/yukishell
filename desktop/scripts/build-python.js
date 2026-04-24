const childProcess = require("child_process");
const fs = require("fs");
const path = require("path");

const root = path.resolve(__dirname, "..");
const target = path.join(root, "python");
const requirements = path.join(root, "requirements-python.txt");

function commandOk(command, args) {
  const result = childProcess.spawnSync(command, args, { stdio: "ignore" });
  return result.status === 0;
}

function findPython() {
  const candidates = [
    process.env.YUKISHELL_PYTHON,
    "/opt/homebrew/bin/python3.12",
    "/opt/homebrew/bin/python3.11",
    "python3.12",
    "python3.11",
    "python3"
  ].filter(Boolean);

  for (const candidate of candidates) {
    if (commandOk(candidate, ["-c", "import sys; raise SystemExit(0 if sys.version_info >= (3, 11) else 1)"])) {
      return candidate;
    }
  }

  throw new Error("No usable Python 3.11+ interpreter found.");
}

function hasImports(python, stdio = "ignore") {
  if (!fs.existsSync(target)) return false;
  const code = [
    "import sys",
    "import importlib.util",
    `sys.path.insert(0, ${JSON.stringify(target)})`,
    "mods=['dotenv','rich','langchain_google_genai','langchain_core','langchain_community','langgraph','serial']",
    "for m in mods:",
    "    __import__(m)",
    "    print(m, 'ok')",
    "native_optional=['sounddevice']",
    "for m in native_optional:",
    "    if importlib.util.find_spec(m) is None:",
    "        raise SystemExit(f'{m} missing')",
    "    print(m, 'present')"
  ].join("\n");
  const result = childProcess.spawnSync(python, ["-c", code], { stdio });
  return result.status === 0;
}

const python = findPython();

if (hasImports(python)) {
  removePycache(target);
  console.log("Python dependency bundle is ready.");
  process.exit(0);
}

fs.rmSync(target, { recursive: true, force: true });
fs.mkdirSync(target, { recursive: true });

const install = childProcess.spawnSync(python, [
  "-m",
  "pip",
  "install",
  "--upgrade",
  "--target",
  target,
  "-r",
  requirements
], { stdio: "inherit" });

if (install.status !== 0) {
  process.exit(install.status || 1);
}

if (!hasImports(python, "inherit")) {
  console.error("Python dependencies installed but import verification failed.");
  process.exit(1);
}

removePycache(target);
console.log("Python dependency bundle installed.");

function removePycache(dir) {
  if (!fs.existsSync(dir)) return;
  for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
    const fullPath = path.join(dir, entry.name);
    if (entry.isDirectory()) {
      if (entry.name === "__pycache__") {
        fs.rmSync(fullPath, { recursive: true, force: true });
      } else {
        removePycache(fullPath);
      }
    } else if (entry.name.endsWith(".pyc") || entry.name.endsWith(".pyo")) {
      fs.rmSync(fullPath, { force: true });
    }
  }
}
