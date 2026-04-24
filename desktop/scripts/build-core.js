const childProcess = require("child_process");
const fs = require("fs");
const path = require("path");

const repoRoot = path.resolve(__dirname, "..", "..");
const binary = path.join(repoRoot, process.platform === "win32" ? "yukishell.exe" : "yukishell");

if (process.platform === "win32") {
  if (fs.existsSync(binary)) {
    console.log("YukiShell core binary already exists.");
    process.exit(0);
  }

  console.log("Skipping native core build on Windows. Use WSL or provide yukishell.exe.");
  process.exit(0);
}

const result = childProcess.spawnSync("make", [], {
  cwd: repoRoot,
  stdio: "inherit"
});

if (result.status !== 0) {
  process.exit(result.status || 1);
}

try {
  fs.chmodSync(binary, 0o755);
} catch (_) {
  // Packaging will still try to include the binary; chmod failure is not fatal here.
}

if (process.platform === "linux" && fs.existsSync("/usr/include/linux/i2c-dev.h")) {
  const i2cBinary = path.join(repoRoot, "i2c_scan");
  const i2cResult = childProcess.spawnSync("gcc", ["i2c_scan.c", "-o", i2cBinary], {
    cwd: repoRoot,
    stdio: "inherit"
  });

  if (i2cResult.status === 0) {
    try {
      fs.chmodSync(i2cBinary, 0o755);
    } catch (_) {}
  }
}
