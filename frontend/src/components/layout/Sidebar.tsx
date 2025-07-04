import React from 'react';
import { Activity, Info, Home, Wifi, Brain } from 'lucide-react';
import { Link, useLocation } from 'react-router-dom';
//
export const Sidebar: React.FC = () => {
  const location = useLocation();

  const isActive = (path: string) => location.pathname === path;

  return (
    <aside className="w-64 h-screen sticky bg-gray-800 border-r border-gray-700 p-4" >
      <nav className="space-y-2">
        <Link to="/" className="flex items-center space-x-3 px-4 py-3 text-gray-300 hover:bg-gray-700 rounded-lg transition-colors">
          <Home className="w-5 h-5" />
          <span>HomePage</span>
        </Link>  
        <Link
          to="/cpu"
          className={`flex items-center space-x-3 px-4 py-3 rounded-lg transition-colors ${isActive('/') ? 'bg-gray-700 text-white' : 'text-gray-300 hover:bg-gray-700'
            }`}
        >
          <Activity className="w-5 h-5" />
          <span>CPU Monitor</span>
        </Link>
        <Link
          to="/network"
          className={`flex items-center space-x-3 px-4 py-3 rounded-lg transition-colors ${isActive('/network') ? 'bg-gray-700 text-white' : 'text-gray-300 hover:bg-gray-700'
            }`}
        >
          <Wifi className="w-5 h-5" />
          <span>Network Monitor</span>
        </Link>
        <Link
          to="/ram"
          className={`flex items-center space-x-3 px-4 py-3 rounded-lg transition-colors ${isActive('/ram') ? 'bg-gray-700 text-white' : 'text-gray-300 hover:bg-gray-700'
            }`}
        >
          <Brain className="w-5 h-5" />
          <span>Ram Monitor</span>
        </Link>
        

        <Link to="/about" className="flex items-center space-x-3 px-4 py-3 text-gray-300 hover:bg-gray-700 rounded-lg transition-colors">
          <Info className="w-5 h-5" />
          <span>About</span>
        </Link>
      </nav>
    </aside>
  );
};        