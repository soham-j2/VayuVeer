/*
  VayuVeer ESP32 – MQ-6 + Telegram + Firebase (Dynamic Threshold)
  ---------------------------------------------------------------
  ✅ Green LED = Safe
  🚨 Red LED + Buzzer + Telegram + Firebase = Leak Detected
  🔧 Threshold auto-updates from Firebase every 10 seconds

  Soham Jadhav | VayuVeer Project | 2025
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ===== WiFi Credentials =====
const char* ssid     = "Dynamic";
const char* password = "*******#";

// ===== Telegram Setup =====
const String BOT_TOKEN = "8458203708:AAE2KFt5JBSMRsf-yvL2Dovp5bTkAA22RmU";
const String CHAT_ID   = "5444833909";

// ===== Firebase Setup =====
const char* FIREBASE_HOST = "vayuveerxe-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH = "VVGPvIQGW91dHdACR7qsZoYO8yLX766uUfrd0mqg";

// ===== Hardware Pins =====
const int MQ_PIN   = 34;   // MQ-6 AOUT
const int RED_LED  = 14;   // Alert LED
const int GREEN_LED= 13;   // Safe LED
const int BUZZER   = 27;   // Buzzer pin

// ===== Calibration =====
const int SAMPLES = 8;
int samples[SAMPLES];
int sampleIdx = 0;

int baseline = 0;
int alarmThreshold = 0;
int thresholdOffset = 120;
bool alarmState = false;

// ===== Timing =====
unsigned long lastFirebaseUpdate = 0;
const unsigned long FIREBASE_UPDATE_INTERVAL = 3000;
unsigned long lastThresholdCheck = 0;

// ===== URL Encoder =====
String urlEncode(const String &str) {
  String encoded = "";
  for (size_t i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (isalnum(c)) encoded += c;
    else if (c == ' ') encoded += "%20";
    else {
      char buf[5];
      sprintf(buf, "%%%02X", (uint8_t)c);
      encoded += buf;
    }
  }
  return encoded;
}

// ===== Telegram Send Function =====
void sendTelegram(const String &message) {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + BOT_TOKEN + "/sendMessage";
  String postData = "chat_id=" + CHAT_ID + "&text=" + urlEncode(message);
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int code = http.POST(postData);
  Serial.printf("Telegram Response: %d\n", code);
  http.end();
}

// ===== Firebase Update Function =====
void updateFirebase(float ppm, const String &status, unsigned long timestamp) {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String url = String("https://") + FIREBASE_HOST + "/vayuveer/latest.json?auth=" + FIREBASE_AUTH;
  String body = "{\"ppm\":" + String(ppm, 2) + 
                ",\"status\":\"" + status + "\"" +
                ",\"timestamp\":" + String(timestamp) + "}";

  https.begin(client, url);
  https.addHeader("Content-Type", "application/json");
  int code = https.PUT(body);
  if (code > 0) Serial.printf("Firebase Response: %d\n", code);
  else Serial.println("Firebase update failed");
  https.end();
}

// ===== Fetch Threshold From Firebase =====
void fetchThresholdFromFirebase() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;

  String url = String("https://") + FIREBASE_HOST + "/vayuveer/config/threshold.json?auth=" + FIREBASE_AUTH;
  https.begin(client, url);
  int code = https.GET();

  if (code == 200) {
    String payload = https.getString();
    int newThreshold = payload.toInt();
    if (newThreshold > 0 && newThreshold != alarmThreshold) {
      alarmThreshold = newThreshold;
      Serial.printf("✅ Threshold updated from Firebase: %d\n", alarmThreshold);
    }
  } else {
    Serial.printf("⚠️ Failed to fetch threshold (%d)\n", code);
  }

  https.end();
}

// ===== Read Smoothed ADC =====
int readSmoothedADC() {
  samples[sampleIdx] = analogRead(MQ_PIN);
  sampleIdx = (sampleIdx + 1) % SAMPLES;
  long s = 0;
  for (int i = 0; i < SAMPLES; i++) s += samples[i];
  return (int)(s / SAMPLES);
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BUZZER, LOW);

  Serial.print("Connecting to WiFi ");
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) Serial.println("\n✅ WiFi connected!");
  else Serial.println("\n❌ WiFi connection failed.");

  Serial.println("Heating MQ6 sensor for 15 seconds...");
  delay(15000);

  Serial.println("Calibrating sensor in clean air...");
  long sum = 0;
  const int CAL_SAMPLES = 50;
  for (int i = 0; i < CAL_SAMPLES; i++) {
    sum += analogRead(MQ_PIN);
    delay(50);
  }
  baseline = sum / CAL_SAMPLES;
  alarmThreshold = baseline + thresholdOffset;
  Serial.printf("Baseline=%d, Threshold=%d\n", baseline, alarmThreshold);

  for (int i = 0; i < SAMPLES; i++) samples[i] = baseline;

  digitalWrite(GREEN_LED, HIGH);
  alarmState = false;

  // Initial Firebase setup
  updateFirebase(0.0, "Starting", millis());
  fetchThresholdFromFirebase();
}

// ===== Loop =====
void loop() {
  if (WiFi.status() != WL_CONNECTED) WiFi.reconnect();

  // Fetch threshold every 10s
  if (millis() - lastThresholdCheck > 10000) {
    fetchThresholdFromFirebase();
    lastThresholdCheck = millis();
  }

  int adc = readSmoothedADC();
  float voltage = (adc / 4095.0) * 3.3;
  float pseudoPPM = voltage * 1000.0;
  unsigned long now = millis();

  Serial.printf("ADC=%d | V=%.2fV | PPM=%.1f | Base=%d | Threshold=%d\n",
                adc, voltage, pseudoPPM, baseline, alarmThreshold);

  // Gas logic
  if (adc >= alarmThreshold) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER, HIGH);

    if (!alarmState) {
      sendTelegram("🚨 *VayuVeer ALERT!* LPG detected.\nPPM: " + String(pseudoPPM, 1));
      updateFirebase(pseudoPPM, "LEAK DETECTED", now);
      alarmState = true;
      lastFirebaseUpdate = now;
    } else if (now - lastFirebaseUpdate >= FIREBASE_UPDATE_INTERVAL) {
      updateFirebase(pseudoPPM, "LEAK ONGOING", now);
      lastFirebaseUpdate = now;
    }
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER, LOW);

    if (alarmState) {
      sendTelegram("✅ *VayuVeer SAFE:* LPG levels normal again.");
      updateFirebase(pseudoPPM, "SAFE", now);
      alarmState = false;
      lastFirebaseUpdate = now;
    } else if (now - lastFirebaseUpdate >= FIREBASE_UPDATE_INTERVAL) {
      updateFirebase(pseudoPPM, "SAFE", now);
      lastFirebaseUpdate = now;
    }
  }

  delay(400);
}
