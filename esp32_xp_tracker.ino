#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>

// WiFi credentials - UPDATE THESE
const char* ssid = "SSID";
const char* password = "Password";

// Web server on port 80
WebServer server(80);

// TFT Display
TFT_eSPI tft = TFT_eSPI();

// Modern color scheme
#define COLOR_BG 0x0841        // Dark blue-grey background
#define COLOR_CARD 0x1082      // Slightly lighter card background
#define COLOR_PRIMARY 0x059F   // Teal/cyan accent
#define COLOR_SUCCESS 0x07E0   // Bright green
#define COLOR_WARNING 0xFD20   // Orange
#define COLOR_TEXT 0xFFFF      // White
#define COLOR_TEXT_DIM 0x7BEF  // Light grey
#define COLOR_PROGRESS_BG 0x2124  // Dark grey for progress bar background

// Current skill data
struct SkillData {
  String skillName;
  int currentXp;
  int level;
  int boostedLevel;
  int xpPerHour;
  int actionsPerHour;
  int xpGained;
  String timeToLevel;
  float progressPercent;
} currentSkill, previousSkill;

bool firstUpdate = true;

void setup() {
  Serial.begin(115200);
  
  // Initialize TFT - ILI9341 240x320
  tft.init();
  tft.setRotation(3); // Landscape mode (320x240)
  tft.fillScreen(COLOR_BG);
  
  // Show startup message
  tft.setTextSize(2);
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setCursor(80, 50);
  tft.println("OSRS Tracker");
  tft.setCursor(60, 80);
  tft.println("Connecting WiFi...");
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Show IP on screen
  tft.fillScreen(COLOR_BG);
  tft.setTextSize(2);
  tft.setTextColor(COLOR_SUCCESS, COLOR_BG);
  tft.setCursor(90, 50);
  tft.println("Connected!");
  tft.setTextColor(COLOR_TEXT, COLOR_BG);
  tft.setCursor(10, 80);
  tft.print("IP: ");
  tft.setTextColor(COLOR_PRIMARY, COLOR_BG);
  tft.println(WiFi.localIP());
  tft.setTextColor(COLOR_TEXT_DIM, COLOR_BG);
  tft.setCursor(60, 110);
  tft.println("Waiting for data...");
  
  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_POST, handleUpdate);
  server.on("/status", HTTP_GET, handleStatus);
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

// Root endpoint
void handleRoot() {
  String html = "<html><body>";
  html += "<h1>OSRS ESP32 XP Tracker</h1>";
  html += "<p>Device IP: " + WiFi.localIP().toString() + "</p>";
  html += "<p>Status: Ready</p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// Update endpoint
void handleUpdate() {
  if (server.hasArg("plain") == false) {
    server.send(400, "text/plain", "Body not received");
    return;
  }
  
  String body = server.arg("plain");
  
  // Parse JSON
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }
  
  // Store previous data
  previousSkill = currentSkill;
  
  // Extract skill data
  currentSkill.skillName = doc["skill"].as<String>();
  currentSkill.currentXp = doc["xp"] | 0;
  currentSkill.level = doc["level"] | 1;
  currentSkill.boostedLevel = doc["boosted_level"] | 1;
  currentSkill.xpPerHour = doc["xp_hr"] | 0;
  currentSkill.actionsPerHour = doc["actions_hr"] | 0;
  currentSkill.xpGained = doc["xp_gained"] | 0;
  currentSkill.timeToLevel = doc["time_to_level"].as<String>();
  currentSkill.progressPercent = doc["progress_percent"] | 0.0;
  
  // Update display
  updateDisplay();
  
  server.send(200, "text/plain", "OK");
}

// Status endpoint
void handleStatus() {
  StaticJsonDocument<512> doc;
  
  doc["skill"] = currentSkill.skillName;
  doc["xp"] = currentSkill.currentXp;
  doc["level"] = currentSkill.level;
  doc["xp_hr"] = currentSkill.xpPerHour;
  
  String response;
  serializeJson(doc, response);
  
  server.send(200, "application/json", response);
}

// Draw a rounded rectangle (for cards)
void drawRoundRect(int x, int y, int w, int h, int r, uint16_t color) {
  tft.fillRoundRect(x, y, w, h, r, color);
}

// Clear a specific area
void clearArea(int x, int y, int width, int height, uint16_t color) {
  tft.fillRect(x, y, width, height, color);
}

