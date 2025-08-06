const char filemanager_html[] PROGMEM = R"rawliteral(


<!DOCTYPE html>
<html>

<head>
    <title>SPIFFS File Manager</title>
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <style>
        body {
            font-family: Arial;
            margin: 2em;
            background: #f5f5f5;
        }

        .container {
            max-width: 800px;
            margin: 0 auto;
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
        }

        .storage-info {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 20px;
        }

        .storage-bar {
            background: rgba(255, 255, 255, 0.3);
            height: 20px;
            border-radius: 10px;
            overflow: hidden;
            margin: 10px 0;
        }

        .storage-fill {
            background: #4CAF50;
            height: 100%;
            transition: width 0.3s ease;
        }

        .storage-stats {
            display: flex;
            justify-content: space-between;
            margin-top: 10px;
            font-size: 14px;
        }

        table {
            border-collapse: collapse;
            width: 100%;
            margin-top: 20px;
        }

        th,
        td {
            border: 1px solid #ddd;
            padding: 12px;
            text-align: left;
        }

        th {
            background: #f8f9fa;
            font-weight: bold;
        }

        tr:hover {
            background: #f5f5f5;
        }

        input[type=file] {
            margin-bottom: 1em;
            padding: 10px;
            border: 2px dashed #ddd;
            border-radius: 5px;
            width: 100%;
        }

        button {
            padding: 8px 16px;
            background: #007bff;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
        }

        button:hover {
            background: #0056b3;
        }

        .delete-btn {
            background: #dc3545;
        }

        .delete-btn:hover {
            background: #c82333;
        }

        .upload-area {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 20px;
        }

        h2,
        h3 {
            color: #333;
        }

        .nav-link {
            display: inline-block;
            margin-right: 15px;
            color: #007bff;
            text-decoration: none;
        }

        .nav-link:hover {
            text-decoration: underline;
        }
    </style>
</head>

<body>
    <div class="container">
        <div style="margin-bottom: 20px;">
            <a href="/" class="nav-link">Back to Home</a>
            <a href="/filemanager.html" class="nav-link">File Manager</a>
        </div>

        <h2>SPIFFS File Manager</h2>

        <!-- Storage Info -->
        <div class="storage-info">
            <h3>Storage Information</h3>
            <div class="storage-bar">
                <div class="storage-fill" id="storageBar"></div>
            </div>
            <div class="storage-stats">
                <span id="usedSpace">Used: 0 KB</span>
                <span id="usedPercent">0%</span>
                <span id="freeSpace">Free: 0 KB</span>
            </div>
            <div style="margin-top: 10px; font-size: 14px;">
                <span id="totalSpace">Total: 0 KB</span>
            </div>
        </div>

        <!-- Upload Area -->
        <div class="upload-area">
            <h3>Upload File</h3>
            <form id="uploadForm">
                <input type="file" id="fileInput" name="file" accept="*/*">
                <button type="submit">Upload to SPIFFS</button>
            </form>
        </div>

        <!-- File List -->
        <h3>Files in SPIFFS</h3>
        <table id="fileTable">
            <thead>
                <tr>
                    <th>Name</th>
                    <th>Size</th>
                    <th>Type</th>
                    <th>Action</th>
                </tr>
            </thead>
            <tbody></tbody>
        </table>

        <script>
            function formatBytes(bytes) {
                if (bytes === 0) return '0 B';
                const k = 1024;
                const sizes = ['B', 'KB', 'MB'];
                const i = Math.floor(Math.log(bytes) / Math.log(k));
                return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
            }

            function getFileType(filename) {
                const ext = filename.split('.').pop().toLowerCase();
                const types = {
                    'html': 'HTML',
                    'css': 'CSS',
                    'js': 'JavaScript',
                    'png': 'Image',
                    'jpg': 'Image',
                    'jpeg': 'Image',
                    'gif': 'Image',
                    'mp4': 'Video',
                    'mp3': 'Audio',
                    'txt': 'Text',
                    'json': 'JSON'
                };
                return types[ext] || 'File';
            }

            function updateStorageInfo(storage) {
                const usedPercent = Math.round((storage.used / storage.total) * 100);
                document.getElementById('storageBar').style.width = usedPercent + '%';
                document.getElementById('usedSpace').textContent = 'Used: ' + formatBytes(storage.used);
                document.getElementById('freeSpace').textContent = 'Free: ' + formatBytes(storage.free);
                document.getElementById('totalSpace').textContent = 'Total: ' + formatBytes(storage.total);
                document.getElementById('usedPercent').textContent = usedPercent + '%';

                // Change color based on usage
                const bar = document.getElementById('storageBar');
                if (usedPercent > 90) bar.style.background = '#f44336';
                else if (usedPercent > 75) bar.style.background = '#ff9800';
                else bar.style.background = '#4CAF50';
            }

            function fetchFiles() {
                fetch('/files').then(r => r.json()).then(data => {
                    // Update storage info
                    updateStorageInfo(data.storage);

                    // Update file table
                    let rows = '';
                    data.files.forEach(f => {
                        rows += `<tr>
                  <td><a href="${f.name}" target="_blank">${f.name}</a></td>
                  <td>${formatBytes(f.size)}</td>
                  <td>${getFileType(f.name)}</td>
                  <td><button class="delete-btn" onclick="deleteFile('${f.name}')">Delete</button></td>
                </tr>`;
                    });
                    document.querySelector('#fileTable tbody').innerHTML = rows;
                }).catch(err => {
                    console.error('Error fetching files:', err);
                });
            }

            function deleteFile(name) {
                if (!confirm('Delete ' + name + '?')) return;
                fetch('/delete', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: 'name=' + encodeURIComponent(name)
                }).then(() => fetchFiles());
            }

            document.getElementById('uploadForm').onsubmit = function (e) {
                e.preventDefault();
                let fileInput = document.getElementById('fileInput');
                if (!fileInput.files.length) {
                    alert('Please select a file first!');
                    return;
                }

                let formData = new FormData();
                formData.append('file', fileInput.files[0]);

                // Show upload progress
                const button = e.target.querySelector('button');
                button.textContent = 'Uploading...';
                button.disabled = true;

                fetch('/upload', {  // GANTI dari '/' ke '/upload'
                    method: 'POST',
                    body: formData
                }).then(response => {
                    if (response.ok) {
                        fileInput.value = '';
                        fetchFiles();
                        alert('File uploaded successfully!');
                    } else {
                        alert('Upload failed!');
                    }
                }).catch(err => {
                    alert('Upload error: ' + err);
                }).finally(() => {
                    button.textContent = 'Upload to SPIFFS';
                    button.disabled = false;
                });
            };

            // Load files on page load
            fetchFiles();

            // Auto refresh every 30 seconds
            setInterval(fetchFiles, 30000);
        </script>
    </div>
</body>

</html>



)rawliteral";