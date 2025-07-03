import * as net from 'net';

const NETWORK_SOCKET_PATH = '/tmp/network_optimizer.sock';

export class NetworkSocketClient {
  private async sendCommand(command: any): Promise<any> {
    return new Promise((resolve, reject) => {
      const client = net.createConnection(NETWORK_SOCKET_PATH);

      client.on('connect', () => {
        client.write(JSON.stringify(command));
      });

      client.on('data', (data) => {
        try {
          const response = data.toString().trim();
          if (response.startsWith('{') || response.startsWith('[')) {
            resolve(JSON.parse(response));
          } else {
            resolve(response);
          }
        } catch (error) {
          resolve(data.toString().trim());
        }
        client.end();
      });

      client.on('error', (error) => {
        reject(error);
      });
    });
  }

  // TODO: Register this endpoint in main.ts
  // Endpoint: network:get-usage
  async getNetworkUsage(): Promise<any[]> {
    try {
      const result = await this.sendCommand({ action: 'network_get_usage' });
      return Array.isArray(result) ? result : [];
    } catch (error) {
      console.error('Error getting network usage:', error);
      return [];
    }
  }

  // TODO: Register this endpoint in main.ts  
  // Endpoint: network:set-speed-cap
  async setSpeedCap(appName: string, speedMBps: number): Promise<boolean> {
    try {
      const result = await this.sendCommand({
        action: 'network_set_speed_cap',
        app_name: appName,
        speed_mbps: speedMBps
      });
      return result === 'Speed cap set';
    } catch (error) {
      console.error('Error setting speed cap:', error);
      return false;
    }
  }

  // TODO: Register this endpoint in main.ts
  // Endpoint: network:reset-cap  
  async resetCap(appName: string): Promise<boolean> {
    try {
      const result = await this.sendCommand({
        action: 'network_reset_cap',
        app_name: appName
      });
      return result === 'Speed cap reset';
    } catch (error) {
      console.error('Error resetting speed cap:', error);
      return false;
    }
  }

  async networkOverall(): Promise<any> {
    try {
      console.log("In the send command block of network overall")
      const result = await this.sendCommand({
        action: 'network_get_overall',
      });
      return result;
    } catch (error) {
      console.error('Error resetting speed cap:', error);
      return false;
    }

  }
}