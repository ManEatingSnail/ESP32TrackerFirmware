# ESP32 XP Tracker Firmware

Arduino firmware for displaying Old School RuneScape XP tracking data on an ESP32 with TFT display.

Works with the [RuneLite ESP32 XP Tracker plugin](https://github.com/ManEatingSnail/ESP32XPTracker).

## Hardware Required

- ESP32 development board (any variant)
- ILI9341 2.8" 240x320 TFT display (SPI)

## Wiring

| ESP32 Pin | ILI9341 Pin |
|-----------|-------------|
| GPIO 23   | MOSI        |
| GPIO 18   | SCK         |
| GPIO 15   | CS          |
| GPIO 2    | DC          |
| GPIO 4    | RST         |
| 3.3V      | VCC         |
| GND       | GND         |

## Installation

### 1. Install Arduino IDE

Download from [arduino.cc](https://www.arduino.cc/en/software)

### 2. Add ESP32 Board Support

1. Open Arduino IDE
2. File → Preferences
3. Add to "Additional Board Manager URLs":
```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
4. Tools → Board → Board Manager
5. Search "ESP32" and install

### 3. Install Libraries

Tools → Manage Libraries, install:
- **TFT_eSPI** by Bodmer
- **ArduinoJson** by Benoit Blanchon

### 4. Configure TFT_eSPI

1. Find your Arduino libraries folder:
   - Windows: `Documents\Arduino\libraries\TFT_eSPI`
   - Mac: `~/Documents/Arduino/libraries/TFT_eSPI`
   - Linux: `~/Arduino/libraries/TFT_eSPI`

2. Edit `User_Setup.h`:
```cpp
   // Uncomment this line:
   #define ILI9341_DRIVER
   
   // Set these pins:
   #define TFT_MISO 19
   #define TFT_MOSI 23
   #define TFT_SCLK 18
   #define TFT_CS   15
   #define TFT_DC   2
   #define TFT_RST  4
   
   // Uncomment these fonts:
   #define LOAD_GLCD
   #define LOAD_FONT2
   #define LOAD_FONT4
   #define LOAD_FONT6
   #define LOAD_FONT7
   #define LOAD_FONT8
   
   // Set SPI speed:
   #define SPI_FREQUENCY  40000000
```

### 5. Upload Firmware

1. Open `esp32_xp_tracker.ino` in Arduino IDE
2. Update WiFi credentials:
```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
```
3. Connect ESP32 via USB
4. Tools → Board → ESP32 Dev Module
5. Tools → Port → (select your COM port)
6. Click Upload (→)
7. Wait for "Done uploading"

### 6. Get IP Address

The ESP32 will display its IP address on screen after connecting to WiFi. Note this down (e.g., `192.168.1.100`).

## Usage

1. Install the [RuneLite plugin](https://github.com/ManEatingSnail/ESP32XPTracker)
2. Enter the ESP32 IP address in plugin settings
3. Start training in OSRS
4. Display updates automatically

## Display Layout
```
┌─────────────────────────────────────┐
│ Woodcutting              [85]       │
├─────────────────────────────────────┤
│ CURRENT XP         XP / HOUR        │
│ 1,234,567          45,000           │
│                                     │
│ XP GAINED          ACTIONS / HR     │
│ 12,345             900              │
│                                     │
│ TIME TO LEVEL                       │
│ 1h 23m                              │
│                                     │
│ ████████████████░░░░░░              │
│           67.5%                     │
└─────────────────────────────────────┘
```

## Troubleshooting

**Display stays blank:**
- Check wiring connections
- Verify TFT_eSPI configured correctly
- Try example sketches from TFT_eSPI library first

**Can't connect to WiFi:**
- Check SSID and password are correct
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Check WiFi signal strength

**"Waiting for data..." never changes:**
- Verify RuneLite plugin is enabled
- Check ESP32 IP matches in plugin settings
- Ensure ESP32 and computer on same network
- Test by opening `http://YOUR_ESP32_IP` in browser

**Wrong pin connections?**
Common pin variations for ESP32 boards:
- Some boards use different default pins
- Check your specific ESP32 board pinout diagram
- Adjust `User_Setup.h` if needed

## Customization

The firmware is fully customizable. Edit `esp32_xp_tracker.ino` to change:
- Colors (defined at top of file)
- Layout positions
- Font sizes
- Display content

## API Endpoints

The ESP32 runs a web server with these endpoints:

- `GET /` - Status page
- `POST /update` - Receive XP data from plugin
- `GET /status` - Current skill data (JSON)
