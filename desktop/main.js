const { app, BrowserWindow, ipcMain, shell, systemPreferences, session } = require("electron");
const path = require("path");
const fs = require("fs");
const os = require("os");
const crypto = require("crypto");
const http = require("http");
const childProcess = require("child_process");
const pty = require("node-pty");

const isDev = !app.isPackaged;
const repoRoot = path.resolve(__dirname, "..");
const activeSessions = new Map();
const cameraBridge = {
  server: null,
  url: "",
  token: crypto.randomBytes(24).toString("hex"),
  latestFrame: "",
  updatedAt: 0,
  status: "starting",
  error: ""
};

function corePath() {
  if (isDev) return path.join(repoRoot, process.platform === "win32" ? "yukishell.exe" : "yukishell");
  return path.join(process.resourcesPath, "core", process.platform === "win32" ? "yukishell.exe" : "yukishell");
}

function coreCwd() {
  if (isDev) return repoRoot;
  return path.join(process.resourcesPath, "core");
}

function bundledPythonPath() {
  if (isDev) return path.join(__dirname, "python");
  return path.join(process.resourcesPath, "python");
}

function pythonBinaryPath() {
  const candidates = [
    process.env.YUKISHELL_PYTHON,
    "/opt/homebrew/bin/python3.12",
    "/opt/homebrew/bin/python3.11",
    "/usr/local/bin/python3.12",
    "/usr/local/bin/python3.11",
    "python3.12",
    "python3.11",
    "python3"
  ].filter(Boolean);

  for (const candidate of candidates) {
    const result = childProcess.spawnSync(candidate, [
      "-c",
      "import sys; raise SystemExit(0 if sys.version_info >= (3, 11) else 1)"
    ], { stdio: "ignore" });
    if (result.status === 0) return candidate;
  }

  return "python3";
}

function envFilePath() {
  const nearbyEnv = findUp(process.resourcesPath || __dirname, ".env");
  const candidates = isDev
    ? [path.join(repoRoot, ".env")]
    : [
        process.env.YUKISHELL_ENV_FILE,
        path.join(app.getPath("userData"), ".env"),
        path.join(os.homedir(), ".yukishell", ".env"),
        nearbyEnv,
        path.join(process.resourcesPath, "core", ".env")
      ];

  for (const candidate of candidates.filter(Boolean)) {
    if (fs.existsSync(candidate)) return candidate;
  }

  return isDev ? path.join(repoRoot, ".env") : path.join(app.getPath("userData"), ".env");
}

function findUp(startDir, filename) {
  let current = path.resolve(startDir);
  for (let depth = 0; depth < 8; depth++) {
    const candidate = path.join(current, filename);
    if (fs.existsSync(candidate)) return candidate;
    const parent = path.dirname(current);
    if (parent === current) break;
    current = parent;
  }
  return null;
}

function commandExists(command) {
  try {
    const checker = process.platform === "win32" ? "where" : "command";
    const args = process.platform === "win32" ? [command] : ["-v", command];
    const result = childProcess.spawnSync(checker, args, { stdio: "ignore", shell: process.platform !== "win32" });
    return result.status === 0;
  } catch (_) {
    return false;
  }
}

function resolveBackend() {
  const binary = corePath();
  const cwd = os.homedir();

  if (fs.existsSync(binary)) {
    if (process.platform !== "win32") {
      try {
        fs.chmodSync(binary, 0o755);
      } catch (_) {
        // Best effort. The PTY spawn below will surface permission errors.
      }
    }
    return {
      command: binary,
      args: [],
      cwd,
      label: "YukiShell"
    };
  }

  if (process.platform === "win32" && commandExists("wsl.exe")) {
    return {
      command: "wsl.exe",
      args: ["--cd", "~", "bash", "-lc", "yukishell || ./yukishell || bash"],
      cwd,
      label: "YukiShell"
    };
  }

  const fallbackShell = process.platform === "win32"
    ? "powershell.exe"
    : process.env.SHELL || "/bin/bash";

  return {
    command: fallbackShell,
    args: [],
    cwd,
    label: "Terminal"
  };
}

function createWindow() {
  const icon = path.join(__dirname, "assets", process.platform === "win32" ? "icon.ico" : "icon.png");
  const win = new BrowserWindow({
    width: 1180,
    height: 760,
    minWidth: 860,
    minHeight: 560,
    backgroundColor: "#0d0f12",
    icon,
    titleBarStyle: process.platform === "darwin" ? "hiddenInset" : "default",
    trafficLightPosition: { x: 18, y: 18 },
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),
      nodeIntegration: false,
      contextIsolation: true,
      sandbox: false
    }
  });

  win.loadFile(path.join(__dirname, "src", "index.html"));

  if (isDev) {
    win.webContents.once("did-finish-load", () => {
      win.webContents.send("app:mode", { isDev: true, repoRoot });
    });
  }
}

function sendJson(res, statusCode, payload) {
  const body = JSON.stringify(payload);
  res.writeHead(statusCode, {
    "Content-Type": "application/json",
    "Content-Length": Buffer.byteLength(body),
    "Cache-Control": "no-store"
  });
  res.end(body);
}

