import React, { useState } from 'react';
import { RotateCcw } from 'lucide-react';
import { AppUsage } from '../../types/network';

interface AppUsageTableProps {
  apps: AppUsage[];
  onSetSpeedCap: (appName: string, speedMBps: number) => void;
  onResetCap: (appName: string) => void;
}

export const AppUsageTable: React.FC<AppUsageTableProps> = ({
  apps,
  onSetSpeedCap,
  onResetCap,
}) => {
  const [speedInputs, setSpeedInputs] = useState<{ [key: string]: string }>({});

  const formatBytes = (bytes: number) => {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
  };

  const handleSpeedInputChange = (appName: string, value: string) => {
    setSpeedInputs(prev => ({ ...prev, [appName]: value }));
  };

  const handleSetSpeedCap = (appName: string) => {
    const speed = parseFloat(speedInputs[appName] || '0');
    if (speed > 0) {
      onSetSpeedCap(appName, speed);
      setSpeedInputs(prev => ({ ...prev, [appName]: '' }));
    }
  };

  const handleResetCap = (appName: string) => {
    onResetCap(appName);
    setSpeedInputs(prev => ({ ...prev, [appName]: '' }));
  };

  return (
    <div className="bg-gray-800 rounded-lg p-6">
      <h2 className="text-xl font-semibold text-white mb-4">Apps</h2>
      <div className="overflow-x-auto">
        <table className="w-full">
          <thead>
            <tr className="text-left text-gray-400 text-sm border-b border-gray-700">
              <th className="pb-3">App Name</th>
              <th className="pb-3">Usage</th>
              <th className="pb-3">Speed Cap (MB/s)</th>
              <th className="pb-3">Actions</th>
            </tr>
          </thead>
          <tbody>
            {apps.map((app, index) => (
              <tr key={index} className="border-b border-gray-700 last:border-b-0">
                <td className="py-4 text-white">{app.name}</td>
                <td className="py-4 text-white">{formatBytes(app.usage * 1024)}</td>
                <td className="py-4">
                  <input
                    type="number"
                    placeholder="Enter MB/s"
                    value={speedInputs[app.name] || ''}
                    onChange={(e) => handleSpeedInputChange(app.name, e.target.value)}
                    className="bg-gray-700 text-white px-3 py-1 rounded border border-gray-600 w-24 text-sm"
                    min="0"
                    step="0.1"
                  />
                </td>
                <td className="py-4">
                  <div className="flex space-x-2">
                    <button
                      onClick={() => handleSetSpeedCap(app.name)}
                      className="bg-orange-600 hover:bg-orange-700 text-white px-3 py-1 rounded text-sm transition-colors"
                    >
                      Set Cap
                    </button>
                    <button
                      onClick={() => handleResetCap(app.name)}
                      className="bg-gray-600 hover:bg-gray-700 text-white px-3 py-1 rounded text-sm transition-colors flex items-center space-x-1"
                    >
                      <RotateCcw className="w-3 h-3" />
                      <span>Reset</span>
                    </button>
                  </div>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
};