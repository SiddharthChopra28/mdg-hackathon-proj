import React, { useEffect, useRef, useState } from 'react';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  ChartOptions,
} from 'chart.js';
import { Line } from 'react-chartjs-2';

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

interface CPUChartProps {
  data: number[];
  processorInfo: any;
}

export const CPUChart: React.FC<CPUChartProps> = ({ data, processorInfo }) => {
  const [timeLabels, setTimeLabels] = useState<string[]>([]);

  useEffect(() => {
    // Generate time labels for the last 30 seconds
    const labels = [];
    for (let i = 29; i >= 0; i--) {
      labels.push(`${i}s`);
    }
    setTimeLabels(labels);
  }, []);

  const chartData = {
    labels: timeLabels,
    datasets: [
      {
        label: 'CPU Usage',
        data: data,
        borderColor: '#f97316',
        backgroundColor: 'rgba(249, 115, 22, 0.1)',
        borderWidth: 2,
        fill: true,
        tension: 0.4,
      },
    ],
  };

  const options: ChartOptions<'line'> = {
    responsive: true,
    maintainAspectRatio: false,
    scales: {
      x: {
        grid: {
          color: 'rgba(255, 255, 255, 0.1)',
        },
        ticks: {
          color: '#9ca3af',
        },
      },
      y: {
        beginAtZero: true,
        max: 100,
        grid: {
          color: 'rgba(255, 255, 255, 0.1)',
        },
        ticks: {
          color: '#9ca3af',
        },
      },
    },
    plugins: {
      legend: {
        display: false,
      },
      tooltip: {
        backgroundColor: 'rgba(0, 0, 0, 0.8)',
        titleColor: '#fff',
        bodyColor: '#fff',
      },
    },
  };

  return (
    <div className="bg-gray-800 rounded-lg p-6">
      <div className="flex items-center justify-between mb-6">
        <h2 className="text-xl font-semibold text-white">% Utilization</h2>
        <span className="text-gray-400 text-sm">
          {processorInfo.model_name || 'AMD Ryzen 5 2600 Six-Core Processor'}
        </span>
      </div>
      
      <div className="h-64 mb-6">
        <Line data={chartData} options={options} />
      </div>

      <div className="grid grid-cols-2 gap-6">
        <div className="space-y-4">
          <div>
            <span className="text-gray-400 text-sm">Utilization</span>
            <p className="text-2xl font-bold text-white">
              {data[data.length - 1] || 0}%
            </p>
          </div>
          <div>
            <span className="text-gray-400 text-sm">Speed</span>
            <p className="text-2xl font-bold text-white">
              {processorInfo.base_speed_ghz || 1.6} GHz
            </p>
          </div>
        </div>
        
        <div className="space-y-4">
          <div>
            <span className="text-gray-400 text-sm">Base Speed:</span>
            <p className="text-lg font-semibold text-white">
              {processorInfo.base_speed_ghz || 3.40} GHz
            </p>
          </div>
          <div>
            <span className="text-gray-400 text-sm">Cores:</span>
            <p className="text-lg font-semibold text-white">
              {processorInfo.core_count || 6}
            </p>
          </div>
        </div>
      </div>
    </div>
  );
};