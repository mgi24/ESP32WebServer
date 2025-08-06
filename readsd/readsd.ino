#include "FS.h"
#include "SD_MMC.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "filemanager.h"
#include <ArduinoOTA.h>
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

const char* forbiddenFiles[] = {
    "index.html",
    "filemanager.html",
    "gallery.html",
    "video.html"
  };
  const size_t forbiddenCount = sizeof(forbiddenFiles) / sizeof(forbiddenFiles[0]);

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

void serverSetup() {
  // Serve static files from SD card root
  server.serveStatic("/", SD_MMC, "/").setDefaultFile("index.html");

  // Handle file listing endpoint
  server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "[";
    File root = SD_MMC.open("/");
    File file = root.openNextFile();
    bool first = true;

    while (file) {
      if (!first) json += ",";
      json += "{\"name\":\"" + String(file.name()) + "\",\"size\":" + String(file.size()) + "}";
      first = false;
      file = root.openNextFile();
    }
    json += "]";

    request->send(200, "application/json", json);
  });

  // Handle simple file manager page
  server.on("/filemanager", HTTP_GET, [](AsyncWebServerRequest *request) {
    String filemanager_html = R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
        <title>File Manager</title>
        <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #f5f5f5;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 30px;
        }
        .info-section {
            margin-bottom: 30px;
            padding: 15px;
            background: #f8f9fa;
            border-radius: 5px;
        }
        .space-bar {
            background: #e0e0e0;
            border-radius: 10px;
            height: 30px;
            margin: 10px 0;
            overflow: hidden;
            position: relative;
        }
        .space-used {
            height: 100%;
            background: linear-gradient(90deg, #4CAF50, #45a049);
            border-radius: 10px;
            transition: width 0.3s ease;
        }
        .space-text {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            font-weight: bold;
            color: #333;
            z-index: 1;
        }
        .upload-form {
            display: flex;
            align-items: center;
            gap: 10px;
            margin-bottom: 25px;
            background: #f0f0f0;
            padding: 12px 16px;
            border-radius: 6px;
            box-shadow: 0 1px 3px rgba(0,0,0,0.04);
            position: relative;
        }
        .upload-form input[type="file"] {
            flex: 1;
        }
        .upload-form button {
            background: #2196F3;
            color: #fff;
            border: none;
            padding: 8px 18px;
            border-radius: 4px;
            cursor: pointer;
            font-weight: bold;
            transition: background 0.2s;
        }
        .upload-form button:hover:enabled {
            background: #1976D2;
        }
        .upload-form button:disabled {
            background: #90caf9;
            cursor: not-allowed;
        }
        .upload-progress-bar-container {
            width: 100%;
            height: 18px;
            background: #e0e0e0;
            border-radius: 8px;
            margin-top: 10px;
            overflow: hidden;
            display: none;
        }
        .upload-progress-bar {
            height: 100%;
            width: 0%;
            background: linear-gradient(90deg, #2196F3, #1976D2);
            border-radius: 8px;
            transition: width 0.2s;
        }
        .upload-progress-text {
            position: absolute;
            left: 50%;
            top: 100%;
            transform: translate(-50%, 0);
            font-size: 12px;
            color: #333;
            margin-top: 2px;
            display: none;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
        }
        th, td {
            border: 1px solid #ddd;
            padding: 12px;
            text-align: left;
        }
        th {
            background-color: #4CAF50;
            color: white;
            font-weight: bold;
        }
        tr:nth-child(even) {
            background-color: #f2f2f2;
        }
        tr:hover {
            background-color: #e8f5e8;
        }
        .file-size {
            text-align: right;
        }
        .loading {
            text-align: center;
            padding: 20px;
            color: #666;
        }
        .delete-btn {
            background-color: #f44336;
            color: white;
            border: none;
            padding: 6px 12px;
            border-radius: 4px;
            cursor: pointer;
            font-size: 12px;
        }
        .delete-btn:hover {
            background-color: #d32f2f;
        }
        .action-column {
            text-align: center;
            width: 100px;
        }
        .file-link {
            color: #2196F3;
            text-decoration: none;
            font-weight: bold;
        }
        .file-link:hover {
            text-decoration: underline;
            color: #1976D2;
        }
        </style>
        </head>
        <body>
        <div class="container">
        <h1>SD Card File Manager</h1>
        <form class="upload-form" id="uploadForm">
            <input type="file" name="file" id="fileInput" required>
            <button type="submit" id="uploadBtn">Upload</button>
        </form>
        <div class="upload-progress-bar-container" id="uploadProgressContainer">
            <div class="upload-progress-bar" id="uploadProgressBar"></div>
        </div>
        <div class="upload-progress-text" id="uploadProgressText">0%</div>
        <div class="info-section">
            <h3>Storage Information</h3>
            <p><strong>Total Size:</strong> <span id='cardSize'></span> MB</p>
            <p><strong>Free Space:</strong> <span id='freeSpace'></span> MB</p>
            <div class="space-bar">
            <div class="space-used" id="spaceBar"></div>
            <div class="space-text" id="spaceText"></div>
            </div>
        </div>
        <h3>Files:</h3>
        <div id='fileList' class="loading">Loading files...</div>
        </div>
        <script>
        const cardSize = )rawliteral"
                  + String(SD_MMC.cardSize() / (1024 * 1024)) + R"rawliteral(;
        const freeSpace = )rawliteral"
                  + String((SD_MMC.cardSize() - SD_MMC.usedBytes()) / (1024 * 1024)) + R"rawliteral(;
        const usedSpace = cardSize - freeSpace;
        
        document.getElementById('cardSize').textContent = cardSize.toFixed(2);
        document.getElementById('freeSpace').textContent = freeSpace.toFixed(2);
        
        // Update space bar
        const usedPercentage = (usedSpace / cardSize) * 100;
        document.getElementById('spaceBar').style.width = usedPercentage + '%';
        document.getElementById('spaceText').textContent = usedPercentage.toFixed(1) + '% Used';
        
        function deleteFile(filename) {
            fetch('/delete?file=' + encodeURIComponent(filename), { method: 'DELETE' })
            .then(r => r.text())
            .then(result => {
            alert(result);
            loadFiles(); // Reload file list
            })
            .catch(err => {
            alert('Error deleting file: ' + err);
            });
        }
        
        function getFileExtension(filename) {
            return filename.split('.').pop().toLowerCase();
        }
        
        function isWebFile(filename) {
            const webExtensions = ['html', 'htm', 'css', 'js', 'json', 'txt'];
            return webExtensions.includes(getFileExtension(filename));
        }
        
        function loadFiles() {
            document.getElementById('fileList').innerHTML = '<div class="loading">Loading files...</div>';
            fetch('/files')
            .then(r => r.json())
            .then(files => {
            let table = '<table><thead><tr><th>File Name</th><th>Size (bytes)</th><th>Size (KB)</th><th>Actions</th></tr></thead><tbody>';
            files.forEach(f => {
            const sizeKB = (f.size / 1024).toFixed(2);
            const fileUrl = '/' + f.name;
            const isWeb = isWebFile(f.name);
            const linkTarget = isWeb ? '_blank' : '_self';
            const downloadAttr = isWeb ? '' : 'download';
            
            table += `<tr>
            <td><a href="${fileUrl}" target="${linkTarget}" ${downloadAttr} class="file-link">${f.name}</a></td>
            <td class="file-size">${f.size.toLocaleString()}</td>
            <td class="file-size">${sizeKB}</td>
            <td class="action-column">
            <button class="delete-btn" onclick="deleteFile('/${f.name}')">Delete</button>
            </td>
            </tr>`;
            });
            table += '</tbody></table>';
            document.getElementById('fileList').innerHTML = table;
            })
            .catch(err => {
            document.getElementById('fileList').innerHTML = '<p style="color: red;">Error loading files: ' + err + '</p>';
            });
        }

        // Upload file handler with progress bar
        document.getElementById('uploadForm').addEventListener('submit', function(e) {
            e.preventDefault();
            const fileInput = document.getElementById('fileInput');
            const uploadBtn = document.getElementById('uploadBtn');
            const progressBar = document.getElementById('uploadProgressBar');
            const progressContainer = document.getElementById('uploadProgressContainer');
            const progressText = document.getElementById('uploadProgressText');
            if (!fileInput.files.length) return;
            uploadBtn.disabled = true;
            const originalText = uploadBtn.textContent;
            uploadBtn.textContent = 'Uploading...';

            // Show progress bar
            progressBar.style.width = '0%';
            progressContainer.style.display = 'block';
            progressText.style.display = 'block';
            progressText.textContent = '0%';

            const formData = new FormData();
            formData.append('file', fileInput.files[0]);

            const xhr = new XMLHttpRequest();
            xhr.open('POST', '/upload', true);

            xhr.upload.onprogress = function(e) {
            if (e.lengthComputable) {
                const percent = (e.loaded / e.total) * 100;
                progressBar.style.width = percent + '%';
                progressText.textContent = percent.toFixed(1) + '%';
            }
            };

            xhr.onload = function() {
            uploadBtn.disabled = false;
            uploadBtn.textContent = originalText;
            progressBar.style.width = '100%';
            progressText.textContent = '100%';
            setTimeout(() => {
                progressContainer.style.display = 'none';
                progressText.style.display = 'none';
            }, 800);
            if (xhr.status === 200) {
                alert(xhr.responseText);
                fileInput.value = '';
                loadFiles();
            } else {
                alert('Upload error: ' + xhr.statusText);
            }
            };

            xhr.onerror = function() {
            uploadBtn.disabled = false;
            uploadBtn.textContent = originalText;
            progressContainer.style.display = 'none';
            progressText.style.display = 'none';
            alert('Upload error');
            };

            xhr.send(formData);
        });
        
        // Load file list initially
        loadFiles();
        </script>
        </body>
        </html>
        )rawliteral";

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
      uint64_t total = SD_MMC.cardSize();
      uint64_t free = SD_MMC.cardSize() - SD_MMC.usedBytes();
      String json = "{\"total\":" + String(total) + ",\"free\":" + String(free) + "}";
      request->send(200, "application/json", json);
    });
  // Handle file upload
  // List of forbidden filenames (lowercase)
  

