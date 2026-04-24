const fs = require("fs");
const path = require("path");

const desktopRoot = path.resolve(__dirname, "..");
const repoRoot = path.resolve(desktopRoot, "..");
const resourcesRoot = path.join(desktopRoot, "resources");
const coreTarget = path.join(resourcesRoot, "core");

function copyIfExists(from, to, executable = false) {
  if (!fs.existsSync(from)) return false;
  fs.mkdirSync(path.dirname(to), { recursive: true });
  fs.copyFileSync(from, to);
  if (executable && process.platform !== "win32") {
    fs.chmodSync(to, 0o755);
  }
  return true;
}

fs.rmSync(resourcesRoot, { recursive: true, force: true });
fs.mkdirSync(coreTarget, { recursive: true });

copyIfExists(path.join(repoRoot, "yuki_ai.py"), path.join(coreTarget, "yuki_ai.py"));
copyIfExists(path.join(repoRoot, "yuki_net.py"), path.join(coreTarget, "yuki_net.py"));
copyIfExists(path.join(repoRoot, "yukishell"), path.join(coreTarget, "yukishell"), true);
copyIfExists(path.join(repoRoot, "yukishell.exe"), path.join(coreTarget, "yukishell.exe"), true);
copyIfExists(path.join(repoRoot, "i2c_scan"), path.join(coreTarget, "i2c_scan"), true);

console.log("Prepared packaged core resources.");
