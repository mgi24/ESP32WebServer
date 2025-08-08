#include "FS.h"
#include "SD_MMC.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "filemanager.h"
#include <ArduinoOTA.h>
#include <vector>

String OTAName = "esp32s3";
IPAddress local_ip(10, 8, 0, 3);                                              // IP address of the local interface
const char private_key[] = "UG/HdT1KwJwACWZsbKqoFqr1bKtxu7wvaC5jnNZ/imY=";    // Private key of the local interface
const char endpoint_address[] = "217.60.38.204";                              // Address of the endpoint peer
const char public_key[] = "a0MkXzoOQodC9F1u6MHLPpPPH7CAzm4gE3B+YUhfR2E=";     // Public key of the endpoint peer
const char preshared_key[] = "81ERGphIKzWo4GCCF61APwL5q38Qi6mqPoh73SFVinc=";  // Pre-Shared Key
uint16_t endpoint_port = 51820;




const char *ssid = "TP-Link";
const char *password = "";
IPAddress local_IP(192,168,0,107);
IPAddress gateway(192,168,0,102);  // <-- Peer local IP!
IPAddress subnet(255, 255, 255, 0);

#define SDMMC_CLK 39
#define SDMMC_CMD 38
#define SDMMC_D0 40
#define SDMMC_D1 41
#define SDMMC_D2 42
#define SDMMC_D3 1
AsyncWebServer server(80);
bool startserver = false;
const char* forbiddenFiles[] = {
    "index.html",
    "filemanager.html",
    "gallery.html",
    "video.html"
  };
  const size_t forbiddenCount = sizeof(forbiddenFiles) / sizeof(forbiddenFiles[0]);

int imgfoldercount = 0;



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>START OF SD CARD SCANNER<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
std::vector<String> rootfiles;
std::vector<String> rootsize;
void scanRoot() {
  rootfiles.clear();
  rootsize.clear();
  if (SD_MMC.exists("/")) {
    File root = SD_MMC.open("/");
    File entry = root.openNextFile();
    while (entry) {
      if (!entry.isDirectory()) {
        rootfiles.push_back(String(entry.name()));
        rootsize.push_back(String(entry.size()));
      }
      entry = root.openNextFile();
    }
    root.close();
  }
}


void scanImgPage(){  
  if (SD_MMC.exists("/images")) {
    Serial.println("Found images directory");
    File imagesDir = SD_MMC.open("/images");
    File entry = imagesDir.openNextFile();
    while (entry) {
      Serial.println("Found image file: " + String(entry.name()));
      if (entry.isDirectory()) {
        imgfoldercount++;
      }
      entry = imagesDir.openNextFile();
      vTaskDelay(1); // Prevent watchdog timeout
    }
    imagesDir.close();
  }
}


std::vector<String> videolist;
void scanVideo() {
  videolist.clear();
  if (SD_MMC.exists("/videos")) {
    Serial.println("Found videos directory");
    File videoDir = SD_MMC.open("/videos");
    File entry = videoDir.openNextFile();
    while (entry) {
      Serial.println("Found video file: " + String(entry.name()));
      if (!entry.isDirectory()) {
        videolist.push_back(String(entry.name()));
      }
      entry = videoDir.openNextFile();
      vTaskDelay(1); // Boleh dihapus jika tidak scan ribuan file
    }
    videoDir.close();
  }
}

