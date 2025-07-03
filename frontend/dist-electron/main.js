"use strict";
Object.defineProperty(exports, Symbol.toStringTag, { value: "Module" });
const electron = require("electron");
const node_url = require("node:url");
const path = require("node:path");
const net = require("net");
var _documentCurrentScript = typeof document !== "undefined" ? document.currentScript : null;
function _interopNamespaceDefault(e) {
  const n = Object.create(null, { [Symbol.toStringTag]: { value: "Module" } });
  if (e) {
    for (const k in e) {
      if (k !== "default") {
        const d = Object.getOwnPropertyDescriptor(e, k);
        Object.defineProperty(n, k, d.get ? d : {
          enumerable: true,
          get: () => e[k]
        });
      }
    }
  }
  n.default = e;
  return Object.freeze(n);
}
const net__namespace = /* @__PURE__ */ _interopNamespaceDefault(net);
const SOCKET_PATH = "/tmp/cpu_optimizer.sock";
class SocketClient {
  async sendCommand(command) {
    return new Promise((resolve, reject) => {
      const client = net__namespace.createConnection(SOCKET_PATH);
      client.on("connect", () => {
        client.write(JSON.stringify(command));
      });
      client.on("data", (data) => {
        try {
          const response = data.toString().trim();
          if (response.startsWith("{") || response.startsWith("[")) {
            resolve(JSON.parse(response));
          } else {
            resolve(response);
          }
        } catch (error) {
          resolve(data.toString().trim());
        }
        client.end();
      });
      client.on("error", (error) => {
        reject(error);
      });
    });
  }
  async getProcesses() {
    try {
      const result = await this.sendCommand({ action: "cpu_print_processes" });
      return Array.isArray(result) ? result : [];
    } catch (error) {
      console.error("Error getting processes:", error);
      return [];
    }
  }
  async getProcessorInfo() {
    try {
      const result = await this.sendCommand({ action: "cpu_processor_info" });
      return result || {};
    } catch (error) {
      console.error("Error getting processor info:", error);
      return {};
    }
  }
  async getWhitelist() {
    try {
      const result = await this.sendCommand({ action: "cpu_list_whitelist" });
      return Array.isArray(result) ? result : [];
    } catch (error) {
      console.error("Error getting whitelist:", error);
      return [];
    }
  }
  async optimizeProcess(name, level) {
    try {
      const result = await this.sendCommand({
        action: "cpu_optimize",
        name,
        level
      });
      return result === "Optimized";
    } catch (error) {
      console.error("Error optimizing process:", error);
      return false;
    }
  }
  async restoreProcess(name) {
    try {
      const result = await this.sendCommand({
        action: "cpu_restore",
        name
      });
      return result === "Restored";
    } catch (error) {
      console.error("Error restoring process:", error);
      return false;
    }
  }
  async restoreAll() {
    try {
      const result = await this.sendCommand({ action: "cpu_restore_all" });
      return result === "All Restored";
    } catch (error) {
      console.error("Error restoring all processes:", error);
      return false;
    }
  }
  async addToWhitelist(name) {
    try {
      const result = await this.sendCommand({
        action: "cpu_add_whitelist",
        name
      });
      return result === "Added to whitelist";
    } catch (error) {
      console.error("Error adding to whitelist:", error);
      return false;
    }
  }
  async removeFromWhitelist(name) {
    try {
      const result = await this.sendCommand({
        action: "cpu_remove_whitelist",
        name
      });
      return result === "Removed from whitelist";
    } catch (error) {
      console.error("Error removing from whitelist:", error);
      return false;
    }
  }
}
const NETWORK_SOCKET_PATH = "/tmp/network_optimizer.sock";
class NetworkSocketClient {
  async sendCommand(command) {
    return new Promise((resolve, reject) => {
      const client = net__namespace.createConnection(NETWORK_SOCKET_PATH);
      client.on("connect", () => {
        client.write(JSON.stringify(command));
      });
      client.on("data", (data) => {
        try {
          const response = data.toString().trim();
          if (response.startsWith("{") || response.startsWith("[")) {
            resolve(JSON.parse(response));
          } else {
            resolve(response);
          }
        } catch (error) {
          resolve(data.toString().trim());
        }
        client.end();
      });
      client.on("error", (error) => {
        reject(error);
      });
    });
  }
  // TODO: Register this endpoint in main.ts
  // Endpoint: network:get-usage
  async getNetworkUsage() {
    try {
      const result = await this.sendCommand({ action: "network_get_usage" });
      return Array.isArray(result) ? result : [];
    } catch (error) {
      console.error("Error getting network usage:", error);
      return [];
    }
  }
  // TODO: Register this endpoint in main.ts  
  // Endpoint: network:set-speed-cap
  async setSpeedCap(appName, speedMBps) {
    try {
      const result = await this.sendCommand({
        action: "network_set_speed_cap",
        app_name: appName,
        speed_mbps: speedMBps
      });
      return result === "Speed cap set";
    } catch (error) {
      console.error("Error setting speed cap:", error);
      return false;
    }
  }
  // TODO: Register this endpoint in main.ts
  // Endpoint: network:reset-cap  
  async resetCap(appName) {
    try {
      const result = await this.sendCommand({
        action: "network_reset_cap",
        app_name: appName
      });
      return result === "Speed cap reset";
    } catch (error) {
      console.error("Error resetting speed cap:", error);
      return false;
    }
  }
  async networkOverall() {
    try {
      console.log("In the send command block of network overall");
      const result = await this.sendCommand({
        action: "network_get_overall"
      });
      return result;
    } catch (error) {
      console.error("Error resetting speed cap:", error);
      return false;
    }
  }
}
const RAM_SOCKET_PATH = "/tmp/ram_optimizer.sock";
class RamSocketClient {
  async sendCommand(command) {
    return new Promise((resolve, reject) => {
      const client = net__namespace.createConnection(RAM_SOCKET_PATH);
      client.on("connect", () => {
        client.write(JSON.stringify(command));
      });
      client.on("data", (data) => {
        try {
          const response = data.toString().trim();
          resolve(JSON.parse(response));
        } catch {
          resolve(data.toString().trim());
        }
        client.end();
      });
      client.on("error", (error) => {
        reject(error);
      });
    });
  }
  // 🧠 RAM usage summary
  async getSystemRamUsage() {
    return this.sendCommand({ action: "ram:get-system-usage" });
  }
  // 📋 Top RAM-consuming processes
  async getTopRamProcesses() {
    return this.sendCommand({ action: "ram:get-top-processes" });
  }
}
const __dirname$1 = path.dirname(node_url.fileURLToPath(typeof document === "undefined" ? require("url").pathToFileURL(__filename).href : _documentCurrentScript && _documentCurrentScript.tagName.toUpperCase() === "SCRIPT" && _documentCurrentScript.src || new URL("main.js", document.baseURI).href));
process.env.APP_ROOT = path.join(__dirname$1, "../..");
const MAIN_DIST = path.join(process.env.APP_ROOT, "dist-electron");
const RENDERER_DIST = path.join(process.env.APP_ROOT, "dist");
const VITE_DEV_SERVER_URL = process.env.VITE_DEV_SERVER_URL;
process.env.VITE_PUBLIC = VITE_DEV_SERVER_URL ? path.join(process.env.APP_ROOT, "public") : RENDERER_DIST;
let win = null;
let socketClient;
let networkSocketClient;
let ramSocketClient;
function createWindow() {
  win = new electron.BrowserWindow({
    width: 1400,
    height: 900,
    minWidth: 1200,
    minHeight: 800,
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      preload: path.join(__dirname$1, "preload.js"),
      sandbox: false
      // Disable sandbox for development
    },
    titleBarStyle: "default",
    frame: true,
    show: false
  });
  win.on("ready-to-show", () => {
    win == null ? void 0 : win.show();
  });
  if (VITE_DEV_SERVER_URL) {
    win.loadURL(VITE_DEV_SERVER_URL);
  } else {
    win.loadFile(path.join(RENDERER_DIST, "index.html"));
  }
}
electron.app.whenReady().then(() => {
  try {
    createWindow();
    socketClient = new SocketClient();
    networkSocketClient = new NetworkSocketClient();
    ramSocketClient = new RamSocketClient();
    registerIpcHandlers();
    console.log("IPC handlers registered.");
  } catch (err) {
    console.error("Error during app startup:", err);
  }
});
electron.app.on("window-all-closed", () => {
  if (process.platform !== "darwin") {
    electron.app.quit();
    win = null;
  }
});
electron.app.on("activate", () => {
  if (electron.BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
function registerIpcHandlers() {
  electron.ipcMain.handle("cpu:get-processes", async () => {
    return await socketClient.getProcesses();
  });
  electron.ipcMain.handle("cpu:get-processor-info", async () => {
    return await socketClient.getProcessorInfo();
  });
  electron.ipcMain.handle("cpu:get-whitelist", async () => {
    return await socketClient.getWhitelist();
  });
  electron.ipcMain.handle("cpu:optimize", async (_, name, level) => {
    return await socketClient.optimizeProcess(name, level);
  });
  electron.ipcMain.handle("cpu:restore", async (_, name) => {
    return await socketClient.restoreProcess(name);
  });
  electron.ipcMain.handle("cpu:restore-all", async () => {
    return await socketClient.restoreAll();
  });
  electron.ipcMain.handle("cpu:add-whitelist", async (_, name) => {
    return await socketClient.addToWhitelist(name);
  });
  electron.ipcMain.handle("cpu:remove-whitelist", async (_, name) => {
    return await socketClient.removeFromWhitelist(name);
  });
  electron.ipcMain.handle("network:get-usage", async () => {
    return await networkSocketClient.getNetworkUsage();
  });
  electron.ipcMain.handle("network:set-speed-cap", async (_, appName, speedMBps) => {
    return await networkSocketClient.setSpeedCap(appName, speedMBps);
  });
  electron.ipcMain.handle("network:get-usage-overall", async () => {
    console.log("request for network overall being sent");
    return await networkSocketClient.networkOverall();
  });
  electron.ipcMain.handle("network:reset-cap", async (_, appName) => {
    return await networkSocketClient.resetCap(appName);
  });
  electron.ipcMain.handle("ram:get-system-usage", async () => {
    try {
      return await ramSocketClient.getSystemRamUsage();
    } catch (err) {
      return { error: "Failed to get RAM usage", details: err.message };
    }
  });
  electron.ipcMain.handle("ram:get-top-processes", async () => {
    try {
      return await ramSocketClient.getTopRamProcesses();
    } catch (err) {
      return { error: "Failed to get process list", details: err.message };
    }
  });
}
exports.MAIN_DIST = MAIN_DIST;
exports.RENDERER_DIST = RENDERER_DIST;
exports.VITE_DEV_SERVER_URL = VITE_DEV_SERVER_URL;
//# sourceMappingURL=main.js.map
