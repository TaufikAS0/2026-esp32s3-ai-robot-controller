# Pin map

| Function | GPIO | Driver |
|---|---:|---|
| Laser | 4 | Digital output |
| Left hand | 5 | Servo LEDC channel 4 |
| Right hand | 6 | Servo LEDC channel 5 |
| Right motor IN1 | 10 | MX1508, LEDC channel 0 |
| Right motor IN2 | 11 | MX1508, LEDC channel 1 |
| Left motor IN1 | 12 | MX1508, LEDC channel 2 |
| Left motor IN2 | 13 | MX1508, LEDC channel 3 |

User's `LT2` is interpreted as left motor IN2. Motor channels share 20 kHz timers; hands share a separate 50 Hz timer. Servo PWM resolution is 14 bits for ESP32-S3. Boot writes outputs low before PWM initialization. Hardware suitability, level shifter, power supply, and physical polarity are not evaluated in this scope.
