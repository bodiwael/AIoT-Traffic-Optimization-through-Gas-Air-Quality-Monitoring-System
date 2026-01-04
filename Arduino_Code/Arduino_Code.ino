/*
 * Dual Sensor Traffic Control with Firebase Integration
 * ESP32 + MQ-2 + MQ-135 + Firebase Realtime Database
 * 
 * Data is sent to Firebase under "traffic2" directory
 * NO HISTORY - Only current readings
 * MQ-2 controls traffic lights, MQ-135 for air quality display
 */

#include <Wire.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// WiFi Credentials
const char* WIFI_SSID = "ITIDA";
const char* WIFI_PASSWORD = "12345678";

// Firebase Credentials
const char* FIREBASE_HOST = "stem-53cdc-default-rtdb.firebaseio.com";
const char* FIREBASE_AUTH = "UlqdAaYSCRjTcqFBRVW0df1Y513SLgoJ2vuZ2lZO";

// Firebase objects
FirebaseData fbData;
FirebaseAuth auth;
FirebaseConfig config;

// Pin Definitions
#define MQ2_PIN 34          // MQ-2 sensor (traffic control)
#define MQ135_PIN 35        // MQ-135 sensor (display)
#define RED_LED 25          // Red traffic light
#define YELLOW_LED 26       // Yellow traffic light
#define GREEN_LED 27        // Green traffic light

// Thresholds
#define YELLOW_THRESHOLD 20
#define GREEN_THRESHOLD 50

// Variables
int mq2_baseline = 0;
int mq2_reading = 0;
int mq2_calibrated = 0;
int mq135_reading = 0;
String trafficLight = "";
String gasLevel = "";

// Timing
unsigned long sendDataPrevMillis = 0;
bool firebaseReady = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║  DUAL SENSOR + FIREBASE SYSTEM         ║");
  Serial.println("║  Data Path: traffic2/                  ║");
  Serial.println("║  Mode: Current readings only           ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Configure pins
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(MQ2_PIN, INPUT);
  pinMode(MQ135_PIN, INPUT);
  
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  
  // Test LEDs
  testTrafficLights();
  
  // Calibrate MQ-2
  calibrateMQ2();
  
  // Connect to WiFi
  connectToWiFi();
  
  // Configure Firebase
  Serial.println("\n→ Configuring Firebase...");
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  
  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  // Set timeout
  fbData.setBSSLBufferSize(1024, 1024);
  fbData.setResponseSize(1024);
  
  Serial.println("→ Connecting to Firebase...");
  delay(2000);
  
  if (Firebase.ready()) {
    Serial.println("✓ Firebase Connected!");
    firebaseReady = true;
    initializeFirebase();
    
    // Delete any existing history data
    Serial.println("→ Cleaning up old history data...");
    if (Firebase.deleteNode(fbData, "/traffic2/history")) {
      Serial.println("✓ History removed from Firebase");
    }
  } else {
    Serial.println("✗ Firebase Connection Failed!");
    Serial.println("System will continue without Firebase");
  }
  
  Serial.println("\n✓ System Ready!");
  Serial.println("Monitoring sensors...\n");
}

void loop() {
  // Read sensors
  mq2_reading = analogRead(MQ2_PIN);
  mq135_reading = analogRead(MQ135_PIN);
  
  // Calibrate MQ-2
  mq2_calibrated = mq2_reading - mq2_baseline;
  if(mq2_calibrated < 0) mq2_calibrated = 0;
  
  // Control traffic lights based on MQ-2
  if(mq2_calibrated < YELLOW_THRESHOLD) {
    trafficLight = "RED";
    gasLevel = "LOW";
    setTrafficLight("RED");
  } else if(mq2_calibrated < GREEN_THRESHOLD) {
    trafficLight = "YELLOW";
    gasLevel = "MODERATE";
    setTrafficLight("YELLOW");
  } else {
    trafficLight = "GREEN";
    gasLevel = "HIGH";
    setTrafficLight("GREEN");
  }
  
  // Display locally
  displayReadings();
  
  // Send to Firebase every 5 seconds
  if (firebaseReady && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    sendDataToFirebase();
  }
  
  delay(2000);
}

void initializeFirebase() {
  Serial.println("\n→ Initializing Firebase structure at traffic2/...");
  
  // Set device information under traffic2/
  if (Firebase.setString(fbData, "/traffic2/device/name", "ESP32_Traffic_Monitor")) {
    Serial.println("✓ Device name set");
  } else {
    Serial.print("✗ Error: ");
    Serial.println(fbData.errorReason());
  }
  
  if (Firebase.setInt(fbData, "/traffic2/device/baseline", mq2_baseline)) {
    Serial.println("✓ Baseline saved");
  }
  
  if (Firebase.setInt(fbData, "/traffic2/thresholds/yellow", YELLOW_THRESHOLD)) {
    Serial.println("✓ Yellow threshold saved");
  }
  
  if (Firebase.setInt(fbData, "/traffic2/thresholds/green", GREEN_THRESHOLD)) {
    Serial.println("✓ Green threshold saved");
  }
  
  Serial.println("✓ Firebase initialized at traffic2/!");
}

