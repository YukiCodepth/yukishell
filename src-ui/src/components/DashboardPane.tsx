import { useEffect, useRef, useState } from 'react'

// --- Oscilloscope mini chart ---
function OscilloscopeChart() {
  const [points, setPoints] = useState<number[]>(Array(60).fill(50))
  const frameRef = useRef(0)

  useEffect(() => {
    let t = 0
    const id = setInterval(() => {
      t += 0.15
      const v = 50 + 35 * Math.sin(t) + (Math.random() - 0.5) * 8
      setPoints(p => [...p.slice(1), Math.max(5, Math.min(95, v))])
    }, 60)
    return () => clearInterval(id)
  }, [])

  const W = 352, H = 100
  const path = points.map((v, i) => `${i === 0 ? 'M' : 'L'}${(i / (points.length - 1)) * W},${H - (v / 100) * H}`).join(' ')

  return (
    <div className="oscilloscope">
      <svg className="oscilloscope-svg" viewBox={`0 0 ${W} ${H}`} preserveAspectRatio="none">
        <defs>
          <linearGradient id="wave-grad" x1="0" y1="0" x2="0" y2="1">
            <stop offset="0%" stopColor="#0ff0fc" stopOpacity="0.5" />
            <stop offset="100%" stopColor="#0ff0fc" stopOpacity="0" />
          </linearGradient>
        </defs>
        {/* Grid lines */}
        {[25, 50, 75].map(y => (
          <line key={y} x1="0" y1={H * y / 100} x2={W} y2={H * y / 100}
            stroke="rgba(255,255,255,0.04)" strokeWidth="1" />
        ))}
        {/* Fill area */}
        <path d={`${path} L${W},${H} L0,${H} Z`} fill="url(#wave-grad)" />
        {/* Waveform line */}
        <path d={path} fill="none" stroke="#0ff0fc" strokeWidth="1.5"
          style={{ filter: 'drop-shadow(0 0 4px #0ff0fc)' }} />
      </svg>
    </div>
  )
}

// --- System resource bars ---
const RESOURCES = [
  { label: 'CPU', value: 34, fill: 'fill-purple' },
  { label: 'RAM', value: 58, fill: 'fill-cyan' },
  { label: 'GPU', value: 12, fill: 'fill-green' },
  { label: 'DISK', value: 71, fill: 'fill-orange' },
]

// --- I2C found devices ---
const I2C_FOUND = ['0x48', '0x68', '0x76']
const I2C_ALL = ['0x40', '0x48', '0x4A', '0x50', '0x60', '0x68', '0x70', '0x76']

// --- UART stream lines ---
const UART_LINES = [
  { ts: '11:42:01.003', data: 'TEMP=24.8C HUM=61.2%' },
  { ts: '11:42:01.503', data: 'TEMP=24.9C HUM=61.0%' },
  { ts: '11:42:02.003', data: 'ACCEL X=0.02 Y=-0.01 Z=9.81' },
  { ts: '11:42:02.503', data: 'BATT=3.74V  RSSI=-68dBm' },
]

// --- Connected hardware ---
const HW_DEVICES = [
  { name: 'STM32F4 Discovery', port: '/dev/ttyUSB0', status: 'connected' as const },
  { name: 'J-Link EDU Mini', port: 'USB-HID', status: 'connected' as const },
  { name: 'Logic Analyzer', port: '/dev/ttyUSB1', status: 'scanning' as const },
]

