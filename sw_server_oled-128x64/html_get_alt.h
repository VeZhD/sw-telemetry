const char index_html_alt[] PROGMEM = { R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Race Timer</title>
  <style>
    :root {
      --bg: #0a0b0f;
      --fg: #e8f0ff;
      --muted: #9aa4b2;
      --accent: #4ade80; /* green */
      --warn: #f59e0b;   /* amber */
      --bad: #ef4444;    /* red */
      --card: #12131a;
      --row: #161823;
    }
    * { box-sizing: border-box; }
    html, body { height: 100%; margin: 0; background: var(--bg); color: var(--fg); font-family: system-ui, -apple-system, Segoe UI, Roboto, Ubuntu, Cantarell, Noto Sans, Arial, "Apple Color Emoji", "Segoe UI Emoji"; }
    .wrap { max-width: 900px; margin: 0 auto; padding: 16px; display: grid; gap: 16px; }
    header { display: flex; align-items: center; justify-content: space-between; }
    h1 { font-size: 20px; margin: 0; letter-spacing: 0.3px; color: var(--fg); }
    .status { display: inline-flex; align-items: center; gap: 8px; font-size: 13px; color: var(--muted); }
    .dot { width: 10px; height: 10px; border-radius: 50%; background: var(--muted); box-shadow: 0 0 0 2px #00000033; }
    .dot.live { background: var(--accent); box-shadow: 0 0 12px 0 #22c55e66; }
    .dot.idle { background: var(--muted); }
    .dot.err  { background: var(--bad); box-shadow: 0 0 12px 0 #ef444480; }

    .cards { display: grid; gap: 16px; grid-template-columns: repeat(auto-fit, minmax(260px, 1fr)); }
    .card { background: var(--card); border: 1px solid #ffffff14; border-radius: 14px; padding: 16px; }

    .big { font-variant-numeric: tabular-nums; font-size: 40px; line-height: 1; letter-spacing: 0.5px; }
    .sub { color: var(--muted); font-size: 13px; margin-top: 8px; }

    .liveTime { color: var(--accent); text-shadow: 0 0 18px #22c55e44; }
    .idleTime { color: var(--fg); opacity: 0.9; }

    .list { margin: 0; padding: 0; list-style: none; max-height: 420px; overflow: auto; border-top: 1px dashed #ffffff22; margin-top: 10px; }
    .row { display: grid; grid-template-columns: 56px 1fr 80px; align-items: center; gap: 10px; padding: 8px 6px; border-bottom: 1px dashed #ffffff10; background: transparent; }
    .row:nth-child(2n) { background: var(--row); }
    .idx { color: var(--muted); font-size: 12px; text-align: right; padding-right: 6px; }
    .time { font-variant-numeric: tabular-nums; }
    .badge { justify-self: end; font-size: 11px; padding: 2px 8px; border-radius: 20px; border: 1px solid #ffffff20; color: var(--muted); }
    .badge.best { color: #22c55e; border-color: #22c55e55; }

    .hint { font-size: 12px; color: var(--muted); }
    .errbox { font-size: 13px; color: #fecaca; background: #7f1d1d; border: 1px solid #fecaca33; padding: 8px 10px; border-radius: 10px; display: none; }
    .footer { color: var(--muted); font-size: 11px; text-align: center; margin-top: 6px; }
    button { background: #1f2937; color: var(--fg); border: 1px solid #ffffff22; border-radius: 10px; padding: 8px 10px; font-size: 12px; cursor: pointer; }
    button:hover { background: #263244; }
  </style>
</head>
<body>
  <div class="wrap">
    <header>
      <h1>Race Timer</h1>
      <div class="status"><span id="dot" class="dot idle" aria-hidden="true"></span><span id="statusText">idle</span></div>
    </header>

    <div class="cards">
      <section class="card" aria-live="polite">
        <div class="sub">Current lap</div>
        <div id="laptime" class="big idleTime">00:00.000</div>
        <div class="sub" id="lapHint">Waiting…</div>
      </section>

      <section class="card">
        <div class="sub">Best time</div>
        <div id="toptime" class="big">—</div>
        <div class="sub" id="topHint">No record yet</div>
      </section>

      <section class="card">
        <div style="display:flex;align-items:center;justify-content:space-between;gap:8px;">
          <div>
            <div class="sub">Attempts history</div>
            <div class="hint">Newer entries appear at the top.</div>
          </div>
          <div style="display:flex;gap:8px;">
            <button id="btnClear" title="Clear list view (visual only)">Clear view</button>
            <button id="btnPause" title="Pause/Resume polling">Pause</button>
          </div>
        </div>
        <ul id="lastList" class="list" aria-live="polite"></ul>
      </section>
    </div>

    <div id="err" class="errbox" role="alert"></div>
    <div class="footer">Endpoint: <code id="endpointLabel"></code> • Poll: <code id="pollMsLabel"></code> ms • Override via <code>?endpoint=&lt;url&gt;&amp;poll_ms=&lt;n&gt;</code></div>
  </div>

  <script>
    // ==== Config (override via URL params) ====
    const params = new URLSearchParams(location.search);
    const API_URL  = params.get('endpoint') || '/status';
    const POLL_MS  = Math.max(100, +(params.get('poll_ms') || 250));

    // ==== Elements ====
    const elLap = document.getElementById('laptime');
    const elLapHint = document.getElementById('lapHint');
    const elTop = document.getElementById('toptime');
    const elTopHint = document.getElementById('topHint');
    const elList = document.getElementById('lastList');
    const elErr = document.getElementById('err');
    const elDot = document.getElementById('dot');
    const elStatus = document.getElementById('statusText');
    const elEndpointLabel = document.getElementById('endpointLabel');
    const elPollMsLabel = document.getElementById('pollMsLabel');
    const btnPause = document.getElementById('btnPause');
    const btnClear = document.getElementById('btnClear');

    elEndpointLabel.textContent = API_URL;
    elPollMsLabel.textContent = POLL_MS;

    // ==== Helpers ====
    function pad(n, w=2) { return String(n).padStart(w, '0'); }
    function formatMs(ms) {
      ms = Math.max(0, Math.floor(ms || 0));
      const m = Math.floor(ms / 60000);
      const s = Math.floor((ms % 60000) / 1000);
      const x = ms % 1000;
      return `${pad(m)}:${pad(s)}.${String(x).padStart(3,'0')}`;
    }

    function setState({ live, err }) {
      elDot.classList.remove('live','idle','err');
      if (err) {
        elDot.classList.add('err');
        elStatus.textContent = 'error';
      } else if (live) {
        elDot.classList.add('live');
        elStatus.textContent = 'live';
      } else {
        elDot.classList.add('idle');
        elStatus.textContent = 'idle';
      }
    }

    function render(data) {
      // data: { laptime, toptime, raceState, LastTime: number[] }
      const { laptime=0, toptime=599999, raceState=0, LastTime=[] } = data || {};

      // Current lap
      elLap.textContent = formatMs(laptime);
      if (raceState) {
        elLap.classList.remove('idleTime');
        elLap.classList.add('liveTime');
        elLapHint.textContent = 'Lap in progress';
        document.title = `⏱️ ${formatMs(laptime)} — Race Timer`;
      } else {
        elLap.classList.remove('liveTime');
        elLap.classList.add('idleTime');
        elLapHint.textContent = 'Idle';
        document.title = `Race Timer`;
      }

      // Best time
      if (typeof toptime === 'number' && toptime >= 0 && toptime < 599999) {
        elTop.textContent = formatMs(toptime);
        elTopHint.textContent = 'Personal best';
      } else {
        elTop.textContent = '—';
        elTopHint.textContent = 'No record yet';
      }

      // Attempts list (show non-zero entries; keep zeros out to reduce noise)
      elList.innerHTML = '';
      const nonZero = (LastTime || []).filter(n => Number(n) > 0);
      if (nonZero.length === 0) {
        const li = document.createElement('li');
        li.className = 'row';
        li.innerHTML = `<div class="idx">—</div><div class="time">No attempts yet</div><div></div>`;
        elList.appendChild(li);
      } else {
        const best = Math.min(...nonZero);
        nonZero.forEach((val, i) => {
          const li = document.createElement('li');
          li.className = 'row';
          const isBest = val === best;
          const delta = (isBest || !isFinite(best)) ? '' : `+${formatMs(val - best)}`;
          li.innerHTML = `
            <div class="idx">#${i+1}</div>
            <div class="time">${formatMs(val)}</div>
            <div class="badge ${isBest ? 'best' : ''}">${isBest ? 'BEST' : (delta || '')}</div>
          `;
          elList.appendChild(li);
        });
        // Auto-scroll to newest
        elList.scrollTop = elList.scrollHeight;
      }
    }

    // ==== Polling ====
    let timer = null;
    let paused = false;

    async function tick() {
      if (paused) return;
      try {
        const res = await fetch(API_URL, { cache: 'no-store' });
        if (!res.ok) throw new Error(`HTTP ${res.status}`);
        const data = await res.json();
        render(data);
        elErr.style.display = 'none';
        setState({ live: data?.raceState === 1, err: false });
      } catch (e) {
        elErr.textContent = `Fetch failed: ${e?.message || e}`;
        elErr.style.display = 'block';
        setState({ live: false, err: true });
      }
    }

    function start() {
      if (timer) clearInterval(timer);
      timer = setInterval(tick, POLL_MS);
      tick();
    }

    // ==== Controls ====
    btnPause.addEventListener('click', () => {
      paused = !paused;
      btnPause.textContent = paused ? 'Resume' : 'Pause';
      if (!paused) tick();
    });

    btnClear.addEventListener('click', () => {
      elList.innerHTML = '';
    });

    // ==== Boot ====
    setState({ live: false, err: false });
    start();
  </script>
</body>
</html>
)rawliteral"};
