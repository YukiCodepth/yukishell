import { useEffect, useState } from 'react'

const BOOT_STEPS = [
  'Initializing E·MUX kernel bridge...',
  'Loading MUX_Ai neural core...',
  'Mounting hardware abstraction layer...',
  'Spawning PTY daemon...',
  'Calibrating telemetry engine...',
  'E·MUX ready.',
]

export default function BootScreen() {
  const [step, setStep] = useState(0)
  const [progress, setProgress] = useState(0)

  useEffect(() => {
    const interval = setInterval(() => {
      setStep(s => {
        const next = Math.min(s + 1, BOOT_STEPS.length - 1)
        setProgress(Math.round((next / (BOOT_STEPS.length - 1)) * 100))
        return next
      })
    }, 480)
    return () => clearInterval(interval)
  }, [])

  return (
    <div className="boot-screen">
      <div className="boot-logo">
        <div className="boot-logo-mark">E·M</div>
        <div className="boot-name">E · M U X</div>
        <div className="boot-subtitle">God-Mode ECE Terminal — v27.0</div>
      </div>
      <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '12px' }}>
        <div className="boot-progress-track">
          <div className="boot-progress-fill" style={{ width: `${progress}%` }} />
        </div>
        <div className="boot-log">{BOOT_STEPS[step]}</div>
      </div>
    </div>
  )
}
