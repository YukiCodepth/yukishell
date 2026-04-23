import { useState, useEffect, useRef } from 'react'
import './index.css'
import BootScreen from './components/BootScreen'
import Sidebar from './components/Sidebar'
import TopBar from './components/TopBar'
import TerminalPane from './components/TerminalPane'
import DashboardPane from './components/DashboardPane'

export default function App() {
  const [booting, setBooting] = useState(true)
  const [activeTab, setActiveTab] = useState('terminal')

  useEffect(() => {
    const t = setTimeout(() => setBooting(false), 3200)
    return () => clearTimeout(t)
  }, [])

  if (booting) return <BootScreen />

  return (
    <div className="app-shell fade-in">
      <Sidebar activeTab={activeTab} setActiveTab={setActiveTab} />
      <div className="main-content">
        <TopBar />
        <div className="workspace">
          <TerminalPane />
          <DashboardPane />
        </div>
      </div>
    </div>
  )
}
