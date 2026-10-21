#pragma once
#include <Preferences.h>
#include <freertos/semphr.h>
#include "demo_planner.h"
#include "exhibition_profile.h"
#include "control_service.h"
#include "network_service.h"

struct DemoSnapshot {
  DemoConfig config;
  DemoPlanner planner;
  uint32_t run=0,sample=0,exhibitionEpoch=0;
  unsigned sampleKind=0;
  float integral=0;
  bool running=false,simulation=false,calibrating=false,zero=false,storage=false,ready=false;
  bool samplePending=false;
  ExhibitionProfile exhibitionProfile;
  bool exhibition=false,exhibitionCenter=false;
  const char* sampleReason="no_sample";
  const char* reason="boot";
};
class DemoService {
 public:
  DemoService(ControlService& control,NetworkService& network):control_(control),network_(network){}
  bool begin();
  DemoSnapshot snapshot();
  int configure(cJSON* root);
  int zero();
  int start(bool simulation);
  int stop(uint32_t run);
  int calibrationStart(unsigned kind);
  int calibrationResult(uint32_t sample,float measured,float coast);
  int configureExhibition(cJSON* root);
  int centerExhibition();
  int startExhibition(bool simulation=false);
  char* statusJson();
 private:
#ifdef ROBOT_DEMO_TEST
  friend struct DemoHarness;
#endif
  ControlService& control_;
  NetworkService& network_;
  Preferences preferences_;
  SemaphoreHandle_t mutex_=nullptr;
  DemoSnapshot state_;
  ControlLogic virtual_;
  uint32_t session_=0,sequence_=0,epoch_=0,last_=0,sent_=0,calAt_=0,sampleEpoch_=0;
  void invalidateSample(const char* reason);
  void refreshSample(const ControlLogic& logic);
  static void task(void* context);
  void tick(uint32_t now);
  bool finish(const char* reason);
  bool persist(const DemoConfig& config,const uint32_t* expectedEpoch=nullptr);
  bool idle(const ControlLogic& logic) const;
  bool armsAllowed(const ControlLogic& logic) const;
  bool exhibitionArmsAllowed(const ControlLogic& logic) const;
};
