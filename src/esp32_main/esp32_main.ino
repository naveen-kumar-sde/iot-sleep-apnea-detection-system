/**
 * =============================================================================
 *  IoT Sleep Apnea Detection System — ESP32 Main Controller
 * =============================================================================
 *  Project  : Intelligent IoT Solution for Sleep Apnea
 *  College  : V.S.B. Engineering College, Karur - 639111
 *  Dept     : Electronics and Communication Engineering
 *  Team     : Naveen Kumar N  (922523106133)
 *             Saktibalan M    (922523106178)
 *             Baranidharan S  (922523106301)
 *             Manoj P         (922523106302)
 *  Supervisor: Dr. P. Surendar, M.E., Ph.D.
 * =============================================================================
 *
 *  Description:
 *    This sketch runs on the ESP32 and acts as the central controller.
 *    It:
 *      - Reads temperature & humidity from DHT11
 *      - Reads sound (snore/breathing) level via an analog sound sensor
 *      - Reads heart rate (BPM) and SpO2 from MAX30100 pulse oximeter
 *      - Receives fall-detection status from Arduino Nano over UART
 *      - Displays live data on a 16x2 LCD
 *      - Activates CPAP relay + buzzer alert when apnea is detected
 *      - Hosts a Wi-Fi Access Point with a real-time web dashboard
 *
 *  Hardware Connections:
 *    DHT11 Data      → GPIO 4
 *    Sound Sensor    → GPIO 34 (ADC)
 *    RELAY (CPAP)    → GPIO 23
 *    BUZZER          → GPIO 5
 *    LCD (RS)        → GPIO 14
 *    LCD (EN)        → GPIO 27
 *    LCD (D4)        → GPIO 26
 *    LCD (D5)        → GPIO 25
 *    LCD (D6)        → GPIO 33
 *    LCD (D7)        → GPIO 32
 *    UART2 RX        → GPIO 16  (from Arduino Nano TX)
 *    UART2 TX        → GPIO 17  (to   Arduino Nano RX)
 *    MAX30100 SDA    → GPIO 21
 *    MAX30100 SCL    → GPIO 22
 *
 *  Libraries Required:
 *    - WiFi.h          (built-in ESP32)
 *    - Wire.h          (built-in)
 *    - WebServer.h     (built-in ESP32)
 *    - DHT.h           (DHT sensor library by Adafruit)
 *    - LiquidCrystal.h (built-in)
 *    - MAX30100_PulseOximeter.h (by OXullo Intersecans)
 * =============================================================================
 */

#include <WiFi.h>
#include <Wire.h>
#include <WebServer.h>
#include <DHT.h>
#include <LiquidCrystal.h>
#include <MAX30100_PulseOximeter.h>

// ─────────────────────────────────────────────────────────────
//  PIN DEFINITIONS
// ─────────────────────────────────────────────────────────────
#define DHTPIN       4     // DHT11 data pin
#define DHTTYPE      DHT11
#define SOUND_PIN    34    // Analog sound sensor pin (ADC)
#define RELAY_PIN    23    // CPAP relay control pin
#define BUZZER_PIN   5     // Buzzer pin

// LCD Pins (16x2 HD44780)
#define RS  14
#define EN  27
#define D4  26
#define D5  25
#define D6  33
#define D7  32

// UART2 Pins for Arduino Nano communication
#define RXD2  16
#define TXD2  17

// ─────────────────────────────────────────────────────────────
//  OBJECTS
// ─────────────────────────────────────────────────────────────
LiquidCrystal  lcd(RS, EN, D4, D5, D6, D7);
DHT            dht(DHTPIN, DHTTYPE);
PulseOximeter  pox;
WebServer      server(80);
HardwareSerial NanoSerial(1);   // UART2 for Nano communication

// ─────────────────────────────────────────────────────────────
//  Wi-Fi ACCESS POINT CREDENTIALS
// ─────────────────────────────────────────────────────────────
const char* ssid     = "SleepApneaMonitor";  // AP SSID
const char* password = "sleep1234";           // AP Password (min 8 chars)

// ─────────────────────────────────────────────────────────────
//  GLOBAL VARIABLES
// ─────────────────────────────────────────────────────────────
float  temperature  = 0.0;
float  humidity     = 0.0;
int    soundValue   = 0;
float  BPM          = 0.0;
float  SpO2         = 0.0;
int    apneaThreshold = 2800;  // ADC threshold for apnea detection
String fallStatus   = "Normal";
bool   airflowOn    = false;

// Timing variables (millis-based non-blocking delays)
unsigned long lastSensorRead = 0;
unsigned long lastLCDUpdate  = 0;
unsigned long lastAlert      = 0;

// ─────────────────────────────────────────────────────────────
//  CALLBACK: Beat detected from MAX30100
// ─────────────────────────────────────────────────────────────
void onBeatDetected() {
  Serial.println("[MAX30100] Heartbeat detected!");
}

