/**
 * =============================================================================
 *  IoT Sleep Apnea Detection System — Arduino Nano (Fall Detection Node)
 * =============================================================================
 *  Project  : Intelligent IoT Solution for Sleep Apnea
 *  College  : V.S.B. Engineering College, Karur - 639111
 *  Team     : Naveen Kumar N, Saktibalan M, Baranidharan S, Manoj P
 * =============================================================================
 *
 *  Description:
 *    This sketch runs on the Arduino Nano and handles fall detection using
 *    an MPU6050 accelerometer/gyroscope module. When a fall event is detected,
 *    it sends a "FALL DETECTED" string to the ESP32 over UART so the ESP32
 *    can include it in the web dashboard and trigger alerts.
 *
 *  Hardware Connections:
 *    MPU6050 VCC → 3.3V / 5V
 *    MPU6050 GND → GND
 *    MPU6050 SDA → A4  (Nano I2C SDA)
 *    MPU6050 SCL → A5  (Nano I2C SCL)
 *    Nano TX (1) → ESP32 GPIO 16 (RXD2)  [cross-connect TX→RX]
 *    Nano RX (0) → ESP32 GPIO 17 (TXD2)  [cross-connect RX→TX]
 *    (Ensure common GND between Nano and ESP32)
 *
 *  Libraries Required:
 *    - Wire.h       (built-in)
 *    - MPU6050.h    (by Electronic Cats or jrowberg/I2Cdevlib)
 * =============================================================================
 */

#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// ─────────────────────────────────────────────────────────────
//  FALL DETECTION PARAMETERS
// ─────────────────────────────────────────────────────────────
// The resultant acceleration magnitude is computed as:
//   accel_magnitude = sqrt(ax^2 + ay^2 + az^2)
// A value significantly higher than 1g (16384 LSB for ±2g range)
// followed by near-zero magnitude indicates a free-fall/fall event.

const float FALL_THRESHOLD_HIGH = 2.5;  // g — sudden spike (impact)
const float FALL_THRESHOLD_LOW  = 0.3;  // g — free-fall window
const int   SAMPLE_INTERVAL_MS  = 100;  // ms between readings

unsigned long lastSample    = 0;
unsigned long lastSentMsg   = 0;
bool          fallDetected  = false;

// ─────────────────────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);      // UART to ESP32
  Wire.begin();

  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1);  // Halt
  }
  Serial.println("MPU6050 ready.");
}

// ─────────────────────────────────────────────────────────────
//  LOOP
// ─────────────────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  if (now - lastSample < SAMPLE_INTERVAL_MS) return;
  lastSample = now;

  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Convert raw values to g (sensitivity = 16384 LSB/g for ±2g range)
  float axG = ax / 16384.0;
  float ayG = ay / 16384.0;
  float azG = az / 16384.0;

  float magnitude = sqrt(axG * axG + ayG * ayG + azG * azG);

  // Detect fall: sudden high-g impact
  if (magnitude > FALL_THRESHOLD_HIGH) {
    fallDetected = true;
  }

  // After impact detection, check if magnitude drops (free-fall / collapse)
  if (fallDetected && magnitude < FALL_THRESHOLD_LOW) {
    Serial.println("FALL DETECTED");  // Sent to ESP32
    fallDetected = false;             // Reset flag
  }

  // Send "Normal" status every 2 seconds when no fall
  if (!fallDetected && (now - lastSentMsg > 2000)) {
    Serial.println("Normal");
    lastSentMsg = now;
  }
}
