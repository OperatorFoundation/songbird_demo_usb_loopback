// usb_loopback_demo.ino - Main Arduino File for Songbird USB Audio Loopback Demo
// Part of USB Audio Loopback Demo
//
// The Songbird device demonstrates real-time USB audio processing with:
// - Direct audio loopback from USB input to USB output
// - Visual feedback via OLED display and LEDs
// - Interactive controls via buttons
// - Professional audio processing using Teensy Audio Library
//
// Hardware Requirements:
// - Songbird device (Teensy 4.0 + Audio Shield + custom hardware)
// - USB-C connection to host computer
// - Built-in OLED display, buttons, and LEDs
//
// Educational Purpose:
// This demo showcases embedded audio processing concepts including:
// - Real-time audio buffering and routing
// - RMS level detection and visualization
// - State machine design for audio applications

// =============================================================================
// SYSTEM INCLUDES
// =============================================================================
// Standard Arduino and Teensy libraries
#include <Arduino.h>

// Audio processing libraries
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// Display libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"

// =============================================================================
// GLOBAL SYSTEM STATE
// =============================================================================
// Current system state - controls overall behavior
SystemState current_system_state = STATE_STANDBY;
SystemState previous_system_state = STATE_STANDBY;

// =============================================================================
// SYSTEM TIMING CONSTANTS
// =============================================================================
// Main loop timing - all updates synchronized to this base rate
const unsigned long MAIN_LOOP_INTERVAL_MS = 20;    // 50 Hz main loop

// Display update timing - prevents excessive OLED refresh
const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 100;  // 10 Hz display updates

// Button scanning timing - fast enough for responsive feel
const unsigned long BUTTON_SCAN_INTERVAL_MS = 50;     // 20 Hz button scanning

// LED update timing - matches display for smooth visual feedback
const unsigned long LED_UPDATE_INTERVAL_MS = 100;     // 10 Hz LED updates

// Button debouncing - prevents false triggers from mechanical contacts
const unsigned long BUTTON_DEBOUNCE_MS = 50;          // 50ms debounce time

// Timing variables for main loop control
unsigned long last_main_loop = 0;
unsigned long system_start_time = 0;

// System status flags
bool system_initialized = false;
bool system_error = false;
String last_error_message = "";

// =============================================================================
// ARDUINO SETUP FUNCTION
// =============================================================================
/**
 * Arduino setup function - called once at startup
 * Initializes all system components and prepares for operation
 */
void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  
  // Wait a moment for serial connection (useful for debugging)
  delay(1000);
  
  Serial.println("========================================");
  Serial.println("Songbird USB Audio Loopback Demo");
  Serial.println("Initializing system...");
  Serial.println("========================================");
  
  // Record system start time
  system_start_time = millis();
  
  // Initialize all system components in order
  if (initializeSystem()) {
    system_initialized = true;
    current_system_state = STATE_STANDBY;
    
    Serial.println("========================================");
    Serial.println("System initialization complete!");
    Serial.println("Songbird ready for operation");
    Serial.println("========================================");
    
    // Show welcome message on display
    displayMessage("Songbird Ready!");
    delay(2000);
    
    // Perform LED startup sequence
    ledStartupSequence();
    
  } else {
    system_error = true;
    Serial.println("========================================");
    Serial.println("ERROR: System initialization failed!");
    Serial.println("Check hardware connections and try again");
    Serial.println("========================================");
    
    // Show error on display if possible
    displayError("Init Failed");
    
    // Error indication with LEDs
    ledErrorIndication();
  }
  
  Serial.println("Starting main loop...");
}

// =============================================================================
// ARDUINO MAIN LOOP
// =============================================================================
/**
 * Arduino main loop function - called continuously
 * Implements the main system state machine and coordinates all components
 */