server.on(
    "/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "File uploaded successfully");
    },
    [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        String path = "/" + filename;

        if (index == 0) {
            // Awal upload, buka file dan simpan di _tempFile
            if (SD_MMC.exists(path)) {
                SD_MMC.remove(path);
            }
            request->_tempFile = SD_MMC.open(path, FILE_WRITE);
        }

        // Tulis data ke file jika file terbuka
        if (request->_tempFile) {
            request->_tempFile.write(data, len);
        }

        // Jika sudah selesai, tutup file
        if (final && request->_tempFile) {
            request->_tempFile.close();
        }
    }
);

  // Handle 404 errors
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "File not found");
  });
}
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
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
  if (!SD_MMC.begin("/sdcard", false, true, SDMMC_FREQ_DEFAULT, 5)) {
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
  serverSetup();
  server.begin();
  Serial.println("Async Web server started");
  ota_setup();
  // Serial.println("Initializing WG interface...");
  // if (wg.begin(local_ip, private_key, endpoint_address, public_key, endpoint_port, preshared_key))
  // {
  //     Serial.println("OK");
  // }
  // else
  // {
  //     Serial.println("FAIL");
  // }
  // Serial.println(local_ip.toString());
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("reboot")) {
      Serial.println("Rebooting ESP...");
      delay(100);
      ESP.restart();
    }
  }
}
