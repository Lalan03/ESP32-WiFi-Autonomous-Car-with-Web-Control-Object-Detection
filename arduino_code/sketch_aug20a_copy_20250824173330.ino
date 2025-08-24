
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Lalan-4G";
const char* password = "Lalan@00";

WebServer server(80);

// Motor pins (L298N)
int ENA = 14;
int IN1 = 27;
int IN2 = 26;
int ENB = 25;
int IN3 = 33;
int IN4 = 32;

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  digitalWrite(ENA, HIGH);  // Enable both motor channels
  digitalWrite(ENB, HIGH);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");
  Serial.print("🌐 ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // Routes
  server.on("/go", [](){ forward(); server.send(200, "text/plain", "➡️ Moving Forward"); });
  server.on("/stop", [](){ stopMotors(); server.send(200, "text/plain", "🛑 Stopped"); });
  server.on("/backward", [](){ backward(); server.send(200, "text/plain", "⬇️ Backward"); });
  server.on("/turn_left", [](){ blockLeft(); server.send(200, "text/plain", "⬅️ Turn Left"); });
  server.on("/turn_right", [](){ blockRight(); server.send(200, "text/plain", "➡️ Turn Right"); });

  server.begin();
}

void loop() {
  server.handleClient();
}

// === Motor Functions ===
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// Block left wheels (Right side moves → car turns left)
void blockLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);   // stop left side
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);  // right side forward
}

// Block right wheels (Left side moves → car turns right)
void blockRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);   // left side forward
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);   // stop right side
}