// Update display - modern card-based layout
void updateDisplay() {
  // First update or skill changed - redraw everything
  if (firstUpdate || currentSkill.skillName != previousSkill.skillName) {
    tft.fillScreen(COLOR_BG);
    firstUpdate = false;
    
    // Draw main card background
    drawRoundRect(5, 5, 310, 230, 8, COLOR_CARD);
    
    // Skill name header
    tft.setTextSize(3);
    tft.setTextColor(COLOR_PRIMARY, COLOR_CARD);
    tft.setCursor(15, 15);
    tft.print(currentSkill.skillName);
    
    // Level badge
    drawRoundRect(250, 12, 55, 30, 6, COLOR_PRIMARY);
    tft.setTextSize(2);
    tft.setTextColor(COLOR_TEXT, COLOR_PRIMARY);
    tft.setCursor(260, 18);
    tft.print(currentSkill.level);
    
    // Subtle separator
    tft.drawFastHLine(15, 50, 290, COLOR_TEXT_DIM);
    
    // Stats grid - draw labels
    tft.setTextSize(1);
    tft.setTextColor(COLOR_TEXT_DIM, COLOR_CARD);
    
    // Left column labels
    tft.setCursor(15, 65);
    tft.print("CURRENT XP");
    tft.setCursor(15, 110);
    tft.print("XP GAINED");
    tft.setCursor(15, 155);
    tft.print("TIME TO LEVEL");
    
    // Right column labels
    tft.setCursor(170, 65);
    tft.print("XP / HOUR");
    tft.setCursor(170, 110);
    tft.print("ACTIONS / HR");
    
    // Force redraw all values
    previousSkill.currentXp = -1;
    previousSkill.xpPerHour = -1;
    previousSkill.xpGained = -1;
    previousSkill.actionsPerHour = -1;
    previousSkill.timeToLevel = "";
    previousSkill.level = -1;
    previousSkill.progressPercent = -1;
  }
  
  // Update level badge if changed
  if (currentSkill.level != previousSkill.level) {
    drawRoundRect(250, 12, 55, 30, 6, COLOR_PRIMARY);
    tft.setTextSize(2);
    tft.setTextColor(COLOR_TEXT, COLOR_PRIMARY);
    tft.setCursor(260, 18);
    tft.print(currentSkill.level);
  }
  
  // Update current XP
  if (currentSkill.currentXp != previousSkill.currentXp) {
    clearArea(15, 80, 140, 20, COLOR_CARD);
    tft.setTextSize(2);
    tft.setTextColor(COLOR_TEXT, COLOR_CARD);
    tft.setCursor(15, 80);
    tft.print(formatNumber(currentSkill.currentXp));
  }
  
  // Update XP/hour
  if (currentSkill.xpPerHour != previousSkill.xpPerHour) {
    clearArea(170, 80, 135, 20, COLOR_CARD);
    tft.setTextSize(2);
    if (currentSkill.xpPerHour > 0) {
      tft.setTextColor(COLOR_SUCCESS, COLOR_CARD);
      tft.setCursor(170, 80);
      tft.print(formatNumber(currentSkill.xpPerHour));
    } else {
      tft.setTextColor(COLOR_TEXT_DIM, COLOR_CARD);
      tft.setCursor(170, 80);
      tft.print("--");
    }
  }
  
  // Update XP gained
  if (currentSkill.xpGained != previousSkill.xpGained) {
    clearArea(15, 125, 140, 20, COLOR_CARD);
    tft.setTextSize(2);
    tft.setTextColor(COLOR_PRIMARY, COLOR_CARD);
    tft.setCursor(15, 125);
    tft.print(formatNumber(currentSkill.xpGained));
  }
  
  // Update actions/hour
  if (currentSkill.actionsPerHour != previousSkill.actionsPerHour) {
    clearArea(170, 125, 135, 20, COLOR_CARD);
    tft.setTextSize(2);
    if (currentSkill.actionsPerHour > 0) {
      tft.setTextColor(COLOR_WARNING, COLOR_CARD);
      tft.setCursor(170, 125);
      tft.print(currentSkill.actionsPerHour);
    } else {
      tft.setTextColor(COLOR_TEXT_DIM, COLOR_CARD);
      tft.setCursor(170, 125);
      tft.print("--");
    }
  }
  
  // Update time to level
  if (currentSkill.timeToLevel != previousSkill.timeToLevel) {
    clearArea(15, 170, 290, 20, COLOR_CARD);
    tft.setTextSize(2);
    if (currentSkill.timeToLevel.length() > 0 && currentSkill.xpPerHour > 0) {
      tft.setTextColor(COLOR_TEXT, COLOR_CARD);
      tft.setCursor(15, 170);
      tft.print(currentSkill.timeToLevel);
    } else {
      tft.setTextColor(COLOR_TEXT_DIM, COLOR_CARD);
      tft.setCursor(15, 170);
      tft.print("--");
    }
  }
  
  // Update progress bar
  if (abs(currentSkill.progressPercent - previousSkill.progressPercent) > 0.01) {
    drawModernProgressBar(15, 200, 290, 25, currentSkill.progressPercent);
  }
}

// Modern progress bar with percentage outside
void drawModernProgressBar(int x, int y, int width, int height, float percent) {
  // Clear area including space for percentage text
  clearArea(x, y, width, height + 15, COLOR_CARD);
  
  // Background
  drawRoundRect(x, y, width, height, 4, COLOR_PROGRESS_BG);
  
  // Fill
  int fillWidth = (width - 4) * (percent / 100.0);
  if (fillWidth > 2) {
    // Use gradient-like effect with slightly rounded corners
    drawRoundRect(x + 2, y + 2, fillWidth, height - 4, 3, COLOR_SUCCESS);
  }
  
  // Percentage text BELOW the bar
  tft.setTextSize(1);
  tft.setTextColor(COLOR_TEXT_DIM, COLOR_CARD);
  tft.setCursor(x + (width / 2) - 15, y + height + 4);
  tft.print(percent, 1);
  tft.print("%");
}

// Format number with commas
String formatNumber(int num) {
  if (num < 1000) return String(num);
  
  String result = "";
  String numStr = String(num);
  int len = numStr.length();
  
  for (int i = 0; i < len; i++) {
    if (i > 0 && (len - i) % 3 == 0) {
      result += ",";
    }
    result += numStr[i];
  }
  
  return result;
}
