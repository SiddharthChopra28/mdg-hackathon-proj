import { useState, useEffect } from 'react';
import { ProcessData, ProcessorInfo } from '../types/process';

export const useCPUData = () => {
  const [processes, setProcesses] = useState<ProcessData[]>([]);
  const [processorInfo, setProcessorInfo] = useState<ProcessorInfo>({
    model_name: '',
    base_speed_ghz: 0,
    core_count: 0,
  });
  const [whitelist, setWhitelist] = useState<string[]>([]);
  const [cpuUsageHistory, setCpuUsageHistory] = useState<number[]>(new Array(30).fill(0));
  const [loading, setLoading] = useState(true);

  const fetchData = async () => {
    try {
      if (window.electronAPI) {
        const [processesData, processorData, whitelistData] = await Promise.all([
          window.electronAPI.cpu.getProcesses(),
          window.electronAPI.cpu.getProcessorInfo(),
          window.electronAPI.cpu.getWhitelist(),
        ]);

        setProcesses(processesData);
        setProcessorInfo(processorData);
        setWhitelist(whitelistData);

        // Calculate overall CPU usage for the chart
        const totalCPU = processesData.reduce((sum: number, process: ProcessData) => sum + process.cpu_percent, 0);
        setCpuUsageHistory(prev => [...prev.slice(1), Math.min(totalCPU, 100)]);
      }
    } catch (error) {
      console.error('Error fetching CPU data:', error);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
    const interval = setInterval(fetchData, 1000); // Changed from 2000 to 5000 (5 seconds)

    return () => clearInterval(interval);
  }, []);

  const optimizeProcess = async (name: string, level: string) => {
    try {
      if (window.electronAPI) {
        await window.electronAPI.cpu.optimize(name, level);
        fetchData(); // Refresh data
      }
    } catch (error) {
      console.error('Error optimizing process:', error);
    }
  };

  const restoreProcess = async (name: string) => {
    try {
      if (window.electronAPI) {
        await window.electronAPI.cpu.restore(name);
        fetchData(); // Refresh data
      }
    } catch (error) {
      console.error('Error restoring process:', error);
    }
  };

  const addToWhitelist = async (name: string) => {
    try {
      if (window.electronAPI) {
        await window.electronAPI.cpu.addToWhitelist(name);
        fetchData(); // Refresh data
      }
    } catch (error) {
      console.error('Error adding to whitelist:', error);
    }
  };

  const removeFromWhitelist = async (name: string) => {
    try {
      if (window.electronAPI) {
        await window.electronAPI.cpu.removeFromWhitelist(name);
        fetchData(); // Refresh data
      }
    } catch (error) {
      console.error('Error removing from whitelist:', error);
    }
  };

  const restoreAll = async () => {
    try {
      if (window.electronAPI) {
        await window.electronAPI.cpu.restoreAll();
        fetchData(); // Refresh data
      }
    } catch (error) {
      console.error('Error restoring all processes:', error);
    }
  };

  return {
    processes,
    processorInfo,
    whitelist,
    cpuUsageHistory,
    loading,
    optimizeProcess,
    restoreProcess,
    addToWhitelist,
    removeFromWhitelist,
    restoreAll,
  };
};