#!/bin/bash
set -e

read -p "Masukkan URL file gost (.tar.gz): " URL
TMP_DIR="/tmp/gost_install"

if [[ -z "$URL" ]]; then
    echo "[ERROR] URL tidak boleh kosong!"
    exit 1
fi

echo "[INFO] Membuat folder sementara..."
mkdir -p "$TMP_DIR"

echo "[INFO] Mengunduh gost dari $URL ..."
curl -L "$URL" -o "$TMP_DIR/gost.tar.gz"

echo "[INFO] Mengekstrak file..."
tar -xzf "$TMP_DIR/gost.tar.gz" -C "$TMP_DIR"

echo "[INFO] Memindahkan binary ke /usr/local/bin ..."
sudo mv "$TMP_DIR/gost" /usr/local/bin/gost

echo "[INFO] Mengatur permission..."
sudo chmod +x /usr/local/bin/gost

echo "[INFO] Membersihkan file sementara..."
rm -rf "$TMP_DIR"

echo "[INFO] Instalasi selesai. Versi gost:"
gost -V
