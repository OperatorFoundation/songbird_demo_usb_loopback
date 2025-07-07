// button_manager.ino - Button Handling and Debouncing for Songbird
// Part of USB Audio Loopback Demo
//
// This module handles all button input processing including:
// - Hardware debouncing for reliable button detection
// - State machine management for system states
// - Non-blocking button scanning
// - Edge detection for button press/release events

#include "config.h"



// =============================================================================
// BUTTON DEFINITIONS
// =============================================================================
// Button information array - makes it easy to add/remove buttons
ButtonInfo buttons[] = {
  {BTN_UP_PIN,    BUTTON_IDLE, BUTTON_IDLE, 0, false, false, "UP"},
  {BTN_DOWN_PIN,  BUTTON_IDLE, BUTTON_IDLE, 0, false, false, "DOWN"},
  {BTN_LEFT_PIN,  BUTTON_IDLE, BUTTON_IDLE, 0, false, false, "LEFT"},
  {BTN_RIGHT_PIN, BUTTON_IDLE, BUTTON_IDLE, 0, false, false, "RIGHT"}
};

const int NUM_BUTTONS = sizeof(buttons) / sizeof(buttons[0]);

// Button scanning timing
unsigned long last_button_scan = 0;

// Button system status
bool button_system_ready = false;

// =============================================================================
// BUTTON INITIALIZATION
// =============================================================================
/**
 * Initialize the button input system
 * Sets up GPIO pins with internal pullups and initializes button states
 * Must be called once during system startup
 */
void setupButtons() {
  Serial.println("Initializing Songbird button system...");
  
  // Configure all button pins as inputs with internal pullups
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    
    // Initialize button states
    buttons[i].current_state = BUTTON_IDLE;
    buttons[i].previous_state = BUTTON_IDLE;
    buttons[i].last_change_time = 0;
    buttons[i].physical_state = false;
    buttons[i].debounced_state = false;
    
    Serial.print("Button ");
    Serial.print(buttons[i].name);
    Serial.print(" configured on pin ");
    Serial.println(buttons[i].pin);
  }
  
  button_system_ready = true;
  Serial.println("Button system initialized successfully");
}

// =============================================================================
// BUTTON SCANNING AND DEBOUNCING
// =============================================================================
/**
 * Scan all buttons and update their states
 * Implements proper debouncing to prevent false triggers
 * Should be called regularly (20Hz) for responsive button feel
 */
void scanButtons() {
  // Check if it's time to scan buttons
  if (millis() - last_button_scan < BUTTON_SCAN_INTERVAL_MS) {
    return;  // Too soon for next scan
  }
  
  if (!button_system_ready) {
    return;  // Button system not initialized
  }
  
  last_button_scan = millis();
  
  // Process each button
  for (int i = 0; i < NUM_BUTTONS; i++) {
    processButton(&buttons[i]);
  }
}

/**
 * Process a single button's state including debouncing
 * @param button Pointer to ButtonInfo structure to process
 */
void processButton(ButtonInfo* button) 
{
  // Read current physical state (buttons are active LOW with pullups)
  bool current_physical = !digitalRead(button->pin);
  
  // Check if physical state has changed
  if (current_physical != button->physical_state) {
    button->physical_state = current_physical;
    button->last_change_time = millis();
  }
  
  // Apply debouncing - only update debounced state after minimum time
  if (millis() - button->last_change_time >= BUTTON_DEBOUNCE_MS) {
    button->debounced_state = button->physical_state;
  }
  
  // Update button state machine
  button->previous_state = button->current_state;
  
  if (button->debounced_state) {
    // Button is pressed
    if (button->current_state == BUTTON_IDLE) {
      button->current_state = BUTTON_PRESSED;
    }
  } else {
    // Button is not pressed
    if (button->current_state == BUTTON_PRESSED) {
      button->current_state = BUTTON_RELEASED;
    } else if (button->current_state == BUTTON_RELEASED) {
      button->current_state = BUTTON_IDLE;
    }
  }
}

