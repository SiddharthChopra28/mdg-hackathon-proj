import React from 'react';
import { RamUsageCircle } from '../components/ram/RamStatsCircle';
import { RamUsageTable } from '../components/ram/RamUsageTable';
import { useRamData } from '../hooks/useRamData';

export const RamMonitor: React.FC = () => {
  const {
    topProcesses,
    ramStats,
    ramUsageHistory,
    loading
  } = useRamData();

  if (loading) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="text-white">Loading...</div>
      </div>
    );
  }

  return (
    <div className="space-y-6">
      {/* Top Section - RAM Circle and Stats */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* RAM Circle - 1/3 of width */}
        <div>
          <RamUsageCircle totalGb={ramStats.total_gb} usedGb={ramStats.used_gb} />
        </div>

        {/* RAM Stats - Fill rest (can extend here later) */}
        <div className="lg:col-span-2 flex items-center justify-center text-white text-lg font-semibold">
          Used {ramStats.used_gb.toFixed(2)} GB / {ramStats.total_gb.toFixed(2)} GB ({ramStats.percent_used.toFixed(1)}%)
        </div>
      </div>

      {/* Bottom Section - Top 20 RAM-Heavy Processes */}
      <div className="grid grid-cols-1">
        <RamUsageTable processes={topProcesses} />
      </div>
    </div>
  );
};
