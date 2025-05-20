#!/bin/bash

# Backup script for Janus WebRTC streaming project
# Saves all modified configuration, web, backend, database, scripts, and Git repository
# Syncs files to /root/gitrobot before Git operations
# Date: May 20, 2025

# Ensure script is run as root
if [ "$(id -u)" != "0" ]; then
  echo "This script must be run as root (use sudo)" 1>&2
  exit 1
fi

# Set timestamp for backup directory
TIMESTAMP=$(date +%Y%m%d_%H%M)
BACKUP_DIR="/root/streaming_backup_${TIMESTAMP}"
ARCHIVE_FILE="/root/streaming_backup_${TIMESTAMP}.tar.gz"
ERROR_LOG="${BACKUP_DIR}/backup_errors.log"

# Create backup directory
mkdir -p "${BACKUP_DIR}"
touch "${ERROR_LOG}"

# Function to log errors
log_error() {
  echo "[ERROR] $1" >> "${ERROR_LOG}"
  echo "[ERROR] $1" >&2
}

# Sync files to /root/gitrobot
echo "Syncing files to /root/gitrobot..."
mkdir -p /root/gitrobot/frontend/js /root/gitrobot/frontend/css /root/gitrobot/frontend/api
mkdir -p /root/gitrobot/configs/nginx/sites-enabled /root/gitrobot/configs/janus /root/gitrobot/configs/systemd
mkdir -p /root/gitrobot/scripts /root/gitrobot/backend

# Copy web files
for file in /var/www/html/index.html /var/www/html/stream.html /var/www/html/favicon.ico /var/www/html/api/stream-key.json; do
  if [ -f "${file}" ]; then
    cp -v "${file}" "/root/gitrobot/frontend/$(basename "${file}")" 2>>"${ERROR_LOG}" || log_error "Failed to copy web file ${file} to /root/gitrobot/frontend/"
  else
    log_error "Web file ${file} does not exist"
  fi
done

# Copy web JS and CSS directories
for dir in /var/www/html/js /var/www/html/css; do
  if [ -d "${dir}" ]; then
    rsync -av --exclude 'hls' "${dir}/" "/root/gitrobot/frontend/$(basename "${dir}")/" 2>>"${ERROR_LOG}" || log_error "Failed to copy directory ${dir} to /root/gitrobot/frontend/"
  else
    log_error "Directory ${dir} does not exist"
  fi
done

# Copy Nginx configs
for file in /etc/nginx/sites-enabled/default /etc/nginx/sites-available/default /etc/nginx/nginx.conf; do
  if [ -f "${file}" ]; then
    cp -v "${file}" "/root/gitrobot/configs/nginx/$(basename "${file}")" 2>>"${ERROR_LOG}" || log_error "Failed to copy Nginx file ${file} to /root/gitrobot/configs/nginx/"
  else
    log_error "Nginx file ${file} does not exist"
  fi
done

