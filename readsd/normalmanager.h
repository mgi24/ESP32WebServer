String normalmanager_html = R"rawliteral(




<!DOCTYPE html>
<html>

<head>
    <title>File Manager</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #181a1b;
            color: #e0e0e0;
        }

        .container {
            max-width: 800px;
            margin: 0 auto;
            background: #23272a;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.5);
        }

        h1 {
            color: #fff;
            text-align: center;
            margin-bottom: 30px;
        }

        .info-section {
            margin-bottom: 30px;
            padding: 15px;
            background: #202225;
            border-radius: 5px;
        }

        .space-bar {
            background: #2c2f33;
            border-radius: 10px;
            height: 30px;
            margin: 10px 0;
            overflow: hidden;
            position: relative;
        }

        .space-used {
            height: 100%;
            background: linear-gradient(90deg, #4CAF50, #388e3c);
            border-radius: 10px;
            transition: width 0.3s ease;
        }

        .space-text {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            font-weight: bold;
            color: #e0e0e0;
            z-index: 1;
        }

        .upload-form {
            display: flex;
            align-items: center;
            gap: 10px;
            margin-bottom: 25px;
            background: #23272a;
            padding: 12px 16px;
            border-radius: 6px;
            box-shadow: 0 1px 3px rgba(0, 0, 0, 0.15);
            position: relative;
        }

        .upload-form input[type="file"] {
            flex: 1;
            color-scheme: dark;
        }

        .upload-form button {
            background: #1976D2;
            color: #fff;
            border: none;
            padding: 8px 18px;
            border-radius: 4px;
            cursor: pointer;
            font-weight: bold;
            transition: background 0.2s;
        }

        .upload-form button:hover:enabled {
            background: #1565c0;
        }

        .upload-form button:disabled {
            background: #37474f;
            cursor: not-allowed;
        }

        .upload-progress-bar-container {
            width: 100%;
            height: 18px;
            background: #2c2f33;
            border-radius: 8px;
            margin-top: 10px;
            overflow: hidden;
            display: none;
        }

        .upload-progress-bar {
            height: 100%;
            width: 0%;
            background: linear-gradient(90deg, #1976D2, #1565c0);
            border-radius: 8px;
            transition: width 0.2s;
        }

        .upload-progress-text {
            position: absolute;
            left: 50%;
            top: 100%;
            transform: translate(-50%, 0);
            font-size: 12px;
            color: #e0e0e0;
            margin-top: 2px;
            display: none;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            margin-top: 20px;
            background: #23272a;
        }

        th,
        td {
            border: 1px solid #333;
            padding: 12px;
            text-align: left;
        }

        th {
            background-color: #388e3c;
            color: #fff;
            font-weight: bold;
        }

        tr:nth-child(even) {
            background-color: #202225;
        }

        tr:hover {
            background-color: #263238;
        }

        .file-size {
            text-align: right;
        }

        .loading {
            text-align: center;
            padding: 20px;
            color: #b0b0b0;
        }

        .delete-btn {
            background-color: #e53935;
            color: white;
            border: none;
            padding: 6px 12px;
            border-radius: 4px;
            cursor: pointer;
            font-size: 12px;
        }

        .delete-btn:hover {
            background-color: #b71c1c;
        }

        .action-column {
            text-align: center;
            width: 100px;
        }

        .file-link {
            color: #42a5f5;
            text-decoration: none;
            font-weight: bold;
        }

        .file-link:hover {
            text-decoration: underline;
            color: #90caf9;
        }
    </style>
</head>

<body>
    <div class="container">
        <div style="display: flex; gap: 12px; justify-content: center; margin-bottom: 24px;">
            <a href="/" style="background:#1976D2;color:#fff;padding:8px 18px;border-radius:4px;text-decoration:none;font-weight:bold;"  rel="noopener noreferrer">Home</a>
            <a href="/filemanager" style="background:#388e3c;color:#fff;padding:8px 18px;border-radius:4px;text-decoration:none;font-weight:bold;"  rel="noopener noreferrer">FileManager</a>
            <a href="/video" style="background:#8e24aa;color:#fff;padding:8px 18px;border-radius:4px;text-decoration:none;font-weight:bold;"  rel="noopener noreferrer">Video Player</a>
            <a href="/gallery" style="background:#fbc02d;color:#23272a;padding:8px 18px;border-radius:4px;text-decoration:none;font-weight:bold;"  rel="noopener noreferrer">Gallery</a>
        </div>
        <h1>SD Card File Manager</h1>
        <form class="upload-form" id="uploadForm">
            <input type="file" name="file" id="fileInput" required>
            <button type="submit" id="uploadBtn">Upload CURRENTLY DISABLED!!!</button>
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
        function updateStorageInfo() {
    fetch('/getStorageInfo')
        .then(r => r.json())
        .then(data => {
            const cardSize = data.total / (1024 * 1024);
            const freeSpace = data.free / (1024 * 1024);
            const usedSpace = cardSize - freeSpace;
            const usedPercentage = cardSize > 0 ? (usedSpace / cardSize) * 100 : 0;

            document.getElementById('cardSize').textContent = cardSize.toFixed(2);
            document.getElementById('freeSpace').textContent = freeSpace.toFixed(2);
            document.getElementById('spaceBar').style.width = usedPercentage + '%';
            document.getElementById('spaceText').textContent = usedPercentage.toFixed(1) + '% Used';
        })
        .catch(err => {
            document.getElementById('cardSize').textContent = 'Error';
            document.getElementById('freeSpace').textContent = 'Error';
            document.getElementById('spaceBar').style.width = '0%';
            document.getElementById('spaceText').textContent = 'Error';
        });
}
updateStorageInfo();
function afterFileChanged() {
    loadFiles();
    updateStorageInfo();
}
        function deleteFile(filename) {
            fetch('/delete?file=' + encodeURIComponent(filename), { method: 'DELETE' })
                .then(r => r.text())
                .then(result => {
                    alert(result);
                    afterFileChanged();
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
                    let table = '<table><thead><tr><th>File Name</th><th>Size (KB)</th></tr></thead><tbody>';
                    files.forEach(f => {
                        const sizeKB = (f.size / 1024).toFixed(2);
                        const fileUrl = '/' + f.name;
                        const isWeb = isWebFile(f.name);
                        const linkTarget = isWeb ? '_blank' : '_self';
                        const downloadAttr = isWeb ? '' : 'download';

                        table += `<tr>
        <td><a href="${fileUrl}" target="${linkTarget}" ${downloadAttr} class="file-link">${f.name}</a></td>
        <td class="file-size">${sizeKB}</td>
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
        document.getElementById('uploadForm').addEventListener('submit', function (e) {
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

            xhr.upload.onprogress = function (e) {
                if (e.lengthComputable) {
                    const percent = (e.loaded / e.total) * 100;
                    progressBar.style.width = percent + '%';
                    progressText.textContent = percent.toFixed(1) + '%';
                }
            };

            xhr.onload = function () {
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

            xhr.onerror = function () {
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