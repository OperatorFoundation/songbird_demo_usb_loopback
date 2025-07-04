// audio_manager.ino - Audio Processing and Routing for Songbird
// Part of USB Audio Loopback Demo
//
// This module handles all audio processing including:
// - USB audio input/output routing
// - Real-time level detection for visual feedback
// - Audio codec initialization and control
// - Sample rate detection and monitoring

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "config.h"

// =============================================================================
// AUDIO LIBRARY OBJECTS
// =============================================================================
// These objects define the audio processing chain using the Teensy Audio Library
// The library uses a node-based system where audio flows between connected objects

// USB Audio Interface - handles communication with host computer
AudioInputUSB        usb_audio_input;   // Receives audio from USB host
AudioOutputUSB       usb_audio_output;  // Sends audio to USB host

// Audio Shield Interface - handles physical audio I/O (headphones, line out)
AudioInputI2S        i2s_audio_input;   // Physical audio input (not used in this demo)
AudioOutputI2S       i2s_audio_output;  // Physical audio output (headphones/line out)

// Level Analysis - measures audio levels for visual feedback
AudioAnalyzeRMS      rms_input_left;    // RMS analysis for left input channel
AudioAnalyzeRMS      rms_input_right;   // RMS analysis for right input channel
AudioAnalyzeRMS      rms_output_left;   // RMS analysis for left output channel  
AudioAnalyzeRMS      rms_output_right;  // RMS analysis for right output channel

// Audio Mixer - allows muting functionality
AudioMixer4          mixer_left;        // Left channel mixer for mute control
AudioMixer4          mixer_right;       // Right channel mixer for mute control

// Audio Shield Control - interfaces with SGTL5000 codec chip
AudioControlSGTL5000 audio_shield;      // Control interface for audio codec

// =============================================================================
// AUDIO CONNECTIONS
// =============================================================================
// These connections define how audio flows between processing objects
// The Teensy Audio Library uses patch cords to connect inputs to outputs

// USB Input to Level Detection - monitor input levels for display
AudioConnection patch_cord_01(usb_audio_input, 0, rms_input_left, 0);   // Left input to RMS
AudioConnection patch_cord_02(usb_audio_input, 1, rms_input_right, 0);  // Right input to RMS

// USB Input to Mixer - allows muting control
AudioConnection patch_cord_03(usb_audio_input, 0, mixer_left, 0);       // Left input to mixer
AudioConnection patch_cord_04(usb_audio_input, 1, mixer_right, 0);      // Right input to mixer

// Mixer to USB Output - the main loopback path
AudioConnection patch_cord_05(mixer_left, 0, usb_audio_output, 0);      // Left mixer to USB out
AudioConnection patch_cord_06(mixer_right, 0, usb_audio_output, 1);     // Right mixer to USB out

// Mixer to I2S Output - physical audio output (headphones)
AudioConnection patch_cord_07(mixer_left, 0, i2s_audio_output, 0);      // Left mixer to I2S out
AudioConnection patch_cord_08(mixer_right, 0, i2s_audio_output, 1);     // Right mixer to I2S out

// USB Output to Level Detection - monitor output levels for display
AudioConnection patch_cord_09(mixer_left, 0, rms_output_left, 0);       // Left output to RMS
AudioConnection patch_cord_10(mixer_right, 0, rms_output_right, 0);     // Right output to RMS

// =============================================================================
// AUDIO PROCESSING VARIABLES
// =============================================================================
// Global variables for audio level monitoring and control

// Current audio levels (0.0 to 1.0 range)
float current_input_left_level = 0.0f;
float current_input_right_level = 0.0f;
float current_output_left_level = 0.0f;
float current_output_right_level = 0.0f;

// Smoothed levels for display (reduces visual flickering)
float smoothed_input_left_level = 0.0f;
float smoothed_input_right_level = 0.0f;
float smoothed_output_left_level = 0.0f;
float smoothed_output_right_level = 0.0f;

// Audio system status
bool audio_system_ready = false;
bool usb_audio_connected = false;
float current_sample_rate = 0.0f;

// =============================================================================
// AUDIO INITIALIZATION
// =============================================================================
/**
 * Initialize the audio processing system
 * Sets up audio codec, allocates memory, and configures audio routing
 * Must be called once during system startup
 */
