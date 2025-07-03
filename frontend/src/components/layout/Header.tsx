import React from 'react';
import { Activity } from 'lucide-react';

export const Header: React.FC = () => {
  return (
    <header className="bg-gray-800 border-b border-gray-700 px-6 py-4">
      <div className="flex items-center justify-between">
        <div className="flex items-center space-x-3">
          <Activity className="w-8 h-8 text-orange-500" />
          <div>
            <h1 className="text-2xl font-bold text-white">CPU Monitor</h1>
            <p className="text-gray-400 text-sm">Manage and Optimize CPU tasks</p>
          </div>
        </div>
        <nav className="flex space-x-6">
          <a href="#" className="text-gray-300 hover:text-white transition-colors">Home</a>
          <a href="#" className="text-gray-300 hover:text-white transition-colors">Dashboard</a>
          <a href="#" className="text-gray-300 hover:text-white transition-colors">About Us</a>
        </nav>
      </div>
    </header>
  );
};