void loop() {
  // Check if it's time for main loop update
  if (millis() - last_main_loop < MAIN_LOOP_INTERVAL_MS) {
    return;  // Too soon for next update
  }
  
  last_main_loop = millis();
  
  // Exit early if system not initialized or in error state
  if (!system_initialized || system_error) {
    handleSystemError();
    return;
  }
  
  // Process all input/output components
  processInputs();
  processSystemState();
  updateOutputs();
  
  // Periodic diagnostics (every 10 seconds)
  static unsigned long last_diagnostics = 0;
  if (millis() - last_diagnostics > 10000) {
    last_diagnostics = millis();
    printSystemDiagnostics();
  }
}

// =============================================================================
// SYSTEM INITIALIZATION
// =============================================================================
/**
 * Initialize all system components
 * @return true if all components initialized successfully
 */
bool initializeSystem() {
  bool all_successful = true;
  
  Serial.println("Initializing system components...");
  
  // Initialize configuration (pin assignments, constants)
  Serial.println("Loading configuration...");
  // Configuration is loaded from config.ino automatically
  
  // Initialize audio system
  Serial.println("Initializing audio system...");
  setupAudio();
  if (!isAudioSystemReady()) {
    Serial.println("ERROR: Audio system initialization failed");
    all_successful = false;
  }
  
  // Initialize display system
  Serial.println("Initializing display system...");
  setupDisplay();
  if (!isDisplayReady()) {
    Serial.println("ERROR: Display system initialization failed");
    all_successful = false;
  }
  
  // Initialize LED system
  Serial.println("Initializing LED system...");
  setupLEDs();
  if (!isLEDSystemReady()) {
    Serial.println("ERROR: LED system initialization failed");
    all_successful = false;
  }
  
  // Initialize button system
  Serial.println("Initializing button system...");
  setupButtons();
  if (!isButtonSystemReady()) {
    Serial.println("ERROR: Button system initialization failed");
    all_successful = false;
  }
  
  // Test all systems if initialization was successful
  if (all_successful) {
    Serial.println("Running system tests...");
    
    // Brief display test
    displayMessage("Testing...");
    delay(500);
    
    // Brief LED test
    setLEDBrightness(128, 128);
    delay(500);
    setLEDBrightness(0, 0);
    
    Serial.println("System tests complete");
  }
  
  return all_successful;
}

// =============================================================================
// INPUT PROCESSING
// =============================================================================
/**
 * Process all system inputs (buttons, audio levels, USB status)
 */
void processInputs() {
  // Scan buttons for state changes
  scanButtons();
  
  // Update audio level measurements
  updateAudioLevels();
  
  // Check for button presses and handle state transitions
  handleButtonPresses();
}

/**
 * Handle button press events and state transitions
 */
void handleButtonPresses() {
  // UP button: Toggle between STANDBY and ACTIVE states
  if (wasUpButtonPressed()) {
    Serial.println("UP button pressed");
    
    if (current_system_state == STATE_STANDBY) {
      // Transition to ACTIVE state
      current_system_state = STATE_ACTIVE;
      Serial.println("State: STANDBY -> ACTIVE");
    } else if (current_system_state == STATE_ACTIVE || current_system_state == STATE_MUTED) {
      // Transition to STANDBY state
      current_system_state = STATE_STANDBY;
      Serial.println("State: -> STANDBY");
    }
  }
  
  // DOWN button: Toggle MUTED state (only when active)
  if (wasDownButtonPressed()) {
    Serial.println("DOWN button pressed");
    
    if (current_system_state == STATE_ACTIVE) {
      // Transition to MUTED state
      current_system_state = STATE_MUTED;
      Serial.println("State: ACTIVE -> MUTED");
    } else if (current_system_state == STATE_MUTED) {
      // Transition back to ACTIVE state
      current_system_state = STATE_ACTIVE;
      Serial.println("State: MUTED -> ACTIVE");
    }
  }
  
  // LEFT and RIGHT buttons reserved for future use
  if (wasLeftButtonPressed()) {
    Serial.println("LEFT button pressed (reserved)");
  }
  
  if (wasRightButtonPressed()) {
    Serial.println("RIGHT button pressed (reserved)");
  }
}

// =============================================================================
// STATE MACHINE PROCESSING
// =============================================================================
/**
 * Process system state machine and handle state transitions
 */
