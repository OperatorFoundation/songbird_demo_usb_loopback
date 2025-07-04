// led_manager.ino - LED Level Indication for Songbird
// Part of USB Audio Loopback Demo
//
// This module handles LED brightness control for audio level indication:
// - PWM-based smooth brightness control
// - Audio level to brightness mapping
// - State-dependent LED behavior
// - Smooth transitions and visual feedback

#include "config.h"

// =============================================================================
// LED CONTROL VARIABLES
// =============================================================================
// Current LED brightness values (0-255 PWM range)
int current_led1_brightness = 0;    // Blue LED (input level indicator)
int current_led2_brightness = 0;    // Pink LED (output level indicator)

// Target brightness values for smooth transitions
int target_led1_brightness = 0;
int target_led2_brightness = 0;

// LED update timing
unsigned long last_led_update = 0;

// LED system status
bool led_system_ready = false;

// Smoothing parameters for LED transitions
const float LED_SMOOTHING_FACTOR = 0.15f;  // How quickly LEDs respond to changes (0.0-1.0)

// =============================================================================
// LED INITIALIZATION
// =============================================================================
/**
 * Initialize the LED control system
 * Sets up PWM pins and prepares LEDs for operation
 * Must be called once during system startup
 */
void setupLEDs() {
  Serial.println("Initializing Songbird LED system...");
  
  // Configure LED pins as outputs
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  
  // Initialize LEDs to off state
  analogWrite(LED_1_PIN, 0);
  analogWrite(LED_2_PIN, 0);
  
  // Reset brightness values
  current_led1_brightness = 0;
  current_led2_brightness = 0;
  target_led1_brightness = 0;
  target_led2_brightness = 0;
  
  led_system_ready = true;
  Serial.println("LED system initialized successfully");
  
  // Perform startup LED test
  testLEDs();
}

// =============================================================================
// LED BRIGHTNESS CONTROL
// =============================================================================
/**
 * Update LED brightness based on current system state and audio levels
 * Should be called regularly (10Hz) for smooth transitions
 * @param system_state Current system state (STANDBY, ACTIVE, MUTED)
 */
void updateLEDs(SystemState system_state) {
  // Check if it's time to update LEDs
  if (millis() - last_led_update < LED_UPDATE_INTERVAL_MS) {
    return;  // Too soon for next update
  }
  
  if (!led_system_ready) {
    return;  // LED system not initialized
  }
  
  last_led_update = millis();
  
  // Calculate target brightness based on system state
  calculateTargetBrightness(system_state);
  
  // Apply smoothing to brightness transitions
  applySmoothTransition();
  
  // Update physical LED outputs
  updatePhysicalLEDs();
}

/**
 * Calculate target LED brightness based on system state and audio levels
 * @param system_state Current system state
 */
void calculateTargetBrightness(SystemState system_state) {
  switch (system_state) {
    case STATE_STANDBY:
      // LEDs off in standby mode
      target_led1_brightness = LED_BRIGHTNESS_OFF;
      target_led2_brightness = LED_BRIGHTNESS_OFF;
      break;
      
    case STATE_ACTIVE:
      // Both LEDs respond to audio levels in active mode
      target_led1_brightness = mapLevelToBrightness(getInputLevel());
      target_led2_brightness = mapLevelToBrightness(getOutputLevel());
      break;
      
    case STATE_MUTED:
      // LED1 responds to input, LED2 is off (muted output)
      target_led1_brightness = mapLevelToBrightness(getInputLevel());
      target_led2_brightness = LED_BRIGHTNESS_OFF;
      break;
  }
}

/**
 * Apply smooth transitions between current and target brightness
 * Uses exponential smoothing for natural-looking transitions
 */
void applySmoothTransition() {
  // Apply smoothing to LED1 (Blue - Input Level)
  float led1_diff = target_led1_brightness - current_led1_brightness;
  current_led1_brightness += (int)(led1_diff * LED_SMOOTHING_FACTOR);
  
  // Apply smoothing to LED2 (Pink - Output Level)
  float led2_diff = target_led2_brightness - current_led2_brightness;
  current_led2_brightness += (int)(led2_diff * LED_SMOOTHING_FACTOR);
  
  // Ensure values stay within valid PWM range
  current_led1_brightness = constrain(current_led1_brightness, 0, 255);
  current_led2_brightness = constrain(current_led2_brightness, 0, 255);
}

