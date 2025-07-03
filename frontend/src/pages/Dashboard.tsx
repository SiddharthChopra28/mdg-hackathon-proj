import React from 'react';
import { CPUChart } from '../components/cpu/CPUChart';
import { ProcessTable } from '../components/cpu/ProcessTable';
import { WhitelistTable } from '../components/cpu/WhitelistTable';
import { SystemStats } from '../components/cpu/SystemStats';
import { useCPUData } from '../hooks/useCPUData';
import { PriorityLevel } from '../types/process';

export const Dashboard: React.FC = () => {
  const {
    processes,
    processorInfo,
    whitelist,
    cpuUsageHistory,
    loading,
    optimizeProcess,
    restoreProcess,
    addToWhitelist,
    removeFromWhitelist,
  } = useCPUData();

  if (loading) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="text-white">Loading...</div>
      </div>
    );
  }

  const handleOptimize = (name: string, level: PriorityLevel) => {
    optimizeProcess(name, level);
  };

  const handleWhitelist = (name: string) => {
    addToWhitelist(name);
  };

  const handleReset = (name: string) => {
    restoreProcess(name);
  };

  const handleRemoveFromWhitelist = (name: string) => {
    removeFromWhitelist(name);
  };

  return (
    <div className="space-y-6">
      {/* Top Section - Landscape Layout */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* CPU Chart - Takes 2/3 of the width */}
        <div className="lg:col-span-2">
          <CPUChart data={cpuUsageHistory} processorInfo={processorInfo} />
        </div>
        
        {/* System Stats - Takes 1/3 of the width */}
        <div>
          <SystemStats 
            processorInfo={processorInfo} 
            processCount={processes.length}
            upTime="2:22:41:35"
          />
        </div>
      </div>

      {/* Bottom Section - Side by Side Tables */}
      <div className="grid grid-cols-1 xl:grid-cols-2 gap-6">
        <ProcessTable
          processes={processes}
          onOptimize={handleOptimize}
          onWhitelist={handleWhitelist}
          onReset={handleReset}
        />
        
        <WhitelistTable
          whitelistItems={whitelist}
          onRemove={handleRemoveFromWhitelist}
        />
      </div>
    </div>
  );
};