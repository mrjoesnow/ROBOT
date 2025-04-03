from collections import deque
from imutils.video import VideoStream
import numpy as np
import argparse
import cv2
import imutils
import time
import subprocess

ap = argparse.ArgumentParser()
ap.add_argument("-i", "--ip", required=True, help="IP address of the camera")
ap.add_argument("-p", "--port", required=True, help="Port of the camera")
ap.add_argument("-b", "--buffer", type=int, default=64, help="max buffer size")
args = vars(ap.parse_args())

whiteLower = (0, 0, 200)
whiteUpper = (180, 25, 255)
pts = deque(maxlen=args["buffer"])

url = f"http://{args['ip']}:{args['port']}/video"
print(f"Attempting to connect to: {url}")

ffmpeg_path = r"C:\temp\ffmpeg\bin\ffmpeg.exe"
command = [
    ffmpeg_path,
    '-i', url,
    '-f', 'image2pipe',
    '-pix_fmt', 'bgr24',
    '-vcodec', 'rawvideo',
    '-'
]

process = subprocess.Popen(command, stdout=subprocess.PIPE, bufsize=10**8)

time.sleep(2.0)

while True:
    raw_frame = process.stdout.read(640 * 480 * 3)

    if not raw_frame:
        print("Failed to grab frame")
        break

    frame = np.frombuffer(raw_frame, np.uint8).reshape((480, 640, 3))

    frame = imutils.resize(frame, width=600)
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)

    mask = cv2.inRange(hsv, whiteLower, whiteUpper)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)

    cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = imutils.grab_contours(cnts)
    center = None

    if len(cnts) > 0:
        c = max(cnts, key=cv2.contourArea)
        ((x, y), radius) = cv2.minEnclosingCircle(c)
        M = cv2.moments(c)
        center = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))

        if radius > 10:
            cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 255), 2)
            cv2.circle(frame, center, 5, (0, 0, 255), -1)

            pts.appendleft(center)

    for i in range(1, len(pts)):
        if pts[i - 1] is None or pts[i] is None:
            continue

        thickness = int(np.sqrt(args["buffer"] / float(i + 1)) * 2.5)
        cv2.line(frame, pts[i - 1], pts[i], (0, 0, 255), thickness)

    cv2.imshow("Frame", frame)
    key = cv2.waitKey(1) & 0xFF

    if key == ord("q"):
        break

process.terminate()

cv2.destroyAllWindows()