/**
 * Update the physical LED outputs with current brightness values
 */
void updatePhysicalLEDs() {
  analogWrite(LED_1_PIN, current_led1_brightness);
  analogWrite(LED_2_PIN, current_led2_brightness);
}

// =============================================================================
// AUDIO LEVEL TO BRIGHTNESS MAPPING
// =============================================================================
/**
 * Map audio level (0.0-1.0) to LED brightness (0-255)
 * Uses logarithmic mapping to match human perception of audio levels
 * @param level Audio level from 0.0 to 1.0
 * @return LED brightness value from 0 to 255
 */
int mapLevelToBrightness(float level) {
  // Ensure level is in valid range
  level = constrain(level, 0.0f, 1.0f);
  
  // Apply minimum threshold - very quiet audio doesn't light LEDs
  if (level < LEVEL_THRESHOLD) {
    return LED_BRIGHTNESS_OFF;
  }
  
  // Apply logarithmic scaling to match human perception
  // Audio levels are perceived logarithmically, so we use a power function
  float scaled_level = pow(level, 0.5f);  // Square root gives good perceptual mapping
  
  // Map to LED brightness range with minimum visible brightness
  int brightness = (int)(scaled_level * (LED_BRIGHTNESS_MAX - LED_BRIGHTNESS_MIN) + LED_BRIGHTNESS_MIN);
  
  // Ensure result is within valid PWM range
  return constrain(brightness, LED_BRIGHTNESS_OFF, LED_BRIGHTNESS_MAX);
}

// =============================================================================
// LED CONTROL UTILITY FUNCTIONS
// =============================================================================
/**
 * Set LED brightness directly (bypasses audio level mapping)
 * Useful for testing and special effects
 * @param led1_brightness Brightness for LED1 (0-255)
 * @param led2_brightness Brightness for LED2 (0-255)
 */
void setLEDBrightness(int led1_brightness, int led2_brightness) {
  if (!led_system_ready) return;
  
  target_led1_brightness = constrain(led1_brightness, 0, 255);
  target_led2_brightness = constrain(led2_brightness, 0, 255);
  
  // Apply immediately without smoothing
  current_led1_brightness = target_led1_brightness;
  current_led2_brightness = target_led2_brightness;
  
  updatePhysicalLEDs();
}

/**
 * Turn off all LEDs immediately
 */
void turnOffLEDs() {
  setLEDBrightness(0, 0);
}

/**
 * Turn on all LEDs to maximum brightness
 */
void turnOnLEDs() {
  setLEDBrightness(LED_BRIGHTNESS_MAX, LED_BRIGHTNESS_MAX);
}

/**
 * Get current LED1 brightness
 * @return current brightness value (0-255)
 */
int getLED1Brightness() {
  return current_led1_brightness;
}

/**
 * Get current LED2 brightness
 * @return current brightness value (0-255)
 */
int getLED2Brightness() {
  return current_led2_brightness;
}

/**
 * Check if LED system is ready
 * @return true if LED system initialized successfully
 */
bool isLEDSystemReady() {
  return led_system_ready;
}

// =============================================================================
// LED TESTING AND DIAGNOSTICS
// =============================================================================
/**
 * Test LED functionality
 * Cycles through different brightness levels and patterns
 */
void testLEDs() {
  if (!led_system_ready) return;
  
  Serial.println("Testing LED functionality...");
  
  // Test 1: Both LEDs fade up and down
  Serial.println("LED Test 1: Fade up/down");
  for (int brightness = 0; brightness <= 255; brightness += 5) {
    setLEDBrightness(brightness, brightness);
    delay(20);
  }
  for (int brightness = 255; brightness >= 0; brightness -= 5) {
    setLEDBrightness(brightness, brightness);
    delay(20);
  }
  
  // Test 2: Alternating LEDs
  Serial.println("LED Test 2: Alternating pattern");
  for (int i = 0; i < 5; i++) {
    setLEDBrightness(255, 0);
    delay(200);
    setLEDBrightness(0, 255);
    delay(200);
  }
  
  // Test 3: Different brightness levels
  Serial.println("LED Test 3: Brightness levels");
  int test_levels[] = {32, 64, 128, 192, 255};
  for (int i = 0; i < 5; i++) {
    setLEDBrightness(test_levels[i], test_levels[i]);
    delay(300);
  }
  
  // Return to off state
  turnOffLEDs();
  Serial.println("LED test complete");
}

