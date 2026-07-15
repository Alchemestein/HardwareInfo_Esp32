#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define I2C_SCL 32
#define I2C_SDA 33

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

// Thick 4px bar with dynamic vertical padding (gap) applied
void drawProperBarRow(String title, String value, int percent, int &y, int gap) {
  display.setCursor(0, y);
  display.print(title);
  
  int valX = 128 - (value.length() * 6);
  display.setCursor(valX, y);
  display.print(value);
  
  // Track starts just below the text
  display.drawRect(0, y + 8, 128, 4, SSD1306_WHITE);
  
  // Solid 2px inner fill
  int fillW = (percent * 126) / 100;
  if (fillW > 0) {
    display.fillRect(1, y + 9, fillW, 2, SSD1306_WHITE);
  }
  
  // Advance by its base height (12) + the dynamic gap
  y += 12 + gap; 
}

// Clean text row with dynamic vertical padding (gap) applied
void drawTextRow(String title, String value, int &y, int gap) {
  display.setCursor(0, y);
  display.print(title);
  
  int valX = 128 - (value.length() * 6);
  display.setCursor(valX, y);
  display.print(value);
  
  // Advance by its base height (8) + the dynamic gap
  y += 8 + gap;
}

// Phone-style battery icon
void drawBattery(int x, int y, int percent, bool charging) {
  display.drawRect(x, y, 16, 8, SSD1306_WHITE);
  display.fillRect(x + 16, y + 2, 2, 4, SSD1306_WHITE);
  int fillW = (percent * 14) / 100;
  display.fillRect(x + 1, y + 1, fillW, 6, SSD1306_WHITE);
  if (charging) {
    display.setCursor(x - 8, y);
    display.print("+");
  }
}

void loop() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    
    String p[12];
    int start = 0;
    for (int i = 0; i < 12; i++) {
      int idx = data.indexOf('|', start);
      if (idx == -1) {
        p[i] = data.substring(start);
        break;
      }
      p[i] = data.substring(start, idx);
      start = idx + 1;
    }

    int cpuT = p[0].toInt();
    int gpuT = p[1].toInt();
    int ram = p[2].toInt();
    int ssdPct = p[3].toInt();
    int ssdFree = p[4].toInt();
    int fan = p[5].toInt();
    int bat = p[6].toInt();
    int chg = p[7].toInt();
    String timeStr = p[8];
    String cpuName = p[9];
    String cpuClock = p[10];
    int clockPct = p[11].toInt();
    
    display.clearDisplay();
    display.setTextSize(1);
    
    // ==========================================
    // TOP STATUS BAR (Clean, no lines)
    // ==========================================
    display.setCursor(0, 0);
    display.print(timeStr);
    if (bat > 0) drawBattery(108, 0, bat, (chg == 1));
    
    // ==========================================
    // DYNAMIC SPACING CALCULATOR
    // ==========================================
    bool hasGpu = (gpuT > 0 && gpuT != cpuT);
    bool hasFan = (fan > 0);
    
    // Base setup: Header (8px), CPU (12px), RAM (12px), SSD (8px)
    int activeRows = 4; 
    int totalHeight = 40; 
    
    if (hasGpu) { activeRows++; totalHeight += 12; }
    if (hasFan) { activeRows++; totalHeight += 8; }
    
    // Calculate how many pixels of empty space remain on the screen
    // The list now starts at Y=12 for a bit of extra top padding
    int availableSpace = 64 - 12; 
    int gap = 0;
    if (availableSpace > totalHeight && activeRows > 1) {
      gap = (availableSpace - totalHeight) / (activeRows - 1);
    }
    
    // ==========================================
    // DYNAMIC UNIFIED LIST
    // ==========================================
    int curr_y = 12; // Start slightly lower now that the line is gone
    
    // 1. CPU Name & Clock Speed blocks
    display.setCursor(0, curr_y);
    display.print(cpuName);
    
    String clockStr = cpuClock + "G";
    int clkX = 128 - (clockStr.length() * 6);
    display.setCursor(clkX, curr_y);
    display.print(clockStr);
    
    int blockX = (cpuName.length() * 6) + 4; 
    int blockW = clkX - blockX - 4; 
    int numBars = blockW / 4; 
    int activeBars = (clockPct * numBars) / 100;
    
    for (int i = 0; i < numBars; i++) {
      if (i < activeBars) {
         int barH = map(i, 0, numBars - 1, 2, 6); 
         int bX = blockX + (i * 4);
         int bY = curr_y + 7 - barH; 
         display.fillRect(bX, bY, 3, barH, SSD1306_WHITE);
      }
    }
    curr_y += 8 + gap; 
    
    // 2. CPU Temp (Thick Bar)
    drawProperBarRow("CPU Temp", String(cpuT) + "C", cpuT, curr_y, gap); 
    
    // 3. GPU Temp (Thick Bar)
    if (hasGpu) {
      drawProperBarRow("GPU Temp", String(gpuT) + "C", gpuT, curr_y, gap);
    }
    
    // 4. RAM (Thick Bar)
    drawProperBarRow("RAM", String(ram) + "%", ram, curr_y, gap);
    
    // 5. SSD (Text Only)
    drawTextRow("SSD", String(ssdFree) + "GB", curr_y, gap);
    
    // 6. FAN (Text Only)
    if (hasFan) {
      drawTextRow("FAN", String(fan) + " RPM", curr_y, 0); 
    }
    
    display.display();
  }
}