function startCameraBridge() {
  if (cameraBridge.server) return Promise.resolve();

  return new Promise((resolve) => {
    cameraBridge.server = http.createServer((req, res) => {
      const requestUrl = new URL(req.url, "http://127.0.0.1");

      if (requestUrl.searchParams.get("token") !== cameraBridge.token) {
        sendJson(res, 403, { ok: false, error: "Camera bridge token rejected." });
        return;
      }

      if (requestUrl.pathname === "/health") {
        sendJson(res, 200, {
          ok: cameraBridge.status === "ready",
          status: cameraBridge.status,
          error: cameraBridge.error,
          updatedAt: cameraBridge.updatedAt
        });
        return;
      }

      if (requestUrl.pathname === "/frame") {
        if (!cameraBridge.latestFrame) {
          sendJson(res, 503, {
            ok: false,
            status: cameraBridge.status,
            error: cameraBridge.error || "Camera frame is not ready yet."
          });
          return;
        }

        sendJson(res, 200, {
          ok: true,
          data: cameraBridge.latestFrame,
          updatedAt: cameraBridge.updatedAt
        });
        return;
      }

      sendJson(res, 404, { ok: false, error: "Unknown camera bridge endpoint." });
    });

    cameraBridge.server.listen(0, "127.0.0.1", () => {
      const address = cameraBridge.server.address();
      cameraBridge.url = `http://127.0.0.1:${address.port}/frame?token=${cameraBridge.token}`;
      resolve();
    });
  });
}

function requestMediaAccess(kind) {
  if (process.platform !== "darwin") return;

  try {
    const status = systemPreferences.getMediaAccessStatus(kind);
    if (status === "not-determined") {
      systemPreferences.askForMediaAccess(kind).catch(() => {});
    }
  } catch (_) {
    // The shell still works if macOS denies or cannot prompt for media access.
  }
}

app.whenReady().then(async () => {
  await startCameraBridge();
  session.defaultSession.setPermissionRequestHandler((_webContents, permission, callback) => {
    callback(permission === "media");
  });
  requestMediaAccess("camera");
  requestMediaAccess("microphone");
  createWindow();

  app.on("activate", () => {
    if (BrowserWindow.getAllWindows().length === 0) createWindow();
  });
});

app.on("window-all-closed", () => {
  for (const session of activeSessions.values()) {
    session.kill();
  }
  activeSessions.clear();

  if (process.platform !== "darwin") app.quit();
});

app.on("before-quit", () => {
  if (cameraBridge.server) {
    cameraBridge.server.close();
    cameraBridge.server = null;
  }
});

ipcMain.on("camera:frame", (_event, { data }) => {
  if (typeof data !== "string" || data.length === 0) return;
  cameraBridge.latestFrame = data;
  cameraBridge.updatedAt = Date.now();
  cameraBridge.status = "ready";
  cameraBridge.error = "";
});

ipcMain.on("camera:status", (_event, payload = {}) => {
  cameraBridge.status = payload.status || "unknown";
  cameraBridge.error = payload.error || "";
  if (cameraBridge.status !== "ready") {
    cameraBridge.latestFrame = "";
    cameraBridge.updatedAt = 0;
  }
});

ipcMain.handle("terminal:create", (event, options = {}) => {
  const id = `${Date.now()}-${Math.random().toString(16).slice(2)}`;
  const backend = resolveBackend();
  const sender = event.sender;
  const cols = Math.max(40, Number(options.cols) || 120);
  const rows = Math.max(10, Number(options.rows) || 34);

  const term = pty.spawn(backend.command, backend.args, {
    name: "xterm-256color",
    cols,
    rows,
    cwd: backend.cwd,
    env: {
      ...process.env,
      TERM: "xterm-256color",
      COLORTERM: "truecolor",
      YUKISHELL_DESKTOP: "1",
      YUKISHELL_CORE_DIR: coreCwd(),
      YUKISHELL_ENV_FILE: envFilePath(),
      YUKISHELL_PYTHON_BIN: pythonBinaryPath(),
      YUKISHELL_PYTHONPATH: bundledPythonPath(),
      YUKISHELL_CAMERA_BRIDGE_URL: cameraBridge.url,
      OPENCV_AVFOUNDATION_SKIP_AUTH: "0",
      PYTHONDONTWRITEBYTECODE: "1",
      PYTHONPATH: [bundledPythonPath(), process.env.PYTHONPATH].filter(Boolean).join(path.delimiter)
    }
  });

  activeSessions.set(id, term);

  term.onData((data) => {
    if (!sender.isDestroyed()) {
      sender.send("terminal:data", { id, data });
    }
  });

  term.onExit(({ exitCode, signal }) => {
    activeSessions.delete(id);
    if (!sender.isDestroyed()) {
      sender.send("terminal:exit", { id, exitCode, signal });
    }
  });

  return { id, backend: backend.label };
});

ipcMain.on("terminal:input", (_event, { id, data }) => {
  const term = activeSessions.get(id);
  if (term) term.write(data);
});

ipcMain.on("terminal:resize", (_event, { id, cols, rows }) => {
  const term = activeSessions.get(id);
  if (term) term.resize(Math.max(40, cols), Math.max(10, rows));
});

ipcMain.on("terminal:kill", (_event, { id }) => {
  const term = activeSessions.get(id);
  if (term) {
    term.kill();
    activeSessions.delete(id);
  }
});

ipcMain.handle("app:paths", () => ({
  repoRoot,
  corePath: corePath(),
  coreExists: fs.existsSync(corePath()),
  pythonPath: bundledPythonPath(),
  pythonBin: pythonBinaryPath(),
  pythonBundleExists: fs.existsSync(bundledPythonPath()),
  envFileExists: fs.existsSync(envFilePath()),
  cameraBridgeReady: Boolean(cameraBridge.url),
  platform: process.platform,
  arch: process.arch
}));

ipcMain.handle("app:openExternal", (_event, url) => {
  return shell.openExternal(url);
});
