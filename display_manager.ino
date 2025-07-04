// display_manager.ino - OLED Display and Graphics for Songbird
// Part of USB Audio Loopback Demo
//
// This module handles all display operations including:
// - OLED initialization and control
// - Audio level bar visualization
// - Status message display
// - Sample rate and system state indication

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "config.h"

// =============================================================================
// DISPLAY OBJECT INITIALIZATION
// =============================================================================
// Create display object using Wire1 (secondary I2C bus) to avoid conflicts
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire1, -1);

// =============================================================================
// DISPLAY LAYOUT CONSTANTS
// =============================================================================
// Simple text-based indicators for left and right channels
const char* LEFT_CHANNEL_INDICATOR = "L";
const char* RIGHT_CHANNEL_INDICATOR = "R";

// =============================================================================
// DISPLAY STATE VARIABLES
// =============================================================================
// Variables for managing display timing

// Display timing
unsigned long last_display_update = 0;

// Previous level values for change detection
float prev_input_level = 0.0f;
float prev_output_level = 0.0f;

// Display initialization status
bool display_ready = false;

// =============================================================================
// DISPLAY INITIALIZATION
// =============================================================================
/**
 * Initialize the OLED display system
 * Sets up I2C communication and prepares display for operation
 * Must be called once during system startup
 */
void setupDisplay() {
  Serial.println("Initializing Songbird display system...");
  
  // Initialize Wire1 (secondary I2C bus) for display communication
  Wire1.begin();
  Wire1.setClock(400000);  // 400kHz I2C speed for faster updates
  
  // Initialize the OLED display
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) 
  {
    display.setRotation(2);  // Rotate display 180 degrees

    Serial.println("OLED display initialized successfully");
    
    // Clear display and set initial parameters
    display.clearDisplay();
    display.setTextSize(1);      // Small text for 128x32 display
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    // Display startup message
    display.println("Songbird Audio");
    display.println("Initializing...");
    display.display();
    
    display_ready = true;
    Serial.println("Display system ready");
  } 
  else 
  {
    Serial.println("ERROR: OLED display initialization failed!");
    display_ready = false;
  }
}

// =============================================================================
// CHANNEL INDICATOR RENDERING
// =============================================================================
/**
 * Draw channel indicators (L/R) at specified positions
 * @param x X coordinate for indicator
 * @param y Y coordinate for indicator
 * @param label Channel label ('L' or 'R')
 * @param active true if channel has audio activity
 */
void drawChannelIndicator(int x, int y, char label, bool active) {
  display.setCursor(x, y);
  
  // Draw label with visual indication of activity
  if (active) {
    display.print("[");
    display.print(label);
    display.print("]");
  } else {
    display.print(" ");
    display.print(label);
    display.print(" ");
  }
}

// =============================================================================
// LEVEL BAR VISUALIZATION
// =============================================================================
/**
 * Draw an audio level bar
 * @param x X coordinate for bar start
 * @param y Y coordinate for bar
 * @param level Audio level (0.0 to 1.0)
 * @param label Single character label for the bar
 */
void drawLevelBar(int x, int y, float level, char label) {
  // Calculate number of segments to fill (0 to LEVEL_BAR_SEGMENTS)
  int filled_segments = (int)((level / LEVEL_BAR_MAX) * LEVEL_BAR_SEGMENTS);
  filled_segments = constrain(filled_segments, 0, LEVEL_BAR_SEGMENTS);
  
  // Draw label
  display.setCursor(x, y);
  display.print(label);
  display.print(":");
  
  // Draw opening bracket
  display.print("[");
  
// Draw filled segments
for (int i = 0; i < filled_segments; i++) {
  display.print("#");  // Filled character
}

// Draw empty segments
for (int i = filled_segments; i < LEVEL_BAR_SEGMENTS; i++) {
  display.print("-");  // Empty character
}
  
  // Draw closing bracket
  display.print("]");
}

// =============================================================================
// COMPLETE DISPLAY UPDATE
// =============================================================================
/**
 * Update the complete display with current system state
 * Should be called at regular intervals (10Hz) for smooth updates
 * @param system_state Current system state (STANDBY, ACTIVE, MUTED)
 */
void updateDisplay(SystemState system_state) {
  // Check if it's time to update display
  if (millis() - last_display_update < DISPLAY_UPDATE_INTERVAL_MS) {
    return;  // Too soon for next update
  }
  
  if (!display_ready) {
    return;  // Display not initialized
  }
  
  last_display_update = millis();
  
  // Clear display for new frame
  display.clearDisplay();
  
  // Get current audio levels
  float input_level = getInputLevel();
  float output_level = getOutputLevel();
  
  // Draw display content based on system state
  switch (system_state) {
    case STATE_STANDBY:
      drawStandbyDisplay();
      break;
      
    case STATE_ACTIVE:
      drawActiveDisplay(input_level, output_level);
      break;
      
    case STATE_MUTED:
      drawMutedDisplay(input_level, output_level);
      break;
  }
  
  // Send frame to display
  display.display();
}

// =============================================================================
// STATE-SPECIFIC DISPLAY FUNCTIONS
// =============================================================================
/**
 * Draw display for STANDBY state
 * Shows channel indicators and startup message
 */