// =============================================================================
// BUTTON STATE QUERY FUNCTIONS
// =============================================================================
/**
 * Check if a button was just pressed (press edge detection)
 * @param button_index Index of button to check (0=UP, 1=DOWN, 2=LEFT, 3=RIGHT)
 * @return true if button was just pressed
 */
bool wasButtonPressed(int button_index) {
  if (button_index < 0 || button_index >= NUM_BUTTONS) {
    return false;
  }
  
  return (buttons[button_index].current_state == BUTTON_PRESSED && 
          buttons[button_index].previous_state == BUTTON_IDLE);
}

/**
 * Check if a button was just released (release edge detection)
 * @param button_index Index of button to check
 * @return true if button was just released
 */
bool wasButtonReleased(int button_index) {
  if (button_index < 0 || button_index >= NUM_BUTTONS) {
    return false;
  }
  
  return (buttons[button_index].current_state == BUTTON_RELEASED);
}

/**
 * Check if a button is currently pressed (level detection)
 * @param button_index Index of button to check
 * @return true if button is currently pressed
 */
bool isButtonPressed(int button_index) {
  if (button_index < 0 || button_index >= NUM_BUTTONS) {
    return false;
  }
  
  return (buttons[button_index].current_state == BUTTON_PRESSED);
}

// =============================================================================
// SPECIFIC BUTTON QUERY FUNCTIONS
// =============================================================================
/**
 * Check if UP button was just pressed
 * @return true if UP button was just pressed
 */
bool wasUpButtonPressed() {
  return wasButtonPressed(0);  // UP button is index 0
}

/**
 * Check if DOWN button was just pressed
 * @return true if DOWN button was just pressed
 */
bool wasDownButtonPressed() {
  return wasButtonPressed(1);  // DOWN button is index 1
}

/**
 * Check if LEFT button was just pressed
 * @return true if LEFT button was just pressed
 */
bool wasLeftButtonPressed() {
  return wasButtonPressed(2);  // LEFT button is index 2
}

/**
 * Check if RIGHT button was just pressed
 * @return true if RIGHT button was just pressed
 */
bool wasRightButtonPressed() {
  return wasButtonPressed(3);  // RIGHT button is index 3
}

/**
 * Check if UP button is currently pressed
 * @return true if UP button is currently pressed
 */
bool isUpButtonPressed() {
  return isButtonPressed(0);
}

/**
 * Check if DOWN button is currently pressed
 * @return true if DOWN button is currently pressed
 */
bool isDownButtonPressed() {
  return isButtonPressed(1);
}

// =============================================================================
// BUTTON SYSTEM STATUS FUNCTIONS
// =============================================================================
/**
 * Check if button system is ready
 * @return true if button system initialized successfully
 */
bool isButtonSystemReady() {
  return button_system_ready;
}

/**
 * Get button name for debugging
 * @param button_index Index of button
 * @return pointer to button name string
 */
const char* getButtonName(int button_index) {
  if (button_index < 0 || button_index >= NUM_BUTTONS) {
    return "INVALID";
  }
  return buttons[button_index].name;
}

// =============================================================================
// BUTTON DIAGNOSTICS AND TESTING
// =============================================================================
/**
 * Print current button states to serial port
 * Useful for debugging button input issues
 */
void printButtonStates() {
  Serial.print("Button States: ");
  
  for (int i = 0; i < NUM_BUTTONS; i++) {
    Serial.print(buttons[i].name);
    Serial.print("=");
    
    if (buttons[i].current_state == BUTTON_PRESSED) {
      Serial.print("PRESSED");
    } else if (buttons[i].current_state == BUTTON_RELEASED) {
      Serial.print("RELEASED");
    } else {
      Serial.print("IDLE");
    }
    
    if (i < NUM_BUTTONS - 1) {
      Serial.print(", ");
    }
  }
  
  Serial.println();
}

/**
 * Test button functionality
 * Monitors button presses and reports them to serial
 * @param test_duration_ms Duration of test in milliseconds
 */
