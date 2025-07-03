"use strict";
const electron = require("electron");
const electronAPI = {
  cpu: {
    getProcesses: () => electron.ipcRenderer.invoke("cpu:get-processes"),
    getProcessorInfo: () => electron.ipcRenderer.invoke("cpu:get-processor-info"),
    getWhitelist: () => electron.ipcRenderer.invoke("cpu:get-whitelist"),
    optimize: (name, level) => electron.ipcRenderer.invoke("cpu:optimize", name, level),
    restore: (name) => electron.ipcRenderer.invoke("cpu:restore", name),
    restoreAll: () => electron.ipcRenderer.invoke("cpu:restore-all"),
    addToWhitelist: (name) => electron.ipcRenderer.invoke("cpu:add-whitelist", name),
    removeFromWhitelist: (name) => electron.ipcRenderer.invoke("cpu:remove-whitelist", name)
  },
  // TODO: Uncomment when network socket endpoints are registered in main.ts
  network: {
    getNetworkUsage: () => electron.ipcRenderer.invoke("network:get-usage"),
    setSpeedCap: (appName, speedMBps) => electron.ipcRenderer.invoke("network:set-speed-cap", appName, speedMBps),
    resetCap: (appName) => electron.ipcRenderer.invoke("network:reset-cap", appName)
  },
  ram: {
    getSystemRamUsage: () => electron.ipcRenderer.invoke("ram:get-system-usage"),
    getTopRamProcesses: () => electron.ipcRenderer.invoke("ram:get-top-processes")
  }
};
electron.contextBridge.exposeInMainWorld("electronAPI", electronAPI);
//# sourceMappingURL=preload.js.map
