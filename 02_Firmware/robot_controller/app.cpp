#include "app.h"
#include "firmware_version.h"
void App::begin() {
  Serial.begin(115200);
  Serial.println(String("AI Robot Controller ") + kFirmwareVersion);
  Serial.println(String("Build UTC: ") + kFirmwareBuildUtc);
  if (!control_.begin()) Serial.println("Actuator initialization failed; control locked");
  if (!eyes_.begin()) Serial.println("Eye task initialization failed; motor control unaffected");
  network_.begin(control_); ota_.begin(control_, network_); web_.begin();
  if(!tcp_.begin())Serial.println("TCP control server unavailable");
}
void App::update() { network_.update(ota_.enabled || ota_.busy); web_.update(); ota_.update(); }
