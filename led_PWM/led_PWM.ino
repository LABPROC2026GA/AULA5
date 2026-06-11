#include <WiFi.h>
#include <WebServer.h>

const char* SSID     = "Controle_LED_PWM";
const char* PASSWORD = "12345678";

#define LED_PIN 2
#define PWM_RESOLUTION 8   

int current_frequency = 5000; 
int current_duty = 128;        

WebServer server(80);

const char* HTML_PWM = R"rawhtml(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Controle de LED PWM</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { font-family: Arial, sans-serif; background: #0f1117; color: #e2e8f0;
           display: flex; justify-content: center; align-items: center;
           min-height: 100vh; }
    .card { background: #1a1d27; border: 1px solid #2e3347; border-radius: 12px;
            padding: 2rem; width: 380px; box-shadow: 0 4px 20px rgba(0,0,0,0.3); }
    h1 { color: #3b82f6; font-size: 1.3rem; margin-bottom: 1.5rem; text-align: center; }
    .control-group { margin-bottom: 1.5rem; }
    label { font-size: 0.85rem; color: #94a3b8; display: flex; justify-content: space-between; margin-bottom: 6px; }
    .val-display { color: #10b981; font-weight: bold; font-family: monospace; }
    input[type=range] { width: 100%; height: 6px; background: #22263a; border-radius: 3px; outline: none; -webkit-appearance: none; }
    input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 18px; height: 18px; border-radius: 50%; background: #3b82f6; cursor: pointer; transition: 0.1s; }
    input[type=range]::-webkit-slider-thumb:hover { transform: scale(1.2); }
    #status-box { background: #22263a; border: 1px solid #2e3347; border-radius: 8px;
                  padding: 1rem; text-align: center; font-size: 0.85rem; color: #94a3b8; }
    .effect-preview { width: 30px; height: 30px; border-radius: 50%; background: #fff; margin: 10px auto 0;
                       box-shadow: 0 0 20px #fff; transition: opacity 0.1s; }
  </style>
</head>
<body>
<div class="card">
  <h1>Controle de LED PWM</h1>

  <div class="control-group">
    <label>Frequência: <span class="val-display" id="freqVal">5000 Hz</span></label>
    <input type="range" id="freqSlider" min="1" max="20000" value="5000" oninput="atualizarParametros()">
  </div>

  <div class="control-group">
    <label>Intensidade (Duty Cycle): <span class="val-display" id="dutyVal">50%</span></label>
    <input type="range" id="dutySlider" min="0" max="255" value="128" oninput="atualizarParametros()">
  </div>

  <div id="status-box">
    <div id="statusText">Configuração aplicada com sucesso.</div>
    <div class="effect-preview" id="ledPreview"></div>
  </div>
</div>

<script>
let timeout = null;

function atualizarParametros() {
  var freq = document.getElementById("freqSlider").value;
  var duty = document.getElementById("dutySlider").value;
  
  document.getElementById("freqVal").textContent = freq + " Hz";
  var percent = Math.round((duty / 255) * 100);
  document.getElementById("dutyVal").textContent = percent + "%";
  
  var preview = document.getElementById("ledPreview");
  preview.style.opacity = duty / 255;
  
  if (freq < 20) {
    preview.style.animation = "pulse " + (1/freq) + "s infinite alternate";
  } else {
    preview.style.animation = "none";
  }

  clearTimeout(timeout);
  timeout = setTimeout(async () => {
    try {
      var resp = await fetch("/update?freq=" + freq + "&duty=" + duty);
      var data = await resp.json();
      document.getElementById("statusText").innerHTML = "Ajustado via Hardware!<br>Frequência: " + data.freq_real + " Hz";
    } catch (e) {
      document.getElementById("statusText").textContent = "Erro ao comunicar com o ESP32";
    }
  }, 50);
}

atualizarParametros();
</script>

<style>
@keyframes pulse {
  from { opacity: 0; }
  to { opacity: 1; }
}
</style>
</body>
</html>
)rawhtml";

void handleUpdate() {
  String sFreq = server.arg("freq");
  String sDuty = server.arg("duty");

  if (sFreq != "" && sDuty != "") {
    int reqFreq = sFreq.toInt();
    int reqDuty = sDuty.toInt();

    ledcAttach(LED_PIN, reqFreq, PWM_RESOLUTION);
    ledcWrite(LED_PIN, reqDuty);

    current_frequency = reqFreq;
    current_duty = reqDuty;

    Serial.printf("[PWM] Freq requisitada: %d Hz | Duty: %d/255 (%d%%)\n", 
                  reqFreq, reqDuty, (reqDuty * 100) / 255);

    String json = "{";
    json += "\"status\":\"sucesso\",";
    json += "\"freq_real\":" + String(reqFreq) + ",";
    json += "\"duty_percent\":" + String((reqDuty * 100) / 255);
    json += "}";
    
    server.send(200, "application/json", json);
  } else {
    server.send(400, "application/json", "{\"status\":\"erro_parametros\"}");
  }
}

void handleRoot()     { server.send(200, "text/html", HTML_PWM); }
void handleNotFound() { server.send(404, "text/plain", "404 Not Found"); }

void setup() {
  Serial.begin(115200);

  ledcAttach(LED_PIN, current_frequency, PWM_RESOLUTION);
  ledcWrite(LED_PIN, current_duty); 

  WiFi.softAP(SSID, PASSWORD);
  Serial.printf("WiFi criado! SSID: %s | IP: %s\n", SSID, WiFi.softAPIP().toString().c_str());

  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("Servidor pronto. Acesse através do navegador: http://192.168.4.1");
}

void loop() {
  server.handleClient();
}