#!/bin/bash

# Restore script for Pong Robot critical files
# Run as root

# Check if backup directory is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <backup_directory>"
    echo "Example: $0 /critical_files/20250514_123456"
    exit 1
fi

# Configuration
BACKUP_DIR="$1"
NGINX_DIR="/etc/nginx"
PHP_DIR="/etc/php/8.1/fpm"
JANUS_DIR="/opt/janus/etc/janus"
SSL_DIR="/etc/letsencrypt/live/stream.pongrobot.com"
LOG_FILE="/critical_files/restore.log"

# Ensure backup directory exists
if [ ! -d "$BACKUP_DIR" ]; then
    echo "[$(date)] Error: Backup directory $BACKUP_DIR does not exist" | tee -a "$LOG_FILE"
    exit 1
fi

# Ensure log directory exists
mkdir -p "$(dirname "$LOG_FILE")"
if [ $? -ne 0 ]; then
    echo "[$(date)] Error: Failed to create log directory for $LOG_FILE" | tee -a "$LOG_FILE"
    exit 1
fi

# Log start
echo "[$(date)] Starting restore from $BACKUP_DIR" | tee -a "$LOG_FILE"

# Install dependencies if missing
echo "[$(date)] Checking and installing dependencies" | tee -a "$LOG_FILE"
apt-get update
apt-get install -y nginx php8.1-fpm janus certbot
if [ $? -eq 0 ]; then
    echo "Installed dependencies: nginx, php8.1-fpm, janus, certbot" | tee -a "$LOG_FILE"
else
    echo "Warning: Some dependencies may not have been installed" | tee -a "$LOG_FILE"
fi

# Stop services to prevent conflicts
echo "[$(date)] Stopping services" | tee -a "$LOG_FILE"
systemctl stop nginx php8.1-fpm janus 2>/dev/null

# NGINX config
if [ -d "$BACKUP_DIR/nginx" ]; then
    rm -rf "$NGINX_DIR"/*  # Clear existing configs
    mkdir -p "$NGINX_DIR"
    cp -r "$BACKUP_DIR/nginx"/* "$NGINX_DIR/" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "Restored NGINX config to $NGINX_DIR" | tee -a "$LOG_FILE"
    else
        echo "Warning: Some NGINX config files may not have been restored" | tee -a "$LOG_FILE"
    fi
    # Fix permissions
    chown -R www-data:www-data "$NGINX_DIR"
    chmod -R 644 "$NGINX_DIR"/*
    chmod 755 "$NGINX_DIR" "$NGINX_DIR/sites-available" "$NGINX_DIR/sites-enabled" "$NGINX_DIR/snippets"
else
    echo "Error: NGINX backup directory $BACKUP_DIR/nginx not found" | tee -a "$LOG_FILE"
fi

# PHP config
if [ -d "$BACKUP_DIR/php" ]; then
    rm -rf "$PHP_DIR"/*  # Clear existing configs
    mkdir -p "$PHP_DIR"
    cp -r "$BACKUP_DIR/php"/* "$PHP_DIR/" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "Restored PHP config to $PHP_DIR" | tee -a "$LOG_FILE"
    else
        echo "Warning: Some PHP config files may not have been restored" | tee -a "$LOG_FILE"
    fi
    # Fix permissions
    chown -R root:root "$PHP_DIR"
    chmod -R 644 "$PHP_DIR"/*
    chmod 755 "$PHP_DIR" "$PHP_DIR/pool.d"
else
    echo "Error: PHP backup directory $BACKUP_DIR/php not found" | tee -a "$LOG_FILE"
fi

# Janus config
if [ -d "$BACKUP_DIR/janus" ]; then
    rm -rf "$JANUS_DIR"/*  # Clear existing configs
    mkdir -p "$JANUS_DIR"
    cp -r "$BACKUP_DIR/janus"/* "$JANUS_DIR/" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "Restored Janus config to $JANUS_DIR" | tee -a "$LOG_FILE"
    else
        echo "Warning: Some Janus config files may not have been restored" | tee -a "$LOG_FILE"
    fi
    # Fix permissions
    chown -R janus:janus "$JANUS_DIR" 2>/dev/null || chown -R root:root "$JANUS_DIR"
    chmod -R 644 "$JANUS_DIR"/*
    chmod 755 "$JANUS_DIR"
else
    echo "Error: Janus backup directory $BACKUP_DIR/janus not found" | tee -a "$LOG_FILE"
fi

# SSL certificates
if [ -d "$BACKUP_DIR/ssl" ]; then
    rm -rf "$SSL_DIR"/*  # Clear existing certs
    mkdir -p "$SSL_DIR"
    cp -r "$BACKUP_DIR/ssl"/* "$SSL_DIR/" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "Restored SSL certificates to $SSL_DIR" | tee -a "$LOG_FILE"
    else
        echo "Warning: Some SSL certificates may not have been restored" | tee -a "$LOG_FILE"
    fi
    # Fix permissions
    chown -R root:root "$SSL_DIR"
    chmod -R 600 "$SSL_DIR"/*
    chmod 755 "$SSL_DIR"
else
    echo "Error: SSL backup directory $BACKUP_DIR/ssl not found" | tee -a "$LOG_FILE"
fi

# Cron jobs
if [ -f "$BACKUP_DIR/cron/root_crontab" ]; then
    crontab "$BACKUP_DIR/cron/root_crontab" 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "Restored cron jobs" | tee -a "$LOG_FILE"
    else
        echo "Warning: Some cron jobs may not have been restored" | tee -a "$LOG_FILE"
    fi
else
    echo "Error: Cron backup file $BACKUP_DIR/cron/root_crontab not found" | tee -a "$LOG_FILE"
fi

# Restart services
echo "[$(date)] Restarting services" | tee -a "$LOG_FILE"
systemctl start nginx php8.1-fpm janus
if [ $? -eq 0 ]; then
    echo "Services restarted: nginx, php8.1-fpm, janus" | tee -a "$LOG_FILE"
else
    echo "Warning: Some services may not have started" | tee -a "$LOG_FILE"
fi

# Verify restoration
echo "[$(date)] Verifying restored files" | tee -a "$LOG_FILE"
for FILE in "$NGINX_DIR/nginx.conf" "$NGINX_DIR/sites-available/default" "$PHP_DIR/php.ini" "$JANUS_DIR/janus.jcfg" "$SSL_DIR/cert.pem"; do
    if [ -f "$FILE" ]; then
        echo "Verified: $FILE exists" | tee -a "$LOG_FILE"
    else
        echo "Warning: $FILE not found after restore" | tee -a "$LOG_FILE"
    fi
done

# Log completion
echo "[$(date)] Restore completed from $BACKUP_DIR" | tee -a "$LOG_FILE"

exit 0
