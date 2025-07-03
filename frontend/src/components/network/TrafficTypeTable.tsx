import React from 'react';
import { TrafficType } from '../../types/network';

interface TrafficTypeTableProps {
  trafficTypes: TrafficType[];
}

export const TrafficTypeTable: React.FC<TrafficTypeTableProps> = ({
  trafficTypes,
}) => {
  const formatBytes = (bytes: number) => {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
  };

  return (
    <div className="bg-gray-800 rounded-lg p-6">
      <h2 className="text-xl font-semibold text-white mb-4">Traffic Type</h2>
      <div className="space-y-3">
        {trafficTypes.map((traffic, index) => (
          <div key={index} className="flex items-center justify-between bg-gray-700 p-3 rounded">
            <span className="text-white">{traffic.type}</span>
            <span className="text-white font-semibold">{formatBytes(traffic.usage * 1024)}</span>
          </div>
        ))}
        {trafficTypes.length === 0 && (
          <p className="text-gray-400 text-sm text-center py-8">
            No traffic data available
          </p>
        )}
      </div>
    </div>
  );
};