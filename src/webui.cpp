#include "webui.h"
#include "storage.h"
#include "fan.h"
#include "bluos.h"

// Forward Declarations of Web Handlers
void handleRoot();
void handleSetSpeed();
void handleTogglePower();
void handleSaveEndpoint();
void handleStatus();
void handleToggleTemp();

void initWebServer() {
  server.on("/", handleRoot);
  server.on("/set", handleSetSpeed);
  server.on("/toggle", handleTogglePower);
  server.on("/save", handleSaveEndpoint);
  server.on("/toggle-temp", handleToggleTemp);
  server.on("/status", handleStatus);
  server.begin();
  Serial.println("[Web] HTTP Web Server initialized");
}

void handleRoot() {
  static const char INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>
  <title>Smart Fan Controller</title>
  <link href='https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;800&display=swap' rel='stylesheet'>
  <script src='https://cdn.jsdelivr.net/npm/chart.js'></script>
  <style>
    :root {
      --bg-gradient: linear-gradient(135deg, #0f172a 0%, #1e1b4b 100%);
      --card-bg: rgba(30, 41, 59, 0.7);
      --accent: #14b8a6;
      --accent-glow: rgba(20, 184, 166, 0.4);
      --power-on: #10b981;
      --power-off: #ef4444;
      --text: #f8fafc;
      --text-muted: #94a3b8;
    }
    body {
      font-family: 'Outfit', sans-serif;
      background: var(--bg-gradient);
      color: var(--text);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      margin: 0;
      padding: 20px;
      box-sizing: border-box;
    }
    .app-container {
      display: flex;
      flex-direction: row;
      align-items: stretch;
      justify-content: center;
      gap: 25px;
      width: 100%;
      max-width: 900px;
      margin: 0 auto;
      transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
    }
    @media (max-width: 800px) {
      .app-container {
        flex-direction: column;
        align-items: center;
      }
    }
    .card {
      background: var(--card-bg);
      backdrop-filter: blur(16px);
      -webkit-backdrop-filter: blur(16px);
      border: 1px solid rgba(255, 255, 255, 0.1);
      border-radius: 24px;
      padding: 35px 25px;
      width: 100%;
      max-width: 420px;
      box-shadow: 0 20px 40px rgba(0, 0, 0, 0.4);
      text-align: center;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      box-sizing: border-box;
    }
    h1 {
      margin: 0 0 5px 0;
      font-size: 28px;
      font-weight: 800;
      background: linear-gradient(to right, #818cf8, #34d399);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }
    .subtitle {
      font-size: 11px;
      color: var(--text-muted);
      margin-bottom: 25px;
      text-transform: uppercase;
      letter-spacing: 2px;
      font-weight: 600;
    }
    
    /* 3-Way Mode Button Group */
    .mode-group {
      display: flex;
      background: rgba(15, 23, 42, 0.5);
      border: 1px solid rgba(255,255,255,0.08);
      border-radius: 14px;
      padding: 4px;
      margin-bottom: 25px;
      gap: 4px;
    }
    .mode-btn {
      flex: 1;
      background: transparent;
      border: 1px solid transparent;
      outline: none;
      color: var(--text-muted);
      font-family: 'Outfit', sans-serif;
      font-size: 12px;
      font-weight: 800;
      padding: 10px 0;
      border-radius: 10px;
      cursor: pointer;
      transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    }
    .mode-btn:hover {
      color: var(--text);
    }
    .mode-btn.active.off {
      background: rgba(239, 68, 68, 0.12);
      color: var(--power-off);
      box-shadow: 0 0 15px rgba(239, 68, 68, 0.15);
      border-color: rgba(239, 68, 68, 0.4);
    }
    .mode-btn.active.on {
      background: rgba(16, 185, 129, 0.12);
      color: var(--power-on);
      box-shadow: 0 0 15px rgba(16, 185, 129, 0.15);
      border-color: rgba(16, 185, 129, 0.4);
    }
    .mode-btn.active.auto {
      background: rgba(99, 102, 241, 0.12);
      color: #818cf8;
      box-shadow: 0 0 15px rgba(99, 102, 241, 0.15);
      border-color: rgba(99, 102, 241, 0.4);
    }

    /* Stats Layout */
    .stats-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 15px;
      margin-bottom: 25px;
    }
    .stat-box {
      background: rgba(15, 23, 42, 0.4);
      border: 1px solid rgba(255,255,255,0.05);
      border-radius: 16px;
      padding: 15px;
      transition: border-color 0.3s;
    }
    .stat-box.active-speed {
      border-color: rgba(20, 184, 166, 0.2);
    }
    .stat-label {
      font-size: 11px;
      color: var(--text-muted);
      text-transform: uppercase;
      font-weight: 600;
      letter-spacing: 1px;
      margin-bottom: 5px;
    }
    .stat-val {
      font-size: 36px;
      font-weight: 800;
      font-variant-numeric: tabular-nums;
    }
    .stat-pct {
      font-size: 13px;
      color: var(--text-muted);
      font-weight: 600;
    }
    .stat-box.active-speed .stat-pct {
      color: var(--accent);
    }

    /* Status Indicator */
    .status-container {
      margin-bottom: 25px;
    }
    .status-indicator {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      padding: 6px 14px;
      border-radius: 99px;
      font-size: 11px;
      font-weight: 800;
      text-transform: uppercase;
      letter-spacing: 1px;
      background: rgba(15, 23, 42, 0.6);
      border: 1px solid rgba(255,255,255,0.05);
    }
    .status-indicator::before {
      content: '';
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: #475569;
    }
    .status-indicator.off::before { background: var(--power-off); }
    .status-indicator.kickstarting::before {
      background: #f59e0b;
      animation: pulse 1s infinite alternate;
    }
    .status-indicator.ramping::before {
      background: #3b82f6;
      animation: pulse 1s infinite alternate;
    }
    .status-indicator.running::before {
      background: var(--power-on);
      box-shadow: 0 0 8px var(--power-on);
    }

    @keyframes pulse {
      0% { opacity: 0.3; transform: scale(0.8); }
      100% { opacity: 1; transform: scale(1.2); }
    }
    
    /* Slider Customization */
    .slider-wrapper {
      position: relative;
      margin: 25px 0 15px 0;
    }
    input[type=range] {
      -webkit-appearance: none;
      width: 100%;
      background: rgba(255, 255, 255, 0.08);
      height: 12px;
      border-radius: 99px;
      outline: none;
      transition: background 0.3s;
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 28px;
      height: 28px;
      border-radius: 50%;
      background: var(--accent);
      cursor: pointer;
      box-shadow: 0 0 15px var(--accent-glow);
      transition: transform 0.1s, background-color 0.3s;
      border: 3px solid #1e293b;
    }
    input[type=range]::-webkit-slider-thumb:hover {
      transform: scale(1.15);
    }
    input[type=range]::-webkit-slider-thumb:active {
      transform: scale(0.95);
    }
    .labels {
      display: flex;
      justify-content: space-between;
      color: var(--text-muted);
      font-size: 10px;
      font-weight: 600;
      margin-top: 12px;
    }
    
    /* Settings Section */
    .settings-section {
      margin-top: 25px;
      border-top: 1px solid rgba(255, 255, 255, 0.08);
      padding-top: 20px;
      text-align: left;
    }
    .settings-title {
      font-size: 11px;
      font-weight: 800;
      color: var(--text);
      text-transform: uppercase;
      letter-spacing: 1px;
      margin-bottom: 10px;
    }
    .input-wrapper {
      display: flex;
      gap: 8px;
    }
    .endpoint-input {
      flex: 1;
      background: rgba(15, 23, 42, 0.6);
      border: 1px solid rgba(255, 255, 255, 0.1);
      border-radius: 10px;
      padding: 9px 12px;
      color: var(--text);
      font-family: 'Outfit', sans-serif;
      font-size: 13px;
      outline: none;
      transition: border-color 0.3s;
    }
    .endpoint-input:focus {
      border-color: var(--accent);
    }
    .save-btn {
      background: var(--accent);
      color: #1e293b;
      border: none;
      border-radius: 10px;
      padding: 0 14px;
      font-family: 'Outfit', sans-serif;
      font-size: 12px;
      font-weight: 800;
      cursor: pointer;
      transition: all 0.3s;
    }
    .save-btn:hover {
      background: #0d9488;
      box-shadow: 0 0 12px rgba(20, 184, 166, 0.4);
    }
    .bluos-status {
      margin-top: 10px;
      font-size: 11px;
      color: var(--text-muted);
      display: flex;
      align-items: center;
      gap: 6px;
    }
    .status-dot {
      width: 6px;
      height: 6px;
      border-radius: 50%;
      background: #475569;
      transition: background-color 0.3s, box-shadow 0.3s;
    }
    .status-dot.online {
      background: var(--power-on);
      box-shadow: 0 0 6px var(--power-on);
    }
    .status-dot.offline {
      background: var(--power-off);
      box-shadow: 0 0 6px var(--power-off);
    }

    /* Keyboard Hint */
    .keyboard-hint {
      margin-top: 20px;
      font-size: 11px;
      color: var(--text-muted);
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 5px;
    }
    .kbd-btn {
      background: rgba(255,255,255,0.08);
      border: 1px solid rgba(255,255,255,0.1);
      border-radius: 4px;
      padding: 2px 6px;
      font-size: 9px;
      font-weight: 800;
    }
    
    /* Switch Toggle Styling */
    .switch input:checked + .slider {
      background-color: var(--accent);
      box-shadow: 0 0 10px var(--accent-glow);
      border-color: rgba(20, 184, 166, 0.3);
    }
    .switch .slider::before {
      position: absolute;
      content: "";
      height: 16px;
      width: 16px;
      left: 3px;
      bottom: 3px;
      background-color: #f8fafc;
      transition: .3s;
      border-radius: 50%;
    }
    .switch input:checked + .slider::before {
      transform: translateX(20px);
      background-color: #1e293b;
    }
  </style>
  <script>
    let lastSent = 0;
    let isDragging = false;
    let powerMode = 0; // 0=OFF, 1=ON, 2=AUTO
    
    let chart = null;
    let tempHistory = {};
    const maxPoints = 300; // Last 5 minutes (at 1Hz status polling)
    let timestamps = Array(maxPoints).fill("");

    function updateSpeed(val) {
      document.getElementById('speed-val').innerText = val;
      let percent = Math.round((val / 255) * 100);
      document.getElementById('speed-pct').innerText = percent + '%';
      
      let now = Date.now();
      if (now - lastSent > 100) {
        fetch('/set?val=' + val);
        lastSent = now;
      }
    }
    
    function sendFinalSpeed(val) {
      fetch('/set?val=' + val);
    }

    function setMode(mode) {
      fetch('/toggle?state=' + mode)
        .then(() => {
          powerMode = mode;
          updateModeButtons();
        });
    }

    function updateModeButtons() {
      document.querySelectorAll('.mode-btn').forEach(btn => btn.classList.remove('active'));
      if (powerMode === 0) document.getElementById('btn-off').classList.add('active');
      else if (powerMode === 1) document.getElementById('btn-on').classList.add('active');
      else if (powerMode === 2) document.getElementById('btn-auto').classList.add('active');
    }

    function saveEndpoint() {
      const endpoint = document.getElementById('endpoint-input').value;
      fetch('/save?endpoint=' + encodeURIComponent(endpoint))
        .then(r => r.text())
        .then(res => {
          alert('Endpoint saved: ' + endpoint);
        });
    }

    function toggleTempMonitor(enabled) {
      fetch('/toggle-temp?state=' + (enabled ? 1 : 0))
        .then(() => {
          checkStatus();
        });
    }

    function checkStatus() {
      fetch('/status')
        .then(r => r.json())
        .then(data => {
          powerMode = data.mode;
          updateModeButtons();

          // Update status indicator
          const statusInd = document.getElementById('status-ind');
          statusInd.innerText = data.state;
          statusInd.className = 'status-indicator ' + data.state.toLowerCase();

          // Update actual speed
          document.getElementById('actual-val').innerText = data.actual;
          let actualPct = Math.round((data.actual / 255) * 100);
          document.getElementById('actual-pct').innerText = actualPct + '%';

          // Update slider IF user is not dragging
          if (!isDragging) {
            const slider = document.getElementById('speed-slider');
            slider.value = data.slider;
            document.getElementById('speed-val').innerText = data.slider;
            let sliderPct = Math.round((data.slider / 255) * 100);
            document.getElementById('speed-pct').innerText = sliderPct + '%';
          }

          // Update Temp Monitor toggle checkbox
          const tempToggle = document.getElementById('temp-mon-toggle');
          if (tempToggle) {
            tempToggle.checked = data.tempMon;
          }

          // Update BluOS Connection status
          const dot = document.getElementById('bluos-dot');
          const txt = document.getElementById('bluos-text');
          const isMonitoring = (data.tempMon || powerMode === 2);
          
          if (!isMonitoring) {
            dot.className = 'status-dot';
            txt.innerText = 'BluOS Monitor: Inactive';
          } else {
            if (data.bluosConnected) {
              dot.className = 'status-dot online';
              txt.innerText = 'BluOS: Connected (' + (data.bluosPower ? 'Player ON' : 'Player OFF') + ')';
            } else {
              dot.className = 'status-dot offline';
              txt.innerText = 'BluOS: Disconnected (Retrying...)';
            }
          }

          // Handle Graph Visibility & Overlay State
          const graphSection = document.getElementById('temp-section');
          if (!data.tempMon) {
            graphSection.style.display = 'none';
            tempHistory = {};
          } else {
            graphSection.style.display = 'block';
            
            const overlay = document.getElementById('temp-offline-overlay');
            const svg = document.getElementById('temp-svg');
            const legend = document.getElementById('temp-legend');
            
            const isOnlineAndPowered = data.bluosConnected && data.bluosPower;
            const canvas = document.getElementById('tempChart');
            if (!isOnlineAndPowered) {
              overlay.style.display = 'flex';
              if (canvas) canvas.style.opacity = '0.15';
              
              const reasonSpan = document.getElementById('temp-offline-reason');
              if (!data.bluosConnected) {
                reasonSpan.innerText = 'Unreachable / Disconnected';
              } else {
                reasonSpan.innerText = 'Device Powered Off (Standby)';
              }
              
              tempHistory = {}; // Clear history when powered off
              timestamps = Array(maxPoints).fill("");
              if (chart) {
                chart.data.datasets = [];
                chart.data.labels = timestamps;
                chart.update();
              }
            } else {
              overlay.style.display = 'none';
              if (canvas) canvas.style.opacity = '1';
              processTemperatures(data.temps);
            }
          }

          // Update endpoint input
          const epInput = document.getElementById('endpoint-input');
          if (epInput && document.activeElement !== epInput) {
            epInput.value = data.endpoint;
          }
        });
    }

    function initChart() {
      const ctx = document.getElementById('tempChart').getContext('2d');
      chart = new Chart(ctx, {
        type: 'line',
        data: {
          labels: timestamps,
          datasets: []
        },
        options: {
          responsive: true,
          maintainAspectRatio: false,
          elements: {
            point: {
              radius: 0,
              hoverRadius: 5,
              hitRadius: 10
            },
            line: {
              borderWidth: 2,
              tension: 0.35
            }
          },
          plugins: {
            legend: {
              display: true,
              position: 'bottom',
              labels: {
                color: '#94a3b8',
                font: { family: 'Outfit', size: 11, weight: '600' },
                padding: 15,
                usePointStyle: true,
                pointStyle: 'circle'
              }
            },
            tooltip: {
              mode: 'index',
              intersect: false,
              backgroundColor: 'rgba(15, 23, 42, 0.95)',
              titleColor: '#f8fafc',
              bodyColor: '#e2e8f0',
              borderColor: 'rgba(255,255,255,0.08)',
              borderWidth: 1,
              titleFont: { family: 'Outfit', size: 12, weight: '800' },
              bodyFont: { family: 'Outfit', size: 11 },
              padding: 10,
              displayColors: true,
              usePointStyle: true,
              callbacks: {
                label: function(context) {
                  let label = context.dataset.label || '';
                  if (label) label += ': ';
                  if (context.parsed.y !== null) label += context.parsed.y + '°C';
                  return label;
                }
              }
            }
          },
          scales: {
            x: {
              grid: { display: false },
              ticks: {
                color: '#64748b',
                font: { family: 'Outfit', size: 9 },
                maxRotation: 0,
                autoSkip: true,
                maxTicksLimit: 6
              }
            },
            y: {
              grid: { color: 'rgba(255,255,255,0.03)' },
              ticks: {
                color: '#64748b',
                font: { family: 'Outfit', size: 10 },
                callback: function(value) { return value + '°C'; }
              }
            }
          }
        }
      });
    }

    function processTemperatures(temps) {
      let hasData = Object.keys(temps).length > 0;
      if (!hasData) return;

      let now = new Date();
      let timeStr = now.toTimeString().split(' ')[0];
      timestamps.push(timeStr);
      if (timestamps.length > maxPoints) timestamps.shift();

      for (let key in temps) {
        if (!tempHistory[key]) {
          tempHistory[key] = Array(maxPoints - 1).fill(null);
        }
      }

      for (let key in tempHistory) {
        if (temps[key] !== undefined) {
          tempHistory[key].push(temps[key]);
        } else {
          let prevVal = tempHistory[key][tempHistory[key].length - 1];
          tempHistory[key].push(prevVal);
        }
        if (tempHistory[key].length > maxPoints) {
          tempHistory[key].shift();
        }
      }

      const tempColors = {
        'Amp.Left': '#818cf8',
        'Amp.Right': '#f43f5e',
        'HeatSink.Left': '#34d399',
        'PSU': '#fbbf24',
        'MCU': '#38bdf8',
        'default': '#a78bfa'
      };

      let datasets = [];
      for (let key in tempHistory) {
        let color = tempColors[key] || tempColors['default'];
        datasets.push({
          label: key,
          data: tempHistory[key],
          borderColor: color,
          borderWidth: 2,
          fill: false
        });
      }

      if (chart) {
        chart.data.datasets = datasets;
        chart.data.labels = timestamps;
        chart.update('none');
      }
    }

    // Keyboard navigation
    window.addEventListener('keydown', function(e) {
      if (e.key === 'ArrowUp') {
        e.preventDefault();
        const slider = document.getElementById('speed-slider');
        let step = e.shiftKey ? 1 : 5;
        let newVal = Math.min(255, parseInt(slider.value) + step);
        slider.value = newVal;
        updateSpeed(newVal);
        sendFinalSpeed(newVal);
      } else if (e.key === 'ArrowDown') {
        e.preventDefault();
        const slider = document.getElementById('speed-slider');
        let step = e.shiftKey ? 1 : 5;
        let newVal = Math.max(0, parseInt(slider.value) - step);
        slider.value = newVal;
        updateSpeed(newVal);
        sendFinalSpeed(newVal);
      }
    });

    // Start status polling & init chart
    setInterval(checkStatus, 1000);
    window.onload = function() {
      initChart();
      checkStatus();
    };
  </script>
</head>
<body>
  <div class='app-container'>
    <div class='card'>
      <h1>Fan Control</h1>
      <div class='subtitle'>Smart Ramping System</div>
      
      <!-- 3-Way Mode Button Group -->
      <div class='mode-group'>
        <button id='btn-off' class='mode-btn off' onclick='setMode(0)'>OFF</button>
        <button id='btn-on' class='mode-btn on' onclick='setMode(1)'>ON</button>
        <button id='btn-auto' class='mode-btn auto' onclick='setMode(2)'>AUTO</button>
      </div>

      <!-- Status Indicator -->
      <div class='status-container'>
        <div id='status-ind' class='status-indicator off'>OFF</div>
      </div>
      
      <!-- Dual Stats Display -->
      <div class='stats-grid'>
        <div class='stat-box'>
          <div class='stat-label'>Target Speed</div>
          <div id='speed-val' class='stat-val'>0</div>
          <div id='speed-pct' class='stat-pct'>0%</div>
        </div>
        <div class='stat-box active-speed'>
          <div class='stat-label'>Actual Speed</div>
          <div id='actual-val' class='stat-val'>0</div>
          <div id='actual-pct' class='stat-pct'>0%</div>
        </div>
      </div>
      
      <!-- Speed Slider -->
      <div class='slider-wrapper'>
        <input type='range' id='speed-slider' min='0' max='255' value='0' 
               onmousedown='isDragging=true' onmouseup='isDragging=false' ontouchstart='isDragging=true' ontouchend='isDragging=false'
               oninput='updateSpeed(this.value)' onchange='sendFinalSpeed(this.value)'>
        <div class='labels'>
          <span>MIN</span>
          <span>50%</span>
          <span>MAX</span>
        </div>
      </div>

      <!-- Settings Card -->
      <div class='settings-section'>
        <div class='settings-title'>BluOS Integration</div>
        <div class='input-wrapper'>
          <input type='text' id='endpoint-input' class='endpoint-input' value='' placeholder='192.168.0.18:23'>
          <button class='save-btn' onclick='saveEndpoint()'>SAVE</button>
        </div>
        <div class='bluos-status'>
          <span id='bluos-dot' class='status-dot'></span>
          <span id='bluos-text'>BluOS Monitor: Inactive</span>
        </div>
        
        <!-- Persistent Temperature Monitor Toggle -->
        <div class='temp-toggle-wrapper' style='display: flex; align-items: center; justify-content: space-between; margin-top: 15px; border-top: 1px solid rgba(255,255,255,0.05); padding-top: 15px;'>
          <span style='font-size: 13px; font-weight: 600; color: var(--text);'>Temperature Monitoring</span>
          <label class='switch' style='position: relative; display: inline-block; width: 44px; height: 24px;'>
            <input type='checkbox' id='temp-mon-toggle' onchange='toggleTempMonitor(this.checked)' style='opacity: 0; width: 0; height: 0;'>
            <span class='slider' style='position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: rgba(255,255,255,0.1); transition: .3s; border-radius: 24px; border: 1px solid rgba(255,255,255,0.05);'></span>
          </label>
        </div>
      </div>

      <!-- Keyboard Hint -->
      <div class='keyboard-hint'>
        <span>Keys:</span>
        <span class='kbd-btn'>▲</span>
        <span class='kbd-btn'>▼</span>
        <span>(±5) |</span>
        <span class='kbd-btn'>Shift</span>
        <span>+</span>
        <span class='kbd-btn'>▲</span>
        <span class='kbd-btn'>▼</span>
        <span>(±1)</span>
      </div>
    </div>

    <!-- Temperature Graph Card (Grows dynamically on the right side) -->
    <div class='card' id='temp-section' style='display:none; max-width: 480px;'>
      <div class='settings-title' style='font-size: 14px; margin-bottom: 20px; font-weight: 800; background: linear-gradient(to right, #818cf8, #34d399); -webkit-background-clip: text; -webkit-text-fill-color: transparent;'>Live Temperatures</div>
      <div style='font-size: 10px; color: var(--text-muted); margin-top: -15px; margin-bottom: 20px; letter-spacing: 1px; text-transform: uppercase; font-weight: 600;'>Time Window: Last 5 minutes (1s steps)</div>
      <div id='temp-chart-container' style='position:relative; margin-top: 15px; height: 180px;'>
        <!-- Offline / Standby Overlay Message -->
        <div id='temp-offline-overlay' style='display:none; position:absolute; top:0; left:0; width:100%; height:100%; flex-direction:column; align-items:center; justify-content:center; background:rgba(15,23,42,0.6); border-radius:12px; border:1px dashed rgba(255,255,255,0.08); box-sizing:border-box; z-index:10;'>
          <span style='font-size: 14px; font-weight: 800; color: var(--text-muted); text-transform: uppercase; letter-spacing: 1.5px;'>Power is Off</span>
          <span id='temp-offline-reason' style='font-size: 10px; color: rgba(255,255,255,0.3); margin-top: 5px; text-transform: uppercase; letter-spacing: 1px;'>BluOS Device Standby</span>
        </div>
        <canvas id='tempChart' style='width: 100%; height: 180px;'></canvas>
      </div>
    </div>
  </div>
</body>
</html>
)rawhtml";

  Serial.println("[Web] GET / (Root UI Page Requested)");
  server.sendHeader("Connection", "close");
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleSetSpeed() {
  if (server.hasArg("val")) {
    sliderSpeed = server.arg("val").toInt();
    saveSettings();
    Serial.printf("[Web] GET /set?val=%d (Slider Target Speed Updated)\n", sliderSpeed);
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "OK");
  } else {
    Serial.println("[Web ERROR] GET /set (Missing val parameter)");
    server.sendHeader("Connection", "close");
    server.send(400, "text/plain", "Missing val");
  }
}

void handleTogglePower() {
  if (server.hasArg("state")) {
    int state = server.arg("state").toInt();
    uint8_t prevMode = powerMode;
    powerMode = (state >= 0 && state <= 2) ? state : 0;
    
    Serial.printf("[Web] GET /toggle?state=%d (Power mode changed from %d to %d)\n", state, prevMode, powerMode);
    
    if (powerMode == 1 && prevMode != 1) {
      triggerKickstart();
    }
    
    if (powerMode == 2 && prevMode != 2) {
      bluosPowerOn = false;
      lastConnectAttempt = 0;
      if (bluosClient.connected()) {
        bluosClient.stop();
      }
      lastBluOSConnected = false;
    }
    
    saveSettings();
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "OK");
  } else {
    Serial.println("[Web ERROR] GET /toggle (Missing state parameter)");
    server.sendHeader("Connection", "close");
    server.send(400, "text/plain", "Missing state");
  }
}

