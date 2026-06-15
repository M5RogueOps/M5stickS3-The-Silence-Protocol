/*
 *  __________________________________________________________
 * |                                                          |
 * |    ██████╗██╗██╗     ███████╗███╗   ██╗ ██████╗███████╗  |
 * |   ██╔════╝██║██║     ██╔════╝████╗  ██║██╔════╝██╔════╝  |
 * |   ╚█████╗ ██║██║     █████╗  ██╔██╗ ██║██║     █████╗    |
 * |    ╚═══██╗██║██║     ██╔══╝  ██║╚██╗██║██║     ██╔══╝    |
 * |   ██████╔╝██║███████╗███████╗██║ ╚████║╚██████╗███████╗  |
 * |   ╚═════╝ ╚═╝╚══════╝╚══════╝╚═╝  ╚═══╝ ╚═════╝╚══════╝  |
 * |                                                          |
 * |           P  R  O  T  O  C  O  L     v 1 . 0            |
 * |   ____________________________________________________   |
 * |                                                          |
 * |   GitHub: https://github.com/M5RogueOps/The-Silence-Protocol |
 * |   Web:    https://www.ethicalhackersden.org              |
 * |__________________________________________________________|
 * 
 */
#include <M5Unified.h>
#include <WiFi.h>
#include <Preferences.h>

Preferences prefs;
uint8_t endingsUnlocked = 0; 
bool hasRFID = false;        

// HUD & RPG Elements
int playerHP = 100;
String locationName = "UNKNOWN";
int mazePath[5];
int mazeStep = 0;

// Custom Hazy CRT Colors
uint16_t CRT_BG      = M5.Display.color565(10, 20, 10);   
uint16_t CRT_GREEN   = M5.Display.color565(80, 220, 80);  
uint16_t CRT_RED     = M5.Display.color565(220, 80, 80);  
uint16_t CRT_CYAN    = M5.Display.color565(80, 200, 200); 
uint16_t CRT_AMBER   = M5.Display.color565(240, 150, 50); 
uint16_t CRT_WHITE   = M5.Display.color565(220, 220, 220); 
uint16_t CRT_BLUE    = M5.Display.color565(80, 80, 220);  
uint16_t CRT_MAGENTA = M5.Display.color565(220, 80, 220);

// Massive Game State Expansion
enum GameState {
  TITLE, LORE_1, LORE_2, LORE_3, LORE_4, APPROACH_1, APPROACH_2, 
  BASEMENT, HACK_KEYPAD, SERVER_ROOM_DESC, SERVER_ROOM, SECURITY_DESK, READ_LOG_1, READ_LOG_2, SEARCH_DESK, CAMERA_FEED,
  CORRIDOR, DEMO_ZONE_DESC, DEMO_ZONE, BLAST_RUBBLE, DATA_ARCHIVE_DESC, DATA_ARCHIVE, R36S_TERMINAL, FLASH_FIRMWARE,
  
  VENT_DESC_1, VENT_DESC_2, VENT_MAZE_GENERATE, VENT_MAZE_NAVIGATE, LASER_GRID, MAINTENANCE_SHAFT,
  WATER_TREATMENT_DESC, WATER_TREATMENT, COOLANT_PUMP, SABOTAGE_COOLING,
  
  ASSEMBLY_DESC_1, ASSEMBLY_DESC_2, ASSEMBLY_FLOOR, CONVEYOR_BELT, WELDING_LASERS, QA_SCANNER, QA_STEALTH_CHECK, SMASHER_TRAP,
  
  SUB_BASEMENT_DESC, SUB_BASEMENT, TRACK_DRONE_DESC, TRACK_DRONE, AMBUSH_DRONE,
  POWER_RELAY_DESC, POWER_RELAY, ESP32_BRIDGE, PI_ZERO_SERVER, FIX_AXIS, DEEP_TUNNEL_DESC, DEEP_TUNNEL, 
  
  CORE_ENTRANCE_DESC, CORE_ENTRANCE, DECRYPT_CORE, OVERRIDE_SWITCH,
  GAME_OVER, VICTORY, PYRRHIC_VICTORY
};

GameState currentState = TITLE;
int currentY = 0; 

// ==========================================
// CUSTOM CENTERED TEXT & HUD ENGINE
// ==========================================

void printCentered(String text) {
  text += "\n"; 
  int startIdx = 0;
  while (startIdx < text.length()) {
    int nextIdx = text.indexOf('\n', startIdx);
    if (nextIdx == -1) break; 
    String line = text.substring(startIdx, nextIdx);
    startIdx = nextIdx + 1;
    
    if (line.length() == 0) {
      currentY += M5.Display.fontHeight() + 2; 
    } else {
      int textWidth = M5.Display.textWidth(line);
      int x = (M5.Display.width() - textWidth) / 2;
      M5.Display.setCursor(x < 0 ? 0 : x, currentY); 
      M5.Display.print(line);
      currentY += M5.Display.fontHeight() + 2; 
    }
  }
}

void drawHUD() {
  if (currentState == TITLE || currentState == GAME_OVER || currentState == VICTORY || currentState == PYRRHIC_VICTORY) return;
  
  M5.Display.fillRect(0, 0, M5.Display.width(), 16, M5.Display.color565(5, 30, 5));
  M5.Display.setTextColor(CRT_GREEN);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(2, 4);
  M5.Display.printf("HP:%d LOC:%s", playerHP, locationName.c_str());
  M5.Display.drawFastHLine(0, 16, M5.Display.width(), CRT_WHITE);
}

