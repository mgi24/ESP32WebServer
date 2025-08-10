String gallery_html = R"rawliteral(





<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Gallery</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            background: #181a1b;
            color: #e0e0e0;
        }
        .navbar {
            display: flex;
            justify-content: center;
            gap: 24px;
            background: #23272a;
            padding: 16px 0;
            box-shadow: 0 2px 8px rgba(0,0,0,0.15);
        }
        .navbar button {
            background: #2c2f33;
            color: #e0e0e0;
            border: none;
            border-radius: 4px;
            padding: 10px 22px;
            font-size: 1em;
            cursor: pointer;
            transition: background 0.2s;
        }
        .navbar button.active,
        .navbar button:hover {
            background: #7289da;
            color: #fff;
        }
        .gallery {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(400px, 1fr));
            gap: 16px;
            padding: 24px;
        }
        .tile {
            background: #23272a;
            border-radius: 8px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.18);
            overflow: hidden;
            display: flex;
            flex-direction: column;
            align-items: center;
            padding: 8px;
            width: 400px;
            box-sizing: border-box;
            margin: 0 auto;
        }
        .tile img {
            width: 100%;
            height: auto;
            object-fit: contain;
            border-radius: 4px;
            background: #181a1b;
            display: block;
        }
        .filename {
            margin-top: 8px;
            font-size: 0.95em;
            color: #b9bbbe;
            word-break: break-all;
            text-align: center;
        }
        h2 {
            text-align: center;
            color: #fff;
            margin-top: 24px;
        }
        #page-info {
            text-align: center;
            margin-bottom: 12px;
            color: #b9bbbe;
            font-size: 1.1em;
        }
    </style>
</head>
<body>
    <div class="navbar">
        <a href="/" draggable="false"><button>Home</button></a>
        <a href="/filemanager" draggable="false"><button>File Manager</button></a>
        <a href="/video" draggable="false"><button>Video</button></a>
        <a href="/gallery" draggable="false"><button class="active">Gallery</button></a>
    </div>
    <h2>Gallery</h2>
    <div id="page-info">Page 1</div>
    <div class="gallery" id="gallery"></div>
    <div id="pagination" style="display:flex;justify-content:center;gap:8px;margin:24px 0;"></div>
    <script>
        let currentPage = 1;
        let totalPage = 1;

        async function fetchTotalPage() {
            try {
            const res = await fetch('/imagepage');
            if (!res.ok) throw new Error('Failed to fetch total page');
            const data = await res.json();
            totalPage = data.imgfoldercount || 1;
            } catch (e) {
            totalPage = 1;
            }
        }

        async function fetchImages(page) {
            try {
                const res = await fetch(`/image?page=${page}`);
                if (!res.ok) throw new Error('Failed to fetch images');
                return await res.json();
            } catch (e) {
                return [];
            }
        }

        function updatePageInfo() {
            document.getElementById('page-info').textContent = `Page ${currentPage}`;
        }

        async function renderGallery(images) {
            const gallery = document.getElementById('gallery');
            gallery.innerHTML = '';

            // Helper to load one image and wait until loaded
            function loadImageSequentially(imgObj, idx) {
            return new Promise((resolve) => {
                const tile = document.createElement('div');
                tile.className = 'tile';
                const image = document.createElement('img');
                const filenameStr = imgObj.filename || imgObj.url || imgObj;
                image.alt = filenameStr;
                const filename = document.createElement('div');
                filename.className = 'filename';
                filename.textContent = filenameStr;
                tile.appendChild(image);
                tile.appendChild(filename);
                gallery.appendChild(tile);

                image.onload = () => resolve();
                image.onerror = () => resolve(); // Skip on error
                image.src = `/images/${currentPage}/${filenameStr}`;
            });
            }

            // Sequentially load images
            for (let i = 0; i < images.length; i++) {
            await loadImageSequentially(images[i], i);
            }
        }

        function renderPagination() {
            const pagination = document.getElementById('pagination');
            pagination.innerHTML = '';
            for (let i = 1; i <= totalPage; i++) {
                const btn = document.createElement('button');
                btn.textContent = i;
                btn.style.background = (i === currentPage) ? '#7289da' : '#2c2f33';
                btn.style.color = (i === currentPage) ? '#fff' : '#e0e0e0';
                btn.style.border = 'none';
                btn.style.borderRadius = '4px';
                btn.style.padding = '8px 16px';
                btn.style.cursor = 'pointer';
                btn.onclick = () => {
                    if (currentPage !== i) {
                        currentPage = i;
                        loadPage();
                    }
                };
                pagination.appendChild(btn);
            }
        }

        async function loadPage() {
            updatePageInfo();
            const images = await fetchImages(currentPage);
            renderGallery(images);
            renderPagination();
        }

        async function init() {
            await fetchTotalPage();
            loadPage();
        }

        init();
    </script>
</body>
</html>





 )rawliteral";