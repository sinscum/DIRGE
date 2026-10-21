// SPDX-License-Identifier: GPL-3.0-only
// DIRGE: the oscilloscope over the panel (web/emu/index.html imports it; web/emu/build.sh copies it). Before the switch-on (and in silence) a flatline with the odd twitch;
// once the emulator runs, its own output, drawn as a phosphor trace that leaves a short afterglow.
const canvas = document.getElementById('scope')
const g = canvas.getContext('2d')
const still = matchMedia('(prefers-reduced-motion: reduce)')
let analyser = null
let buf = null

export function attachScope(ctx, node) {
  analyser = ctx.createAnalyser()
  analyser.fftSize = 2048
  analyser.smoothingTimeConstant = 0
  buf = new Float32Array(analyser.fftSize)
  node.connect(analyser)
}

function fit() {
  const r = canvas.getBoundingClientRect()
  const d = Math.min(devicePixelRatio || 1, 2)
  canvas.width = Math.max(1, Math.round(r.width * d))
  canvas.height = Math.max(1, Math.round(r.height * d))
}
addEventListener('resize', fit)
fit()

const css = (name) => getComputedStyle(document.documentElement).getPropertyValue(name).trim()

// start where the wave crosses zero going up, so a held note stands still instead of crawling
function trigger(b, n) {
  for (let i = 1; i < b.length - n; i++) if (b[i - 1] < 0 && b[i] >= 0) return i
  return 0
}

let t = 0
function draw() {
  const w = canvas.width
  const h = canvas.height
  const mid = h / 2
  g.fillStyle = 'rgba(5, 3, 4, 0.32)' // the afterglow: last frames fade instead of vanishing
  g.fillRect(0, 0, w, h)
  g.strokeStyle = css('--scope-grid')
  g.lineWidth = 1
  g.beginPath()
  for (let x = 0; x <= w; x += w / 16) g.moveTo(Math.round(x) + 0.5, 0), g.lineTo(Math.round(x) + 0.5, h)
  g.moveTo(0, Math.round(mid) + 0.5)
  g.lineTo(w, Math.round(mid) + 0.5)
  g.stroke()

  const n = 1024
  let pts = null
  let level = 0
  if (analyser) {
    analyser.getFloatTimeDomainData(buf)
    const s = trigger(buf, n)
    pts = buf.subarray(s, s + n)
    for (const v of pts) level = Math.max(level, Math.abs(v))
  }
  g.strokeStyle = css('--blood-hi')
  g.lineWidth = Math.max(1.5, h / 60)
  g.shadowColor = css('--blood')
  g.shadowBlur = h / 8
  g.beginPath()
  if (level > 0.0005) {
    const gain = Math.min(0.9 / level, 6) * 0.45 // quiet drones still fill the screen, loud ones don't clip it
    for (let i = 0; i < n; i++) {
      const y = mid - pts[i] * gain * h
      i ? g.lineTo((i / (n - 1)) * w, y) : g.moveTo(0, y)
    }
  } else {
    // no signal: the flatline, and now and then a twitch that crosses it
    const beat = still.matches ? -1 : (t % 240) / 240
    for (let x = 0; x <= w; x += 2) {
      const p = x / w
      const d = p - beat
      const blip = beat >= 0 && Math.abs(d) < 0.03 ? Math.sin((d / 0.03) * Math.PI * 2) * Math.exp(-((d / 0.015) ** 2)) * 0.35 : 0
      const y = mid - blip * h
      x ? g.lineTo(x, y) : g.moveTo(0, y)
    }
  }
  g.stroke()
  g.shadowBlur = 0
  t++
  requestAnimationFrame(draw)
}
requestAnimationFrame(draw)