// ==========================================
// AUDIO & VISUAL FX
// ==========================================

void drawScanlines() {
  for (int y = 0; y < M5.Display.height(); y += 3) {
    M5.Display.drawFastHLine(0, y, M5.Display.width(), M5.Display.color565(5, 10, 5));
  }
}

void effectStatic(int durationMs) {
  uint32_t start = millis();
  while (millis() - start < durationMs) {
    for(int i=0; i<8; i++) {
      int y = random(0, M5.Display.height());
      M5.Display.drawFastHLine(0, y, M5.Display.width(), CRT_WHITE); delay(2);
      M5.Display.drawFastHLine(0, y, M5.Display.width(), CRT_BG);
    }
  }
}

void effectFlash(uint16_t color, int flashes) {
  for (int i = 0; i < flashes; i++) {
    M5.Display.fillScreen(color); drawScanlines(); delay(40);
    M5.Display.fillScreen(CRT_BG); drawScanlines(); delay(40);
  }
}

void playBeep() { M5.Speaker.tone(4000, 100); }
void playClick() { M5.Speaker.tone(1500, 20); } 
void playError() { M5.Speaker.tone(300, 300); }
void playAlarm() {
  for(int i=0; i<3; i++) { M5.Speaker.tone(1200, 200); delay(200); M5.Speaker.tone(800, 200); delay(200); }
}

void takeDamage(int amount) {
  playerHP -= amount;
  effectFlash(CRT_RED, 3);
  playAlarm();
  if (playerHP <= 0) {
    currentState = GAME_OVER;
    locationName = "FATAL ERROR";
  }
}

// ==========================================
// HARDWARE MINIGAMES
// ==========================================

// NEW: Camera Hack (Frequency Sync)
bool runCamHackMinigame() {
  M5.Display.fillScreen(CRT_BG); currentY = 20; M5.Display.setTextSize(2); M5.Display.setTextColor(CRT_CYAN);
  printCentered("SYNC FREQ\nPress [A] in box"); drawScanlines();
  delay(1500);

  int locks = 0;
  int misses = 0;
  float cursorX = 0;
  float speed = 3.5;
  int dir = 1;
  int targetW = 40;
  int targetX = (M5.Display.width() - targetW) / 2;

  M5.Display.fillScreen(CRT_BG);
  
  while (locks < 3 && misses < 3) {
    M5.update();

    // Move cursor
    cursorX += speed * dir;
    if (cursorX <= 0) { cursorX = 0; dir = 1; }
    if (cursorX >= M5.Display.width() - 10) { cursorX = M5.Display.width() - 10; dir = -1; }

    // Clear game area
    M5.Display.fillRect(0, 40, M5.Display.width(), 40, CRT_BG); 
    
    // Draw target box & scanning cursor
    M5.Display.drawRect(targetX, 45, targetW, 30, CRT_AMBER); 
    M5.Display.fillRect((int)cursorX, 45, 10, 30, CRT_GREEN); 
    
    // Draw UI overlay
    M5.Display.fillRect(0, 0, M5.Display.width(), 30, CRT_BG);
    M5.Display.setCursor(5, 5); M5.Display.setTextColor(CRT_WHITE); M5.Display.setTextSize(1);
    M5.Display.printf("LOCKS: %d/3   ERR: %d/3", locks, misses);
    drawScanlines();

    if (M5.BtnA.wasPressed()) {
      if ((cursorX + 5) >= targetX && (cursorX + 5) <= (targetX + targetW)) {
        locks++;
        speed += 1.5;   // Faster
        targetW -= 8; // Smaller target
        M5.Speaker.tone(3000, 100);
        M5.Display.fillScreen(CRT_WHITE); delay(50); M5.Display.fillScreen(CRT_BG);
        targetX = random(10, M5.Display.width() - targetW - 10);
      } else {
        misses++;
        M5.Speaker.tone(300, 200);
        M5.Display.fillScreen(CRT_RED); delay(50); M5.Display.fillScreen(CRT_BG);
      }
    }
    delay(20);
  }
  
  if (locks >= 3) { playBeep(); return true; }
  playError(); return false;
}

bool runOverloadMinigame() {
  M5.Display.fillScreen(CRT_BG); currentY = 20; M5.Display.setTextSize(2); M5.Display.setTextColor(CRT_RED);
  printCentered("FIREWALL!"); M5.Display.setTextColor(CRT_AMBER);
  printCentered("MASH [A] & [B]\nRAPIDLY!"); drawScanlines(); delay(1500);

  uint32_t start = millis(); int presses = 0;
  while(millis() - start < 5000) {
    M5.update();
    if(M5.BtnA.wasPressed()) { presses++; M5.Speaker.tone(1000 + (presses*50), 30); }
    if(M5.BtnB.wasPressed()) { presses++; M5.Speaker.tone(1500 + (presses*50), 30); }

    int barWidth = map(presses, 0, 30, 0, M5.Display.width());
    if (barWidth > M5.Display.width()) barWidth = M5.Display.width();
    M5.Display.fillRect(0, M5.Display.height() - 20, M5.Display.width(), 20, CRT_BG);
    M5.Display.fillRect(0, M5.Display.height() - 20, barWidth, 20, CRT_MAGENTA);
    drawScanlines();

    if(presses >= 30) { playBeep(); return true; }
    delay(10);
  }
  playError(); return false;
}

