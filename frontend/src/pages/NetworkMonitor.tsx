import React from 'react';
import { NetworkChart } from '../components/network/NetworkChart';
import { NetworkStats } from '../components/network/NetworkStats';
import { AppUsageTable } from '../components/network/AppUsageTable';
import { TrafficTypeTable } from '../components/network/TrafficTypeTable';
import { useNetworkData } from '../hooks/useNetworkData';

export const NetworkMonitor: React.FC = () => {
  const {
    networkData,
    appUsage,
    trafficTypes,
    loading,
    setSpeedCap,
    resetCap,
  } = useNetworkData();

  if (loading) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="text-white">Loading...</div>
      </div>
    );
  }

  const handleSpeedCap = (appName: string, speedMBps: number) => {
    setSpeedCap(appName, speedMBps);
  };

  const handleResetCap = (appName: string) => {
    resetCap(appName);
  };

  return (
    <div className="space-y-6">
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        <div className="lg:col-span-2">
          <NetworkChart data={networkData} />
        </div>
        
        <div>
          <NetworkStats networkData={networkData} />
        </div>
      </div>

      <div className="grid grid-cols-1 xl:grid-cols-2 gap-6">
        <AppUsageTable
          apps={appUsage}
          onSetSpeedCap={handleSpeedCap}
          onResetCap={handleResetCap}
        />
        
        <TrafficTypeTable
          trafficTypes={trafficTypes}
        />
      </div>
    </div>
  );
};