# Copy Janus configs
if [ -d /opt/janus/etc/janus ]; then
  cp -v /opt/janus/etc/janus/*.jcfg "/root/gitrobot/configs/janus/" 2>>"${ERROR_LOG}" || log_error "Failed to copy Janus configs to /root/gitrobot/configs/janus/"
else
  log_error "Janus config directory /opt/janus/etc/janus does not exist"
fi

# Copy systemd services
for file in /etc/systemd/system/ffmpeg-stream.service; do
  if [ -f "${file}" ]; then
    cp -v "${file}" "/root/gitrobot/configs/systemd/" 2>>"${ERROR_LOG}" || log_error "Failed to copy service file ${file} to /root/gitrobot/configs/systemd/"
  else
    log_error "Service file ${file} does not exist"
  fi
done

# Copy scripts
for file in /usr/local/bin/backup_streaming_config.sh /usr/local/bin/install_pongrobot.sh /usr/local/bin/restore_pongrobot.sh /usr/local/bin/purge-large-logs.sh; do
  if [ -f "${file}" ]; then
    cp -v "${file}" "/root/gitrobot/scripts/" 2>>"${ERROR_LOG}" || log_error "Failed to copy script ${file} to /root/gitrobot/scripts/"
  else
    log_error "Script ${file} does not exist"
  fi
done

# Copy backend files and database
if [ -d /var/www/pongrobot-backend ]; then
  rsync -av --exclude 'node_modules' /var/www/pongrobot-backend/ "/root/gitrobot/backend/" 2>>"${ERROR_LOG}" || log_error "Failed to copy backend files to /root/gitrobot/backend/"
else
  log_error "Backend directory /var/www/pongrobot-backend does not exist"
fi

# Copy FFmpeg and systemd service files to backup
mkdir -p "${BACKUP_DIR}/systemd"
for file in /etc/systemd/system/ffmpeg-stream.service; do
  if [ -f "${file}" ]; then
    cp -v "${file}" "${BACKUP_DIR}/systemd/" 2>>"${ERROR_LOG}" || log_error "Failed to copy service file ${file}"
  else
    log_error "Service file ${file} does not exist"
  fi
done

# Copy all Janus configuration files to backup
mkdir -p "${BACKUP_DIR}/janus"
if [ -d /opt/janus/etc/janus ]; then
  cp -v /opt/janus/etc/janus/*.jcfg "${BACKUP_DIR}/janus/" 2>>"${ERROR_LOG}" || log_error "Failed to copy Janus configs from /opt/janus/etc/janus/"
else
  log_error "Janus config directory /opt/janus/etc/janus does not exist"
fi

# Copy Nginx configuration files to backup
mkdir -p "${BACKUP_DIR}/nginx"
for file in /etc/nginx/sites-enabled/default /etc/nginx/sites-available/default /etc/nginx/nginx.conf; do
  if [ -f "${file}" ]; then
    cp -v "${file}" "${BACKUP_DIR}/nginx/" 2>>"${ERROR_LOG}" || log_error "Failed to copy Nginx file ${file}"
  else
    log_error "Nginx file ${file} does not exist"
  fi
done

# Copy web files to backup, excluding hls folder
mkdir -p "${BACKUP_DIR}/www/html"
if [ -d /var/www/html ]; then
  rsync -av --exclude 'hls' /var/www/html/ "${BACKUP_DIR “

/www/html/" 2>>"${ERROR_LOG}" || log_error "Failed to copy web files from /var/www/html/"
else
  log_error "Web directory /var/www/html does not exist"
fi

# Copy backend files to backup
mkdir -p "${BACKUP_DIR}/www/pongrobot-backend"
if [ -d /var/www/pongrobot-backend ]; then
  rsync -av --exclude 'node_modules' /var/www/pongrobot-backend/ "${BACKUP_DIR}/www/pongrobot-backend/" 2>>"${ERROR_LOG}" || log_error "Failed to copy backend files from /var/www/pongrobot-backend/"
else
  log_error "Backend directory /var/www/pongrobot-backend does not exist"
fi

# Copy scripts from /usr/local/bin to backup
mkdir -p "${BACKUP_DIR}/usr/local/bin"
if [ -d /usr/local/bin ]; then
  cp -rv /usr/local/bin/* "${BACKUP_DIR}/usr/local/bin/" 2>>"${ERROR_LOG}" || log_error "Failed to copy scripts from /usr/local/bin/"
else
  log_error "Scripts directory /usr/local/bin does not exist"
fi

# Copy Git repository to backup
mkdir -p "${BACKUP_DIR}/gitrobot"
if [ -d /root/gitrobot ]; then
  rsync -av --exclude '.git' /root/gitrobot/ "${BACKUP_DIR}/gitrobot/" 2>>"${ERROR_LOG}" || log_error "Failed to copy Git repository from /root/gitrobot/"
else
  log_error "Git repository /root/gitrobot does not exist"
fi

# Copy Git credentials (PAT) to backup
mkdir -p "${BACKUP_DIR}/root"
if [ -f /root/.git-credentials ]; then
  cp -v /root/.git-credentials "${BACKUP_DIR}/root/" 2>>"${ERROR_LOG}" || log_error "Failed to copy Git credentials /root/.git-credentials"
else
  log_error "Git credentials /root/.git-credentials does not exist"
fi

# Perform Git operations (commit and push)
if [ -d /root/gitrobot/.git ]; then
  cd /root/gitrobot
  git add . 2>>"${ERROR_LOG}" || log_error "Failed to stage Git changes"
  git commit -m "Automated backup commit: ${TIMESTAMP}" 2>>"${ERROR_LOG}" || log_error "Failed to commit Git changes"
  if git push origin main 2>>"${ERROR_LOG}"; then
    echo "Git push successful to https://github.com/mrjoesnow/robot"
  else
    log_error "Failed to push to Git repository"
  fi
  cd - >/dev/null
else
  log_error "Git repository not initialized at /root/gitrobot"
fi

# Update manifest with Git details
cat << EOF > "${BACKUP_DIR}/manifest.txt"
Janus WebRTC Streaming Project Backup
Date: ${TIMESTAMP}
Project: RTMP to WebRTC streaming via Janus
Stream URL: https://pongrobot.com/stream.html
RTMP Input: rtmp://127.0.0.1:1935/live/23672E6t
Janus Ports: Audio (5004, Opus), Video (5006, H.264)
Git Repository: https://github.com/mrjoesnow/robot

Files Backed Up:
- FFmpeg Service: /etc/systemd/system/ffmpeg-stream.service
- Janus: All *.jcfg files in /opt/janus/etc/janus/
- Nginx: /etc/nginx/sites-enabled/default, /etc/nginx/sites-available/default, /etc/nginx/nginx.conf
- Web: Files in /var/www/html/ excluding hls folder (e.g., stream.html, index.html, js/stream.js, js/janus.js, js/adapter-latest.js, api/stream-key.json)
- Backend: Files in /var/www/pongrobot-backend/ excluding node_modules (e.g., index.js, package.json, pongrobot.db)
- Scripts: Selected files in /usr/local/bin/ (e.g., backup_streaming_config.sh, install_pongrobot.sh, restore_pongrobot.sh, purge-large-logs.sh)
- Git Repository: /root/gitrobot/ excluding .git folder
- Git Credentials: /root/.git-credentials (contains PAT)

Notes:
- Ensure OBS is streaming to rtmp://127.0.0.1:1935/live with key 23672E6t.
- Janus configuration uses HTTPS on port 8089, base_path /janus.
- FFmpeg runs as a systemd service.
- Backend includes SQLite database at /var/www/pongrobot-backend/pongrobot.db.