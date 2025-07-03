import React from 'react';

interface RamProcess {
  pid: number;
  name: string;
  ram_kb: number;
}

interface RamUsageTableProps {
  processes: RamProcess[];
}

export const RamUsageTable: React.FC<RamUsageTableProps> = ({ processes }) => {
  const formatMemory = (kb: number) => {
    if (kb < 1024) return `${kb} KB`;
    const mb = kb / 1024;
    if (mb < 1024) return `${mb.toFixed(1)} MB`;
    return `${(mb / 1024).toFixed(2)} GB`;
  };

  return (
    <div className="bg-gray-800 rounded-lg p-6">
      <h2 className="text-xl font-semibold text-white mb-4">Top RAM Consuming Processes</h2>
      <div className="overflow-x-auto">
        <table className="w-full">
          <thead>
            <tr className="text-left text-gray-400 text-sm border-b border-gray-700">
              <th className="pb-3">PID</th>
              <th className="pb-3">Process Name</th>
              <th className="pb-3">RAM Usage</th>
            </tr>
          </thead>
          <tbody>
            {processes.map((proc, index) => (
              <tr key={index} className="border-b border-gray-700 last:border-b-0">
                <td className="py-3 text-white">{proc.pid}</td>
                <td className="py-3 text-white">{proc.name}</td>
                <td className="py-3 text-white">{formatMemory(proc.ram_kb)}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
};