bool runSSHMinigame() {
  M5.Display.fillScreen(CRT_BG); currentY = 20; M5.Display.setTextSize(2); M5.Display.setTextColor(CRT_CYAN);
  printCentered("SSH HANDSHAKE\n[A]=0   [B]=1"); drawScanlines(); delay(2000);

  int seq[6]; String displaySeq = "";
  for(int i=0; i<6; i++) { seq[i] = random(0, 2); displaySeq += String(seq[i]) + " "; }

  M5.Display.fillScreen(CRT_BG); currentY = 20; M5.Display.setTextColor(CRT_GREEN);
  printCentered("ENTER KEY:\n\n" + displaySeq); drawScanlines();

  int step = 0; uint32_t start = millis();
  while(millis() - start < 8000) {
    M5.update();
    if(M5.BtnA.wasPressed()) {
      if(seq[step] == 0) { step++; playClick(); } else { playError(); return false; }
    } else if(M5.BtnB.wasPressed()) {
      if(seq[step] == 1) { step++; playClick(); } else { playError(); return false; }
    }
    if(step >= 6) { playBeep(); return true; }
  }
  playError(); return false; 
}

bool runStealthMinigame() {
  M5.Display.fillScreen(CRT_BG); currentY = 25; M5.Display.setTextSize(2); M5.Display.setTextColor(CRT_RED);
  printCentered("REMAIN\nSILENT!"); drawScanlines();
  
  M5.Mic.begin(); uint32_t start = millis();
  int16_t micData[256]; bool heard = false;
  
  while(millis() - start < 5000) {
    M5.update();
    if(M5.Mic.record(micData, 256, 16000)) {
      int maxNoise = 0;
      for(int i=0; i<256; i++) { if(abs(micData[i]) > maxNoise) maxNoise = abs(micData[i]); }
      int barWidth = map(maxNoise, 0, 2500, 0, M5.Display.width());
      if (barWidth > M5.Display.width()) barWidth = M5.Display.width();
      M5.Display.fillRect(0, M5.Display.height() - 10, M5.Display.width(), 10, CRT_BG);
      M5.Display.fillRect(0, M5.Display.height() - 10, barWidth, 10, CRT_RED);
      drawScanlines(); 
      if(maxNoise > 1800) { heard = true; break; } 
    } delay(10);
  } M5.Mic.end();
  
  if(heard) { playError(); return false; }
  playBeep(); return true;
}

bool runTimingMinigame() {
  M5.Display.fillScreen(CRT_BG); currentY = 25; M5.Display.setTextSize(2); M5.Display.setTextColor(CRT_WHITE);
  printCentered("ALIGN PACKET\nPress A on CYAN"); drawScanlines(); delay(1500);
  uint16_t colors[] = {CRT_RED, CRT_BLUE, CRT_AMBER, CRT_CYAN, CRT_WHITE};
  for(int i=0; i<8; i++) {
    uint16_t c = colors[random(0, 5)]; M5.Display.fillScreen(c); drawScanlines();
    if(c == CRT_CYAN) M5.Speaker.tone(2000, 50);
    uint32_t flashStart = millis(); bool pressed = false;
    while(millis() - flashStart < 400) { M5.update(); if(M5.BtnA.wasPressed()) { pressed = true; break; } }
    if(pressed) { if(c == CRT_CYAN) { playBeep(); return true; } else { playError(); return false; } }
  } return false;
}

bool runShakeMinigame() {
  M5.Display.fillScreen(CRT_BG); currentY = 25; M5.Display.setTextSize(2); M5.Display.setTextColor(CRT_CYAN);
  printCentered("SHAKE RAPIDLY\nTO CHARGE EMP"); drawScanlines();
  uint32_t start = millis(); float ax, ay, az; int shakes = 0;
  while(millis() - start < 3500) {
    M5.update(); M5.Imu.getAccelData(&ax, &ay, &az);
    if (abs(ax) > 1.5 || abs(ay) > 1.5) shakes++;
    if (shakes % 5 == 0 && shakes > 0) M5.Speaker.tone(1000 + (shakes*50), 50); delay(50);
  } return (shakes > 15);
}

bool runBalanceMinigame() {
  M5.Display.fillScreen(CRT_BG); currentY = 25; M5.Display.setTextSize(2); M5.Display.setTextColor(CRT_AMBER);
  printCentered("KEEP DEVICE\nFLAT TO CROSS"); drawScanlines(); delay(1500);
  uint32_t start = millis(); float ax, ay, az;
  while(millis() - start < 5000) {
    M5.update(); M5.Imu.getAccelData(&ax, &ay, &az);
    if (abs(ax) > 0.3 || abs(ay) > 0.3) { playError(); return false; }
    M5.Display.fillScreen(CRT_BLUE); drawScanlines(); delay(50);
  } return true;
}

