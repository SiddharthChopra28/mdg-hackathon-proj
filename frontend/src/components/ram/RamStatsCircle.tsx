import React from 'react';

interface RamUsageCircleProps {
  totalGb: number;
  usedGb: number;
}

export const RamUsageCircle: React.FC<RamUsageCircleProps> = ({ totalGb, usedGb }) => {
  const percentUsed = (usedGb / totalGb) * 100;
  const circumference = 314; // 2 * π * r where r = 50
  const usedStroke = (percentUsed / 100) * circumference;

  return (
    <div className="flex justify-center mb-8">
      <div className="relative w-32 h-32">
        <svg className="w-32 h-32 transform -rotate-90" viewBox="0 0 120 120">
          <circle
            cx="60"
            cy="60"
            r="50"
            stroke="rgba(75, 85, 99, 0.3)" // Tailwind gray-600 at 30%
            strokeWidth="8"
            fill="none"
          />
          <circle
            cx="60"
            cy="60"
            r="50"
            stroke="#f97316" // orange-500
            strokeWidth="8"
            fill="none"
            strokeDasharray={`${usedStroke} ${circumference}`}
            strokeLinecap="round"
          />
        </svg>
        <div className="absolute inset-0 flex flex-col items-center justify-center">
          <span className="text-gray-400 text-sm">Used</span>
          <span className="text-white text-xl font-bold">{usedGb.toFixed(1)} GB</span>
        </div>
      </div>
    </div>
  );
};
