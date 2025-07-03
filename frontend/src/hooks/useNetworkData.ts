import { useState, useEffect } from 'react';
import { NetworkData, AppUsage, TrafficType } from '../types/network';

export const useNetworkData = () => {
  const [networkData, setNetworkData] = useState<NetworkData>({
    total: 40,
    incoming: 37.5,
    outgoing: 2.4,
    external: 39.9,
    local: 0.079,
  });

  const [appUsage, setAppUsage] = useState<AppUsage[]>([
    { app_name: 'Microsoft Edge', download_bytes: 37500, upload_bytes: 123 },
    { app_name: 'Dropbox', download_bytes: 362, upload_bytes: 2133 },
  ]);

  const [trafficTypes, setTrafficTypes] = useState<TrafficType[]>([
    { type: 'Hypertext Transfer Protocol over SSL/TLS', usage: 2800 },
    { type: 'Other', usage: 118 },
    { type: 'Domain Name System (DNS)', usage: 111.8 },
    { type: 'Microsoft SSDP', usage: 94.6 },
    { type: 'NetBIOS Name Services', usage: 11.1 },
    { type: 'Simple Network Management', usage: 0.66 },
  ]);

  const [loading, setLoading] = useState(true);

  const fetchData = async () => {
    try { 
      // TODO: Replace with actual socket calls when network socket is implemented
      if (window.electronAPI && window.electronAPI.network) {
        const apps = await window.electronAPI.network.getNetworkUsage();
        const networkDataOverall = await window.electronAPI.network.getOverall();
        console.log("network data:", apps);
        console.log("overall network data: ", networkDataOverall);
        setAppUsage(apps);
        setNetworkData(networkDataOverall);

        // You can keep or remove the mock variation for `networkData`:
        setNetworkData(prev => ({
          ...prev,
          incoming: prev.incoming + (Math.random() - 0.5) * 2,
          outgoing: prev.outgoing + (Math.random() - 0.5) * 0.5,
        }));
      }

      // For now, using mock data with slight variations
      setNetworkData(prev => ({
        ...prev,
        incoming: prev.incoming + (Math.random() - 0.5) * 2,
        outgoing: prev.outgoing + (Math.random() - 0.5) * 0.5,
      }));
    } catch (error) {
      console.error('Error fetching network data:', error);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
    const interval = setInterval(fetchData, 2000); // Update every 2 seconds

    return () => clearInterval(interval);
  }, []);

  const setSpeedCap = async (appName: string, speedMBps: number) => {
    try {
      // TODO: Implement actual socket call
      if (window.electronAPI && window.electronAPI.network) {
        await window.electronAPI.network.setSpeedCap(appName, speedMBps);
        fetchData(); // Refresh data
      }
      console.log(`Setting speed cap for ${appName}: ${speedMBps} MB/s`);
    } catch (error) {
      console.error('Error setting speed cap:', error);
    }
  };

  const resetCap = async (appName: string) => {
    try {
      // TODO: Implement actual socket call
      if (window.electronAPI && window.electronAPI.network) {
        await window.electronAPI.network.resetCap(appName);
        fetchData(); // Refresh data
      }
      console.log(`Resetting speed cap for ${appName}`);
    } catch (error) {
      console.error('Error resetting speed cap:', error);
    }
  };

  return {
    networkData,
    appUsage,
    trafficTypes,
    loading,
    setSpeedCap,
    resetCap,
  };
};