void runWifiScanner() {
  M5.Display.fillScreen(CRT_BG); currentY = 15; M5.Display.setTextSize(2); M5.Display.setTextColor(CRT_GREEN);
  printCentered("SCANNING NODES"); drawScanlines(); playBeep();
  int n = WiFi.scanNetworks();
  M5.Display.fillScreen(CRT_BG); currentY = 15; printCentered("TARGETS:");
  for (int i = 0; i < min(n, 3); ++i) { printCentered(WiFi.SSID(i)); delay(300); }
  M5.Display.setTextColor(CRT_CYAN); printCentered("\nInjecting..."); drawScanlines(); delay(2500);
}

// ==========================================
// CORE GAME ENGINE
// ==========================================

void setup() {
  auto cfg = M5.config(); M5.begin(cfg);
  M5.Display.setRotation(1); M5.Display.setTextWrap(false); 
  M5.Speaker.setVolume(128);
  prefs.begin("silence", false); endingsUnlocked = prefs.getUInt("endings", 0);
  drawScreen();
}

void loop() {
  M5.update(); 
  
  switch (currentState) {
    case TITLE: hasRFID = false; playerHP = 100; if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) { currentState = LORE_1; playClick(); drawScreen(); } break;
    case LORE_1: if (M5.BtnA.wasPressed()) { currentState = LORE_2; playClick(); drawScreen(); } break;
    case LORE_2: if (M5.BtnA.wasPressed()) { currentState = LORE_3; playClick(); drawScreen(); } break;
    case LORE_3: if (M5.BtnA.wasPressed()) { currentState = LORE_4; playClick(); drawScreen(); } break;
    case LORE_4: if (M5.BtnA.wasPressed()) { currentState = APPROACH_1; playClick(); drawScreen(); } break;
    case APPROACH_1: if (M5.BtnA.wasPressed()) { currentState = APPROACH_2; playClick(); drawScreen(); } break;
    case APPROACH_2: if (M5.BtnA.wasPressed()) { currentState = BASEMENT; playClick(); drawScreen(); } break;

    case BASEMENT:
      locationName = "HUB ENTRY";
      if (M5.BtnA.wasPressed()) { 
        if(runTimingMinigame()) { effectStatic(300); effectFlash(CRT_GREEN, 2); currentState = HACK_KEYPAD; drawScreen(); } 
        else { takeDamage(25); drawScreen(); }
      } else if (M5.BtnB.wasPressed()) { currentState = VENT_DESC_1; playClick(); drawScreen(); } break;

    case HACK_KEYPAD: if (M5.BtnA.wasPressed()) { currentState = SERVER_ROOM_DESC; playClick(); drawScreen(); } break;
    
    case SERVER_ROOM_DESC: locationName = "DATACENTER"; if (M5.BtnA.wasPressed()) { currentState = SERVER_ROOM; playClick(); drawScreen(); } break;
    case SERVER_ROOM:
      if (M5.BtnA.wasPressed()) { currentState = SECURITY_DESK; playClick(); drawScreen(); }
      else if (M5.BtnB.wasPressed()) { takeDamage(40); drawScreen(); } break;
      
    // CAM HACK MINIGAME IMPLEMENTED HERE
    case SECURITY_DESK:
      if (M5.BtnA.wasPressed()) { 
        if(runCamHackMinigame()) { effectStatic(200); playBeep(); currentState = CAMERA_FEED; drawScreen(); }
        else { takeDamage(20); drawScreen(); }
      }
      else if (M5.BtnB.wasPressed()) { currentState = READ_LOG_1; playClick(); drawScreen(); } break;

    case READ_LOG_1: if (M5.BtnA.wasPressed()) { currentState = READ_LOG_2; playClick(); drawScreen(); } break;
    case READ_LOG_2: if (M5.BtnA.wasPressed()) { currentState = SEARCH_DESK; playClick(); drawScreen(); } break;
    case SEARCH_DESK: if (M5.BtnA.wasPressed()) { hasRFID = true; playBeep(); currentState = CORRIDOR; drawScreen(); } break;
      
    case CAMERA_FEED: if (M5.BtnA.wasPressed()) { currentState = CORRIDOR; playClick(); drawScreen(); } break;
      
    case CORRIDOR:
      locationName = "NW WING";
      if (M5.BtnA.wasPressed()) { currentState = DEMO_ZONE_DESC; playClick(); drawScreen(); }
      else if (M5.BtnB.wasPressed()) { currentState = DATA_ARCHIVE_DESC; playClick(); drawScreen(); } break;
      
    case DEMO_ZONE_DESC: if (M5.BtnA.wasPressed()) { currentState = DEMO_ZONE; playClick(); drawScreen(); } break;
    case DEMO_ZONE:
      if (M5.BtnA.wasPressed()) { effectFlash(CRT_WHITE, 3); M5.Speaker.tone(200, 500); delay(500); effectStatic(600); currentState = BLAST_RUBBLE; drawScreen(); }
      else if (M5.BtnB.wasPressed()) { takeDamage(30); drawScreen(); } break;
    case BLAST_RUBBLE: if (M5.BtnA.wasPressed()) { currentState = DATA_ARCHIVE_DESC; playClick(); drawScreen(); } break;
      
    case DATA_ARCHIVE_DESC: locationName = "ARCHIVES"; if (M5.BtnA.wasPressed()) { currentState = DATA_ARCHIVE; playClick(); drawScreen(); } break;
    case DATA_ARCHIVE:
      if (M5.BtnA.wasPressed()) { 
        if(hasRFID) { effectFlash(CRT_GREEN, 2); playBeep(); currentState = FLASH_FIRMWARE; } 
        else { currentState = R36S_TERMINAL; } drawScreen(); 
      } else if (M5.BtnB.wasPressed() && !hasRFID) { takeDamage(15); drawScreen(); } break;
      
    case R36S_TERMINAL:
      if (M5.BtnA.wasPressed()) { effectFlash(CRT_GREEN, 3); playBeep(); currentState = FLASH_FIRMWARE; drawScreen(); }
      else if (M5.BtnB.wasPressed()) { takeDamage(20); drawScreen(); } break;
    case FLASH_FIRMWARE: if (M5.BtnA.wasPressed()) { currentState = ASSEMBLY_DESC_1; playClick(); drawScreen(); } break; 

    case VENT_DESC_1: locationName = "HVAC DUCTS"; if (M5.BtnA.wasPressed()) { currentState = VENT_DESC_2; playClick(); drawScreen(); } break;
    case VENT_DESC_2: if (M5.BtnA.wasPressed()) { currentState = VENT_MAZE_GENERATE; playClick(); drawScreen(); } break;
    
    case VENT_MAZE_GENERATE:
      for(int i=0; i<5; i++) mazePath[i] = random(0, 2);
      mazeStep = 0;
      currentState = VENT_MAZE_NAVIGATE; drawScreen(); break;
      
    case VENT_MAZE_NAVIGATE:
      if (M5.BtnA.wasPressed()) { 
        if(mazePath[mazeStep] == 0) { mazeStep++; playBeep(); } else { takeDamage(15); }
      } else if (M5.BtnB.wasPressed()) { 
        if(mazePath[mazeStep] == 1) { mazeStep++; playBeep(); } else { takeDamage(15); }
      }
      if (playerHP > 0 && mazeStep >= 5) { currentState = MAINTENANCE_SHAFT; }
      if (playerHP > 0) drawScreen();
      break;

    case MAINTENANCE_SHAFT: if (M5.BtnA.wasPressed()) { currentState = WATER_TREATMENT_DESC; playClick(); drawScreen(); } break;
    
    case WATER_TREATMENT_DESC: locationName = "PUMP ROOM"; if (M5.BtnA.wasPressed()) { currentState = WATER_TREATMENT; playClick(); drawScreen(); } break;
    case WATER_TREATMENT:
      if (M5.BtnA.wasPressed()) { currentState = COOLANT_PUMP; playClick(); drawScreen(); } 
      else if (M5.BtnB.wasPressed()) { currentState = ASSEMBLY_DESC_1; playClick(); drawScreen(); } break; 
    case COOLANT_PUMP:
      if (M5.BtnA.wasPressed()) { effectStatic(800); effectFlash(CRT_AMBER, 4); playAlarm(); currentState = SABOTAGE_COOLING; drawScreen(); }
      else if (M5.BtnB.wasPressed()) { currentState = ASSEMBLY_DESC_1; playClick(); drawScreen(); } break;
    case SABOTAGE_COOLING:
      if (M5.BtnA.wasPressed()) { endingsUnlocked |= 2; prefs.putUInt("endings", endingsUnlocked); currentState = PYRRHIC_VICTORY; drawScreen(); } break;

    case ASSEMBLY_DESC_1: locationName = "FACTORY FLR"; if (M5.BtnA.wasPressed()) { currentState = ASSEMBLY_DESC_2; playClick(); drawScreen(); } break;
    case ASSEMBLY_DESC_2: if (M5.BtnA.wasPressed()) { currentState = ASSEMBLY_FLOOR; playClick(); drawScreen(); } break;
    case ASSEMBLY_FLOOR:
      if (M5.BtnA.wasPressed()) { currentState = CONVEYOR_BELT; playClick(); drawScreen(); }
      else if (M5.BtnB.wasPressed()) { takeDamage(30); drawScreen(); } break;
    case CONVEYOR_BELT:
      if (M5.BtnA.wasPressed()) { currentState = WELDING_LASERS; playClick(); drawScreen(); }
      else if (M5.BtnB.wasPressed()) { currentState = QA_SCANNER; playClick(); drawScreen(); } break;
    case WELDING_LASERS:
      if (M5.BtnA.wasPressed()) { effectFlash(CRT_WHITE, 3); playBeep(); currentState = SUB_BASEMENT_DESC; drawScreen(); }
      else if (M5.BtnB.wasPressed()) { takeDamage(40); drawScreen(); } break;
    case QA_SCANNER:
      if (M5.BtnA.wasPressed()) { currentState = QA_STEALTH_CHECK; playClick(); drawScreen(); }
      else if (M5.BtnB.wasPressed()) { takeDamage(35); if(playerHP > 0) currentState = SUB_BASEMENT_DESC; drawScreen(); } break;
    
    case QA_STEALTH_CHECK:
      if(runStealthMinigame()) { currentState = SUB_BASEMENT_DESC; drawScreen(); } 
      else { takeDamage(50); if(playerHP > 0) currentState = SUB_BASEMENT_DESC; drawScreen(); } break;

    case SUB_BASEMENT_DESC: locationName = "SUB-BASEMENT"; if (M5.BtnA.wasPressed()) { currentState = SUB_BASEMENT; playClick(); drawScreen(); } break;
    case SUB_BASEMENT:
      if (M5.BtnA.wasPressed()) { currentState = TRACK_DRONE_DESC; playClick(); drawScreen(); }
      else if (M5.BtnB.wasPressed()) { takeDamage(25); drawScreen(); } break;
      
    case TRACK_DRONE_DESC: if (M5.BtnA.wasPressed()) { currentState = TRACK_DRONE; playClick(); drawScreen(); } break;
    case TRACK_DRONE:
      if (M5.BtnA.wasPressed()) { 
        if(runShakeMinigame()) { effectFlash(CRT_CYAN, 4); effectStatic(200); currentState = AMBUSH_DRONE; drawScreen(); } 
        else { takeDamage(100); drawScreen(); } 
      } else if (M5.BtnB.wasPressed()) { takeDamage(60); drawScreen(); } break;
      
    case AMBUSH_DRONE: if (M5.BtnA.wasPressed()) { currentState = POWER_RELAY_DESC; playClick(); drawScreen(); } break;
      
    case POWER_RELAY_DESC: locationName = "CHASM"; if (M5.BtnA.wasPressed()) { currentState = POWER_RELAY; playClick(); drawScreen(); } break;
    case POWER_RELAY:
      if (M5.BtnA.wasPressed()) { effectFlash(CRT_AMBER, 3); playBeep(); currentState = ESP32_BRIDGE; drawScreen(); }
      else if (M5.BtnB.wasPressed()) { 
        if(runBalanceMinigame()) { effectFlash(CRT_GREEN, 2); playBeep(); currentState = ESP32_BRIDGE; drawScreen(); } 
        else { takeDamage(100); drawScreen(); } 
      } break;
      
    case ESP32_BRIDGE: if (M5.BtnA.wasPressed()) { currentState = PI_ZERO_SERVER; playClick(); drawScreen(); } break;
      
    case PI_ZERO_SERVER:
      if (M5.BtnA.wasPressed()) { 
        if(runSSHMinigame()) { effectStatic(300); playBeep(); currentState = FIX_AXIS; drawScreen(); }
        else { takeDamage(25); drawScreen(); }
      }
      else if (M5.BtnB.wasPressed()) { takeDamage(20); drawScreen(); } break;
      
    case FIX_AXIS: if (M5.BtnA.wasPressed()) { currentState = DEEP_TUNNEL_DESC; playClick(); drawScreen(); } break;
      
    case DEEP_TUNNEL_DESC: locationName = "TUNNEL"; if (M5.BtnA.wasPressed()) { currentState = DEEP_TUNNEL; playClick(); drawScreen(); } break;
    case DEEP_TUNNEL:
      if (M5.BtnA.wasPressed()) { currentState = CORE_ENTRANCE_DESC; playClick(); drawScreen(); }
      else if (M5.BtnB.wasPressed()) { takeDamage(10); drawScreen(); } break;
      
    case CORE_ENTRANCE_DESC: locationName = "MAIN CORE"; if (M5.BtnA.wasPressed()) { currentState = CORE_ENTRANCE; playClick(); drawScreen(); } break;
    
    case CORE_ENTRANCE:
      if (M5.BtnA.wasPressed()) { 
        if(runOverloadMinigame()) {
          runWifiScanner(); 
          effectStatic(500); effectFlash(CRT_CYAN, 3); playBeep(); 
          currentState = DECRYPT_CORE; drawScreen(); 
        } else {
          takeDamage(45); drawScreen();
        }
      }
      else if (M5.BtnB.wasPressed()) { takeDamage(50); drawScreen(); } break;
      
    case DECRYPT_CORE: if (M5.BtnA.wasPressed()) { currentState = OVERRIDE_SWITCH; playClick(); drawScreen(); } break;
      
    case OVERRIDE_SWITCH:
      if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) { 
        effectFlash(CRT_WHITE, 5); M5.Speaker.tone(1500, 1000); endingsUnlocked |= 1; 
        prefs.putUInt("endings", endingsUnlocked); currentState = VICTORY; drawScreen(); 
      } break;

    case GAME_OVER:
    case VICTORY:
    case PYRRHIC_VICTORY:
      if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) { currentState = TITLE; playClick(); drawScreen(); } break;
  }
}

