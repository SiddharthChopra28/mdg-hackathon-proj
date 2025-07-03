
import { RamMonitor } from './pages/RamMonitor';
import { NetworkMonitor } from './pages/NetworkMonitor';

import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom'; // <-- Import Navigate
import { Layout } from './components/layout/Layout';
import { Dashboard } from './pages/Dashboard';
import { HomePage } from './pages/HomePage';

function App() {
  return (
    <Router>
      <Routes>
        <Route path="/" element={<HomePage />} />

        <Route path="/about" element={<Navigate to="/#about" replace />} />

        <Route element={<Layout />}>
          <Route path="/cpu" element={<Dashboard />} />
           <Route path="/ram" element={<RamMonitor />} />
           <Route path="/network" element={<NetworkMonitor />} />
        </Route>
      </Routes>
    </Router>
  );
}

export default App;