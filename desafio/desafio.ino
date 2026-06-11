#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* SSID     = "Controle_Servo_ESP32";
const char* PASSWORD = "12345678";

#define SERVO_PIN 4
#define LED_PIN   8 

Servo meuServo;

int current_angle    = 90;
int current_led_freq = 5000;
int current_led_duty = 128;

WebServer server(80);

const char* HTML_PAINEL = R"rawhtml(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Painel de Controle PWM</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { font-family: Arial, sans-serif; background: #0f1117; color: #e2e8f0;
           display: flex; justify-content: center; align-items: center; min-height: 100vh; padding: 1rem; }
    .container { width: 100%; max-width: 500px; }
    .card { background: #1a1d27; border: 1px solid #2e3347; border-radius: 12px; padding: 2rem; margin-bottom: 1.5rem; box-shadow: 0 4px 20px rgba(0,0,0,0.3); }
    h1 { color: #3b82f6; font-size: 1.4rem; margin-bottom: 1.5rem; text-align: center; }
    h2 { font-size: 1.1rem; margin-bottom: 1rem; color: #94a3b8; border-bottom: 1px solid #2e3347; padding-bottom: 0.3rem; }
    .control-group { margin-bottom: 1.25rem; }
    label { font-size: 0.85rem; color: #94a3b8; display: flex; justify-content: space-between; margin-bottom: 6px; }
    .val-display { font-weight: bold; font-family: monospace; }
    .led-txt { color: #10b981; }
    .servo-txt { color: #f59e0b; }
    input[type=range] { width: 100%; height: 6px; background: #22263a; border-radius: 3px; outline: none; -webkit-appearance: none; }
    input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 18px; height: 18px; border-radius: 50%; cursor: pointer; }
    #ledDuty::-webkit-slider-thumb, #ledFreq::-webkit-slider-thumb { background: #10b981; }
    #servoAngle::-webkit-slider-thumb { background: #f59e0b; }
    .preview-box { display: flex; justify-content: space-around; align-items: center; background: #22263a; border: 1px solid #2e3347; border-radius: 8px; padding: 1rem; margin-top: 1rem; }
    .preview-item { text-align: center; font-size: 0.75rem; color: #64748b; }
    .led-preview { width: 25px; height: 25px; border-radius: 50%; background: #10b981; margin: 8px auto 0; box-shadow: 0 0 15px #10b981; transition: opacity 0.05s; }
    .servo-base { width: 70px; height: 30px; background: #3b82f6; margin: 15px auto 0; position: relative; border-radius: 3px; }
    .servo-horn { width: 54px; height: 8px; background: #e2e8f0; position: absolute; top: -4px; left: 8px; transform-origin: center center; transition: transform 0.1s ease-out; border-radius: 4px; }
    #statusText { text-align: center; font-size: 0.8rem; color: #64748b; margin-top: 0.5rem; }
  </style>
</head>
<body>
<div class="container">
  <div class="card">
    <h1>Painel Integrado de Atuadores</h1>

    <h2>Controle do LED Externo</h2>
    <div class="control-group">
      <label>Frequência: <span class="val-display led-txt" id="ledFreqVal">5000 Hz</span></label>
      <input type="range" id="ledFreq" min="10" max="20000" value="5000" oninput="enviarLed()">
    </div>
    <div class="control-group">
      <label>Intensidade: <span class="val-display led-txt" id="ledDutyVal">50%</span></label>
      <input type="range" id="ledDuty" min="0" max="255" value="128" oninput="enviarLed()">
    </div>

    <h2>Controle do Servomotor</h2>
    <div class="control-group">
      <label>Ângulo do Braço: <span class="val-display servo-txt" id="servoAngleVal">90</span></label>
      <input type="range" id="servoAngle" min="0" max="180" value="90" oninput="enviarServo()">
    </div>

    <div class="preview-box">
      <div class="preview-item">
        <p>LED Físico</p>
        <div class="led-preview" id="ledVisual"></div>
      </div>
      <div class="preview-item">
        <p>Servo Físico</p>
        <div class="servo-base"><div class="servo-horn" id="servoVisual"></div></div>
      </div>
    </div>
    <div id="statusText">Sistema online. Aguardando comandos...</div>
  </div>
</div>

<script>
let ledTimeout   = null;
let servoTimeout = null;

function enviarLed() {
  var freq = document.getElementById("ledFreq").value;
  var duty = document.getElementById("ledDuty").value;
  document.getElementById("ledFreqVal").textContent  = freq + " Hz";
  document.getElementById("ledDutyVal").textContent  = Math.round((duty / 255) * 100) + "%";
  document.getElementById("ledVisual").style.opacity = duty / 255;
  clearTimeout(ledTimeout);
  ledTimeout = setTimeout(async () => {
    try {
      let r = await fetch(`/updateLed?freq=${freq}&duty=${duty}`);
      let d = await r.json();
      document.getElementById("statusText").textContent = `LED atualizado! Freq real: ${d.freq_real} Hz`;
    } catch(e) {
      document.getElementById("statusText").textContent = "Erro de conexao com o LED";
    }
  }, 50);
}

function enviarServo() {
  var angulo = document.getElementById("servoAngle").value;
  document.getElementById("servoAngleVal").textContent = angulo;
  document.getElementById("servoVisual").style.transform = `rotate(${angulo - 90}deg)`;
  clearTimeout(servoTimeout);
  servoTimeout = setTimeout(async () => {
    try {
      await fetch(`/updateServo?angle=${angulo}`);
      document.getElementById("statusText").textContent = `Servo posicionado em ${angulo}`;
    } catch(e) {
      document.getElementById("statusText").textContent = "Erro de conexao com o Servo";
    }
  }, 40);
}

enviarLed();
enviarServo();
</script>
</body>
</html>
)rawhtml";

void handleRoot() {
  server.send(200, "text/html", HTML_PAINEL);
}

void handleUpdateLed() {
  String sFreq = server.arg("freq");
  String sDuty = server.arg("duty");
  if (sFreq != "" && sDuty != "") {
    int reqFreq = constrain(sFreq.toInt(), 10, 20000);
    int reqDuty = constrain(sDuty.toInt(), 0, 255);

    ledcChangeFrequency(LED_PIN, reqFreq, 8);
    ledcWrite(LED_PIN, reqDuty);

    current_led_freq = reqFreq;
    current_led_duty = reqDuty;
    Serial.printf("[LED] Freq: %d Hz | Duty: %d\n", reqFreq, reqDuty);
    server.send(200, "application/json",
      "{\"status\":\"ok\",\"freq_real\":" + String(reqFreq) + "}");
  } else {
    server.send(400, "text/plain", "Faltam parametros do LED.");
  }
}

void handleUpdateServo() {
  String sAngle = server.arg("angle");
  if (sAngle != "") {
    int angle = constrain(sAngle.toInt(), 0, 180);
    meuServo.write(angle);
    current_angle = angle;
    Serial.printf("[SERVO] Angulo: %d graus\n", angle);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "text/plain", "Falta o parametro angulo.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  meuServo.setPeriodHertz(50);
  meuServo.attach(SERVO_PIN, 500, 2400);

  delay(500); meuServo.write(0);
  delay(800); meuServo.write(90);
  delay(800); meuServo.write(180);
  delay(800); meuServo.write(current_angle);

  ledcAttach(LED_PIN, current_led_freq, 8);
  ledcWrite(LED_PIN, current_led_duty);

  WiFi.softAP(SSID, PASSWORD);
  delay(500);
  Serial.printf("IP: http://%s\n", WiFi.softAPIP().toString().c_str());

  server.on("/",            handleRoot);
  server.on("/updateLed",   handleUpdateLed);
  server.on("/updateServo", handleUpdateServo);
  server.begin();
}

void loop() {
  server.handleClient();
}