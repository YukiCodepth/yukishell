import { useState, useRef, useEffect } from 'react'
import { Plus } from 'lucide-react'

interface TermLine {
  type: 'cmd' | 'output' | 'block'
  content: string
  style?: string
  blockType?: string
  blockLines?: { text: string; cls: string }[]
}

const INITIAL_HISTORY: TermLine[] = [
  { type: 'output', content: '  ███████╗  ███╗   ███╗ ██╗   ██╗ ██╗  ██╗', style: 'info' },
  { type: 'output', content: '  ██╔════╝  ████╗ ████║ ██║   ██║ ╚██╗██╔╝', style: 'info' },
  { type: 'output', content: '  █████╗    ██╔████╔██║ ██║   ██║  ╚███╔╝ ', style: 'accent' },
  { type: 'output', content: '  ██╔══╝    ██║╚██╔╝██║ ██║   ██║  ██╔██╗ ', style: 'info' },
  { type: 'output', content: '  ███████╗  ██║ ╚═╝ ██║ ╚██████╔╝ ██╔╝ ██╗', style: 'info' },
  { type: 'output', content: '  ╚══════╝  ╚═╝     ╚═╝  ╚═════╝  ╚═╝  ╚═╝', style: 'info' },
  { type: 'output', content: '' },
  { type: 'output', content: '  E·MUX v27.0 — God-Mode ECE Terminal', style: 'success' },
  { type: 'output', content: '  MUX_AI Core · Hardware Bridge · Edge AI Deployer', style: '' },
  { type: 'output', content: '' },
  { type: 'cmd', content: 'scan --i2c /dev/i2c-1' },
  {
    type: 'block', content: '', blockType: 'scan',
    blockLines: [
      { text: 'I2C BUS SCAN — /dev/i2c-1', cls: '' },
      { text: '  0x48 ✓ ADS1115  (16-bit ADC)', cls: 'ok' },
      { text: '  0x68 ✓ MPU-6050 (IMU Gyro)', cls: 'ok' },
      { text: '  0x76 ✓ BME280   (Temp/Pressure)', cls: 'ok' },
      { text: '  3 devices found in 48ms', cls: 'info' },
    ]
  },
  { type: 'cmd', content: 'ask "What is the I2C address range for ADS1115?"' },
  {
    type: 'block', content: '', blockType: 'ai',
    blockLines: [
      { text: 'MUX_AI — Hardware Context', cls: '' },
      { text: '  ADS1115 I2C address: 0x48–0x4B', cls: 'ok' },
      { text: '  Configured by ADDR pin: GND=0x48,', cls: '' },
      { text: '  VDD=0x49, SDA=0x4A, SCL=0x4B', cls: '' },
    ]
  },
]

const CANNED: Record<string, TermLine[]> = {
  help: [
    { type: 'output', content: '  scan --i2c <bus>     — scan I2C bus for devices', style: 'info' },
    { type: 'output', content: '  monitor --uart <port> — live UART monitor', style: 'info' },
    { type: 'output', content: '  ask "<query>"         — query MUX_AI', style: 'info' },
    { type: 'output', content: '  build                 — compile C core (emux)', style: 'info' },
    { type: 'output', content: '  xnet <ip>             — TCP port scanner', style: 'info' },
  ],
  build: [
    {
      type: 'block', content: '', blockType: 'success',
      blockLines: [
        { text: 'BUILD — E·MUX Core (src-core/)', cls: '' },
        { text: '  gcc -Wall -g -Iinclude -c src/main.c', cls: 'info' },
        { text: '  gcc -Wall -g -Iinclude -c src/builtins.c', cls: 'info' },
        { text: '  gcc -Wall -g -Iinclude -c src/executor.c', cls: 'info' },
        { text: '  gcc -Wall -g -Iinclude -c src/parser.c', cls: 'info' },
        { text: '  Linked → ./emux  [0 errors, 0 warnings]', cls: 'ok' },
      ]
    }
  ],
}

export default function TerminalPane() {
  const [history, setHistory] = useState<TermLine[]>(INITIAL_HISTORY)
  const [input, setInput] = useState('')
  const bottomRef = useRef<HTMLDivElement>(null)

  useEffect(() => {
    bottomRef.current?.scrollIntoView({ behavior: 'smooth' })
  }, [history])

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault()
    const cmd = input.trim()
    if (!cmd) return
    const newLines: TermLine[] = [{ type: 'cmd', content: cmd }]
    const key = cmd.toLowerCase().split(' ')[0]
    if (CANNED[key]) {
      newLines.push(...CANNED[key])
    } else {
      newLines.push({ type: 'output', content: `  emux: command processed: ${cmd}`, style: '' })
    }
    setHistory(h => [...h, ...newLines])
    setInput('')
  }

  const getOutputStyle = (style?: string) => {
    if (style === 'success') return 'term-output success'
    if (style === 'error') return 'term-output error'
    if (style === 'info') return 'term-output info'
    if (style === 'accent') return 'term-output info'
    return 'term-output'
  }

  return (
    <div className="terminal-pane">
      <div className="pane-header">
        <div className="tab-bar">
          <div className="tab active">⚡ emux — main</div>
          <div className="tab">🔌 uart-monitor</div>
          <div className="tab">🔬 i2c-scanner</div>
        </div>
        <button className="sidebar-btn" style={{ width: 28, height: 28 }} title="New tab">
          <Plus size={14} />
        </button>
      </div>

      <div className="terminal-body">
        {history.map((line, i) => {
          if (line.type === 'cmd') {
            return (
              <div key={i} className="term-line" style={{ marginTop: i > 0 ? '8px' : 0 }}>
                <span className="term-prompt">emux ❯</span>
                <span className="term-cmd">{line.content}</span>
              </div>
            )
          }
          if (line.type === 'block') {
            return (
              <div key={i} className={`term-block ${line.blockType || ''}`} style={{ marginTop: 6 }}>
                <div className="term-block-header">{line.blockLines?.[0]?.text}</div>
                {line.blockLines?.slice(1).map((bl, j) => (
                  <div key={j} className={`build-line ${bl.cls}`}>{bl.text}</div>
                ))}
              </div>
            )
          }
          return (
            <div key={i} className={getOutputStyle(line.style)}
              style={{ fontFamily: 'var(--font-mono)', fontSize: 13 }}>
              {line.content}
            </div>
          )
        })}
        <div ref={bottomRef} />
      </div>

      <form className="terminal-input-row" onSubmit={handleSubmit}>
        <span className="terminal-input-prompt">emux ❯</span>
        <input
          id="terminal-input"
          className="terminal-input"
          value={input}
          onChange={e => setInput(e.target.value)}
          placeholder="type a command... (try: help, build, scan --i2c)"
          autoFocus
          spellCheck={false}
          autoComplete="off"
        />
      </form>
    </div>
  )
}
