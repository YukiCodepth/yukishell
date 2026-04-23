export default function TopBar() {
  return (
    <header className="topbar">
      <span className="topbar-title">E·MUX</span>
      <span className="topbar-badge badge-purple">v27.0</span>
      <span className="topbar-badge badge-green">
        <span className="status-dot green" />
        CORE ONLINE
      </span>
      <span className="topbar-badge badge-cyan">MUX_AI READY</span>
      <span className="topbar-badge badge-orange">ECE MODE</span>
      <div className="topbar-spacer" />
      <span className="topbar-badge badge-green">feature/v27-emux-monorepo</span>
    </header>
  )
}
