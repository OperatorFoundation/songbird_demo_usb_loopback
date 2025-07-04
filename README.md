# Songbird USB Audio Loopback Demo

An embedded audio processing demonstration using the Songbird hardware platform. This project showcases real-time USB audio loopback with visual feedback and interactive controls.

## Table of Contents

- [Project Overview](#project-overview)
- [Hardware Setup](#hardware-setup)
- [Software Installation](#software-installation)
- [Operation Guide](#operation-guide)
- [Technical Details](#technical-details)
- [Troubleshooting](#troubleshooting)
- [Development](#development)
- [Educational Resources](#educational-resources)

## Project Overview

### What is Songbird?

Songbird is a commercial audio demonstration device that showcases embedded audio processing capabilities. It features:

- **Real-time USB Audio Processing**: Direct loopback from USB input to output with minimal latency
- **Visual Feedback**: OLED display showing audio levels and system status
- **Interactive Controls**: Buttons for system state control and audio muting
- **Professional Audio Quality**: 16-bit/44.1kHz-48kHz audio processing
- **Educational Value**: Well-documented code demonstrating embedded audio concepts

### Key Features

- **USB Audio Interface**: Appears as a standard USB audio device to any computer
- **Zero-Latency Loopback**: Direct audio passthrough for monitoring applications
- **Real-time Level Metering**: Visual audio level indication on display and LEDs
- **State Machine Control**: Professional state management for reliable operation
- **Comprehensive Diagnostics**: Built-in system monitoring and testing capabilities

### Target Audience

- **Audio Engineers**: Understand embedded audio processing concepts
- **Embedded Systems Students**: Learn professional code organization and audio programming
- **Makers and Hobbyists**: Explore advanced Teensy Audio Library applications
- **Educators**: Demonstrate real-time audio processing principles

## Hardware Setup

### Songbird Device Overview

The Songbird device integrates several components:

- **Teensy 4.0 Microcontroller**: 600MHz ARM Cortex-M7 processor
- **Audio Shield**: SGTL5000 codec for high-quality audio processing
- **128x32 OLED Display**: Real-time visual feedback with automatic rotation correction
- **Two Color LEDs**: Blue and pink LEDs for level indication
- **Four Control Buttons**: UP, DOWN, LEFT, RIGHT for system control
- **USB-C Connector**: Audio interface and power supply

### Physical Connections

1. **USB-C Connection**: Connect Songbird to your computer using the included USB-C cable
2. **Power**: Songbird is powered entirely through the USB connection
3. **Audio**: No additional audio cables required - all audio is transmitted via USB

### First-Time Setup Verification

1. **Power LED Check**: Confirm the Teensy power LED illuminates when connected
2. **Display Test**: The OLED display should show "Songbird Ready!" on startup
3. **Button Test**: Press the UP button to verify display changes
4. **LED Test**: LEDs should briefly illuminate during startup sequence

### Audio Connection Setup

1. **Connect Songbird**: Plug the USB-C cable into your computer
2. **Audio Device Recognition**: Your operating system should automatically detect Songbird as a USB audio device
3. **Audio Settings**: Configure Songbird as both input and output device in your system audio settings

## Software Installation

### Prerequisites

- **Arduino IDE**: Version 1.8.15 or later
- **Teensyduino**: Latest version for Teensy 4.0 support
- **Required Libraries**: See library installation section below

### Arduino IDE Setup

1. **Download Arduino IDE**:
   - Visit [arduino.cc](https://www.arduino.cc/en/software)
   - Download and install the latest version

2. **Install Teensyduino**:
   - Visit [pjrc.com/teensy/teensyduino.html](https://www.pjrc.com/teensy/teensyduino.html)
   - Download and install Teensyduino
   - Follow the installation wizard to add Teensy support to Arduino IDE

3. **Board Selection**:
   - Open Arduino IDE
   - Go to Tools → Board → Teensyduino → "Teensy 4.0"
   - Set USB Type to "Audio"
   - Set CPU Speed to "600 MHz"

### Library Installation

Install the following libraries via Arduino IDE Library Manager (Tools → Manage Libraries):

#### Required Libraries

1. **Adafruit SSD1306** (for OLED display)
   - Search for "Adafruit SSD1306"
   - Install latest version

2. **Adafruit GFX Library** (for graphics primitives)
   - Search for "Adafruit GFX"
   - Install latest version

3. **Teensy Audio Library** (built-in with Teensyduino)
   - Automatically included with Teensyduino installation

#### Library Versions

- Adafruit SSD1306: 2.5.7 or later
- Adafruit GFX: 1.11.5 or later
- Teensy Audio: Included with Teensyduino

### Compilation and Upload

1. **Download Source Code**:
   - Extract all `.ino` files to a folder named `usb_loopback_demo`
   - Ensure all files are in the same directory

2. **Open Project**:
   - Open `usb_loopback_demo.ino` in Arduino IDE
   - Verify all other `.ino` files appear as tabs

3. **Compile**:
   - Press Ctrl+R (or Sketch → Verify/Compile)
   - Ensure compilation completes without errors

4. **Upload**:
   - Connect Songbird via USB-C
   - Press the program button on Teensy while plugging in (first time only)
   - Press Ctrl+U (or Sketch → Upload)
   - Wait for "Download Complete" message

### Troubleshooting Installation

**Common Issues:**

- **Board not found**: Ensure Teensyduino is properly installed
- **Library errors**: Verify all required libraries are installed
- **Upload fails**: Try pressing the program button on Teensy
- **Compilation errors**: Check that all `.ino` files are present

## Operation Guide

### System States

Songbird operates in three main states:

#### 1. STANDBY State
- **Description**: System ready, no audio processing
- **Display**: Shows "Press UP to start"
- **LEDs**: Off
- **Audio**: No processing active

#### 2. ACTIVE State
- **Description**: Full audio loopback operation
- **Display**: Shows live audio levels and "USB Loopback Active"
- **LEDs**: Respond to audio levels
- **Audio**: Direct USB input to output passthrough

#### 3. MUTED State
- **Description**: Input monitored, output muted
- **Display**: Shows input levels and "Output Muted"
- **LEDs**: Input LED responds, output LED off
- **Audio**: No output, input still processed for display

### Button Controls

#### UP Button (Top)
- **From STANDBY**: Activates audio loopback (→ ACTIVE)
- **From ACTIVE/MUTED**: Returns to standby (→ STANDBY)

#### DOWN Button (Bottom)
- **From ACTIVE**: Mutes audio output (→ MUTED)
- **From MUTED**: Unmutes audio output (→ ACTIVE)
- **From STANDBY**: No effect

#### LEFT/RIGHT Buttons
- **Currently Reserved**: For future functionality

### Display Elements

The 128x32 OLED display shows:

#### Status Line
- **Top of display**: Current system state and controls
- **Sample Rate**: Shows current USB audio sample rate (44k/48k)

#### Level Meters
- **Format**: `I:[####----]` for input, `O:[####----]` for output
- **Segments**: 8 segments representing audio level (0-8)
- **Characters**: `#` for active segments, `-` for inactive segments
- **Update Rate**: 10 times per second for smooth indication

#### Channel Indicators
- **L**: Left channel indicator, brackets show activity `[L]`
- **R**: Right channel indicator, brackets show activity `[R]`
- **Active State**: Brackets appear when audio is present
- **Inactive State**: Simple text without brackets

### LED Indicators

#### Blue LED (LED1)
- **Function**: Input level indicator
- **Brightness**: Proportional to input audio level
- **Behavior**: Smooth transitions, logarithmic scaling

#### Pink LED (LED2)
- **Function**: Output level indicator
- **Brightness**: Proportional to output audio level
- **Behavior**: Mirrors input LED in ACTIVE state, off when muted

### Using Songbird as USB Audio Interface

1. **Connect**: Plug Songbird into computer via USB-C
2. **Configure Audio Settings**:
   - **Windows**: Sound Settings → Choose Songbird for input/output
   - **macOS**: System Preferences → Sound → Select Songbird
   - **Linux**: Use `alsamixer` or PulseAudio to select Songbird

3. **Activate Processing**: Press UP button to enter ACTIVE state
4. **Test Audio**: Play music or speak into microphone
5. **Monitor Levels**: Watch display and LEDs for audio activity

### Professional Audio Applications

- **Audio Monitoring**: Use as a reference loopback device
- **System Testing**: Verify audio paths and levels
- **Educational Demonstrations**: Show real-time audio processing
- **Development Tool**: Test USB audio applications

## Technical Details

### Audio Specifications

| Specification | Value |
|---------------|-------|
| **Sample Rates** | 44.1 kHz, 48 kHz (host-determined) |
| **Bit Depth** | 16-bit |
| **Channels** | 2 (Stereo) |
| **Latency** | <10ms (typical) |
| **Frequency Response** | 20Hz - 20kHz |
| **Dynamic Range** | >90dB |

### Processing Performance

| Metric | Value |
|--------|-------|
| **CPU Usage** | <15% typical |
| **Memory Usage** | ~182KB RAM, ~76KB Flash |
| **Audio Memory** | 32 blocks (4KB total) |
| **Display Update** | 10 Hz |
| **Button Scan** | 20 Hz |
| **Main Loop** | 50 Hz |

### Hardware Specifications

#### Teensy 4.0
- **Processor**: 600MHz ARM Cortex-M7
- **RAM**: 1MB
- **Flash**: 2MB
- **Audio Processing**: Hardware-accelerated

#### Audio Shield
- **Codec**: SGTL5000
- **Input**: Line/Mic configurable
- **Output**: Line/Headphone
- **Control**: I2C interface

#### Display
- **Type**: SSD1306 OLED
- **Size**: 128x32 pixels
- **Interface**: I2C (Wire1)
- **Address**: 0x3C
- **Rotation**: Automatically corrected for proper orientation

### System Architecture

```
USB Audio ─→ Teensy 4.0 ─→ Audio Shield ─→ Physical Output
    ↓            ↓              ↓
    └─→ Level Detection ─→ Display/LEDs
```

### Pin Assignments

| Function | Pin | Description |
|----------|-----|-------------|
| **Audio Interface** |
| I2S_DIN | 7 | Audio data input |
| I2S_DOUT | 8 | Audio data output |
| I2S_SCLK | 21 | Serial clock |
| I2S_LRCLK | 20 | Left/right clock |
| SYS_MCLK | 23 | Master clock |
| **User Interface** |
| OLED_SDA | 17 | Display data (Wire1) |
| OLED_SCL | 16 | Display clock (Wire1) |
| BTN_UP | 5 | Up button |
| BTN_DOWN | 4 | Down button |
| LED_1 | 14 | Blue LED |
| LED_2 | 15 | Pink LED |

### Memory Usage

| Component | RAM | Flash |
|-----------|-----|-------|
| Teensy Audio Library | ~180KB | ~50KB |
| Display System | 512B | ~8KB |
| Application Logic | ~2KB | ~15KB |
| **Total** | **~182KB** | **~76KB** |

## Troubleshooting

### Audio Issues

#### No Audio Output
- **Check**: USB connection secure
- **Verify**: Songbird selected as audio device
- **Confirm**: System in ACTIVE state (press UP button)
- **Test**: Try different audio source

#### Audio Crackling/Distortion
- **Reduce**: Input levels on source device
- **Check**: USB cable quality
- **Verify**: Proper sample rate matching
- **Monitor**: CPU usage via diagnostics

#### High Latency
- **Ensure**: USB 2.0 or higher connection
- **Check**: Computer USB performance
- **Verify**: No other high-bandwidth USB devices
- **Consider**: Dedicated USB port

### Display Problems

#### Display Blank
- **Check**: I2C connections (pins 16/17)
- **Verify**: Display power via USB
- **Reset**: Restart Songbird
- **Test**: Known working display

#### Display Upside Down
- **Solution**: Automatically corrected in software with `setRotation(2)`
- **No action needed**: Display should appear correctly oriented

#### Display Garbled/Corrupted
- **Cause**: Uses standard ASCII characters for compatibility
- **Check**: Level bars should show `#` and `-` characters
- **Verify**: No special Unicode characters displayed
- **Reset**: Restart Songbird if display corruption persists

#### Display Flickering
- **Check**: Loose connections
- **Verify**: Stable power supply
- **Update**: Display refresh rate in code
- **Test**: Different USB port

### Button Issues

#### Buttons Not Responding
- **Check**: Physical connections
- **Verify**: Pull-up resistors enabled
- **Test**: Button diagnostic mode
- **Clean**: Button contacts

#### False Button Presses
- **Adjust**: Debounce timing
- **Check**: Electrical interference
- **Verify**: Proper grounding
- **Test**: Individual button response

### LED Issues

#### LEDs Not Responding
- **Check**: PWM pin connections
- **Verify**: LED polarity
- **Test**: Manual LED control
- **Check**: Current limiting resistors

#### Incorrect Brightness
- **Adjust**: Level mapping algorithm
- **Check**: PWM resolution
- **Verify**: Audio level detection
- **Test**: Different audio sources

### System Issues

#### System Won't Start
- **Check**: Power via USB
- **Verify**: Teensy 4.0 functionality
- **Test**: Basic Arduino sketch
- **Check**: Bootloader integrity

#### Random Crashes
- **Monitor**: Memory usage
- **Check**: Stack overflow
- **Verify**: Interrupt handling
- **Test**: Simplified configuration

#### USB Connection Problems
- **Try**: Different USB cables
- **Test**: Multiple computers
- **Check**: USB driver installation
- **Verify**: Teensyduino USB type setting

### Diagnostic Tools

#### Built-in Diagnostics
- **Serial Monitor**: View system status and debug messages
- **LED Test**: Hold UP+DOWN during startup
- **Button Test**: Hold LEFT+RIGHT during startup
- **Display Test**: Automatic on startup

#### Development Tools
- **Arduino Serial Monitor**: Real-time system diagnostics
- **Teensyduino Audio System Design Tool**: Visual audio routing
- **Logic Analyzer**: For I2C and digital signal debugging
- **Oscilloscope**: For audio signal analysis

## Development

### Code Organization

The project is organized into modular components:

#### Core Files
- **`usb_loopback_demo.ino`**: Main control loop and state machine
- **`config.ino`**: Hardware configuration and constants
- **`audio_manager.ino`**: Audio processing and routing
- **`display_manager.ino`**: OLED display control
- **`led_manager.ino`**: LED brightness control
- **`button_manager.ino`**: Button input handling

#### Architecture Benefits
- **Modular Design**: Each component can be developed/tested independently
- **Clean Interfaces**: Well-defined functions between modules
- **Educational Value**: Clear separation of concerns
- **Maintainability**: Easy to modify individual components

### Customization Options

#### Audio Processing
- **Sample Rate**: Modify for different rates
- **Bit Depth**: Upgrade to 24-bit processing
- **Effects**: Add reverb, EQ, or other processing
- **Multi-channel**: Expand beyond stereo

#### Display Customization
- **Layout**: Modify visual arrangement
- **Graphics**: Add custom indicators or patterns
- **Information**: Display additional system data
- **Themes**: Create different visual styles

#### Control Expansion
- **Additional Buttons**: Add more control options
- **Rotary Encoders**: Implement continuous controls
- **Touch Interface**: Add touch-sensitive controls
- **MIDI**: Add MIDI control capabilities

### Performance Optimization

#### Memory Optimization
- **Buffer Sizes**: Adjust for memory constraints
- **String Usage**: Minimize dynamic string allocation
- **Display Buffer**: Optimize graphics memory usage
- **Audio Memory**: Balance quality vs. memory usage

#### CPU Optimization
- **Update Rates**: Adjust component update frequencies
- **Interrupt Handling**: Optimize ISR performance
- **Floating Point**: Consider fixed-point arithmetic
- **Compiler Options**: Use optimization flags

### Testing Framework

#### Unit Testing
- **Component Tests**: Individual module verification
- **Integration Tests**: System-level functionality
- **Performance Tests**: CPU and memory usage validation
- **Hardware Tests**: Physical interface verification

#### System Testing
- **Audio Quality**: Frequency response and distortion analysis
- **Latency Measurement**: Real-time performance validation
- **Stress Testing**: Extended operation validation
- **Environmental Testing**: Temperature and power variation

## Educational Resources

### Learning Objectives

This project demonstrates:

- **Real-time Audio Processing**: Understanding of digital audio concepts
- **Embedded Systems Design**: Professional code organization and architecture
- **State Machine Implementation**: Robust system state management
- **Hardware Integration**: Combining multiple subsystems effectively
- **User Interface Design**: Creating intuitive control systems

### Concepts Demonstrated

#### Audio Processing
- **Digital Audio Fundamentals**: Sample rates, bit depth, channels
- **Real-time Constraints**: Latency, buffering, interrupt handling
- **Level Detection**: RMS analysis and visualization
- **Audio Routing**: Signal flow and processing chains

#### Embedded Programming
- **Modular Architecture**: Separating concerns and creating reusable components
- **State Machines**: Managing system behavior and transitions
- **Interrupt Handling**: Real-time processing and timing
- **Resource Management**: Memory and CPU optimization

#### Hardware Integration
- **I2C Communication**: Display and sensor interfaces
- **PWM Control**: LED brightness and motor control
- **GPIO Management**: Button debouncing and digital I/O
- **USB Communication**: Host interface and protocol handling

### Extended Learning

#### Advanced Audio Projects
- **Audio Effects**: Implement reverb, echo, or filtering
- **Spectrum Analysis**: Add FFT-based frequency visualization
- **Multi-channel Processing**: Expand to surround sound
- **Synthesis**: Generate tones and music programmatically

#### System Enhancements
- **Wireless Control**: Add WiFi or Bluetooth interfaces
- **Data Logging**: Record audio levels and system performance
- **Remote Monitoring**: Web-based control and status
- **Configuration Management**: Save/load system settings

#### Professional Applications
- **Test Equipment**: Audio analyzer and signal generator
- **Monitoring Systems**: Professional audio level monitoring
- **Educational Tools**: Classroom demonstration equipment
- **Development Platform**: Base for custom audio applications

## License and Support

### License
This project is provided as educational material with full source code and documentation.

### Support Resources
- **Arduino Community**: [arduino.cc/forum](https://forum.arduino.cc/)
- **Teensy Community**: [forum.pjrc.com](https://forum.pjrc.com/)
- **Audio Library Documentation**: [pjrc.com/teensy/td_libs_Audio.html](https://www.pjrc.com/teensy/td_libs_Audio.html)
- **Adafruit Learning**: [learn.adafruit.com](https://learn.adafruit.com/)

### Contributing
Contributions to improve the code, documentation, or educational value are welcome. Please follow professional coding standards and include comprehensive documentation for any changes.

---

**Project Version**: 1.0  
**Last Updated**: July 2025  
**Hardware**: Songbird Audio Demonstration Platform  
**Software**: Arduino IDE with Teensyduino