/**
 * Demonstrate LED response to simulated audio levels
 * Shows how LEDs respond to different audio input levels
 */
void demonstrateLEDResponse() {
  if (!led_system_ready) return;
  
  Serial.println("Demonstrating LED audio response...");
  
  // Simulate various audio levels
  float test_levels[] = {0.0f, 0.1f, 0.2f, 0.4f, 0.6f, 0.8f, 1.0f};
  int num_levels = sizeof(test_levels) / sizeof(test_levels[0]);
  
  for (int i = 0; i < num_levels; i++) {
    float level = test_levels[i];
    int brightness = mapLevelToBrightness(level);
    
    Serial.print("Audio level ");
    Serial.print(level, 2);
    Serial.print(" -> LED brightness ");
    Serial.println(brightness);
    
    setLEDBrightness(brightness, brightness);
    delay(1000);
  }
  
  turnOffLEDs();
  Serial.println("LED response demonstration complete");
}

/**
 * Create a breathing effect on LEDs
 * Smooth fade in/out pattern for visual appeal
 * @param duration_ms Duration of breathing effect in milliseconds
 */
void breathingEffect(unsigned long duration_ms) {
  if (!led_system_ready) return;
  
  unsigned long start_time = millis();
  
  while (millis() - start_time < duration_ms) {
    // Calculate breathing brightness using sine wave
    float time_ratio = (float)(millis() - start_time) / duration_ms;
    float breathing_level = (sin(time_ratio * 2 * PI * 2) + 1.0f) / 2.0f;  // 2 cycles over duration
    
    int brightness = (int)(breathing_level * LED_BRIGHTNESS_MAX);
    setLEDBrightness(brightness, brightness);
    
    delay(50);  // 20 Hz update rate for smooth breathing
  }
  
  turnOffLEDs();
}

/**
 * Display LED system diagnostics
 * Shows current LED states and system information
 */
void printLEDDiagnostics() {
  Serial.println("=== Songbird LED Diagnostics ===");
  Serial.print("LED System Ready: "); Serial.println(led_system_ready ? "YES" : "NO");
  
  Serial.println("\n--- Current LED States ---");
  Serial.print("LED1 (Blue) - Current: "); Serial.print(current_led1_brightness);
  Serial.print(" | Target: "); Serial.println(target_led1_brightness);
  Serial.print("LED2 (Pink) - Current: "); Serial.print(current_led2_brightness);
  Serial.print(" | Target: "); Serial.println(target_led2_brightness);
  
  Serial.println("\n--- Audio Level Mapping ---");
  Serial.print("Input Level: "); Serial.print(getInputLevel(), 3);
  Serial.print(" -> Brightness: "); Serial.println(mapLevelToBrightness(getInputLevel()));
  Serial.print("Output Level: "); Serial.print(getOutputLevel(), 3);
  Serial.print(" -> Brightness: "); Serial.println(mapLevelToBrightness(getOutputLevel()));
  
  Serial.println("==================================\n");
}

// =============================================================================
// SPECIAL LED EFFECTS
// =============================================================================
/**
 * LED startup sequence
 * Attractive power-on animation to show system is ready
 */
void ledStartupSequence() {
  if (!led_system_ready) return;
  
  Serial.println("LED startup sequence");
  
  // Sequence 1: Quick flash both LEDs
  setLEDBrightness(255, 255);
  delay(100);
  setLEDBrightness(0, 0);
  delay(100);
  
  // Sequence 2: Fade in both LEDs
  for (int brightness = 0; brightness <= 128; brightness += 4) {
    setLEDBrightness(brightness, brightness);
    delay(30);
  }
  
  // Sequence 3: Fade out both LEDs
  for (int brightness = 128; brightness >= 0; brightness -= 4) {
    setLEDBrightness(brightness, brightness);
    delay(30);
  }
  
  Serial.println("LED startup sequence complete");
}

/**
 * LED error indication
 * Distinctive pattern to indicate system errors
 */
void ledErrorIndication() {
  if (!led_system_ready) return;
  
  // Rapid alternating flash pattern
  for (int i = 0; i < 6; i++) {
    setLEDBrightness(255, 0);
    delay(150);
    setLEDBrightness(0, 255);
    delay(150);
  }
  
  turnOffLEDs();
}