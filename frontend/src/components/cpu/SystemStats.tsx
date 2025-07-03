import React from 'react';
import { ProcessorInfo } from '../../types/process';

interface SystemStatsProps {
  processorInfo: ProcessorInfo;
  processCount: number;
  upTime: string;
}

export const SystemStats: React.FC<SystemStatsProps> = ({
  processorInfo,
  processCount,
  upTime,
}) => {
  return (
    <div className="bg-gray-800 rounded-lg p-6">
      <h2 className="text-xl font-semibold text-white mb-4">System Information</h2>
      
      <div className="grid grid-cols-2 gap-4 mb-6">
        <div>
          <span className="text-gray-400 text-sm">Processes</span>
          <p className="text-2xl font-bold text-white">{processCount}</p>
        </div>
        <div>
          <span className="text-gray-400 text-sm">Up Time</span>
          <p className="text-2xl font-bold text-white">{upTime}</p>
        </div>
      </div>

      <div className="space-y-3">
        <div className="flex justify-between">
          <span className="text-gray-400">Base Speed:</span>
          <span className="text-white">{processorInfo.base_speed_ghz} GHz</span>
        </div>
        <div className="flex justify-between">
          <span className="text-gray-400">Cores:</span>
          <span className="text-white">{processorInfo.core_count}</span>
        </div>
        <div className="flex justify-between">
          <span className="text-gray-400">Sockets:</span>
          <span className="text-white">1</span>
        </div>
        <div className="flex justify-between">
          <span className="text-gray-400">Logical Processors:</span>
          <span className="text-white">{processorInfo.core_count * 2}</span>
        </div>
      </div>
    </div>
  );
};