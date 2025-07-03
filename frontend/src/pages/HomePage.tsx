import React, { useEffect, useRef } from 'react';
import { useNavigate, useLocation } from 'react-router-dom';
import { motion } from 'framer-motion';
import { FaMicrochip, FaMemory, FaNetworkWired, FaAngleDown } from 'react-icons/fa';

export const HomePage: React.FC = () => {
  const navigate = useNavigate();
  const location = useLocation();


  const aboutSectionRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (location.hash === '#about' && aboutSectionRef.current) {
      aboutSectionRef.current.scrollIntoView({
        behavior: 'smooth',
        block: 'start',
      });
    }
  }, [location]); 

  const cards = [
    {
      title: 'RAM',
      description: 'Monitor real-time memory usage and performance.',
      icon: <FaMemory className="h-10 w-10 text-cyan-400" />,
      path: '/ram' 
    },
    {
      title: 'CPU',
      description: 'Analyze core utilization and processing loads.',
      icon: <FaMicrochip className="h-10 w-10 text-rose-400" />,
      path: '/cpu' 
    },
    {
      title: 'Network',
      description: 'Track data packets and network activity.',
      icon: <FaNetworkWired className="h-10 w-10 text-indigo-400" />,
      path: '/network'
    }
  ];

  const containerVariants = {
    hidden: { opacity: 0 },
    visible: { opacity: 1, transition: { staggerChildren: 0.2 } },
  };

  const cardVariants = {
    hidden: { y: 50, opacity: 0 },
    visible: { y: 0, opacity: 1 },
  };

  return (
    <div className="min-h-screen bg-slate-900 text-white overflow-y-auto">
      <div className="absolute inset-0 z-0 h-full w-full bg-transparent bg-[linear-gradient(to_right,#80808012_1px,transparent_1px),linear-gradient(to_bottom,#80808012_1px,transparent_1px)] bg-[size:24px_24px]"></div>
      
      <div className="relative z-10 flex h-screen flex-col items-center justify-center text-center px-4">
        <motion.h1 
          className="text-5xl font-bold md:text-7xl bg-clip-text text-transparent bg-gradient-to-r from-cyan-400 to-blue-600"
          initial={{ opacity: 0, y: -50 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.7 }}
        >
          System Monitor App
        </motion.h1>
        <motion.p 
          className="mt-4 max-w-2xl text-lg text-slate-300"
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          transition={{ delay: 0.5, duration: 0.7 }}
        >
          An elegant way to visualize your system's core metrics.
          Scroll down to explore.
        </motion.p>
        <motion.div
          className="absolute bottom-10"
          animate={{ y: [0, 10, 0] }}
          transition={{ duration: 1.5, repeat: Infinity, repeatType: 'loop' }}
        >
          <FaAngleDown className="h-8 w-8 text-slate-500" />
        </motion.div>
      </div>

      <motion.div
        ref={aboutSectionRef}
        className="relative z-10 w-full max-w-4xl mx-auto px-8 py-24 text-center"
        initial={{ opacity: 0 }}
        whileInView={{ opacity: 1 }}
        transition={{ duration: 0.8 }}
        viewport={{ once: true, amount: 0.5 }}
      >
        <h2 className="text-4xl font-bold mb-6 bg-clip-text text-transparent bg-gradient-to-r from-purple-400 to-indigo-500">
          About Our Project
        </h2>
        <p className="text-lg text-slate-300 leading-relaxed">
          We are a team of four passionate members from MDG Space, participating in an internal hackathon.
          Our project is a high-performance system monitor that provides deep insights into 
          <span className="font-semibold text-cyan-400"> RAM</span>,
          <span className="font-semibold text-rose-400"> CPU</span>, and
          <span className="font-semibold text-indigo-400"> Network</span> activity.
          By leveraging the power of <strong className="text-white">eBPF</strong> for efficient kernel-level data collection and integrating familiar <strong className="text-white">CLI tools</strong>, 
          we've built a robust and elegant solution for real-time system analysis using eBPF.
        </p>
      </motion.div>

      <motion.div
        className="relative z-10 w-full max-w-6xl mx-auto grid grid-cols-1 md:grid-cols-3 gap-8 px-8 pb-24"
        variants={containerVariants}
        initial="hidden"
        whileInView="visible"
        viewport={{ once: true, amount: 0.5 }}
      >
        {cards.map((card) => (
          <motion.div
            key={card.title}
            className="group relative cursor-pointer overflow-hidden rounded-2xl bg-slate-800/50 p-8 text-center backdrop-blur-sm border border-white/10"
            variants={cardVariants}
            onClick={() => navigate(card.path)}
          >
            <div className="absolute inset-0 bg-cyan-400/10 opacity-0 transition-opacity duration-300 group-hover:opacity-100"></div>
            <div className="relative z-10 flex flex-col items-center">
              <div className="mb-4 flex h-20 w-20 items-center justify-center rounded-full bg-slate-900/80">
                {card.icon}
              </div>
              <h2 className="text-2xl font-semibold text-white">{card.title}</h2>
              <p className="mt-2 text-slate-400">{card.description}</p>
            </div>
          </motion.div>
        ))}
      </motion.div>
    </div>
  );
};