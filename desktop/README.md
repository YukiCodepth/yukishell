# YukiShell Desktop

Cross-platform desktop terminal for the YukiShell C core.

## Development

```bash
cd desktop
npm install
npm run dev
```

`npm run dev` builds the native core with the repo `Makefile`, then launches Electron.

## Packaging

```bash
npm run dist:mac
npm run dist:linux
npm run dist:win
```

macOS and Linux package the POSIX YukiShell binary directly. Windows packaging currently ships the desktop UI and uses `yukishell.exe` when provided, otherwise it tries WSL and then falls back to the system shell. A native Windows core needs a dedicated port because the current C engine uses POSIX APIs such as `fork`, `termios`, `readline`, `/proc`, and Linux namespaces.

## API keys

Release builds do not include `.env` files. Keep keys local:

```bash
mkdir -p ~/.yukishell
cp ../.env ~/.yukishell/.env
```
