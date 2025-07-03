import React from 'react';
import { Trash2, Check } from 'lucide-react';

interface WhitelistTableProps {
  whitelistItems: string[];
  onRemove: (name: string) => void;
}

export const WhitelistTable: React.FC<WhitelistTableProps> = ({
  whitelistItems,
  onRemove,
}) => {
  return (
    <div className="bg-gray-800 rounded-lg p-6">
      <h2 className="text-xl font-semibold text-white mb-4">Whitelist Tasks</h2>
      <div className="space-y-3">
        {whitelistItems.map((item, index) => (
          <div key={index} className="flex items-center justify-between bg-gray-700 p-3 rounded">
            <div className="flex items-center space-x-3">
              <Check className="w-4 h-4 text-green-500" />
              <span className="text-white">{item}</span>
            </div>
            <button
              onClick={() => onRemove(item)}
              className="text-red-400 hover:text-red-300 transition-colors"
            >
              <Trash2 className="w-4 h-4" />
            </button>
          </div>
        ))}
        {whitelistItems.length === 0 && (
          <p className="text-gray-400 text-sm text-center py-8">
            No whitelisted tasks
          </p>
        )}
      </div>
    </div>
  );
};