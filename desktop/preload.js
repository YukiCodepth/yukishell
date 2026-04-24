const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("yuki", {
  createTerminal: (options) => ipcRenderer.invoke("terminal:create", options),
  writeTerminal: (id, data) => ipcRenderer.send("terminal:input", { id, data }),
  resizeTerminal: (id, cols, rows) => ipcRenderer.send("terminal:resize", { id, cols, rows }),
  killTerminal: (id) => ipcRenderer.send("terminal:kill", { id }),
  updateCameraFrame: (data) => ipcRenderer.send("camera:frame", { data }),
  updateCameraStatus: (status) => ipcRenderer.send("camera:status", status),
  getPaths: () => ipcRenderer.invoke("app:paths"),
  openExternal: (url) => ipcRenderer.invoke("app:openExternal", url),
  onTerminalData: (callback) => {
    const handler = (_event, payload) => callback(payload);
    ipcRenderer.on("terminal:data", handler);
    return () => ipcRenderer.removeListener("terminal:data", handler);
  },
  onTerminalExit: (callback) => {
    const handler = (_event, payload) => callback(payload);
    ipcRenderer.on("terminal:exit", handler);
    return () => ipcRenderer.removeListener("terminal:exit", handler);
  },
  onAppMode: (callback) => {
    const handler = (_event, payload) => callback(payload);
    ipcRenderer.on("app:mode", handler);
    return () => ipcRenderer.removeListener("app:mode", handler);
  }
});