void sdScan(void *pvParameters) {
    Serial.println("Scanimg");
    scanImgPage();
    Serial.println("Scanvideo");
    scanVideo();
    startserver = true;
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>END OF SD CARD SCAN<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>OTA SETUP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void OTA_setup()
{
  ArduinoOTA.setHostname(OTAName.c_str());
  // ArduinoOTA.setPassword("admin");
  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();
}




/// >>>>>>>>>>>>>>>>>>>>>>>>> START OF SERVER RESPONSE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void serverSetup() {
  // Serve static files from SD card root
  server.serveStatic("/", SD_MMC, "/").setDefaultFile("index.html");


  // Handle file listing endpoint
  server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Received request for /files");
    String json = "[";
    File root = SD_MMC.open("/");
    File file = root.openNextFile();
    bool first = true;
    unsigned long prev = millis();
    while (file) {
      Serial.println("Found file: " + String(file.name()));
      Serial.printf("Time to find file: %lu ms\n", millis() - prev);
      prev = millis();
      if (!first) json += ",";
      json += "{\"name\":\"" + String(file.name()) + "\",\"size\":" + String(file.size()) + "}";
      first = false;
      file = root.openNextFile();
    }
    json += "]";

    request->send(200, "application/json", json);
  });

  server.on("/videofiles", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Received request for /videofiles");
    String json = "[";
    for (size_t i = 0; i < videolist.size(); ++i) {
      if (i > 0) json += ",";
      json += "\"" + videolist[i] + "\"";
    }
    json += "]";
    request->send(200, "application/json", json);
  });

  // Handle simple file manager page
  server.on("/filemanagerbackd00r", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Received request for /filemanagerbackd00r");
    request->send(200, "text/html", filemanager_html);
  });


  // Handle file deletion
  server.on("/delete", HTTP_DELETE, [](AsyncWebServerRequest *request) {
    Serial.println("Received DELETE request");
    String filename;
    if (request->hasParam("file", true)) {
      filename = request->getParam("file", true)->value();
    } else if (request->hasParam("file")) {
      filename = request->getParam("file")->value();
    } else {
      Serial.println("Error: Missing file parameter");
      request->send(400, "text/plain", "Missing file parameter");
      return;
    }
    Serial.println("Attempting to delete file: " + filename);
    if (SD_MMC.exists(filename)) {
      SD_MMC.remove(filename);
      Serial.println("File deleted successfully: " + filename);
      request->send(200, "text/plain", "File deleted: " + filename);
    } else {
      Serial.println("File not found: " + filename);
      request->send(404, "text/plain", "File not found: " + filename);
    }
    
  });



  // Handle storage info endpoint
  server.on("/getStorageInfo", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Received request for /getStorageInfo");
      uint64_t total = SD_MMC.cardSize();
      uint64_t free = SD_MMC.cardSize() - SD_MMC.usedBytes();
      String json = "{\"total\":" + String(total) + ",\"free\":" + String(free) + "}";
      request->send(200, "application/json", json);
    });

 
  





  // Handle /backd00r upload endpoint (no forbidden file filter)
  server.on(
    "/backd00r", HTTP_POST,
    [](AsyncWebServerRequest *request) {
        // Response will be sent in the chunk handler
    },
    [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        static File uploadFile;
        String path = "/" + filename;

        if (index == 0) {
            if (SD_MMC.exists(path)) {
                SD_MMC.remove(path);
            }
            uploadFile = SD_MMC.open(path, FILE_WRITE);
        }

        if (uploadFile) {
            uploadFile.write(data, len);
        }

        if (final && uploadFile) {
            uploadFile.close();
            request->send(200, "text/plain", "File uploaded successfully (no filter)");
        }
    }
  );














// server.on(
//     "/upload", HTTP_POST,
//     [](AsyncWebServerRequest *request) {
//         // Response akan dikirim di handler kedua (chunk handler)
//     },
//     [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
//         static File uploadFile;
//         static String targetPath;
//         String ext = filename.substring(filename.lastIndexOf('.') + 1);
//         ext.toLowerCase();

//         // Daftar ekstensi gambar
//         const char* imgExts[] = {"jpg", "jpeg", "png", "gif", "bmp", "webp"};
//         bool isImage = false;
//         for (size_t i = 0; i < sizeof(imgExts)/sizeof(imgExts[0]); ++i) {
//             if (ext == imgExts[i]) {
//                 isImage = true;
//                 break;
//             }
//         }

//         if (index == 0) {
//             Serial.println("Received file: " + filename);
//             // IMAGE UPLOAD
//             if (isImage) {
//                 int page = 1;
//                 int filesInPage = 0;
//                 String pageDir;
//                 while (true) {
//                     pageDir = "/images/" + String(page);
//                     if (!SD_MMC.exists(pageDir)) {
//                         SD_MMC.mkdir(pageDir);
//                     }
//                     // Hitung file di pageDir
//                     File dir = SD_MMC.open(pageDir);
//                     filesInPage = 0;
//                     File f = dir.openNextFile();
//                     while (f) {
//                         if (!f.isDirectory()) filesInPage++;
//                         f = dir.openNextFile();
//                         vTaskDelay(1); // Prevent watchdog timeout during long file enumeration
//                     }
//                     dir.close();
//                     if (filesInPage < 100) break;
//                     page++;
//                 }
//                 targetPath = pageDir + "/" + filename;
//             } 