// ─────────────────────────────────────────────────────────────
//  WEB SERVER: Root handler — returns HTML dashboard
// ─────────────────────────────────────────────────────────────
void handleRoot() {
  String html = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta http-equiv="refresh" content="5">
  <title>Sleep Apnea Monitor</title>
  <style>
    body { font-family: Arial, sans-serif; background:#1a1a2e; color:#eee; margin:0; padding:20px; }
    h1   { color:#00d4ff; text-align:center; }
    .grid{ display:grid; grid-template-columns:repeat(auto-fit,minmax(150px,1fr)); gap:16px; margin-top:20px; }
    .card{ background:#16213e; border-radius:12px; padding:20px; text-align:center; border:1px solid #0f3460; }
    .card .label{ font-size:13px; color:#aaa; }
    .card .value{ font-size:28px; font-weight:bold; color:#00d4ff; margin-top:8px; }
    .alert { background:#ff4444; border-radius:8px; padding:12px; text-align:center; font-weight:bold; display:none; }
  </style>
</head>
<body>
  <h1>&#128164; IoT Sleep Apnea Monitor</h1>
  <div class="grid">
    <div class="card"><div class="label">Temperature</div><div class="value">)rawhtml";
  html += String(temperature, 1) + R"rawhtml( &deg;C</div></div>
    <div class="card"><div class="label">Humidity</div><div class="value">)rawhtml";
  html += String(humidity, 1) + R"rawhtml( %</div></div>
    <div class="card"><div class="label">Sound Level</div><div class="value">)rawhtml";
  html += String(soundValue) + R"rawhtml(</div></div>
    <div class="card"><div class="label">Heart Rate</div><div class="value">)rawhtml";
  html += String(BPM, 1) + R"rawhtml( BPM</div></div>
    <div class="card"><div class="label">SpO&sup2;</div><div class="value">)rawhtml";
  html += String(SpO2, 1) + R"rawhtml( %</div></div>
    <div class="card"><div class="label">Fall Status</div><div class="value">)rawhtml";
  html += fallStatus + R"rawhtml(</div></div>
    <div class="card"><div class="label">CPAP Airflow</div><div class="value">)rawhtml";
  html += (airflowOn ? "ON" : "OFF") + R"rawhtml(</div></div>
  </div>
  <p style="text-align:center;color:#555;margin-top:30px;font-size:12px;">Auto-refreshes every 5 seconds</p>
</body>
</html>)rawhtml";

  server.send(200, "text/html", html);
}

// ─────────────────────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  NanoSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Wire.begin();
  dht.begin();

  // LCD initialisation
  lcd.begin(16, 2);
  lcd.print("Sleep Apnea IoT");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  // GPIO setup
  pinMode(SOUND_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // MAX30100 pulse oximeter
  if (!pox.begin()) {
    Serial.println("[ERROR] MAX30100 not found! Check wiring.");
  } else {
    Serial.println("[OK] MAX30100 initialized.");
    pox.setOnBeatDetectedCallback(onBeatDetected);
  }

  // Wi-Fi Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(
    IPAddress(192, 168, 4, 1),
    IPAddress(192, 168, 4, 1),
    IPAddress(255, 255, 255, 0)
  );
  WiFi.softAP(ssid, password);
  delay(1000);

  Serial.println("\n[WiFi] Access Point started.");
  Serial.print("[WiFi] SSID : "); Serial.println(ssid);
  Serial.print("[WiFi] IP   : "); Serial.println(WiFi.softAPIP());

  // Web server routes
  server.on("/", handleRoot);
  server.begin();
  Serial.println("[Server] HTTP server started.");

  delay(1500);
  lcd.clear();
}

// ─────────────────────────────────────────────────────────────
//  LOOP
// ─────────────────────────────────────────────────────────────
void loop() {
  server.handleClient();
  pox.update();

  unsigned long now = millis();

  // ── Sensor reading every 1 second ──────────────────────────
  if (now - lastSensorRead > 1000) {
    lastSensorRead = now;

    temperature = dht.readTemperature();
    humidity    = dht.readHumidity();
    soundValue  = analogRead(SOUND_PIN);
    BPM         = pox.getHeartRate();
    SpO2        = pox.getSpO2();

    // Read fall-detection data sent by Arduino Nano
    if (NanoSerial.available()) {
      fallStatus = NanoSerial.readStringUntil('\n');
      fallStatus.trim();
      if (fallStatus == "") fallStatus = "Normal";
    }

    // ── Apnea Detection Logic ───────────────────────────────
    //  If sound level drops below threshold (i.e., no/low breathing
    //  sounds detected) for more than 6 seconds → trigger alert
    if (soundValue < apneaThreshold) {
      if (now - lastAlert > 6000) {
        digitalWrite(BUZZER_PIN, HIGH);   // Audible alert
        digitalWrite(RELAY_PIN, HIGH);    // Turn on CPAP airflow
        airflowOn = true;
        lastAlert = now;
        Serial.println("[ALERT] Apnea detected! CPAP activated.");
      }
    } else {
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(RELAY_PIN, LOW);
      airflowOn = false;
    }

    // Serial monitor log
    Serial.printf("[Data] Temp=%.1f°C  Hum=%.1f%%  Sound=%d  BPM=%.1f  SpO2=%.1f%%  Fall=%s  CPAP=%s\n",
      temperature, humidity, soundValue, BPM, SpO2,
      fallStatus.c_str(), airflowOn ? "ON" : "OFF");
  }

  // ── LCD update every 2 seconds ──────────────────────────────
  if (now - lastLCDUpdate > 2000) {
    lastLCDUpdate = now;

    lcd.clear();

    // Line 0: Temperature and Humidity
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print(" H:");
    lcd.print(humidity, 0);
    lcd.print("%");

    // Line 1: Heart Rate and SpO2
    lcd.setCursor(0, 1);
    lcd.print("HR:");
    lcd.print(BPM, 0);
    lcd.print(" Sp:");
    lcd.print(SpO2, 0);
    lcd.print("%");
  }
}
