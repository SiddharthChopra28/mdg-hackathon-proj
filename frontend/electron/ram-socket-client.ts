import * as net from 'net';

const RAM_SOCKET_PATH = '/tmp/ram_optimizer.sock';

export class RamSocketClient {
  private async sendCommand(command: any): Promise<any> {
    return new Promise((resolve, reject) => {
      const client = net.createConnection(RAM_SOCKET_PATH);

      client.on('connect', () => {
        client.write(JSON.stringify(command));
      });

      client.on('data', (data) => {
        try {
          const response = data.toString().trim();
          resolve(JSON.parse(response));
        } catch {
          resolve(data.toString().trim());
        }
        client.end();
      });

      client.on('error', (error) => {
        reject(error);
      });
    });
  }

  // 🧠 RAM usage summary
  async getSystemRamUsage(): Promise<any> {
    return this.sendCommand({ action: 'ram:get-system-usage' });
  }

  // 📋 Top RAM-consuming processes
  async getTopRamProcesses(): Promise<any[]> {
    return this.sendCommand({ action: 'ram:get-top-processes' });
  }
}
