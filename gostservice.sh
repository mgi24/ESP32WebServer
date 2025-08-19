#!/bin/bash

# Prompt for gost command
read -p "Masukkan command gost (misal: gost -L tcp://:80/10.8.0.2:80): " GOST_CMD

# Path untuk service file
SERVICE_FILE="/etc/systemd/system/gost.service"

# Buat file service
cat > "$SERVICE_FILE" <<EOF
[Unit]
Description=GOST Service
After=network.target

[Service]
Type=simple
ExecStart=$GOST_CMD
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF

# Reload systemd, enable dan start service
systemctl daemon-reload
systemctl enable gost
systemctl restart gost

echo "Service gost telah dibuat, di-enable, dan dijalankan."