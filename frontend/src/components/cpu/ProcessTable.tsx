import React from 'react';
import { ChevronDown, Plus, RotateCcw } from 'lucide-react';
import { ProcessData, PriorityLevel } from '../../types/process';

interface ProcessTableProps {
  processes: ProcessData[];
  onOptimize: (name: string, level: PriorityLevel) => void;
  onWhitelist: (name: string) => void;
  onReset: (name: string) => void;
}

export const ProcessTable: React.FC<ProcessTableProps> = ({
  processes,
  onOptimize,
  onWhitelist,
  onReset,
}) => {
  const handlePriorityChange = (processName: string, priority: PriorityLevel) => {
    onOptimize(processName, priority);
  };

  return (
    <div className="bg-gray-800 rounded-lg p-6">
      <h2 className="text-xl font-semibold text-white mb-4">Active Tasks</h2>
      <div className="overflow-x-auto">
        <table className="w-full">
          <thead>
            <tr className="text-left text-gray-400 text-sm border-b border-gray-700">
              <th className="pb-3">Task</th>
              <th className="pb-3">%</th>
              <th className="pb-3">PIDs</th>
              <th className="pb-3">Priority</th>
              <th className="pb-3">Actions</th>
            </tr>
          </thead>
          <tbody>
            {processes.map((process, index) => (
              <tr key={index} className="border-b border-gray-700 last:border-b-0">
                <td className="py-4 text-white">{process.name}</td>
                <td className="py-4 text-white">{process.cpu_percent.toFixed(1)}%</td>
                <td className="py-4 text-white">{process.pid_count}</td>
                <td className="py-4">
                  <div className="relative">
                    <select
                      className="bg-gray-700 text-white px-3 py-1 rounded border border-gray-600 appearance-none pr-8"
                      onChange={(e) => handlePriorityChange(process.name, e.target.value as PriorityLevel)}
                    >
                      <option value="med">Medium</option>
                      <option value="high">High</option>
                      <option value="low">Low</option>
                    </select>
                    <ChevronDown className="absolute right-2 top-1/2 transform -translate-y-1/2 w-4 h-4 text-gray-400 pointer-events-none" />
                  </div>
                </td>
                <td className="py-4">
                  <div className="flex space-x-2">
                    <button
                      onClick={() => onWhitelist(process.name)}
                      className="bg-blue-600 hover:bg-blue-700 text-white px-3 py-1 rounded text-sm transition-colors flex items-center space-x-1"
                    >
                      <Plus className="w-3 h-3" />
                      <span>Whitelist</span>
                    </button>
                    <button
                      onClick={() => onReset(process.name)}
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