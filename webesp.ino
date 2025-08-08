#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Ganti dengan SSID dan password WiFi Anda
const char* ssid = "TP-Link";
const char* password = "";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset='UTF-8'>
    <title>Maintenance</title>
  </head>
  <body style='font-family:sans-serif;text-align:center;margin-top:10%;'>
    <h1>WEBSITE IS UNDER MAINTENANCE</h1>
    <p>ONLINE LAGI BESOK TANGGAL <b>9 Agustus 2025</b>!</p>
    <p>Semua file yang telah diupload akan disimpan, kecuali file ddos!</p>
    <p>BTW Halaman ini masih dihandle sama esp32 wleee :P </p>
    <div id="memory"></div>
    <script>
    async function fetchMemory() {
      const res = await fetch('/memory');
      const data = await res.json();
      document.getElementById('memory').innerHTML =
      `<p><b>Memory Info:</b><br>
      Free Heap: ${data.free_heap} bytes<br>
      Used Heap: ${data.used_heap} bytes<br>
      Total Heap: ${data.total_heap} bytes<br>
      Usage Percent: ${data.usage_percent}%</p>`;
    }
    fetchMemory();
    setInterval(fetchMemory, 1000);
    </script>
  </body>
  </html>
  )rawliteral";
    request->send(200, "text/html", html);
  });
    
  server.on("/memory", HTTP_GET, [](AsyncWebServerRequest *request){
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t usedHeap = totalHeap - freeHeap;
    float usagePercent = totalHeap > 0 ? (usedHeap * 100.0f / totalHeap) : 0.0f;
    String json = "{";
    json += "\"free_heap\":" + String(freeHeap) + ",";
    json += "\"total_heap\":" + String(totalHeap) + ",";
    json += "\"used_heap\":" + String(usedHeap) + ",";
    json += "\"usage_percent\":" + String(usagePercent, 2);
    json += "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  // Tidak perlu apa-apa di loop
}