void processSystemState() {
  // Handle state entry actions (executed once when entering a state)
  if (current_system_state != previous_system_state) {
    handleStateEntry(current_system_state);
    previous_system_state = current_system_state;
  }
  
  // Handle state-specific processing (executed continuously while in state)
  handleStateProcessing(current_system_state);
}

/**
 * Handle actions when entering a new state
 * @param new_state The state being entered
 */
void handleStateEntry(SystemState new_state) {
  Serial.print("Entering state: ");
  
  switch (new_state) {
    case STATE_STANDBY:
      Serial.println("STANDBY");
      // Disable audio processing
      setAudioMuted(true);
      break;
      
    case STATE_ACTIVE:
      Serial.println("ACTIVE");
      // Enable audio processing
      setAudioMuted(false);
      break;
      
    case STATE_MUTED:
      Serial.println("MUTED");
      // Mute audio output but keep processing for display
      setAudioMuted(true);
      break;
  }
}

/**
 * Handle continuous processing for current state
 * @param current_state The current system state
 */
void handleStateProcessing(SystemState current_state) {
  // State-specific processing can be added here
  // For now, most processing is handled in updateOutputs()
  
  switch (current_state) {
    case STATE_STANDBY:
      // In standby, we might want to check for USB connection
      // and automatically transition to active when audio is detected
      if (isUSBAudioConnected() && getInputLevel() > LEVEL_THRESHOLD) {
        // Auto-transition to active when audio is detected
        // (This is optional - can be disabled for manual control only)
        // current_system_state = STATE_ACTIVE;
      }
      break;
      
    case STATE_ACTIVE:
      // In active state, monitor for loss of USB connection
      if (!isUSBAudioConnected()) {
        // Could auto-transition to standby if USB is disconnected
        // (This is optional - depends on desired behavior)
        // current_system_state = STATE_STANDBY;
      }
      break;
      
    case STATE_MUTED:
      // In muted state, continue processing input for display
      // but output remains muted
      break;
  }
}

// =============================================================================
// OUTPUT PROCESSING
// =============================================================================
/**
 * Update all system outputs (display, LEDs, audio routing)
 */
void updateOutputs() {
  // Update display with current state and audio levels
  updateDisplay(current_system_state);
  
  // Update LED brightness based on audio levels and state
  updateLEDs(current_system_state);
  
  // Audio routing is handled automatically by the audio library
  // and state-specific muting in handleStateEntry()
}

// =============================================================================
// ERROR HANDLING
// =============================================================================
/**
 * Handle system error conditions
 */
void handleSystemError() {
  // Flash error indication on LEDs
  static unsigned long last_error_flash = 0;
  if (millis() - last_error_flash > 500) {
    last_error_flash = millis();
    ledErrorIndication();
  }
  
  // Show error message on display
  displayError(last_error_message.c_str());
  
  // Print error information to serial
  static unsigned long last_error_print = 0;
  if (millis() - last_error_print > 5000) {
    last_error_print = millis();
    Serial.println("SYSTEM ERROR - Check hardware and restart");
  }
}

/**
 * Set system error state with message
 * @param error_message Description of the error
 */
void setSystemError(const String& error_message) {
  system_error = true;
  last_error_message = error_message;
  
  Serial.print("SYSTEM ERROR: ");
  Serial.println(error_message);
}

// =============================================================================
// SYSTEM DIAGNOSTICS
// =============================================================================
/**
 * Print comprehensive system diagnostics
 */
