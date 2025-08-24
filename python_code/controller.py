import cv2
import requests
from threading import Thread
from ultralytics import YOLO
import time

# Load YOLOv8 Nano (light & fast)
model = YOLO("yolov8n.pt")

# Camera feed (phone/ESP32-CAM stream)
ip_camera_url = "http://192.168.29.90:4747/video"

# ESP32 IP (from Serial Monitor)
ESP32_IP = "192.168.29.7"

# Threaded Video Capture
class VideoStream:
    def __init__(self, src):
        self.cap = cv2.VideoCapture(src)
        self.cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)  # drop old frames
        self.ret, self.frame = self.cap.read()
        self.running = True
        Thread(target=self.update, daemon=True).start()

    def update(self):
        while self.running:
            if self.cap.isOpened():
                self.ret, self.frame = self.cap.read()

    def read(self):
        return self.ret, self.frame

    def stop(self):
        self.running = False
        self.cap.release()

# Start camera stream
stream = VideoStream(ip_camera_url)

last_state = None   # "stop" or "go"
last_sent_time = 0  # rate limit ESP32 requests

while True:
    ret, frame = stream.read()
    if not ret:
        continue

    # Resize for faster YOLO
    frame = cv2.resize(frame, (320, 240))

    # Run YOLO (disable verbose for speed)
    results = model(frame, imgsz=320, verbose=False)

    danger = False
    for r in results:
        for box in r.boxes:
            x1, y1, x2, y2 = map(int, box.xyxy[0])  
            conf = float(box.conf[0])               
            cls = int(box.cls[0])                   
            w, h = x2 - x1, y2 - y1
            cx, cy = (x1 + x2) // 2, (y1 + y2) // 2

            # Example condition: object in center & close
            if conf > 0.5 and (100 < cx < 220) and h > 120:
                danger = True
                cv2.putText(frame, "DANGER - STOP", (50, 50),
                            cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)

    # Send ESP32 command only when state changes (to avoid spam)
    new_state = "stop" if danger else "go"
    if new_state != last_state and time.time() - last_sent_time > 0.5:
        try:
            requests.get(f"http://{ESP32_IP}/{new_state}", timeout=0.2)
            print(f"➡️ Sent command: {new_state}")
            last_state = new_state
            last_sent_time = time.time()
        except:
            print("⚠️ Could not connect to ESP32")

    # Show detection
    annotated = results[0].plot()
    cv2.imshow("Detection + Commands", annotated)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

stream.stop()
cv2.destroyAllWindows()


