# 🚦 AIoT-Traffic-Optimization-through-Gas-Air-Quality-Monitoring-System

A real-time IoT traffic monitoring system using ESP32, gas sensors (MQ-2 & MQ-135), and Firebase integration with a Flutter mobile app for remote monitoring.

## 📋 Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Circuit Connections](#circuit-connections)
- [Firebase Setup](#firebase-setup)
- [Arduino Setup](#arduino-setup)
- [Flutter App Setup](#flutter-app-setup)
- [How It Works](#how-it-works)
- [Usage](#usage)
- [License](#license)

## 🎯 Overview

This project implements an intelligent traffic monitoring system that:
- Monitors air quality using MQ-2 and MQ-135 gas sensors
- Controls traffic lights based on gas levels
- Sends real-time data to Firebase Realtime Database
- Provides a Flutter mobile app for remote monitoring
- Supports dual sensor systems (traffic1 and traffic2)

## ✨ Features

- **Real-time Monitoring**: Continuous sensor readings every 2 seconds
- **Smart Traffic Control**: Automatic traffic light switching based on gas levels
- **Cloud Integration**: Firebase Realtime Database for data storage
- **Mobile App**: Cross-platform Flutter app for iOS and Android
- **Color-Coded Display**: Visual indicators for traffic light status and gas levels
- **Dual System Support**: Monitor multiple traffic points simultaneously
- **Auto Calibration**: MQ-2 sensor baseline calibration on startup

## 🔧 Hardware Requirements

### Components
- **ESP32 Development Board** (x1 or x2)
- **MQ-2 Gas Sensor** (x1 or x2) - For traffic control
- **MQ-135 Gas Sensor** (x1 or x2) - For air quality monitoring
- **LED Traffic Lights** (x3 per system):
  - Red LED
  - Yellow LED
  - Green LED
- **Resistors** (220Ω) - 3 per system for LEDs
- **Breadboard and Jumper Wires**
- **Power Supply** (5V/3.3V)

## 💻 Software Requirements

### Arduino IDE
- Arduino IDE 1.8.x or higher
- ESP32 Board Support Package
- Required Libraries:
  - `WiFi.h`
  - `FirebaseESP32.h` by Mobizt
  - `Wire.h`

### Flutter Development
- Flutter SDK 3.0 or higher
- Dart SDK 2.17 or higher
- Android Studio / VS Code
- Firebase CLI
- FlutterFire CLI

## 🔌 Circuit Connections

### ESP32 Pinout

```
ESP32 Pin    →    Component
═══════════════════════════════════════
GPIO 34      →    MQ-2 Analog Output
GPIO 35      →    MQ-135 Analog Output
GPIO 25      →    Red LED (+ 220Ω resistor)
GPIO 26      →    Yellow LED (+ 220Ω resistor)
GPIO 27      →    Green LED (+ 220Ω resistor)
GND          →    All sensor GND + LED cathodes
3.3V         →    MQ-2 VCC
3.3V         →    MQ-135 VCC
```

### Connection Diagram

```
                    ESP32
               ┌─────────────┐
               │             │
               │    GPIO 34  ├──────► MQ-2 (Analog Out)
               │    GPIO 35  ├──────► MQ-135 (Analog Out)
               │             │
               │    GPIO 25  ├──[220Ω]──► Red LED ──► GND
               │    GPIO 26  ├──[220Ω]──► Yellow LED ──► GND
               │    GPIO 27  ├──[220Ω]──► Green LED ──► GND
               │             │
               │    3.3V     ├──────► MQ-2 VCC
               │    3.3V     ├──────► MQ-135 VCC
               │    GND      ├──────► All GND connections
               │             │
               └─────────────┘
```

### MQ-2 Sensor Pinout
```
MQ-2
┌─────────┐
│  VCC    ├───► 3.3V
│  GND    ├───► GND
│  AOUT   ├───► GPIO 34
│  DOUT   │ (Not used)
└─────────┘
```

### MQ-135 Sensor Pinout
```
MQ-135
┌─────────┐
│  VCC    ├───► 3.3V
│  GND    ├───► GND
│  AOUT   ├───► GPIO 35
│  DOUT   │ (Not used)
└─────────┘
```

### Traffic Light Wiring
```
Red LED:     GPIO 25 ──[220Ω]──►|──► GND
Yellow LED:  GPIO 26 ──[220Ω]──►|──► GND
Green LED:   GPIO 27 ──[220Ω]──►|──► GND
```

## 🔥 Firebase Setup

### 1. Create Firebase Project
1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Click "Add Project"
3. Enter project name: `stem-53cdc` (or your preferred name)
4. Follow the setup wizard

### 2. Enable Realtime Database
1. In Firebase Console, select your project
2. Go to "Build" → "Realtime Database"
3. Click "Create Database"
4. Choose location (us-central1 recommended)
5. Start in **test mode** for development

### 3. Get Database Credentials
1. Go to Project Settings (gear icon)
2. Copy your database URL: `https://stem-53cdc-default-rtdb.firebaseio.com`
3. Go to "Project Settings" → "Service Accounts"
4. Find your database secret (legacy token)

### 4. Database Rules (Test Mode)
```json
{
  "rules": {
    ".read": true,
    ".write": true
  }
}
```

**⚠️ Important**: Change these rules for production!

### Database Structure
```
stem-53cdc-default-rtdb
│
├── traffic1/
│   ├── current/
│   │   ├── gas_level: "LOW"
│   │   ├── mq135: 0
│   │   ├── mq2_calibrated: 0
│   │   ├── mq2_raw: 1456
│   │   ├── timestamp: 140
│   │   └── traffic_light: "RED"
│   ├── device/
│   │   ├── name: "ESP32_Traffic_Monitor"
│   │   └── baseline: 1450
│   └── thresholds/
│       ├── yellow: 20
│       └── green: 50
│
└── traffic2/
    └── (same structure as traffic1)
```

## 🔨 Arduino Setup

### 1. Install Arduino IDE
Download from [arduino.cc](https://www.arduino.cc/en/software)

### 2. Add ESP32 Board Support
1. Open Arduino IDE
2. Go to `File` → `Preferences`
3. Add to "Additional Board Manager URLs":
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. Go to `Tools` → `Board` → `Boards Manager`
5. Search "ESP32" and install

### 3. Install Required Libraries
Go to `Sketch` → `Include Library` → `Manage Libraries`

Install:
- **Firebase ESP32 Client** by Mobizt

### 4. Configure the Code

Open the Arduino sketch and update:

```cpp
// WiFi Credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Firebase Credentials
const char* FIREBASE_HOST = "your-project-id.firebaseio.com";
const char* FIREBASE_AUTH = "YOUR_DATABASE_SECRET";
```

### 5. Upload Code
1. Connect ESP32 to computer via USB
2. Select board: `Tools` → `Board` → `ESP32 Dev Module`
3. Select correct port: `Tools` → `Port`
4. Click Upload button

### 6. Monitor Serial Output
1. Open Serial Monitor: `Tools` → `Serial Monitor`
2. Set baud rate to **115200**
3. Watch for calibration and connection status

## 📱 Flutter App Setup

### 1. Prerequisites
Install Flutter: [flutter.dev](https://flutter.dev/docs/get-started/install)

### 2. Install Firebase CLI
```bash
npm install -g firebase-tools
```

### 3. Login to Firebase
```bash
firebase login
```

### 4. Create Flutter Project
```bash
flutter create traffic_monitor_app
cd traffic_monitor_app
```

### 5. Add Dependencies
Edit `pubspec.yaml`:
```yaml
dependencies:
  flutter:
    sdk: flutter
  firebase_core: ^2.24.2
  firebase_database: ^10.4.0
```

Run:
```bash
flutter pub get
```

### 6. Configure Firebase
```bash
flutterfire configure --project=stem-53cdc
```

Select platforms (Android/iOS)

### 7. Copy App Code
Replace `lib/main.dart` with the provided Flutter code

### 8. Run the App
```bash
flutter run
```

## ⚙️ How It Works

### Traffic Light Logic

The system uses MQ-2 calibrated readings to control traffic lights:

| MQ-2 Calibrated Value | Traffic Light | Gas Level | Action |
|----------------------|---------------|-----------|--------|
| < 20 | 🔴 RED | LOW | Stop traffic |
| 20 - 50 | 🟡 YELLOW | MODERATE | Caution |
| > 50 | 🟢 GREEN | HIGH | Go |

### Data Flow

```
┌─────────┐      ┌─────────┐      ┌──────────┐      ┌─────────┐
│ ESP32 + │ WiFi │ Firebase│ Real │  Flutter │ User │ Mobile  │
│ Sensors ├─────►│ Realtime├─────►│   App    ├─────►│  Phone  │
│         │      │   DB    │ time │          │      │         │
└─────────┘      └─────────┘      └──────────┘      └─────────┘
```

### Calibration Process

1. **Warmup**: 30-second sensor warmup period
2. **Sampling**: Takes 10 readings from MQ-2 in clean air
3. **Baseline**: Calculates average as baseline
4. **Calibrated Value**: `reading - baseline`

### Update Cycle

- **Sensors**: Read every 2 seconds
- **Firebase**: Update every 5 seconds
- **Display**: Serial output every 2 seconds
- **Mobile App**: Real-time Firebase listeners

## 🚀 Usage

### Starting the System

1. **Power on ESP32**: Connect to power supply
2. **Wait for calibration**: 30-second warmup (keep in clean air)
3. **Check WiFi**: Verify connection in Serial Monitor
4. **Verify Firebase**: Check for "Firebase Connected!" message
5. **Open Mobile App**: Launch Flutter app on your phone

### Monitoring Data

#### Serial Monitor Output
```
╔════════════════════════════════════════╗
║       SENSOR READINGS                  ║
╠════════════════════════════════════════╣
║ MQ-2 Raw:        1456                  ║
║ MQ-2 Baseline:   1450                  ║
║ MQ-2 Calibrated: 6                     ║
║ MQ-135:          340                   ║
╠════════════════════════════════════════╣
║ Gas Level:       LOW                   ║
║ Traffic Light:   🔴 RED                ║
╚════════════════════════════════════════╝
```

#### Mobile App
- Tap traffic card to expand details
- Pull down to refresh data
- Color-coded indicators for quick status check

### Troubleshooting

| Issue | Solution |
|-------|----------|
| WiFi not connecting | Check SSID and password |
| Firebase error | Verify database URL and auth token |
| Incorrect readings | Re-calibrate sensor in clean air |
| App not updating | Check Firebase rules and internet connection |
| LED not lighting | Check wiring and resistor values |

## 📊 Customization

### Adjust Thresholds
In Arduino code:
```cpp
#define YELLOW_THRESHOLD 20  // Change this value
#define GREEN_THRESHOLD 50   // Change this value
```

### Change Update Intervals
```cpp
delay(2000);  // Sensor reading interval (2 seconds)
```

```cpp
if (millis() - sendDataPrevMillis > 5000) // Firebase update (5 seconds)
```

### Add More Traffic Systems
1. Duplicate Arduino code
2. Change Firebase path from `/traffic2/` to `/traffic3/`
3. Update Flutter app to read from new path

## 📄 License

This project is open source and available under the MIT License.

## 🔗 Resources

- [ESP32 Documentation](https://docs.espressif.com/projects/esp-32/en/latest/)
- [Firebase Documentation](https://firebase.google.com/docs)
- [Flutter Documentation](https://flutter.dev/docs)
- [MQ-2 Datasheet](https://www.pololu.com/file/0J309/MQ2.pdf)
- [MQ-135 Datasheet](https://www.olimex.com/Products/Components/Sensors/Gas/SNS-MQ135/resources/SNS-MQ135.pdf)
