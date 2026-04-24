const terminalElement = document.getElementById("terminal");
const backendStatus = document.getElementById("backendStatus");
const restartButton = document.getElementById("restartButton");
const clearButton = document.getElementById("clearButton");
const copyButton = document.getElementById("copyButton");
const pasteButton = document.getElementById("pasteButton");

let terminalId = null;

const term = new Terminal({
  cursorBlink: true,
  cursorStyle: "bar",
  fontFamily: "SFMono-Regular, Menlo, Monaco, Consolas, monospace",
  fontSize: 13,
  lineHeight: 1.2,
  letterSpacing: 0,
  scrollback: 10000,
  allowProposedApi: false,
  theme: {
    background: "#0a0c0f",
    foreground: "#ddd6ce",
    cursor: "#f5c2e7",
    cursorAccent: "#0a0c0f",
    selectionBackground: "#28313d",
    black: "#0a0c0f",
    red: "#f38ba8",
    green: "#a6e3a1",
    yellow: "#f9e2af",
    blue: "#89b4fa",
    magenta: "#cba6f7",
    cyan: "#74c7ec",
    white: "#ddd6ce",
    brightBlack: "#6c7086",
    brightRed: "#f38ba8",
    brightGreen: "#a6e3a1",
    brightYellow: "#f9e2af",
    brightBlue: "#89b4fa",
    brightMagenta: "#f5c2e7",
    brightCyan: "#94e2d5",
    brightWhite: "#f5f1ea"
  }
});

const fitAddon = new FitAddon.FitAddon();
term.loadAddon(fitAddon);
term.open(terminalElement);

function setStatus(label, active = true) {
  backendStatus.classList.toggle("offline", !active);
  backendStatus.querySelector("b").textContent = label;
}

function fitAndResize() {
  fitAddon.fit();
  if (terminalId) {
    window.yuki.resizeTerminal(terminalId, term.cols, term.rows);
  }
}

async function startTerminal() {
  if (terminalId) {
    window.yuki.killTerminal(terminalId);
    terminalId = null;
  }

  term.reset();
  setStatus("Starting", true);
  fitAddon.fit();

  const session = await window.yuki.createTerminal({
    cols: term.cols,
    rows: term.rows
  });

  terminalId = session.id;
  setStatus("Ready", true);
  term.focus();
}

window.yuki.onTerminalData(({ id, data }) => {
  if (id === terminalId) term.write(data);
});

window.yuki.onTerminalExit(({ id }) => {
  if (id !== terminalId) return;
  setStatus("Closed", false);
  term.writeln("");
  term.writeln("\x1b[38;2;255;96;125mSession closed.\x1b[0m");
});

term.onData((data) => {
  if (terminalId) window.yuki.writeTerminal(terminalId, data);
});

window.addEventListener("resize", fitAndResize);
restartButton.addEventListener("click", startTerminal);

clearButton.addEventListener("click", () => {
  if (terminalId) window.yuki.writeTerminal(terminalId, "clear\r");
  term.focus();
});

copyButton.addEventListener("click", async () => {
  const selection = term.getSelection();
  if (selection) await navigator.clipboard.writeText(selection);
  term.focus();
});

pasteButton.addEventListener("click", async () => {
  const text = await navigator.clipboard.readText();
  if (text && terminalId) window.yuki.writeTerminal(terminalId, text);
  term.focus();
});

async function startCameraBridge() {
  if (!navigator.mediaDevices?.getUserMedia) {
    window.yuki.updateCameraStatus({
      status: "unavailable",
      error: "Camera API is not available in this Electron view."
    });
    return;
  }

  try {
    const stream = await navigator.mediaDevices.getUserMedia({
      video: {
        width: { ideal: 1280 },
        height: { ideal: 720 },
        facingMode: "environment"
      },
      audio: false
    });

    const video = document.createElement("video");
    video.muted = true;
    video.playsInline = true;
    video.srcObject = stream;
    await video.play();

    const canvas = document.createElement("canvas");
    const context = canvas.getContext("2d", { alpha: false });

    const captureFrame = () => {
      if (!video.videoWidth || !video.videoHeight) return;
      canvas.width = video.videoWidth;
      canvas.height = video.videoHeight;
      context.drawImage(video, 0, 0, canvas.width, canvas.height);
      const image = canvas.toDataURL("image/jpeg", 0.82).replace(/^data:image\/jpeg;base64,/, "");
      window.yuki.updateCameraFrame(image);
    };

    captureFrame();
    window.setInterval(captureFrame, 700);
  } catch (error) {
    window.yuki.updateCameraStatus({
      status: "error",
      error: error?.message || "Camera permission was denied."
    });
  }
}

requestAnimationFrame(() => {
  fitAndResize();
  startTerminal();
  startCameraBridge();
});