void handleSaveEndpoint() {
  if (server.hasArg("endpoint")) {
    bluosEndpoint = server.arg("endpoint");
    bluosEndpoint.trim();
    
    Serial.printf("[Web] GET /save?endpoint=%s (Target BluOS Endpoint Updated)\n", bluosEndpoint.c_str());
    
    if (bluosClient.connected()) {
      bluosClient.stop();
    }
    lastBluOSConnected = false;
    bluosPowerOn = false;
    lastConnectAttempt = 0;

    saveSettings();
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "OK");
  } else {
    Serial.println("[Web ERROR] GET /save (Missing endpoint parameter)");
    server.sendHeader("Connection", "close");
    server.send(400, "text/plain", "Missing endpoint");
  }
}

void handleToggleTemp() {
  if (server.hasArg("state")) {
    int state = server.arg("state").toInt();
    tempMonitorEnabled = (state != 0);
    
    Serial.printf("[Web] GET /toggle-temp?state=%d (Temp Monitoring: %s)\n", state, tempMonitorEnabled ? "ENABLED" : "DISABLED");
    
    if (!tempMonitorEnabled && powerMode != 2) {
      if (bluosClient.connected()) {
        bluosClient.stop();
      }
      lastBluOSConnected = false;
      bluosPowerOn = false;
    } else if (tempMonitorEnabled) {
      lastConnectAttempt = 0; // trigger immediate connect
    }
    
    saveSettings();
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "OK");
  } else {
    Serial.println("[Web ERROR] GET /toggle-temp (Missing state parameter)");
    server.sendHeader("Connection", "close");
    server.send(400, "text/plain", "Missing state");
  }
}

