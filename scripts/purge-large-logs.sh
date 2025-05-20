#!/bin/bash
find /var/log/nginx/janus_error.log* /var/log/nginx/error.log* /var/log/ffmpeg.log -type f -size +100M -mtime +7 -exec rm -f {} \;
