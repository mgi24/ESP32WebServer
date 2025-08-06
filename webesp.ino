#include <WiFi.h>
#include <WireGuard-ESP32.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "filemanager.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// WireGuard configuration --- UPDATE this configuration from JSON
IPAddress local_ip( 10,8,0,4);                                      // IP address of the local interface
const char private_key[] = "oI86S6pXz1iAwmhTsvEyjAjirVO34HhToBNze5f1BXo=";   // Private key of the local interface
const char endpoint_address[] = "217.60.38.204";                           // Address of the endpoint peer
const char public_key[] = "a0MkXzoOQodC9F1u6MHLPpPPH7CAzm4gE3B+YUhfR2E=";    // Public key of the endpoint peer
const char preshared_key[] = "iOXBjgWJKDTpdTkRmC08yUcAALbh8cBVTwRaSyfbA1g="; // Pre-Shared Key
uint16_t endpoint_port = 51820;                                              // Port of the endpoint peer

static WireGuard wg;

const char *ssid = "TP-Link";
const char *password = "";
IPAddress local_IP(192.168.0.107);
IPAddress gateway(192.168.0.100);  // <-- Peer local IP!
IPAddress subnet(255, 255, 255, 0);


AsyncWebServer server(80);
void serverSetup()
{
  // Route untuk halaman utama - langsung dari SPIFFS
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  // Serve static files dari SPIFFS
  server.serveStatic("/", SPIFFS, "/");

  // Handle file not found
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/plain", "File not found"); });

  // Endpoint untuk menampilkan daftar file di SPIFFS (API)
  server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  // Get SPIFFS info
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  size_t freeBytes = totalBytes - usedBytes;
  
  String json = "{";
  json += "\"storage\":{";
  json += "\"total\":" + String(totalBytes) + ",";
  json += "\"used\":" + String(usedBytes) + ",";
  json += "\"free\":" + String(freeBytes) + ",";
  json += "\"usedPercent\":" + String((usedBytes * 100) / totalBytes);
  json += "},";
  json += "\"files\":[";
  
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  bool first = true;
  while(file){
    if (!first) json += ",";
    json += "{\"name\":\"" + String(file.name()) + "\",\"size\":" + String(file.size()) + "}";
    file = root.openNextFile();
    first = false;
  }
  json += "]}";
  request->send(200, "application/json", json); });

  // Endpoint untuk menghapus file di SPIFFS (API)
  server.on("/delete", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (!request->hasParam("name", true)) {
      request->send(400, "text/plain", "Missing file name");
      return;
    }
    String filename = request->getParam("name", true)->value();
    Serial.println("Deleting file: " + filename);
    if (SPIFFS.exists(filename)) {
      SPIFFS.remove(filename);
      request->send(200, "text/plain", "File deleted");
    } else {
      request->send(404, "text/plain", "File not found");
    } });

  // Route khusus untuk video streaming dengan header yang benar
  server.on("/demo.mp4", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/demo.mp4", "video/mp4");
    response->addHeader("Accept-Ranges", "bytes");
    response->addHeader("Content-Type", "video/mp4");
    request->send(response); });

  // Route khusus untuk upload file
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "Upload complete"); }, [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
            {
    static File uploadFile;
    if(index == 0){
      String path = "/" + filename;
      uploadFile = SPIFFS.open(path, FILE_WRITE);
      if(!uploadFile){
        Serial.println("Failed to open file for writing");
        return;
      }
    }
    if(uploadFile){
      uploadFile.write(data, len);
    }
    if(final){
      if(uploadFile){
        uploadFile.close();
        Serial.println("File upload completed: " + filename);
      }
    } });

  // Serve halaman UI file manager di /filemanager
  server.on("/filemanager", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", filemanager_html); });
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Adjusting system time...");
  configTime(9 * 60 * 60, 0, "ntp.jst.mfeed.ad.jp", "ntp.nict.jp", "time.google.com");
  serverSetup();

  server.begin();
  Serial.println("Async Web server started");
  Serial.println("Initializing WG interface...");
  if (wg.begin(local_ip, private_key, endpoint_address, public_key, endpoint_port, preshared_key)) {
    Serial.println("OK");
  } else {
    Serial.println("FAIL");
  }
  Serial.println(local_ip.toString());
}

void loop()
{

  // AsyncWebServer berjalan di background, tidak perlu handleClient()
}