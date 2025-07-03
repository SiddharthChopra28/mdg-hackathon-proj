import { app, BrowserWindow, ipcMain } from 'electron';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import { SocketClient } from './socket-client.js';
import { NetworkSocketClient } from './network-socket-client.js';
import { RamSocketClient } from './ram-socket-client.js';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

process.env.APP_ROOT = path.join(__dirname, '../..');

export const MAIN_DIST = path.join(process.env.APP_ROOT, 'dist-electron');
export const RENDERER_DIST = path.join(process.env.APP_ROOT, 'dist');
export const VITE_DEV_SERVER_URL = process.env.VITE_DEV_SERVER_URL;

process.env.VITE_PUBLIC = VITE_DEV_SERVER_URL
  ? path.join(process.env.APP_ROOT, 'public')
  : RENDERER_DIST;

let win: BrowserWindow | null = null;
let socketClient: SocketClient;
let networkSocketClient: NetworkSocketClient;
let ramSocketClient: RamSocketClient;

function createWindow() {
  win = new BrowserWindow({
    width: 1400,
    height: 900,
    minWidth: 1200,
    minHeight: 800,
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      preload: path.join(__dirname, 'preload.js'),
      sandbox: false, // Disable sandbox for development
    },
    titleBarStyle: 'default',
    frame: true,
    show: false,
  });

  win.on('ready-to-show', () => {
    win?.show();
  });

  if (VITE_DEV_SERVER_URL) {
    win.loadURL(VITE_DEV_SERVER_URL);
  } else {
    win.loadFile(path.join(RENDERER_DIST, 'index.html'));
  }
}

app.whenReady().then(() => {
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


app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
    win = null;
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});

function registerIpcHandlers() {
  // CPU Socket Endpoints
  ipcMain.handle('cpu:get-processes', async () => {
    return await socketClient.getProcesses();
  });

  ipcMain.handle('cpu:get-processor-info', async () => {
    return await socketClient.getProcessorInfo();
  });

  ipcMain.handle('cpu:get-whitelist', async () => {
    return await socketClient.getWhitelist();
  });

  ipcMain.handle('cpu:optimize', async (_, name: string, level: string) => {
    return await socketClient.optimizeProcess(name, level);
  });

  ipcMain.handle('cpu:restore', async (_, name: string) => {
    return await socketClient.restoreProcess(name);
  });

  ipcMain.handle('cpu:restore-all', async () => {
    return await socketClient.restoreAll();
  });

  ipcMain.handle('cpu:add-whitelist', async (_, name: string) => {
    return await socketClient.addToWhitelist(name);
  });

  ipcMain.handle('cpu:remove-whitelist', async (_, name: string) => {
    return await socketClient.removeFromWhitelist(name);
  });

  // TODO: Network Socket Endpoints - Register these when network socket is ready
  // These endpoints will communicate with /tmp/network_optimizer.sock

  ipcMain.handle('network:get-usage', async () => {
    return await networkSocketClient.getNetworkUsage();
  });

  ipcMain.handle('network:set-speed-cap', async (_, appName: string, speedMBps: number) => {
    return await networkSocketClient.setSpeedCap(appName, speedMBps);
  });

  ipcMain.handle('network:reset-cap', async (_, appName: string) => {
    return await networkSocketClient.resetCap(appName);
  });
  ipcMain.handle('ram:get-system-usage', async () => {
    try {
      return await ramSocketClient.getSystemRamUsage();
    } catch (err) {
      return { error: 'Failed to get RAM usage', details: err.message };
    }
  });

  // Register IPC endpoint: Get top RAM-consuming processes
  ipcMain.handle('ram:get-top-processes', async () => {
    try {
      return await ramSocketClient.getTopRamProcesses();
    } catch (err) {
      return { error: 'Failed to get process list', details: err.message };
    }
  });
}