void testButtons(unsigned long test_duration_ms) {
  Serial.println("Starting button test...");
  Serial.println("Press buttons to test - results will be displayed");
  
  unsigned long start_time = millis();
  
  while (millis() - start_time < test_duration_ms) {
    // Scan buttons as normal
    scanButtons();
    
    // Report any button presses
    for (int i = 0; i < NUM_BUTTONS; i++) {
      if (wasButtonPressed(i)) {
        Serial.print("Button ");
        Serial.print(buttons[i].name);
        Serial.println(" PRESSED");
      }
      
      if (wasButtonReleased(i)) {
        Serial.print("Button ");
        Serial.print(buttons[i].name);
        Serial.println(" RELEASED");
      }
    }
    
    delay(10);  // Small delay for serial output
  }
  
  Serial.println("Button test complete");
}

/**
 * Print button system diagnostics
 * Shows detailed information about button system state
 */
void printButtonDiagnostics() {
  Serial.println("=== Songbird Button Diagnostics ===");
  Serial.print("Button System Ready: "); Serial.println(button_system_ready ? "YES" : "NO");
  Serial.print("Number of Buttons: "); Serial.println(NUM_BUTTONS);
  
  Serial.println("\n--- Button States ---");
  for (int i = 0; i < NUM_BUTTONS; i++) {
    Serial.print(buttons[i].name);
    Serial.print(" (Pin ");
    Serial.print(buttons[i].pin);
    Serial.print("): ");
    
    Serial.print("Physical=");
    Serial.print(buttons[i].physical_state ? "HIGH" : "LOW");
    Serial.print(", Debounced=");
    Serial.print(buttons[i].debounced_state ? "HIGH" : "LOW");
    Serial.print(", State=");
    
    switch (buttons[i].current_state) {
      case BUTTON_IDLE:
        Serial.print("IDLE");
        break;
      case BUTTON_PRESSED:
        Serial.print("PRESSED");
        break;
      case BUTTON_RELEASED:
        Serial.print("RELEASED");
        break;
    }
    
    Serial.print(", Last Change: ");
    Serial.print(millis() - buttons[i].last_change_time);
    Serial.println("ms ago");
  }
  
  Serial.println("======================================\n");
}

/**
 * Wait for any button press
 * Blocks until a button is pressed, useful for user confirmation
 * @param timeout_ms Maximum time to wait in milliseconds (0 = no timeout)
 * @return true if button was pressed, false if timeout occurred
 */
bool waitForButtonPress(unsigned long timeout_ms = 0) {
  unsigned long start_time = millis();
  
  Serial.println("Waiting for button press...");
  
  while (true) {
    scanButtons();
    
    // Check if any button was pressed
    for (int i = 0; i < NUM_BUTTONS; i++) {
      if (wasButtonPressed(i)) {
        Serial.print("Button ");
        Serial.print(buttons[i].name);
        Serial.println(" pressed!");
        return true;
      }
    }
    
    // Check timeout
    if (timeout_ms > 0 && (millis() - start_time) >= timeout_ms) {
      Serial.println("Button press timeout");
      return false;
    }
    
    delay(10);
  }
}

// =============================================================================
// ADVANCED BUTTON FEATURES
// =============================================================================
/**
 * Check if a button has been held for a specific duration
 * @param button_index Index of button to check
 * @param hold_duration_ms Required hold duration in milliseconds
 * @return true if button has been held for the specified duration
 */
bool isButtonHeld(int button_index, unsigned long hold_duration_ms) {
  if (button_index < 0 || button_index >= NUM_BUTTONS) {
    return false;
  }
  
  if (buttons[button_index].current_state != BUTTON_PRESSED) {
    return false;
  }
  
  // Check if button has been in pressed state long enough
  return (millis() - buttons[button_index].last_change_time) >= hold_duration_ms;
}

/**
 * Reset button states (useful for clearing any stuck states)
 */
void resetButtonStates() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].current_state = BUTTON_IDLE;
    buttons[i].previous_state = BUTTON_IDLE;
    buttons[i].last_change_time = millis();
  }
  
  Serial.println("Button states reset");
}