void printSystemDiagnostics() {
  Serial.println("\n=== SONGBIRD SYSTEM DIAGNOSTICS ===");
  
  // System status
  Serial.print("System Initialized: "); Serial.println(system_initialized ? "YES" : "NO");
  Serial.print("System Error: "); Serial.println(system_error ? "YES" : "NO");
  Serial.print("Current State: ");
  switch (current_system_state) {
    case STATE_STANDBY: Serial.println("STANDBY"); break;
    case STATE_ACTIVE: Serial.println("ACTIVE"); break;
    case STATE_MUTED: Serial.println("MUTED"); break;
  }
  
  // Timing information
  Serial.print("System Uptime: "); Serial.print((millis() - system_start_time) / 1000); Serial.println(" seconds");
  Serial.print("Main Loop Rate: "); Serial.print(1000.0 / MAIN_LOOP_INTERVAL_MS); Serial.println(" Hz");
  
  // Component status
  Serial.print("Audio System Ready: "); Serial.println(isAudioSystemReady() ? "YES" : "NO");
  Serial.print("Display Ready: "); Serial.println(isDisplayReady() ? "YES" : "NO");
  Serial.print("LED System Ready: "); Serial.println(isLEDSystemReady() ? "YES" : "NO");
  Serial.print("Button System Ready: "); Serial.println(isButtonSystemReady() ? "YES" : "NO");
  
  // Print detailed diagnostics from each component
  printAudioDiagnostics();
  printLEDDiagnostics();
  printButtonDiagnostics();
  
  Serial.println("=====================================\n");
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================
/**
 * Get current system state as string
 * @return String representation of current state
 */
String getSystemStateString() {
  switch (current_system_state) {
    case STATE_STANDBY: return "STANDBY";
    case STATE_ACTIVE: return "ACTIVE";
    case STATE_MUTED: return "MUTED";
    default: return "UNKNOWN";
  }
}

/**
 * Get system uptime in seconds
 * @return System uptime in seconds
 */
unsigned long getSystemUptime() {
  return (millis() - system_start_time) / 1000;
}

/**
 * Check if system is ready for operation
 * @return true if all systems are ready
 */
bool isSystemReady() {
  return system_initialized && !system_error &&
         isAudioSystemReady() && isDisplayReady() && 
         isLEDSystemReady() && isButtonSystemReady();
}

// =============================================================================
// DEVELOPMENT AND TESTING FUNCTIONS
// =============================================================================
/**
 * Run comprehensive system tests
 * Useful for hardware verification and troubleshooting
 */
void runSystemTests() {
  Serial.println("Running comprehensive system tests...");
  
  // Test display
  Serial.println("Testing display system...");
  testDisplay();
  
  // Test LEDs
  Serial.println("Testing LED system...");
  testLEDs();
  
  // Test buttons
  Serial.println("Testing button system...");
  testButtons(5000);  // 5 second button test
  
  // Test audio (basic verification)
  Serial.println("Testing audio system...");
  printAudioDiagnostics();
  
  Serial.println("System tests complete!");
}

/**
 * Enter diagnostic mode
 * Provides interactive system testing and monitoring
 */
void enterDiagnosticMode() {
  Serial.println("Entering diagnostic mode...");
  Serial.println("Press any button to exit diagnostic mode");
  
  displayMessage("Diagnostic Mode");
  
  while (true) {
    // Update all systems
    scanButtons();
    updateAudioLevels();
    
    // Show diagnostics on display
    displayDiagnostics();
    
    // Check for button press to exit
    for (int i = 0; i < 4; i++) {
      if (wasButtonPressed(i)) {
        Serial.println("Exiting diagnostic mode");
        return;
      }
    }
    
    // Print diagnostics every 2 seconds
    static unsigned long last_diag_print = 0;
    if (millis() - last_diag_print > 2000) {
      last_diag_print = millis();
      printSystemDiagnostics();
    }
    
    delay(100);
  }
}

/**
 * Check for special button combinations at startup
 * Allows entering test modes for hardware verification
 */
void checkStartupButtonCombinations() {
  Serial.println("Checking for startup button combinations...");
  
  // Scan buttons
  scanButtons();
  
  // Check for test mode combinations
  if (isButtonPressed(0) && isButtonPressed(1)) {
    // UP + DOWN = Run system tests
    Serial.println("Test mode detected - running system tests");
    runSystemTests();
  } else if (isButtonPressed(2) && isButtonPressed(3)) {
    // LEFT + RIGHT = Enter diagnostic mode
    Serial.println("Diagnostic mode detected");
    enterDiagnosticMode();
  }
}