#!/bin/bash

# Pong Robot installation script for Ubuntu 22.04 (without backup restoration)
# Run as root

# Configuration
LOG_FILE="/critical_files/install.log"
WEB_DIR="/var/www/html"

# Ensure log directory exists
mkdir -p "$(dirname "$LOG_FILE")"
echo "[$(date)] Starting Pong Robot installation" | tee -a "$LOG_FILE"

# Update system
echo "[$(date)] Updating system" | tee -a "$LOG_FILE"
apt update && apt upgrade -y

# Enable Universe repository
echo "[$(date)] Enabling Universe repository" | tee -a "$LOG_FILE"
add-apt-repository universe -y
apt update

# Install Nginx with RTMP module
echo "[$(date)] Installing Nginx and RTMP module" | tee -a "$LOG_FILE"
apt install -y nginx libnginx-mod-rtmp
nginx -V 2>&1 | grep rtmp && echo "RTMP module installed" | tee -a "$LOG_FILE"

# Install PHP 8.1
echo "[$(date)] Installing PHP 8.1" | tee -a "$LOG_FILE"
apt install -y software-properties-common
add-apt-repository ppa:ondrej/php -y
apt update
apt install -y php8.1 php8.1-fpm php8.1-cli php8.1-common

# Install FFmpeg
echo "[$(date)] Installing FFmpeg" | tee -a "$LOG_FILE"
add-apt-repository ppa:savoury1/ffmpeg4 -y
apt update
apt install -y ffmpeg

# Install Janus WebRTC Server
echo "[$(date)] Installing Janus dependencies" | tee -a "$LOG_FILE"
apt install -y libmicrohttpd-dev libjansson-dev libssl-dev libsrtp2-dev \
    libsofia-sip-ua-dev libglib2.0-dev libopus0 libogg-dev libcurl4-openssl-dev \
    liblua5.3-dev libconfig-dev pkg-config gengetopt libtool automake cmake \
    libnice-dev libwebsockets-dev
echo "[$(date)] Installing Janus" | tee -a "$LOG_FILE"
cd /tmp
wget https://github.com/meetecho/janus-gateway/archive/refs/tags/v0.11.8.tar.gz
tar -xzf v0.11.8.tar.gz
cd janus-gateway-0.11.8
./autogen.sh
./configure --prefix=/opt/janus
make
make install
cat <<EOF >/etc/systemd/system/janus.service
[Unit]
Description=Janus WebRTC Server
After=network.target

[Service]
ExecStart=/opt/janus/bin/janus --stun-server=stun.l.google.com:19302
Restart=always

[Install]
WantedBy=multi-user.target
EOF
systemctl daemon-reload
systemctl enable janus

# Install Certbot
echo "[$(date)] Installing Certbot" | tee -a "$LOG_FILE"
apt install -y certbot python3-certbot-nginx

# Install additional dependencies
echo "[$(date)] Installing additional dependencies" | tee -a "$LOG_FILE"
apt install -y cron rsync tar gzip libopus0 libnice10 libsrtp2-1 libwebsockets16

# Ensure web directory exists
mkdir -p "$WEB_DIR/js"
chown -R www-data:www-data "$WEB_DIR"
chmod -R 644 "$WEB_DIR"/*
chmod 755 "$WEB_DIR" "$WEB_DIR/js"

# Copy web files (stream.html, js) - assuming copied to /tmp
echo "[$(date)] Copying web files" | tee -a "$LOG_FILE"
cp /tmp/stream.html "$WEB_DIR/" 2>/dev/null
cp -r /tmp/js/* "$WEB_DIR/js/" 2>/dev/null
chown -R www-data:www-data "$WEB_DIR"
chmod -R 644 "$WEB_DIR"/*
chmod 755 "$WEB_DIR" "$WEB_DIR/js"

# Copy backup and restore scripts
echo "[$(date)] Copying backup and restore scripts" | tee -a "$LOG_FILE"
cp /tmp/backup_pongrobot.sh /usr/local/bin/backup_pongrobot.sh 2>/dev/null
cp /tmp/restore_pongrobot.sh /usr/local/bin/restore_pongrobot.sh 2>/dev/null
chmod +x /usr/local/bin/backup_pongrobot.sh /usr/local/bin/restore_pongrobot.sh

# Start services
echo "[$(date)] Starting services" | tee -a "$LOG_FILE"
systemctl start nginx php8.1-fpm janus
systemctl enable nginx php8.1-fpm janus

# Start FFmpeg
echo "[$(date)] Starting FFmpeg" | tee -a "$LOG_FILE"
pkill -f ffmpeg
nohup ffmpeg -re -i rtmp://66.179.82.23:1935/live/23672E6t -map 0:v -c:v copy -b:v 500k -payload_type 100 -f rtp rtp://66.179.82.23:5006 -map 0:a -c:a libopus -b:a 64k -payload_type 111 -f rtp rtp://66.179.82.23:5004 -loglevel verbose -max_delay 0 -avioflags direct -fflags +genpts -flags +global_header -flush_packets 0 > /var/log/ffmpeg.log 2>&1 &

# Log completion
echo "[$(date)] Installation completed (backup restoration skipped)" | tee -a "$LOG_FILE"
echo "[$(date)] Run '/usr/local/bin/restore_pongrobot.sh $BACKUP_DIR' to restore configs" | tee -a "$LOG_FILE"

exit 0
