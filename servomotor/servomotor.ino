#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* SSID     = "Controle_Servo_ESP32";
const char* PASSWORD = "12345678";

#define SERVO_PIN 4        

Servo meuServo;

int current_angle = 90;

WebServer server(80);

const char* HTML_SERVO = R"rawhtml(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Controle de Servo</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { font-family: Arial, sans-serif; background: #0f1117; color: #e2e8f0;
           display: flex; justify-content: center; align-items: center; min-height: 100vh; }
    .card { background: #1a1d27; border: 1px solid #2e3347; border-radius: 12px; padding: 2rem; width: 360px; }
    h1 { color: #3b82f6; font-size: 1.3rem; margin-bottom: 1.5rem; text-align: center; }
    label { font-size: 0.85rem; color: #94a3b8; display: flex; justify-content: space-between; margin-bottom: 8px; }
    .angle-display { color: #f59e0b; font-weight: bold; font-size: 1.1rem; }
    input[type=range] { width: 100%; height: 6px; background: #22263a; border-radius: 3px; outline: none; -webkit-appearance: none; margin-bottom: 1.5rem; }
    input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 20px; height: 20px; border-radius: 50%; background: #f59e0b; cursor: pointer; }
    .visualizer-container { background: #22263a; border: 1px solid #2e3347; border-radius: 8px; padding: 1.5rem; text-align: center; }
    .servo-base { width: 100px; height: 40px; background: #3b82f6; margin: 30px auto 0; position: relative; border-radius: 4px; }
    .servo-horn { width: 80px; height: 12px; background: #e2e8f0; position: absolute; top: -6px; left: 10px; transform-origin: center center; transition: transform 0.1s ease-out; border-radius: 6px; }
  </style>
</head>
<body>
<div class="card">
  <h1>Controle de Servomotor</h1>
  
  <label>Ângulo do Servo: <span class="angle-display" id="angleVal">90°</span></label>
  <input type="range" id="angleSlider" min="0" max="180" value="90" oninput="moverServo(this.value)">

  <div class="visualizer-container">
    <p style="font-size: 0.8rem; color: #64748b;">Simulação Visual</p>
    <div class="servo-base">
      <div class="servo-horn" id="horn"></div>
    </div>
  </div>
</div>

<script>
let debounceTimeout = null;

function moverServo(angulo) {
  document.getElementById("angleVal").textContent = angulo + "°";
  document.getElementById("horn").style.transform = "rotate(" + (angulo - 90) + "deg)";

  clearTimeout(debounceTimeout);
  debounceTimeout = setTimeout(async () => {
    try {
      await fetch("/setServo?angle=" + angulo);
    } catch (e) {
      console.error("Erro ao enviar comando ao ESP32");
    }
  }, 40);
}
</script>
</body>
</html>
)rawhtml";

void handleSetServo() {
  String sAngle = server.arg("angle");
  if (sAngle != "") {
    int angle = sAngle.toInt();
    angle = constrain(angle, 0, 180);
    
    meuServo.write(angle);
    current_angle = angle;

    Serial.printf("[SERVO] Angulo definido: %d°\n", angle);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "text/plain", "Parâmetro ausente.");
  }
}

void handleRoot() { server.send(200, "text/html", HTML_SERVO); }

void setup() {
  Serial.begin(115200);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  meuServo.setPeriodHertz(50); 
  

  meuServo.attach(SERVO_PIN, 500, 2400); 
  
  meuServo.write(current_angle);

  WiFi.softAP(SSID, PASSWORD);
  Serial.printf("Ponto de Acesso Iniciado. IP: http://%s\n", WiFi.softAPIP().toString().c_str());

  server.on("/", handleRoot);
  server.on("/setServo", handleSetServo);
  server.begin();
}

void loop() {
  server.handleClient();
}#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* SSID     = "Controle_Servo_ESP32";
const char* PASSWORD = "12345678";

#define SERVO_PIN 4        

Servo meuServo;

int current_angle = 90;

WebServer server(80);

const char* HTML_SERVO = R"rawhtml(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Controle de Servo</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { font-family: Arial, sans-serif; background: #0f1117; color: #e2e8f0;
           display: flex; justify-content: center; align-items: center; min-height: 100vh; }
    .card { background: #1a1d27; border: 1px solid #2e3347; border-radius: 12px; padding: 2rem; width: 360px; }
    h1 { color: #3b82f6; font-size: 1.3rem; margin-bottom: 1.5rem; text-align: center; }
    label { font-size: 0.85rem; color: #94a3b8; display: flex; justify-content: space-between; margin-bottom: 8px; }
    .angle-display { color: #f59e0b; font-weight: bold; font-size: 1.1rem; }
    input[type=range] { width: 100%; height: 6px; background: #22263a; border-radius: 3px; outline: none; -webkit-appearance: none; margin-bottom: 1.5rem; }
    input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 20px; height: 20px; border-radius: 50%; background: #f59e0b; cursor: pointer; }
    .visualizer-container { background: #22263a; border: 1px solid #2e3347; border-radius: 8px; padding: 1.5rem; text-align: center; }
    .servo-base { width: 100px; height: 40px; background: #3b82f6; margin: 30px auto 0; position: relative; border-radius: 4px; }
    .servo-horn { width: 80px; height: 12px; background: #e2e8f0; position: absolute; top: -6px; left: 10px; transform-origin: center center; transition: transform 0.1s ease-out; border-radius: 6px; }
  </style>
</head>
<body>
<div class="card">
  <h1>Controle de Servomotor</h1>
  
  <label>Ângulo do Servo: <span class="angle-display" id="angleVal">90°</span></label>
  <input type="range" id="angleSlider" min="0" max="180" value="90" oninput="moverServo(this.value)">

  <div class="visualizer-container">
    <p style="font-size: 0.8rem; color: #64748b;">Simulação Visual</p>
    <div class="servo-base">
      <div class="servo-horn" id="horn"></div>
    </div>
  </div>
</div>

<script>
let debounceTimeout = null;

function moverServo(angulo) {
  document.getElementById("angleVal").textContent = angulo + "°";
  document.getElementById("horn").style.transform = "rotate(" + (angulo - 90) + "deg)";

  clearTimeout(debounceTimeout);
  debounceTimeout = setTimeout(async () => {
    try {
      await fetch("/setServo?angle=" + angulo);
    } catch (e) {
      console.error("Erro ao enviar comando ao ESP32");
    }
  }, 40);
}
</script>
</body>
</html>
)rawhtml";

void handleSetServo() {
  String sAngle = server.arg("angle");
  if (sAngle != "") {
    int angle = sAngle.toInt();
    angle = constrain(angle, 0, 180);
    
    meuServo.write(angle);
    current_angle = angle;

    Serial.printf("[SERVO] Angulo definido: %d°\n", angle);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "text/plain", "Parâmetro ausente.");
  }
}

void handleRoot() { server.send(200, "text/html", HTML_SERVO); }

void setup() {
  Serial.begin(115200);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  meuServo.setPeriodHertz(50); 
  

  meuServo.attach(SERVO_PIN, 500, 2400); 
  
  meuServo.write(current_angle);

  WiFi.softAP(SSID, PASSWORD);
  Serial.printf("Ponto de Acesso Iniciado. IP: http://%s\n", WiFi.softAPIP().toString().c_str());

  server.on("/", handleRoot);
  server.on("/setServo", handleSetServo);
  server.begin();
}

void loop() {
  server.handleClient();
}