void setupAudio() {
  Serial.println("Initializing Songbird audio system...");
  
  // Allocate audio memory - needed for audio library buffer management
  // 32 blocks provides good balance between memory usage and audio quality
  AudioMemory(AUDIO_MEMORY_BLOCKS);
  
  // Initialize audio shield control pins
  // These pins control the headphone amplifier on the audio shield
  pinMode(HPAMP_VOL_CLK, OUTPUT);
  pinMode(HPAMP_VOL_UD, OUTPUT);
  pinMode(HPAMP_SHUTDOWN, OUTPUT);
  
  // Start with headphone amp enabled
  digitalWrite(HPAMP_SHUTDOWN, HIGH);  // HIGH = amp enabled
  digitalWrite(HPAMP_VOL_CLK, LOW);
  digitalWrite(HPAMP_VOL_UD, LOW);
  
  // Initialize the audio shield codec
  if (audio_shield.enable()) {
    Serial.println("Audio shield initialized successfully");
    
    // Configure audio shield settings
    audio_shield.inputSelect(AUDIO_INPUT_LINEIN);  // Set input source
    audio_shield.volume(0.8f);                     // Set headphone volume (0.0-1.0)
    audio_shield.lineInLevel(5);                   // Set line input level (0-15)
    audio_shield.lineOutLevel(13);                 // Set line output level (13-31)
    
    // Configure mixers for unity gain (1.0 = no attenuation)
    mixer_left.gain(0, 1.0f);   // Channel 0 (USB input) at unity gain
    mixer_right.gain(0, 1.0f);  // Channel 0 (USB input) at unity gain
    
    // Disable unused mixer channels
    mixer_left.gain(1, 0.0f);   // Channel 1 unused
    mixer_left.gain(2, 0.0f);   // Channel 2 unused
    mixer_left.gain(3, 0.0f);   // Channel 3 unused
    mixer_right.gain(1, 0.0f);  // Channel 1 unused
    mixer_right.gain(2, 0.0f);  // Channel 2 unused
    mixer_right.gain(3, 0.0f);  // Channel 3 unused
    
    audio_system_ready = true;
    Serial.println("Audio processing chain configured");
  } else {
    Serial.println("ERROR: Audio shield initialization failed!");
    audio_system_ready = false;
  }
  
  Serial.println("Audio system initialization complete");
}

// =============================================================================
// AUDIO LEVEL MONITORING
// =============================================================================
/**
 * Update audio level measurements
 * Reads RMS values from analyzers and applies smoothing
 * Should be called regularly (10Hz) for smooth level display
 */
void updateAudioLevels() {
  // Check if RMS analyzers have new data available
  if (rms_input_left.available()) {
    current_input_left_level = rms_input_left.read();
  }
  if (rms_input_right.available()) {
    current_input_right_level = rms_input_right.read();
  }
  if (rms_output_left.available()) {
    current_output_left_level = rms_output_left.read();
  }
  if (rms_output_right.available()) {
    current_output_right_level = rms_output_right.read();
  }
  
  // Apply smoothing to reduce visual flickering
  // Uses exponential smoothing: new_value = old_value * (1-factor) + current * factor
  smoothed_input_left_level = smoothed_input_left_level * (1.0f - LEVEL_SMOOTHING) + 
                             current_input_left_level * LEVEL_SMOOTHING;
  smoothed_input_right_level = smoothed_input_right_level * (1.0f - LEVEL_SMOOTHING) + 
                              current_input_right_level * LEVEL_SMOOTHING;
  smoothed_output_left_level = smoothed_output_left_level * (1.0f - LEVEL_SMOOTHING) + 
                              current_output_left_level * LEVEL_SMOOTHING;
  smoothed_output_right_level = smoothed_output_right_level * (1.0f - LEVEL_SMOOTHING) + 
                               current_output_right_level * LEVEL_SMOOTHING;
}

// =============================================================================
// AUDIO CONTROL FUNCTIONS
// =============================================================================
/**
 * Set mute state for audio output
 * When muted, input is still processed for level display but output is silent
 * @param muted true to mute output, false to enable output
 */
void setAudioMuted(bool muted) {
  if (muted) {
    // Mute by setting mixer gain to zero
    mixer_left.gain(0, 0.0f);
    mixer_right.gain(0, 0.0f);
    Serial.println("Audio output muted");
  } else {
    // Unmute by restoring unity gain
    mixer_left.gain(0, 1.0f);
    mixer_right.gain(0, 1.0f);
    Serial.println("Audio output enabled");
  }
}

/**
 * Check if USB audio is connected and active
 * @return true if USB audio connection is active
 */
