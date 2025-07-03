import * as net from 'net';

const SOCKET_PATH = '/tmp/cpu_optimizer.sock';

export class SocketClient {
  private async sendCommand(command: any): Promise<any> {
    return new Promise((resolve, reject) => {
      const client = net.createConnection(SOCKET_PATH);
      
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

  async getProcesses(): Promise<any[]> {
    try {
      const result = await this.sendCommand({ action: 'cpu_print_processes' });
      return Array.isArray(result) ? result : [];
    } catch (error) {
      console.error('Error getting processes:', error);
      return [];
    }
  }

  async getProcessorInfo(): Promise<any> {
    try {
      const result = await this.sendCommand({ action: 'cpu_processor_info' });
      return result || {};
    } catch (error) {
      console.error('Error getting processor info:', error);
      return {};
    }
  }

  async getWhitelist(): Promise<string[]> {
    try {
      const result = await this.sendCommand({ action: 'cpu_list_whitelist' });
      return Array.isArray(result) ? result : [];
    } catch (error) {
      console.error('Error getting whitelist:', error);
      return [];
    }
  }

  async optimizeProcess(name: string, level: string): Promise<boolean> {
    try {
      const result = await this.sendCommand({ 
        action: 'cpu_optimize', 
        name, 
        level 
      });
      return result === 'Optimized';
    } catch (error) {
      console.error('Error optimizing process:', error);
      return false;
    }
  }

  async restoreProcess(name: string): Promise<boolean> {
    try {
      const result = await this.sendCommand({ 
        action: 'cpu_restore', 
        name 
      });
      return result === 'Restored';
    } catch (error) {
      console.error('Error restoring process:', error);
      return false;
    }
  }

  async restoreAll(): Promise<boolean> {
    try {
      const result = await this.sendCommand({ action: 'cpu_restore_all' });
      return result === 'All Restored';
    } catch (error) {
      console.error('Error restoring all processes:', error);
      return false;
    }
  }

  async addToWhitelist(name: string): Promise<boolean> {
    try {
      const result = await this.sendCommand({ 
        action: 'cpu_add_whitelist', 
        name 
      });
      return result === 'Added to whitelist';
    } catch (error) {
      console.error('Error adding to whitelist:', error);
      return false;
    }
  }

  async removeFromWhitelist(name: string): Promise<boolean> {
    try {
      const result = await this.sendCommand({ 
        action: 'cpu_remove_whitelist', 
        name 
      });
      return result === 'Removed from whitelist';
    } catch (error) {
      console.error('Error removing from whitelist:', error);
      return false;
    }
  }
}