import React from 'react';
import { NetworkData } from '../../types/network';

interface NetworkStatsProps {
  networkData: NetworkData;
}

export const NetworkStats: React.FC<NetworkStatsProps> = ({ networkData }) => {
  const formatBytes = (bytes: number) => {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
  };

  return (
    <div className="bg-gray-800 rounded-lg p-6">
      <h2 className="text-xl font-semibold text-white mb-6">Network Statistics</h2>
      
      {/* Total Usage Circle */}
      <div className="flex justify-center mb-8">
        <div className="relative w-32 h-32">
          <svg className="w-32 h-32 transform -rotate-90" viewBox="0 0 120 120">
            <circle
              cx="60"
              cy="60"
              r="50"
              stroke="rgba(75, 85, 99, 0.3)"
              strokeWidth="8"
              fill="none"
            />
            <circle
              cx="60"
              cy="60"
              r="50"
              stroke="#f97316"
              strokeWidth="8"
              fill="none"
              strokeDasharray={`${(networkData.total / 100) * 314} 314`}
              strokeLinecap="round"
            />
          </svg>
          <div className="absolute inset-0 flex flex-col items-center justify-center">
            <span className="text-gray-400 text-sm">Total</span>
            <span className="text-white text-xl font-bold">{formatBytes(networkData.total * 1024 * 1024)}</span>
          </div>
        </div>
      </div>

      {/* Stats Grid */}
      <div className="grid grid-cols-2 gap-4 mb-6">
        <div className="text-center">
          <span className="text-gray-400 text-sm block">Incoming</span>
          <span className="text-white text-lg font-bold">{formatBytes(networkData.incoming * 1024 * 1024)}</span>
          <div className="w-full bg-gray-700 rounded-full h-2 mt-2">
            <div 
              className="bg-orange-500 h-2 rounded-full" 
              style={{ width: `${(networkData.incoming / networkData.total) * 100}%` }}
            ></div>
          </div>
        </div>
        
        <div className="text-center">
          <span className="text-gray-400 text-sm block">Outgoing</span>
          <span className="text-white text-lg font-bold">{formatBytes(networkData.outgoing * 1024 * 1024)}</span>
          <div className="w-full bg-gray-700 rounded-full h-2 mt-2">
            <div 
              className="bg-orange-500 h-2 rounded-full" 
              style={{ width: `${(networkData.outgoing / networkData.total) * 100}%` }}
            ></div>
          </div>
        </div>
      </div>

      <div className="grid grid-cols-2 gap-4">
        <div className="text-center">
          <span className="text-gray-400 text-sm block">External</span>
          <span className="text-white text-lg font-bold">{formatBytes(networkData.external * 1024 * 1024)}</span>
        </div>
        
        <div className="text-center">
          <span className="text-gray-400 text-sm block">Local</span>
          <span className="text-white text-lg font-bold">{formatBytes(networkData.local * 1024)}</span>
        </div>
      </div>
    </div>
  );
};