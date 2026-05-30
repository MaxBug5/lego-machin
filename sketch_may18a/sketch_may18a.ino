#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>

// ==========================================
// ДАНІ ТВОГО ДОМАШНЬОГО ІНТЕРНЕТУ
const char* ssid = "Home WiFi";
const char* password = "Max_2243";
// ==========================================

const int CS_PIN = 5; // Наш GPIO 5 (CS) для зв'язку з Nano
WebServer server(80);

// HTML-сторінка з кнопками WASD + X (STOP) + Q/E
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
  <title>LEGO WASD Remote</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background: #222; color: white; padding-top: 20px; user-select: none; }
    .grid-container { display: grid; grid-template-columns: repeat(3, 100px); grid-gap: 10px; justify-content: center; margin-bottom: 15px; }
    .btn { padding: 20px; font-size: 20px; font-weight: bold; background: #4CAF50; color: white; border: none; border-radius: 8px; cursor: pointer; }
    .btn:active { background: #3e8e41; }
    .btn-stop { background: #f44336; grid-column: span 3; width: 320px; margin: 0 auto; padding: 15px; font-size: 24px; }
    .btn-stop:active { background: #da190b; }
    .btn-speed { background: #ff9800; }
    .btn-back { background: #2196F3; }
  </style>
</head>
<body>
  <h2>LEGO Пульт: WASD + STOP</h2>
  
  <div class="grid-container">
    <button class="btn btn-speed" onclick="send('q')">Q-</button>
    <button class="btn" onclick="send('w')">W</button>
    <button class="btn btn-speed" onclick="send('e')">E+</button>
    
    <button class="btn" onclick="send('a')">A</button>
    <button class="btn btn-back" onclick="send('s')">S (Назад)</button>
    <button class="btn" onclick="send('d')">D</button>
  </div>

  <div style="text-align: center; margin-top: 10px;">
    <button class="btn btn-stop" onclick="send('x')">🛑 СТОП</button>
  </div>

  <p>W - Вперед | S - Назад | A - Лівий | D - Правий</p>
  <p>Q/E - Швидкість | Кнопка СТОП - Зупинка</p>

  <script>
    function send(key) {
      var xhttp = new XMLHttpRequest();
      xhttp.open("GET", "/action?key=" + key, true);
      xhttp.send();
    }
  </script>
</body>
</html>
)rawliteral";

// Функція відправки одного символу по SPI
void sendSPI(char data) {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(CS_PIN, LOW);   
  SPI.transfer(data);          
  digitalWrite(CS_PIN, HIGH);  
  SPI.endTransaction();
}

void setup() {
  Serial.begin(115200);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  // На ESP32 Dev Module апаратний VSPI запуститься автоматично на:
  // GPIO 23 (MOSI) та GPIO 18 (CLK)
  SPI.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Адреса робота: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/action", HTTP_GET, []() {
    if (server.hasArg("key")) {
      String key = server.arg("key");
      char charKey = key.charAt(0);
      
      Serial.print("Натиснуто клавішу: ");
      Serial.println(charKey);
      
      sendSPI(charKey); 
    }
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  server.handleClient();
}