// Function to handle rendering text
void drawScreen() {
  M5.Display.fillScreen(CRT_BG); 
  drawHUD();
  currentY = 24; 
  M5.Display.setTextSize(2); 
  
  switch (currentState) {
    case TITLE: 
      M5.Display.fillScreen(CRT_BG); 
      currentY = 15;
      M5.Display.setTextColor(CRT_AMBER); M5.Display.setTextSize(3); printCentered("SILENCE\nPROTOCOL"); 
      M5.Display.setTextColor(CRT_WHITE); M5.Display.setTextSize(2); printCentered("\nPress ANY");
      if(endingsUnlocked == 3) { M5.Display.setTextColor(CRT_GREEN); M5.Display.setTextSize(1); printCentered("\n[DEV MODE UNLOCKED]"); }
      break;

    case LORE_1: M5.Display.setTextColor(CRT_WHITE); printCentered("The year is\n2028."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Next"); break;
    case LORE_2: M5.Display.setTextColor(CRT_WHITE); printCentered("The 'Silence'\nmalware burned\nthe grid."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Next"); break;
    case LORE_3: M5.Display.setTextColor(CRT_WHITE); printCentered("Heavy servers\nare dead.\nOnly M5Stack\nsurvives."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Next"); break;
    case LORE_4: M5.Display.setTextColor(CRT_WHITE); printCentered("You are an\nengineer for\nEthical Hackers."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Next"); break;
    case APPROACH_1: M5.Display.setTextColor(CRT_WHITE); printCentered("Rain lashes\nyour jacket."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Next"); break;
    case APPROACH_2: M5.Display.setTextColor(CRT_WHITE); printCentered("The NE Hub\nlooms ahead.\nYou slip inside"); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Enter"); break;

    case BASEMENT: M5.Display.setTextColor(CRT_WHITE); printCentered("Hub Door\nLocked."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Hack\n[B] Vent"); break;
    case HACK_KEYPAD: M5.Display.setTextColor(CRT_GREEN); printCentered("Hack Success.\nDoor Opens."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Enter"); break;
    case SERVER_ROOM_DESC: M5.Display.setTextColor(CRT_WHITE); printCentered("The air is cold\nand smells of\nozone."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Look"); break;
    case SERVER_ROOM: M5.Display.setTextColor(CRT_WHITE); printCentered("Lasers active."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Sneak\n[B] Run"); break;
    case SECURITY_DESK: M5.Display.setTextColor(CRT_WHITE); printCentered("Cam sweeps\nleft to right."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Hack Cam\n[B] Search"); break;
    case READ_LOG_1: M5.Display.setTextColor(CRT_AMBER); printCentered("LOG: They came\nthrough the\nrouters."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Next"); break;
    case READ_LOG_2: M5.Display.setTextColor(CRT_AMBER); printCentered("LOG: I hid my\nRFID in the\nbottom drawer."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Take it"); break;
    case SEARCH_DESK: M5.Display.setTextColor(CRT_GREEN); printCentered("Found Admin\nRFID Card!"); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Leave"); break;
    case CAMERA_FEED: M5.Display.setTextColor(CRT_GREEN); printCentered("Cams looped."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Proceed"); break;
    case CORRIDOR: M5.Display.setTextColor(CRT_WHITE); printCentered("Fork ahead."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Rubble\n[B] Archive"); break;
    case DEMO_ZONE_DESC: M5.Display.setTextColor(CRT_WHITE); printCentered("Ceiling caved.\nSteel bars block\nthe way."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Assess"); break;
    case DEMO_ZONE: M5.Display.setTextColor(CRT_WHITE); printCentered("Path blocked."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Blast\n[B] Dig"); break;
    case BLAST_RUBBLE: M5.Display.setTextColor(CRT_GREEN); printCentered("Charge set.\nPath clear."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Walk"); break;
    case DATA_ARCHIVE_DESC: M5.Display.setTextColor(CRT_WHITE); printCentered("Dusty servers.\nA single screen\nglows red."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Approach"); break;
    case DATA_ARCHIVE: M5.Display.setTextColor(CRT_WHITE); printCentered("Locked term."); M5.Display.setTextColor(CRT_CYAN); 
      if(hasRFID) { printCentered("\n[A] Swipe RFID"); } else { printCentered("\n[A] Patch\n[B] Smash"); } break;
    case R36S_TERMINAL: M5.Display.setTextColor(CRT_WHITE); printCentered("Retro board\nfound."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Flash\n[B] Format"); break;
    case FLASH_FIRMWARE: M5.Display.setTextColor(CRT_GREEN); printCentered("Access Open.\nMoving down."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Descend"); break;

    case VENT_DESC_1: M5.Display.setTextColor(CRT_WHITE); printCentered("You unscrew the\nrusty grate."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Crawl"); break;
    case VENT_DESC_2: M5.Display.setTextColor(CRT_WHITE); printCentered("It is tight.\nDust chokes you."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Keep going"); break;
    
    case VENT_MAZE_NAVIGATE: 
      M5.Display.setTextColor(CRT_WHITE); printCentered("Vent Maze"); 
      M5.Display.setTextColor(CRT_GREEN); M5.Display.setTextSize(1);
      char buf[32]; sprintf(buf, "Depth: %d/5", mazeStep); printCentered(buf);
      M5.Display.setTextSize(2); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Left\n[B] Right"); 
      break;

    case MAINTENANCE_SHAFT: M5.Display.setTextColor(CRT_GREEN); printCentered("Safe drop."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Land"); break;
    case WATER_TREATMENT_DESC: M5.Display.setTextColor(CRT_WHITE); printCentered("Massive pipes\npump water to\nthe servers."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Look"); break;
    case WATER_TREATMENT: M5.Display.setTextColor(CRT_WHITE); printCentered("Cooling pumps."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Sabotage\n[B] Sneak"); break;
    case COOLANT_PUMP: M5.Display.setTextColor(CRT_AMBER); printCentered("WARNING: Hub\nwill melt down."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] DO IT\n[B] Back out"); break;
    case SABOTAGE_COOLING: M5.Display.setTextColor(CRT_GREEN); printCentered("Pipes burst.\nAlarms blare."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Run"); break;

    case ASSEMBLY_DESC_1: M5.Display.setTextColor(CRT_WHITE); printCentered("You enter a\nmassive factory\nfloor."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Next"); break;
    case ASSEMBLY_DESC_2: M5.Display.setTextColor(CRT_WHITE); printCentered("The Silence is\nbuilding war\ndrones here."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Hide"); break;
    case ASSEMBLY_FLOOR: M5.Display.setTextColor(CRT_WHITE); printCentered("Robot arms\nswing wildly."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Ride Belt\n[B] Walk Floor"); break;
    case CONVEYOR_BELT: M5.Display.setTextColor(CRT_WHITE); printCentered("Riding belt.\nObstacle ahead."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Dodge Laser\n[B] Roll left"); break;
    case WELDING_LASERS: M5.Display.setTextColor(CRT_GREEN); printCentered("Dodged it.\nYou jump off."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Move on"); break;
    case QA_SCANNER: M5.Display.setTextColor(CRT_WHITE); printCentered("Scanner beams\nsearching..."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Freeze\n[B] Sprint"); break;

    case SUB_BASEMENT_DESC: M5.Display.setTextColor(CRT_WHITE); printCentered("You reach the\nsub-basement."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Listen"); break;
    case SUB_BASEMENT: M5.Display.setTextColor(CRT_WHITE); printCentered("Heavy footsteps\necho close."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Track\n[B] Hide"); break;
    case TRACK_DRONE_DESC: M5.Display.setTextColor(CRT_WHITE); printCentered("A massive\nsecurity drone\nblocks you."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Ready EMP"); break;
    case TRACK_DRONE: M5.Display.setTextColor(CRT_WHITE); printCentered("It turns to you"); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Use EMP\n[B] Throw rock"); break;
    case AMBUSH_DRONE: M5.Display.setTextColor(CRT_GREEN); printCentered("Drone fries and\ncollapses."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Step over"); break;
    case POWER_RELAY_DESC: M5.Display.setTextColor(CRT_WHITE); printCentered("A chasm opens\nbefore the\nserver room."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Look down"); break;
    case POWER_RELAY: M5.Display.setTextColor(CRT_WHITE); printCentered("Bridge is out.\nSparks fly."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Hack Relay\n[B] Balance"); break;
    case ESP32_BRIDGE: M5.Display.setTextColor(CRT_GREEN); printCentered("Bridge safe.\nYou cross."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Enter"); break;
    case PI_ZERO_SERVER: M5.Display.setTextColor(CRT_WHITE); printCentered("Pi Zero gate\ncontrols."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] SSH\n[B] Kick door"); break;
    case FIX_AXIS: M5.Display.setTextColor(CRT_GREEN); printCentered("Fixed axis.\nGate open."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Enter"); break;
    case DEEP_TUNNEL_DESC: M5.Display.setTextColor(CRT_WHITE); printCentered("The air grows\nwarm. Red lights\npulse slowly."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Walk"); break;
    case DEEP_TUNNEL: M5.Display.setTextColor(CRT_WHITE); printCentered("Final tunnel."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Go to Core\n[B] Rest here"); break;
    case CORE_ENTRANCE_DESC: M5.Display.setTextColor(CRT_WHITE); printCentered("The Main Core.\nA monolithic\nblack pillar."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Interface"); break;
    case CORE_ENTRANCE: M5.Display.setTextColor(CRT_WHITE); printCentered("Firewall active."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Overload\n[B] Pull wires"); break;
    
    case DECRYPT_CORE: M5.Display.setTextColor(CRT_GREEN); printCentered("Core Hacked.\nShields down."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Approach"); break;
    case OVERRIDE_SWITCH: M5.Display.setTextColor(CRT_AMBER); printCentered("Manual override\nexposed."); M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] PULL\n[B] PULL"); break;

    case GAME_OVER: 
      M5.Display.fillScreen(CRT_BG); currentY = 15;
      M5.Display.setTextColor(CRT_RED); M5.Display.setTextSize(3); printCentered("FAILED"); 
      M5.Display.setTextColor(CRT_WHITE); M5.Display.setTextSize(2); printCentered("\nSilence wins."); 
      M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Restart"); break;
    case VICTORY: 
      M5.Display.fillScreen(CRT_BG); currentY = 15;
      M5.Display.setTextColor(CRT_GREEN); M5.Display.setTextSize(3); printCentered("ONLINE"); 
      M5.Display.setTextColor(CRT_WHITE); M5.Display.setTextSize(2); printCentered("\nGrid saved."); 
      M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Restart"); break;
    case PYRRHIC_VICTORY: 
      M5.Display.fillScreen(CRT_BG); currentY = 15;
      M5.Display.setTextColor(CRT_AMBER); M5.Display.setTextSize(3); printCentered("PYRRHIC\nVICTORY"); 
      M5.Display.setTextColor(CRT_WHITE); M5.Display.setTextSize(2); printCentered("\nHub lost."); 
      M5.Display.setTextColor(CRT_CYAN); printCentered("\n[A] Restart"); break;
  }
  
  drawScanlines();
}
