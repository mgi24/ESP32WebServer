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
# Tampilkan status service
systemctl status gost --no-pager

# Aktifkan IP forwarding IPv4 secara permanen
sysctl -w net.ipv4.ip_forward=1
if ! grep -q "^net.ipv4.ip_forward=1" /etc/sysctl.conf; then
    echo "net.ipv4.ip_forward=1" >> /etc/sysctl.conf
fi

echo "Service gost telah dibuat, di-enable, dan dijalankan."