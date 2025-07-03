import { contextBridge, ipcRenderer } from 'electron';

export interface IElectronAPI {
  cpu: {
    getProcesses: () => Promise<any[]>;
    getProcessorInfo: () => Promise<any>;
    getWhitelist: () => Promise<string[]>;
    optimize: (name: string, level: string) => Promise<boolean>;
    restore: (name: string) => Promise<boolean>;
    restoreAll: () => Promise<boolean>;
    addToWhitelist: (name: string) => Promise<boolean>;
    removeFromWhitelist: (name: string) => Promise<boolean>;
  };
  // TODO: Uncomment when network socket endpoints are registered in main.ts
  // network: {
  //   getNetworkUsage: () => Promise<any[]>;
  //   setSpeedCap: (appName: string, speedMBps: number) => Promise<boolean>;
  //   resetCap: (appName: string) => Promise<boolean>;
  // };

  ram: {
    getSystemRamUsage: () => Promise<any>;
    getTopRamProcesses: () => Promise<any[]>;
  }
}

const electronAPI: IElectronAPI = {
  cpu: {
    getProcesses: () => ipcRenderer.invoke('cpu:get-processes'),
    getProcessorInfo: () => ipcRenderer.invoke('cpu:get-processor-info'),
    getWhitelist: () => ipcRenderer.invoke('cpu:get-whitelist'),
    optimize: (name: string, level: string) => ipcRenderer.invoke('cpu:optimize', name, level),
    restore: (name: string) => ipcRenderer.invoke('cpu:restore', name),
    restoreAll: () => ipcRenderer.invoke('cpu:restore-all'),
    addToWhitelist: (name: string) => ipcRenderer.invoke('cpu:add-whitelist', name),
    removeFromWhitelist: (name: string) => ipcRenderer.invoke('cpu:remove-whitelist', name),
  },
  // TODO: Uncomment when network socket endpoints are registered in main.ts
  // network: {
  //   getNetworkUsage: () => ipcRenderer.invoke('network:get-usage'),
  //   setSpeedCap: (appName: string, speedMBps: number) => ipcRenderer.invoke('network:set-speed-cap', appName, speedMBps),
  //   resetCap: (appName: string) => ipcRenderer.invoke('network:reset-cap', appName),
  // },

  ram: {
    getSystemRamUsage: () => ipcRenderer.invoke('ram:get-system-usage'),
    getTopRamProcesses: () => ipcRenderer.invoke('ram:get-top-processes')
  }
};

contextBridge.exposeInMainWorld('electronAPI', electronAPI);