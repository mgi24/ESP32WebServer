#!/bin/bash

WG_CONF="/etc/wireguard/wg0.conf"
WG_INTERFACE="wg0"
SERVER_PUBKEY="" # Ganti dengan public key server Anda
ENDPOINT="IP VPS:51820"

read -p "Input nama peer: " PEER_NAME

# Generate keypair dan preshared key
PRIVKEY=$(wg genkey)
PUBKEY=$(echo "$PRIVKEY" | wg pubkey)
PRESHARED=$(wg genpsk)

# Cari IP terakhir yang digunakan dengan benar
LAST_IP=$(grep "AllowedIPs" $WG_CONF | grep -o "10\.8\.0\.[0-9]*" | cut -d'.' -f4 | sort -n | tail -1)
echo "Last used IP: $LAST_IP"
if [[ -z "$LAST_IP" ]]; then
    PEER_IP=2
else
    PEER_IP=$((LAST_IP+1))
fi

wg set $WG_INTERFACE peer $PUBKEY preshared-key <(echo "$PRESHARED") allowed-ips 10.8.0.$PEER_IP/32

# Simpan konfigurasi aktif ke file
wg-quick save $WG_INTERFACE

# Cetak konfigurasi untuk peer
cat <<EOC

# Config untuk $PEER_NAME
[Interface]
PrivateKey = $PRIVKEY
Address = 10.8.0.$PEER_IP/24

[Peer]
PublicKey = $SERVER_PUBKEY
PresharedKey = $PRESHARED
AllowedIPs = 10.8.0.0/24
PersistentKeepalive = 0
Endpoint = $ENDPOINT

EOC