void sendDataToFirebase() {
  // Update ONLY current readings under traffic2/current/
  // NO HISTORY TRACKING
  
  Firebase.setInt(fbData, "/traffic2/current/mq2_raw", mq2_reading);
  Firebase.setInt(fbData, "/traffic2/current/mq2_calibrated", mq2_calibrated);
  Firebase.setInt(fbData, "/traffic2/current/mq135", mq135_reading);
  Firebase.setString(fbData, "/traffic2/current/traffic_light", trafficLight);
  Firebase.setString(fbData, "/traffic2/current/gas_level", gasLevel);
  Firebase.setInt(fbData, "/traffic2/current/timestamp", millis() / 1000);
  
  Serial.println("✓ Firebase updated (current data only)");
}

void calibrateMQ2() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║       MQ-2 CALIBRATION PROCESS         ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println("Warming up (30 seconds)...");
  Serial.println("Keep in CLEAN AIR!\n");
  
  for(int i = 30; i > 0; i--) {
    Serial.print("→ ");
    Serial.print(i);
    Serial.println(" seconds");
    delay(1000);
  }
  
  Serial.println("\nTaking samples...");
  long sum = 0;
  for(int i = 0; i < 10; i++) {
    int reading = analogRead(MQ2_PIN);
    sum += reading;
    Serial.print("  Sample ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(reading);
    delay(500);
  }
  
  mq2_baseline = sum / 10;
  
  Serial.println("\n✓ CALIBRATION COMPLETE!");
  Serial.print("Baseline: ");
  Serial.println(mq2_baseline);
  Serial.println("════════════════════════════════════════\n");
}

void connectToWiFi() {
  Serial.print("\n→ Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n✗ WiFi Failed!");
    Serial.println("Check SSID and password");
  }
}

void setTrafficLight(String color) {
  if(color == "RED") {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
  } else if(color == "YELLOW") {
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  } else if(color == "GREEN") {
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }
}

void displayReadings() {
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║       SENSOR READINGS                  ║");
  Serial.println("╠════════════════════════════════════════╣");
  
  Serial.print("║ MQ-2 Raw:        ");
  Serial.print(mq2_reading);
  printSpaces(mq2_reading, 22);
  Serial.println("║");
  
  Serial.print("║ MQ-2 Baseline:   ");
  Serial.print(mq2_baseline);
  printSpaces(mq2_baseline, 22);
  Serial.println("║");
  
  Serial.print("║ MQ-2 Calibrated: ");
  Serial.print(mq2_calibrated);
  printSpaces(mq2_calibrated, 22);
  Serial.println("║");
  
  Serial.print("║ MQ-135:          ");
  Serial.print(mq135_reading);
  printSpaces(mq135_reading, 22);
  Serial.println("║");
  
  Serial.println("╠════════════════════════════════════════╣");
  
  Serial.print("║ Gas Level:       ");
  Serial.print(gasLevel);
  for(int i = gasLevel.length(); i < 22; i++) Serial.print(" ");
  Serial.println("║");
  
  Serial.print("║ Traffic Light:   ");
  if(trafficLight == "RED") {
    Serial.print("🔴 RED");
    Serial.print("              ");
  } else if(trafficLight == "YELLOW") {
    Serial.print("🟡 YELLOW");
    Serial.print("         ");
  } else if(trafficLight == "GREEN") {
    Serial.print("🟢 GREEN");
    Serial.print("          ");
  }
  Serial.println("║");
  
  Serial.println("╚════════════════════════════════════════╝\n");
}

void printSpaces(int value, int totalWidth) {
  String valStr = String(value);
  for(int i = valStr.length(); i < totalWidth; i++) {
    Serial.print(" ");
  }
}

void testTrafficLights() {
  Serial.println("→ Testing Traffic Lights...");
  
  Serial.println("  RED LED...");
  digitalWrite(RED_LED, HIGH);
  delay(800);
  digitalWrite(RED_LED, LOW);
  delay(300);
  
  Serial.println("  YELLOW LED...");
  digitalWrite(YELLOW_LED, HIGH);
  delay(800);
  digitalWrite(YELLOW_LED, LOW);
  delay(300);
  
  Serial.println("  GREEN LED...");
  digitalWrite(GREEN_LED, HIGH);
  delay(800);
  digitalWrite(GREEN_LED, LOW);
  delay(300);
  
  Serial.println("✓ All LEDs tested!\n");
}
