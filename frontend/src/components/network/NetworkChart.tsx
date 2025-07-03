import React, { useEffect, useState } from 'react';
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
import { NetworkData } from '../../types/network';

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

interface NetworkChartProps {
  data: NetworkData;
}

export const NetworkChart: React.FC<NetworkChartProps> = ({ data }) => {
  const [timeLabels, setTimeLabels] = useState<string[]>([]);
  const [incomingHistory, setIncomingHistory] = useState<number[]>(new Array(30).fill(0));
  const [outgoingHistory, setOutgoingHistory] = useState<number[]>(new Array(30).fill(0));

  useEffect(() => {
    // Generate time labels for the last 30 seconds
    const labels = [];
    for (let i = 29; i >= 0; i--) {
      labels.push(`${i}s`);
    }
    setTimeLabels(labels);
  }, []);

  useEffect(() => {
    // Update history with new data
    setIncomingHistory(prev => [...prev.slice(1), data.incoming]);
    setOutgoingHistory(prev => [...prev.slice(1), data.outgoing]);
  }, [data]);

  const chartData = {
    labels: timeLabels,
    datasets: [
      {
        label: 'Incoming',
        data: incomingHistory,
        borderColor: '#f97316',
        backgroundColor: 'rgba(249, 115, 22, 0.1)',
        borderWidth: 2,
        fill: false,
        tension: 0.4,
      },
      {
        label: 'Outgoing',
        data: outgoingHistory,
        borderColor: '#3b82f6',
        backgroundColor: 'rgba(59, 130, 246, 0.1)',
        borderWidth: 2,
        fill: false,
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
        grid: {
          color: 'rgba(255, 255, 255, 0.1)',
        },
        ticks: {
          color: '#9ca3af',
          callback: function(value) {
            return value + ' MB/s';
          },
        },
      },
    },
    plugins: {
      legend: {
        display: true,
        labels: {
          color: '#9ca3af',
        },
      },
      tooltip: {
        backgroundColor: 'rgba(0, 0, 0, 0.8)',
        titleColor: '#fff',
        bodyColor: '#fff',
        callbacks: {
          label: function(context) {
            return `${context.dataset.label}: ${context.parsed.y} MB/s`;
          },
        },
      },
    },
  };

  return (
    <div className="bg-gray-800 rounded-lg p-6">
      <div className="flex items-center justify-between mb-6">
        <h2 className="text-xl font-semibold text-white">Network Usage</h2>
        <span className="text-gray-400 text-sm">
          Detailed information on network data
        </span>
      </div>
      
      <div className="h-64">
        <Line data={chartData} options={options} />
      </div>
    </div>
  );
};