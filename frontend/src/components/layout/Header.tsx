import React from 'react';
import { Activity } from 'lucide-react';
import { Link } from 'react-router-dom';

export const Header: React.FC = () => {
  return (
    <header className="bg-gray-800 border-b border-gray-700 px-6 py-4">
      <div className="flex items-center justify-between">
        <div className="flex items-center space-x-3">
          <Activity className="w-8 h-8 text-orange-500" />
          <div>
            <h1 className="text-2xl font-bold text-white">System Monitor</h1>
            <p className="text-gray-400 text-sm">Monitor and Optimize system tasks</p>
          </div>
        </div>
        <nav className="flex space-x-6">
          <Link to="/" className="text-gray-300 hover:text-white transition-colors">Home</Link>
          <Link to="/about" className="text-gray-300 hover:text-white transition-colors">About Us</Link>
        </nav>
      </div>
    </header>
  );
};