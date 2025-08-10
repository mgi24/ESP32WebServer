String video_html = R"rawliteral(




<!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">
    <title>SD Card Video Player</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #222;
            color: #fff;
            margin: 0;
            padding: 0;
        }

        .container {
            max-width: 700px;
            margin: 40px auto;
            background: #333;
            border-radius: 12px;
            box-shadow: 0 4px 24px rgba(0, 0, 0, 0.3);
            padding: 32px 24px 24px 24px;
        }

        h1 {
            text-align: center;
            margin-bottom: 30px;
            color: #4CAF50;
        }

        .file-list {
            margin-bottom: 30px;
        }

        .file-item {
            display: flex;
            align-items: center;
            justify-content: space-between;
            background: #444;
            border-radius: 6px;
            margin-bottom: 10px;
            padding: 10px 16px;
        }

        .file-name {
            font-size: 16px;
            color: #fff;
            word-break: break-all;
        }

        .play-btn {
            background: #4CAF50;
            color: #fff;
            border: none;
            border-radius: 4px;
            padding: 8px 18px;
            font-size: 15px;
            cursor: pointer;
            transition: background 0.2s;
        }

        .play-btn:hover {
            background: #388e3c;
        }

        .video-player {
            display: none;
            margin-top: 30px;
            background: #111;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 2px 12px rgba(0, 0, 0, 0.4);
        }

        video {
            width: 100%;
            max-height: 400px;
            border-radius: 8px;
            background: #000;
        }

        .close-btn {
            background: #f44336;
            color: #fff;
            border: none;
            border-radius: 4px;
            padding: 6px 14px;
            font-size: 14px;
            cursor: pointer;
            float: right;
            margin-bottom: 10px;
        }

        .close-btn:hover {
            background: #b71c1c;
        }

        .no-video {
            text-align: center;
            color: #bbb;
            margin-top: 40px;
        }

        @media (max-width: 600px) {
            .container {
                padding: 12px 4px;
            }

            video {
                max-height: 220px;
            }
        }
    </style>
</head>

<body>
    <div class="container">
        <h1>SD Card Video Player</h1>
        <div style="display: flex; gap: 12px; justify-content: center; margin-bottom: 24px;">
            <a class="play-btn" href="/" target="_self" style="text-decoration:none;display:inline-block;">Home</a>
            <a class="play-btn" href="/filemanager" target="_self" style="text-decoration:none;display:inline-block;">File Manager</a>
            <a class="play-btn" href="/video" target="_self" style="text-decoration:none;display:inline-block;">Video Player</a>
            <a class="play-btn" href="/gallery" target="_self" style="text-decoration:none;display:inline-block;">Gallery</a>
        </div>
        <div id="fileList" class="file-list"></div>
    </div>
    <script>
        // Track the currently playing video element
        let currentPlayingVideo = null;

        // Mendapatkan daftar file dari backend
        function fetchFiles() {
            fetch('/videofiles')
            .then(response => response.json())
            .then(files => {
                const fileList = document.getElementById('fileList');
                fileList.innerHTML = '';
                if (!files.length) {
                fileList.innerHTML = '<div class="no-video">No video files found.</div>';
                return;
                }
                files.forEach((file, idx) => {
                // file: {name: "...", size: ...}
                const filename = file.name;
                const size = file.size;
                const ext = filename.split('.').pop().toLowerCase();
                if (ext !== 'mp4' && ext !== 'avi') return;
                const videoId = 'video_' + idx;
                // Format size to human readable
                function formatSize(bytes) {
                    if (bytes < 1024) return bytes + ' B';
                    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
                    if (bytes < 1024 * 1024 * 1024) return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
                    return (bytes / (1024 * 1024 * 1024)).toFixed(1) + ' GB';
                }
                fileList.innerHTML += `
                    <div class="file-item">
                    <span class="file-name">${decodeURIComponent(filename)}</span>
                    <span style="color:#aaa;font-size:13px;margin-left:10px;">(${formatSize(size)})</span>
                    <button class="play-btn" onclick="playVideo('${encodeURIComponent(filename)}','${ext}','${videoId}')">Play</button>
                    </div>
                    <div class="video-player" id="${videoId}_player">
                    <button class="close-btn" onclick="closePlayer('${videoId}')">Close</button>
                    <div id="${videoId}_info">File: ${decodeURIComponent(filename)} (${formatSize(size)})</div>
                    <video id="${videoId}_video" controls>
                        <source id="${videoId}_source" src="" type="">
                        Your browser does not support the video tag.
                    </video>
                    </div>
                `;
                });
            })
            .catch(err => {
                document.getElementById('fileList').innerHTML = '<div class="no-video">Failed to load video list.</div>';
            });
        }

        // Show player under the selected file
        function playVideo(filename, ext, videoId) {
            // Stop and hide previous player if any
            if (currentPlayingVideo) {
            currentPlayingVideo.pause();
            currentPlayingVideo.currentTime = 0;
            currentPlayingVideo.closest('.video-player').style.display = 'none';
            }
            // Hide all other players
            document.querySelectorAll('.video-player').forEach(el => el.style.display = 'none');
            // Set up this player
            const player = document.getElementById(videoId + '_player');
            const video = document.getElementById(videoId + '_video');
            const source = document.getElementById(videoId + '_source');
            const info = document.getElementById(videoId + '_info');
            let url = '/videos/' + decodeURIComponent(filename);
            source.src = url;
            source.type = (ext === 'mp4') ? 'video/mp4' : 'video/avi';
            video.load();
            player.style.display = 'block';
            setTimeout(() => { player.scrollIntoView({ behavior: "smooth" }); }, 200);
            currentPlayingVideo = video;
        }

        function closePlayer(videoId) {
            const player = document.getElementById(videoId + '_player');
            const video = document.getElementById(videoId + '_video');
            video.pause();
            player.style.display = 'none';
            // Clear currentPlayingVideo if this is the one being closed
            if (currentPlayingVideo === video) {
            currentPlayingVideo = null;
            }
        }

        fetchFiles();
    </script>










)rawliteral";