void drawStandbyDisplay() {
  // Draw channel indicators (inactive)
  drawChannelIndicator(0, 0, 'L', false);
  drawChannelIndicator(115, 0, 'R', false);
  
  // Draw status message
  display.setCursor(20, 8);
  display.print("Songbird Ready");
  
  display.setCursor(15, 20);
  display.print(STATUS_STANDBY);
}

/**
 * Draw display for ACTIVE state
 * Shows channel indicators, level bars, and system status
 * @param input_level Current input audio level
 * @param output_level Current output audio level
 */
void drawActiveDisplay(float input_level, float output_level) {
  // Draw channel indicators with activity
  bool input_active = input_level > LEVEL_THRESHOLD;
  bool output_active = output_level > LEVEL_THRESHOLD;
  
  drawChannelIndicator(0, 0, 'L', input_active);
  drawChannelIndicator(115, 0, 'R', output_active);
  
  // Draw level bars
  display.setCursor(0, 10);
  drawLevelBar(0, 10, input_level, 'I');   // Input level bar
  
  display.setCursor(0, 18);
  drawLevelBar(0, 18, output_level, 'O');  // Output level bar
  
  // Draw sample rate
  display.setCursor(102, 10);
  display.print(getSampleRateString());
  
  // Draw status message
  display.setCursor(8, 24);
  display.print(STATUS_ACTIVE);
}

/**
 * Draw display for MUTED state
 * Shows input monitoring but indicates output is muted
 * @param input_level Current input audio level
 * @param output_level Current output audio level (should be 0)
 */
void drawMutedDisplay(float input_level, float output_level) {
  // Draw channel indicators - left responds to input, right shows muted
  bool input_active = input_level > LEVEL_THRESHOLD;
  
  drawChannelIndicator(0, 0, 'L', input_active);
  drawChannelIndicator(115, 0, 'R', false);  // Right always inactive when muted
  
  // Draw level bars
  display.setCursor(0, 10);
  drawLevelBar(0, 10, input_level, 'I');   // Input level bar
  
  display.setCursor(0, 18);
  drawLevelBar(0, 18, 0.0f, 'O');          // Output level bar (always zero)
  
  // Draw muted indicator
  display.setCursor(95, 10);
  display.print("MUTE");
  
  // Draw status message
  display.setCursor(15, 24);
  display.print(STATUS_MUTED);
}

// =============================================================================
// DISPLAY UTILITY FUNCTIONS
// =============================================================================
/**
 * Display a simple message on screen
 * Useful for debugging and status messages
 * @param message Text message to display
 */
void displayMessage(const char* message) {
  if (!display_ready) return;
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Songbird");
  display.println();
  display.println(message);
  display.display();
}

/**
 * Display error message
 * Shows error in a consistent format
 * @param error_msg Error message to display
 */
void displayError(const char* error_msg) {
  if (!display_ready) return;
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("ERROR:");
  display.println();
  display.println(error_msg);
  display.display();
}

/**
 * Check if display system is ready
 * @return true if display initialized successfully
 */
bool isDisplayReady() {
  return display_ready;
}

// =============================================================================
// DISPLAY DIAGNOSTICS
// =============================================================================
/**
 * Show system diagnostics on display
 * Useful for troubleshooting and system monitoring
 */
void displayDiagnostics() {
  if (!display_ready) return;
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  
  // Show memory usage
  display.print("Mem: ");
  display.print(AudioMemoryUsage());
  display.print("/");
  display.print(AudioMemoryUsageMax());
  display.println();
  
  // Show CPU usage
  display.print("CPU: ");
  display.print(AudioProcessorUsage(), 0);
  display.print("%");
  display.println();
  
  // Show audio levels
  display.print("In: ");
  display.print(getInputLevel(), 2);
  display.print(" Out: ");
  display.print(getOutputLevel(), 2);
  display.println();
  
  // Show system state
  display.print("USB: ");
  display.print(isUSBAudioConnected() ? "OK" : "NO");
  
  display.display();
}

/**
 * Test display functionality
 * Cycles through different display elements for testing
 */
void testDisplay() {
  if (!display_ready) return;
  
  Serial.println("Testing display functionality...");
  
  // Test 1: Basic text
  displayMessage("Display Test 1");
  delay(1000);
  
  // Test 2: Channel indicators
  display.clearDisplay();
  drawChannelIndicator(10, 5, 'L', false);   // Quiet
  drawChannelIndicator(50, 5, 'L', true);    // Active
  drawChannelIndicator(10, 15, 'R', false);  // Quiet
  drawChannelIndicator(50, 15, 'R', true);   // Active
  display.display();
  delay(1000);
  
  // Test 3: Level bars
  display.clearDisplay();
  drawLevelBar(10, 5, 0.25f, 'T');
  drawLevelBar(10, 15, 0.75f, 'T');
  display.display();
  delay(1000);
  
  // Test 4: Complete layout
  display.clearDisplay();
  drawChannelIndicator(0, 0, 'L', true);
  drawChannelIndicator(115, 0, 'R', false);
  drawLevelBar(0, 10, 0.5f, 'I');
  display.setCursor(0, 25);
  display.print("Test Complete");
  display.display();
  delay(1000);
  
  Serial.println("Display test complete");
}