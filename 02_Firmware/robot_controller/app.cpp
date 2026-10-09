#include "app.h"
#include "firmware_version.h"
void App::begin() {
  Serial.begin(115200);
  Serial.println(String("AI Robot Controller ") + kFirmwareVersion);
  if (!control_.begin()) Serial.println("Actuator initialization failed; control locked");
  network_.begin(); ota_.begin(control_, network_); web_.begin();
}
void App::update() { network_.update(); web_.update(); ota_.update(); }
