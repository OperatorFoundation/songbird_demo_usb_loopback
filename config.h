// config.ino - Pin Definitions and Constants for Songbird Hardware
// Part of USB Audio Loopback Demo
// 
// This file contains all hardware-specific pin assignments and system constants for Songbird.

#ifndef CONFIG_H
#define CONFIG_H

// =============================================================================
// AUDIO INTERFACE PINS
// =============================================================================
// These pins are used by the Teensy Audio Library for I2S communication
// with the SGTL5000 audio codec on the Audio Shield
#define I2S_DIN_PIN      7    // Audio data input from codec
#define I2S_DOUT_PIN     8    // Audio data output to codec  
#define I2S_SCLK_PIN     21   // Audio serial clock (bit clock)
#define I2S_LRCLK_PIN    20   // Audio word select (L/R clock)
#define SYS_MCLK_PIN     23   // Master clock for audio system

// =============================================================================
// AUDIO SHIELD CONTROL PINS
// =============================================================================
// These pins control the headphone amplifier on the Audio Shield
// Required for proper Songbird hardware initialization
#define HPAMP_VOL_CLK    0    // Headphone amp volume control clock
#define HPAMP_VOL_UD     1    // Headphone amp volume up/down control
#define HPAMP_SHUTDOWN   2    // Headphone amp shutdown control

// =============================================================================
// USER INTERFACE PINS
// =============================================================================
// Button inputs - active LOW with internal pullups
#define BTN_UP_PIN       5    // Top button: Toggle STANDBY/ACTIVE states
#define BTN_DOWN_PIN     4    // Bottom button: Toggle MUTED state
#define BTN_LEFT_PIN     6    // Left button: Reserved for future use
#define BTN_RIGHT_PIN    3    // Right button: Reserved for future use

// LED outputs - PWM capable pins for brightness control
#define LED_1_PIN        14   // Blue LED: Input level indicator
#define LED_2_PIN        15   // Pink LED: Output level indicator

// =============================================================================
// DISPLAY CONFIGURATION
// =============================================================================
// OLED display on secondary I2C bus (Wire1) to avoid conflicts
#define OLED_SDA_PIN     17   // I2C data line (Wire1)
#define OLED_SCL_PIN     16   // I2C clock line (Wire1)
#define OLED_I2C_ADDR    0x3C // Standard SSD1306 I2C address
#define OLED_WIDTH       128  // Display width in pixels
#define OLED_HEIGHT      32   // Display height in pixels

// =============================================================================
// SYSTEM STATE DEFINITIONS
// =============================================================================
// Main system states for the Songbird audio processor
enum SystemState {
  STATE_STANDBY,    // System ready, no audio processing, LEDs off
  STATE_ACTIVE,     // Audio loopback enabled, full functionality
  STATE_MUTED       // Input processed for display, output muted
};

// =============================================================================
// AUDIO PROCESSING CONSTANTS
// =============================================================================
// Audio memory allocation - number of 128-sample blocks
const int AUDIO_MEMORY_BLOCKS = 32;

// Level detection parameters
const float LEVEL_THRESHOLD = 0.01f;    // Minimum level to trigger "singing" birds
const float LEVEL_SMOOTHING = 0.1f;     // Smoothing factor for level meters (0.0-1.0)

// Level bar display mapping
const int LEVEL_BAR_SEGMENTS = 8;        // Number of segments in level bar [████----]
const float LEVEL_BAR_MAX = 0.8f;       // Maximum level for full bar (allows headroom)

// =============================================================================
// DISPLAY LAYOUT CONSTANTS
// =============================================================================
// Character positions for display elements on 128x32 OLED
const int BIRD_LEFT_X = 0;       // Left bird sprite X position
const int BIRD_RIGHT_X = 120;    // Right bird sprite X position
const int BIRD_Y = 0;            // Bird sprite Y position
const int BIRD_WIDTH = 8;        // Bird sprite width in pixels
const int BIRD_HEIGHT = 8;       // Bird sprite height in pixels

// Level bar positions
const int LEVEL_BAR_Y = 8;       // Level bar Y position
const int LEVEL_BAR_SPACING = 64;// Space between input and output bars

// Status text position
const int STATUS_TEXT_Y = 24;    // Status message Y position

// =============================================================================
// LED BRIGHTNESS CONSTANTS
// =============================================================================
// PWM values for LED brightness control (0-255)
const int LED_BRIGHTNESS_OFF = 0;     // LEDs completely off
const int LED_BRIGHTNESS_MAX = 255;   // Maximum LED brightness
const int LED_BRIGHTNESS_MIN = 8;     // Minimum visible brightness

// =============================================================================
// BUTTON STATE DEFINITIONS
// =============================================================================
// Button states for debouncing and edge detection
enum ButtonState {
  BUTTON_IDLE,      // Button not pressed
  BUTTON_PRESSED,   // Button currently pressed
  BUTTON_RELEASED   // Button just released (trigger state)
};

// =============================================================================
// BUTTON STATE STRUCTURES
// =============================================================================
// Structure to track individual button states
struct ButtonInfo {
  int pin;                          // GPIO pin number
  ButtonState current_state;        // Current button state
  ButtonState previous_state;       // Previous button state for edge detection
  unsigned long last_change_time;   // Time of last state change (for debouncing)
  bool physical_state;              // Raw pin reading (true = pressed)
  bool debounced_state;             // Debounced button state
  const char* name;                 // Button name for debugging
};

// =============================================================================
// AUDIO SAMPLE RATE DEFINITIONS
// =============================================================================
// Supported sample rates for display - actual rate determined by USB host
const char* SAMPLE_RATE_44K = "44k";
const char* SAMPLE_RATE_48K = "48k";
const char* SAMPLE_RATE_UNKNOWN = "??k";

// =============================================================================
// STATUS MESSAGE STRINGS
// =============================================================================
// Pre-defined status messages for display
const char* STATUS_STANDBY = "Press UP to start";
const char* STATUS_ACTIVE = "USB Loopback Active";
const char* STATUS_MUTED = "Output Muted";
const char* STATUS_NO_USB = "Connect USB Audio";

#endif // CONFIG_H