import { useState, useEffect } from 'react';
import { RamProcess, RamStats } from '../types/ram';


export const useRamData = () => {
  const [topProcesses, setTopProcesses] = useState<RamProcess[]>([]);
  const [ramStats, setRamStats] = useState<RamStats>({
    total_gb: 0,
    used_gb: 0,
    percent_used: 0,
  });

  const [ramUsageHistory, setRamUsageHistory] = useState<number[]>(new Array(30).fill(0));
  const [loading, setLoading] = useState(true);

  const fetchData = async () => {
    try {
      if (window.electronAPI && window.electronAPI.ram) {
        const [stats, processes] = await Promise.all([
          window.electronAPI.ram.getSystemRamUsage(),
          window.electronAPI.ram.getTopRamProcesses(),
        ]);

        setRamStats(stats);

        const filtered = Object.values(processes).filter(
          (entry: any) => typeof entry === 'object' && 'pid' in entry
        ) as RamProcess[];

        setTopProcesses(filtered);

        // Track usage over time for chart
        setRamUsageHistory(prev => [
          ...prev.slice(1),
          Math.min(stats.percent_used, 100),
        ]);
      }
    } catch (error) {
      console.error('Error fetching RAM data:', error);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
    const interval = setInterval(fetchData, 1000); // 1 seconds like CPU hook
    return () => clearInterval(interval);
  }, []);

  return {
    topProcesses,
    ramStats,
    ramUsageHistory,
    loading,
  };
};
