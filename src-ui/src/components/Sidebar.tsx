import { Terminal, Cpu, Radio, Shield, GitBranch, Settings } from 'lucide-react'

const NAV = [
  { id: 'terminal', icon: Terminal, label: 'Terminal' },
  { id: 'hardware', icon: Cpu, label: 'Hardware' },
  { id: 'network', icon: Radio, label: 'Network' },
  { id: 'security', icon: Shield, label: 'Security' },
  { id: 'git', icon: GitBranch, label: 'Git' },
]

interface Props {
  activeTab: string
  setActiveTab: (t: string) => void
}

export default function Sidebar({ activeTab, setActiveTab }: Props) {
  return (
    <aside className="sidebar">
      <div className="sidebar-logo" title="E·MUX">E·M</div>
      <div className="sidebar-divider" />
      {NAV.map(({ id, icon: Icon, label }) => (
        <button
          key={id}
          className={`sidebar-btn ${activeTab === id ? 'active' : ''}`}
          title={label}
          onClick={() => setActiveTab(id)}
          id={`sidebar-${id}`}
        >
          <Icon size={18} />
        </button>
      ))}
      <div className="sidebar-spacer" />
      <div className="sidebar-divider" />
      <button className="sidebar-btn" title="Settings" id="sidebar-settings">
        <Settings size={18} />
      </button>
    </aside>
  )
}