void handleStatus() {
  String stateStr = "OFF";
  if (powerMode == 1) {
    if (isKickstarting) {
      stateStr = "KICKSTARTING";
    } else if (currentSpeed != targetSpeed) {
      stateStr = "RAMPING";
    } else {
      stateStr = "RUNNING";
    }
  } else if (powerMode == 2) {
    if (bluosPowerOn) {
      if (isKickstarting) {
        stateStr = "KICKSTARTING";
      } else if (currentSpeed != targetSpeed) {
        stateStr = "RAMPING";
      } else {
        stateStr = "RUNNING";
      }
    } else {
      if (currentSpeed > 0) {
        stateStr = "RAMPING";
      } else {
        stateStr = "OFF";
      }
    }
  } else {
    if (currentSpeed > 0) {
      stateStr = "RAMPING";
    } else {
      stateStr = "OFF";
    }
  }
  
  String json = "{";
  json += "\"mode\":" + String(powerMode) + ",";
  json += "\"slider\":" + String(sliderSpeed) + ",";
  json += "\"actual\":" + String(currentSpeed) + ",";
  json += "\"state\":\"" + stateStr + "\",";
  json += "\"endpoint\":\"" + bluosEndpoint + "\",";
  json += "\"bluosConnected\":" + String(bluosClient.connected() ? "true" : "false") + ",";
  json += "\"bluosPower\":" + String(bluosPowerOn ? "true" : "false") + ",";
  json += "\"tempMon\":" + String(tempMonitorEnabled ? "true" : "false") + ",";
  
  // Add live temperatures directly inside status JSON
  json += "\"temps\":{";
  bool first = true;
  for (int i = 0; i < sensorCount; i++) {
    if (millis() - tempSensors[i].lastUpdate < 15000) {
      if (!first) json += ",";
      json += "\"" + tempSensors[i].name + "\":" + String(tempSensors[i].value);
      first = false;
    }
  }
  json += "}";
  json += "}";
  
  server.sendHeader("Connection", "close");
  server.send(200, "application/json", json);
}

