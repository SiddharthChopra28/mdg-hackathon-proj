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
  }
  // TODO: Uncomment when network socket endpoints are registered in main.ts
  // network: {
  //   getNetworkUsage: () => ipcRenderer.invoke('network:get-usage'),
  //   setSpeedCap: (appName: string, speedMBps: number) => ipcRenderer.invoke('network:set-speed-cap', appName, speedMBps),
  //   resetCap: (appName: string) => ipcRenderer.invoke('network:reset-cap', appName),
  // },
};
electron.contextBridge.exposeInMainWorld("electronAPI", electronAPI);
//# sourceMappingURL=preload.js.map