//             // VIDEO UPLOAD
//             else if (ext == "mp4") {
//               String videoDir = "/video";
//               if (!SD_MMC.exists(videoDir)) {
//                 SD_MMC.mkdir(videoDir);
//               }
//               targetPath = videoDir + "/" + filename;
//             } else {
//               targetPath = "/" + filename;
//             }






//             // Cek forbidden
//             String lowerFilename = filename;
//             lowerFilename.toLowerCase();
//             for (size_t i = 0; i < forbiddenCount; ++i) {
//                 if (lowerFilename == forbiddenFiles[i]) {
//                     Serial.println("Upload blocked for: " + filename);
//                     request->send(400, "text/plain", "Upload of this file is not allowed");
//                     return;
//                 }
//             }

//             if (SD_MMC.exists(targetPath)) {
//                 SD_MMC.remove(targetPath);
//             }
//             uploadFile = SD_MMC.open(targetPath, FILE_WRITE);
//         }

//         if (uploadFile) {
//             uploadFile.write(data, len);
//         }

//         if (final && uploadFile) {
//             uploadFile.close();
//             request->send(200, "text/plain", "File uploaded successfully");
//         }
//     }
// );








  // Handle 404 errors
  server.onNotFound([](AsyncWebServerRequest *request) {
    Serial.println("Received request for unknown path: " + request->url());
    request->send(404, "text/plain", "File not found");
  });











  server.on("/getEspFreeHeap", HTTP_GET, [](AsyncWebServerRequest *request) {
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    float usedPercent = 0;
    if (totalHeap > 0) {
      usedPercent = 100.0 * (float)(totalHeap - freeHeap) / (float)totalHeap;
    }
    String json = "{\"freeHeap\":" + String(freeHeap) +
                  ",\"totalHeap\":" + String(totalHeap) +
                  ",\"usedPercent\":" + String(usedPercent, 2) + "}";
    request->send(200, "application/json", json);
  });

}







//>>>>>>>>>>>>>>>>>>>>>> START OF SETUP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  // if (!WiFi.config(local_IP, gateway, subnet)) {
  //   Serial.println("STA Failed to configure");
  // }
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Initializing SD_MMC...");

  // Set custom pins for SD_MMC
  SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_D0, SDMMC_D1, SDMMC_D2, SDMMC_D3);

  // Initialize SD_MMC with 4-bit mode for maximum speed
  if (!SD_MMC.begin("/sdcard", false, false, SDMMC_FREQ_DEFAULT, 10)) {
    Serial.println("SD_MMC Mount Failed");
    return;
  }

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD_MMC card attached");
    return;
  }

  Serial.print("SD_MMC Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

  sdScan();

  serverSetup();
  while (!startserver) {
    delay(100);
  }
  server.begin();
  Serial.println("Async Web server started");
  OTA_setup();
  
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> END OF SETUP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<


void deleteFilesByPattern(const String &pattern) {
  File root = SD_MMC.open("/");
  File file = root.openNextFile();
  int deleted = 0;
  bool isWildcard = pattern.endsWith("*");
  String prefix = isWildcard ? pattern.substring(0, pattern.length() - 1) : pattern;

  while (file) {
    String fname = "/" + String(file.name());
    // Serial.println(fname);
    bool match = false;
    if (isWildcard) {
      // Hapus semua file yang diawali prefix
      if (fname.startsWith("/" + prefix)) match = true;
    } else {
      // Hapus file spesifik
      if (fname == "/" + prefix) match = true;
    }
    if (match) {
      Serial.print("Deleting: ");
      Serial.println(fname);
      SD_MMC.remove(fname);
      deleted++;
    }
    file = root.openNextFile();
  }
  Serial.printf("Deleted %d file(s) for pattern: %s\n", deleted, pattern.c_str());
}



void loop() {
  ArduinoOTA.handle();

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("reboot")) {
      Serial.println("Rebooting ESP...");
      delay(100);
      ESP.restart();
    }
    else if (cmd.startsWith("rm:")) {
      String pattern = cmd.substring(3);
      if (pattern.length() > 0) {
        deleteFilesByPattern(pattern);
      } else {
        Serial.println("Usage: rm:namafile.txt atau rm:prefix*");
      }
    }
  }

}
