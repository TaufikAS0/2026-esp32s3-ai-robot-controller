# Pin map

| Function | GPIO | Driver |
|---|---:|---|
| OLED I2C SDA (both) | 8 | Wire, 400 kHz |
| OLED I2C SCL (both) | 9 | Wire, 400 kHz |
| Laser | 4 | Digital output |
| Left hand | 5 | Servo LEDC channel 4 |
| Right hand | 6 | Servo LEDC channel 5 |
| Right motor IN1 | 10 | MX1508, LEDC channel 0 |
| Right motor IN2 | 11 | MX1508, LEDC channel 1 |
| Left motor IN1 | 12 | MX1508, LEDC channel 2 |
| Left motor IN2 | 13 | MX1508, LEDC channel 3 |

User's `LT2` is interpreted as left motor IN2. Motor channels share 20 kHz timers; hands share a separate 50 Hz timer. Servo PWM resolution is 14 bits for ESP32-S3. Boot writes outputs low before PWM initialization. Hardware suitability, level shifter, power supply, and physical polarity are not evaluated in this scope.

## Wheel polarity v0.3.2

Logical positive means configured forward for both wheels. The driver inverts the left motor only (config.h): forward uses GPIO 13/10, reverse uses GPIO 12/11; the paired input remains zero. Status commanded_output retains logical signs; PWM duty fields retain physical GPIO identities. Frequency readback follows the mapped active pin.

## Current mapping v0.5.0

Both motor polarities flipped after user reported forward/back reversed. Current config: left not inverted, right inverted. Positive logical forward uses left GPIO12 / right GPIO11; reverse uses13/10. Relative opposite polarity between wheels remains. Physical wheel direction still requires observation after flashing.

OLED: SSD1306 128x64, right0x3C/left0x3D (write-byte0x78/0x7A). Shared SDA8/SCL9; firmware swap exchanges logical eye images only, not bus addresses. Both hardware addresses must differ.