bool isUSBAudioConnected() {
  // The Teensy Audio Library doesn't provide a direct way to check USB audio connection
  // We infer connection status by checking if we're receiving audio data
  // This is a simplified approach - in production, more sophisticated detection might be used
  
  static unsigned long last_activity_check = 0;
  static bool previous_connection_state = false;
  
  // Check connection status every second to avoid excessive processing
  if (millis() - last_activity_check > 1000) {
    last_activity_check = millis();
    
    // Consider USB connected if we have any significant audio input
    bool has_audio_activity = (smoothed_input_left_level > 0.001f || 
                              smoothed_input_right_level > 0.001f);
    
    // Update connection status (with some hysteresis to prevent flickering)
    if (has_audio_activity && !previous_connection_state) {
      usb_audio_connected = true;
      Serial.println("USB audio connection detected");
    } else if (!has_audio_activity && previous_connection_state) {
      // Wait a bit longer before declaring disconnection
      static unsigned long no_activity_start = 0;
      if (no_activity_start == 0) {
        no_activity_start = millis();
      } else if (millis() - no_activity_start > 5000) {  // 5 second timeout
        usb_audio_connected = false;
        no_activity_start = 0;
        Serial.println("USB audio connection lost");
      }
    } else {
      // Reset timeout if we have activity
      if (has_audio_activity) {
        static unsigned long no_activity_start = 0;
        no_activity_start = 0;
      }
    }
    
    previous_connection_state = usb_audio_connected;
  }
  
  return usb_audio_connected;
}

/**
 * Get current sample rate string for display
 * @return pointer to sample rate string (e.g., "44k", "48k", "??k")
 */
const char* getSampleRateString() {
  // The Teensy Audio Library doesn't provide direct sample rate detection
  // In a real implementation, this could be detected through USB descriptor analysis
  // For this demo, we'll return a default value
  
  // TODO: Implement actual sample rate detection if needed
  return SAMPLE_RATE_48K;  // Most common for USB audio
}

// =============================================================================
// AUDIO LEVEL ACCESSOR FUNCTIONS
// =============================================================================
/**
 * Get smoothed input level for left channel
 * @return level value from 0.0 to 1.0
 */
float getInputLeftLevel() {
  return smoothed_input_left_level;
}

/**
 * Get smoothed input level for right channel
 * @return level value from 0.0 to 1.0
 */
float getInputRightLevel() {
  return smoothed_input_right_level;
}

/**
 * Get smoothed output level for left channel
 * @return level value from 0.0 to 1.0
 */
float getOutputLeftLevel() {
  return smoothed_output_left_level;
}

/**
 * Get smoothed output level for right channel
 * @return level value from 0.0 to 1.0
 */
float getOutputRightLevel() {
  return smoothed_output_right_level;
}

/**
 * Get combined (stereo) input level
 * @return maximum of left and right input levels
 */
float getInputLevel() {
  return max(smoothed_input_left_level, smoothed_input_right_level);
}

/**
 * Get combined (stereo) output level
 * @return maximum of left and right output levels
 */
float getOutputLevel() {
  return max(smoothed_output_left_level, smoothed_output_right_level);
}

/**
 * Check if audio system is ready for operation
 * @return true if audio system initialized successfully
 */
bool isAudioSystemReady() {
  return audio_system_ready;
}

// =============================================================================
// AUDIO DIAGNOSTICS
// =============================================================================
/**
 * Print audio system diagnostics to serial port
 * Useful for debugging audio processing issues
 */
void printAudioDiagnostics() {
  Serial.println("=== Songbird Audio Diagnostics ===");
  Serial.print("System Ready: "); Serial.println(audio_system_ready ? "YES" : "NO");
  Serial.print("USB Connected: "); Serial.println(usb_audio_connected ? "YES" : "NO");
  Serial.print("Sample Rate: "); Serial.println(getSampleRateString());
  
  Serial.println("\n--- Audio Levels ---");
  Serial.print("Input L: "); Serial.print(smoothed_input_left_level, 3);
  Serial.print(" | Input R: "); Serial.println(smoothed_input_right_level, 3);
  Serial.print("Output L: "); Serial.print(smoothed_output_left_level, 3);
  Serial.print(" | Output R: "); Serial.println(smoothed_output_right_level, 3);
  
  Serial.println("\n--- Memory Usage ---");
  Serial.print("Audio Memory: "); Serial.print(AudioMemoryUsage());
  Serial.print(" / "); Serial.print(AudioMemoryUsageMax());
  Serial.print(" blocks ("); Serial.print((float)AudioMemoryUsageMax() / AUDIO_MEMORY_BLOCKS * 100.0);
  
  Serial.println("% max)");
  
  Serial.println("\n--- Processor Usage ---");
  Serial.print("Audio CPU: "); Serial.print(AudioProcessorUsage(), 1);
  Serial.print("% | Max: "); Serial.print(AudioProcessorUsageMax(), 1);
  Serial.println("%");
  
  Serial.println("=====================================\n");
}