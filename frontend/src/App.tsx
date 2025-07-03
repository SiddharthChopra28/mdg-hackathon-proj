import React from 'react';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import { Layout } from './components/layout/Layout';
import { Dashboard } from './pages/Dashboard';
import { NetworkMonitor } from './pages/NetworkMonitor';
import { RamMonitor } from './pages/RamMonitor';

function App() {
  return (
    <Router>
      <Layout>
        <Routes>
          <Route path="/" element={<Dashboard />} />
          <Route path="/network" element={<NetworkMonitor />} />
          <Route path="/ram" element={<RamMonitor />} />
        </Routes>
      </Layout>
    </Router>
  );
}

export default App;