export default function DashboardPane() {
  const [cpuVal, setCpuVal] = useState(34)
  useEffect(() => {
    const id = setInterval(() => setCpuVal(v => Math.max(10, Math.min(95, v + (Math.random() - 0.5) * 10))), 1500)
    return () => clearInterval(id)
  }, [])

  return (
    <div className="dashboard-pane slide-in-right">

      {/* Live Telemetry */}
      <div className="widget">
        <div className="widget-title cyan">Live Telemetry — UART</div>
        <OscilloscopeChart />
        <div className="uart-stream" style={{ marginTop: 10 }}>
          {UART_LINES.map((l, i) => (
            <div key={i} className="uart-line">
              <span className="uart-ts">{l.ts}</span>
              <span className="uart-data">{l.data}</span>
            </div>
          ))}
        </div>
      </div>

      {/* System Resources */}
      <div className="widget">
        <div className="widget-title purple">System Resources</div>
        <div className="stat-grid" style={{ marginBottom: 12 }}>
          <div className="stat-card">
            <div className="stat-label">CPU</div>
            <div className="stat-value accent-purple">{cpuVal.toFixed(0)}%</div>
            <div className="stat-sub">8-core ARM</div>
          </div>
          <div className="stat-card">
            <div className="stat-label">RAM</div>
            <div className="stat-value accent-cyan">3.7 GB</div>
            <div className="stat-sub">/ 8.0 GB</div>
          </div>
          <div className="stat-card">
            <div className="stat-label">PROCESSES</div>
            <div className="stat-value accent-green">247</div>
            <div className="stat-sub">3 shadow</div>
          </div>
          <div className="stat-card">
            <div className="stat-label">UPTIME</div>
            <div className="stat-value accent-orange">4h 12m</div>
            <div className="stat-sub">stable</div>
          </div>
        </div>
        {RESOURCES.map(r => (
          <div key={r.label} className="progress-bar-container">
            <div className="progress-label">
              <span>{r.label}</span>
              <span>{r.value}%</span>
            </div>
            <div className="progress-track">
              <div className={`progress-fill ${r.fill}`} style={{ width: `${r.value}%` }} />
            </div>
          </div>
        ))}
      </div>

      {/* I2C Bus */}
      <div className="widget">
        <div className="widget-title green">I²C Bus Scan — /dev/i2c-1</div>
        <div className="i2c-grid">
          {I2C_ALL.map(addr => (
            <span key={addr} className={`i2c-addr ${I2C_FOUND.includes(addr) ? 'found' : 'empty'}`}>
              {addr}
            </span>
          ))}
        </div>
        <div style={{ marginTop: 10, fontSize: 11, color: 'var(--text-muted)', fontFamily: 'var(--font-mono)' }}>
          3 devices found · ADS1115 · MPU-6050 · BME280
        </div>
      </div>

      {/* Hardware Devices */}
      <div className="widget">
        <div className="widget-title orange">Connected Hardware</div>
        <div className="hw-status-row">
          {HW_DEVICES.map(d => (
            <div key={d.name} className="hw-device">
              <span className="status-dot" style={{
                background: d.status === 'connected' ? 'var(--accent-green)' : 'var(--accent-yellow)',
                boxShadow: `0 0 6px ${d.status === 'connected' ? 'var(--accent-green)' : 'var(--accent-yellow)'}`,
                width: 7, height: 7, borderRadius: '50%', flexShrink: 0
              }} />
              <span className="hw-device-name">{d.name}</span>
              <span className="hw-device-port">{d.port}</span>
              <span className={`hw-device-status ${d.status}`}>{d.status.toUpperCase()}</span>
            </div>
          ))}
        </div>
      </div>

      {/* MUX AI */}
      <div className="widget">
        <div className="widget-title purple">MUX_AI — Neural Co-Pilot</div>
        <div className="ai-thinking" style={{ marginBottom: 10 }}>
          <div className="ai-dots">
            <div className="ai-dot" /><div className="ai-dot" /><div className="ai-dot" />
          </div>
          Analyzing hardware context...
        </div>
        <div className="ai-response-card">
          <strong style={{ color: 'var(--accent-primary)' }}>Hardware Insight:</strong><br />
          MPU-6050 at 0x68 is reporting stable IMU data. Consider enabling the FIFO buffer
          (register 0x74) to reduce I²C polling overhead by ~60% on your STM32 interrupt handler.
        </div>
        <div style={{ marginTop: 8, display: 'flex', gap: 6 }}>
          <span className="phase-badge">Phase v27</span>
          <span className="topbar-badge badge-green" style={{ fontSize: 10 }}>Gemini 2.0</span>
        </div>
      </div>

    </div>
  )
}
