#include <cassert>
#include "../02_Firmware/robot_controller/demo_service.h"
static ControlLogic logic;
static unsigned commands=0,acquires=0;
static bool releaseAcknowledged=true;
ControlSnapshot ControlService::snapshot(){return {logic,0,{}};}
bool ControlService::beginNetworkChange(bool lost){return logic.beginNetworkChange(lost);}
void ControlService::endNetworkChange(){logic.endNetworkChange();}
bool ControlService::acquireDemo(uint32_t& session,uint32_t epoch){session=77;++acquires;return logic.acquireDemo(session,fakeNow,epoch);}
bool ControlService::commandDemo(const Command& c,uint8_t eye){++commands;const bool ok=logic.command(c,fakeNow,0,true);if(ok)logic.demoEye=eye;return ok;}
bool ControlService::releaseDemo(uint32_t session){return logic.release(session,0,true)&&releaseAcknowledged;}
struct DemoHarness {static void tick(DemoService& d){d.tick(fakeNow);}};
static void advance(DemoService& d,unsigned count=1){for(unsigned i=0;i<count;++i){fakeNow+=20;logic.tick(fakeNow-10);logic.tick(fakeNow);DemoHarness::tick(d);}}
static DemoConfig config(){DemoConfig c;c.width=140;c.length=100;c.radius=10;c.armsConfirmed=true;return c;}
static ApiJson::Json request(const DemoConfig& c){auto j=c.json();for(const char* key:{"linear_gain","cw_gain","ccw_gain","calibration"})cJSON_DeleteItemFromObject(j.get(),key);return j;}
int main(){
  ControlService control;NetworkService network;network.begin(control);DemoService demo(control,network);assert(demo.begin());
  auto c=config();auto j=request(c);assert(demo.configure(j.get())==200);assert(Preferences::data.count("demoV1"));
  assert(demo.start(false)==409);assert(demo.start(true)==202);auto id=demo.snapshot().run;
  advance(demo,20000);assert(demo.snapshot().planner.visits>3&&commands==0&&acquires==0);
  for(unsigned i=0;i<50;++i) {
    char* text=demo.statusJson();assert(text);ApiJson::Json status(cJSON_Parse(text),cJSON_Delete);cJSON_free(text);
    assert(cJSON_IsTrue(ApiJson::item(status.get(),"estimated")));
    assert(cJSON_GetArraySize(ApiJson::item(status.get(),"trace"))==256);
    assert(cJSON_IsTrue(ApiJson::item(status.get(),"simulation")));
  }
  assert(demo.stop(id+1)==409&&demo.snapshot().running);assert(demo.stop(id)==200);
  logic.mode=ControlMode::Auto;
  for(unsigned kind=0;kind<3;++kind){
    assert(demo.zero()==200);assert(demo.calibrationStart(kind)==202);advance(demo,50);auto s=demo.snapshot();
    assert(!s.running&&!s.zero&&s.integral>0&&logic.output.left==0&&!logic.demoOwned);
    assert(s.samplePending);advance(demo,6500);assert(demo.snapshot().samplePending); // >120 seconds, no lease held.
    assert(demo.calibrationResult(s.sample,-1,0)==400);
    assert(demo.calibrationResult(s.sample,s.integral*(kind?240:40),0)==200);
    assert(demo.calibrationResult(s.sample,1,0)==409);
  }
  assert(demo.snapshot().config.ready());assert(demo.start(false)==409);
  auto pending=[&](){logic.output.servoEnabled=false;assert(demo.zero()==200);assert(demo.calibrationStart(1)==202);advance(demo,50);assert(demo.snapshot().samplePending);return demo.snapshot().sample;};
  auto sample=pending();logic.stop("global_stop");assert(demo.calibrationResult(sample,45,0)==409&&!demo.snapshot().samplePending);
  sample=pending();assert(logic.acquire(Owner::Program,99,fakeNow,4));assert(!demo.snapshot().samplePending);logic.stop("test");assert(demo.calibrationResult(sample,45,0)==409);
  sample=pending();assert(logic.acquire(Owner::Program,99,fakeNow,4));logic.stop("quick_release");assert(demo.calibrationResult(sample,45,0)==409); // Entire foreign session between demo ticks.
  sample=pending();logic.beginMaintenance();advance(demo);assert(!demo.snapshot().samplePending);logic.endMaintenance();
  sample=pending();assert(logic.beginNetworkChange());advance(demo);assert(!demo.snapshot().samplePending);logic.endNetworkChange();
  sample=pending();j=request(c);assert(demo.configure(j.get())==200);assert(!demo.snapshot().samplePending&&demo.calibrationResult(sample,45,0)==409);
  sample=pending();assert(demo.zero()==200);assert(demo.calibrationStart(2)==202);assert(!demo.snapshot().samplePending);advance(demo,50);assert(demo.calibrationResult(sample,45,0)==409);assert(demo.stop(demo.snapshot().run)==200);
  sample=pending();DemoService pendingReboot(control,network);assert(pendingReboot.begin());assert(!pendingReboot.snapshot().samplePending&&pendingReboot.calibrationResult(sample,45,0)==409);assert(demo.stop(demo.snapshot().run)==200);
  logic.output.servoEnabled=false;assert(demo.zero()==200&&demo.calibrationStart(1)==202);releaseAcknowledged=false;advance(demo,50);releaseAcknowledged=true;assert(!demo.snapshot().samplePending);
  assert(demo.zero()==200);assert(demo.start(false)==202);advance(demo,500);
  assert(demo.snapshot().running&&logic.demoOwned&&commands>0&&!logic.dynamicActive);
  fakeNow+=301;DemoHarness::tick(demo);assert(!demo.snapshot().running&&logic.owner==Owner::None&&logic.output.left==0);
  assert(demo.zero()==200);logic.stop("external");assert(demo.start(false)==409);
  assert(demo.zero()==200);assert(demo.start(false)==202);logic.stop("global_stop");advance(demo);assert(!demo.snapshot().running&&!demo.snapshot().zero);
  assert(demo.zero()==200);assert(demo.start(false)==202);logic.stop("test");assert(logic.acquire(Owner::Program,99,fakeNow,4));advance(demo);assert(logic.session==99);logic.stop("test");
  assert(demo.zero()==200);assert(demo.start(false)==202);logic.beginMaintenance();advance(demo);assert(!demo.snapshot().running&&logic.maintenance);logic.endMaintenance();
  for(DemoPhase phase:{DemoPhase::Choose,DemoPhase::Turn,DemoPhase::Drive,DemoPhase::Brake,DemoPhase::Gesture}) {
    logic.output.servoEnabled=false; // Test fixture: operator has accepted the base pose again.
    assert(demo.zero()==200);assert(demo.start(false)==202);
    unsigned count=0;while(demo.snapshot().planner.phase!=phase&&count++<20000)advance(demo);
    assert(demo.snapshot().planner.phase==phase);auto s=demo.snapshot();const float arm=logic.output.armLeft;
    assert(demo.stop(s.run)==200);advance(demo,10);assert(!demo.snapshot().running&&logic.output.left==0&&logic.output.right==0&&logic.output.armLeft==arm);
  }
  c.pwm=.8f;c.minPwm=.6f;c.deadbandPwm=.5f;j=request(c);assert(demo.configure(j.get())==200);assert(demo.snapshot().config.calibration==0);
  for(unsigned kind=0;kind<3;++kind){
    logic.output.servoEnabled=false;assert(demo.zero()==200);assert(demo.calibrationStart(kind)==202);advance(demo,50);const auto s=demo.snapshot();
    assert(!s.running&&s.integral>0&&s.integral<.3f);
    assert(demo.calibrationResult(s.sample,s.integral*(kind==0?20:120),0)==200);
  }
  assert(demo.snapshot().config.ready());
  logic.output.servoEnabled=false;assert(demo.zero()==200);WiFi.gotIp();assert(demo.start(false)==202);WiFi.disconnect();advance(demo);
  assert(!demo.snapshot().running&&!demo.snapshot().zero&&logic.owner==Owner::None);
  auto stored=Preferences::data["demoV1"];DemoService reboot(control,network);assert(reboot.begin());assert(!reboot.snapshot().zero&&!reboot.snapshot().running&&!reboot.snapshot().config.armsConfirmed);
  assert(reboot.snapshot().config.calibration==7&&Preferences::data["demoV1"]==stored);
  NetworkService exhibitNetwork;exhibitNetwork.begin(control);DemoService exhibit(control,exhibitNetwork);assert(exhibit.begin());
  const auto unchanged=Preferences::data["demoV1"];logic.stop("fixture");logic.mode=ControlMode::Manual;logic.output.servoEnabled=false;
  assert(exhibit.centerExhibition()==409&&exhibit.startExhibition()==409);
  logic.mode=ControlMode::Auto;logic.armPolicy.leftMax=100;assert(exhibit.centerExhibition()==409);logic.armPolicy.leftMax=180;
  auto area=ApiJson::Json(cJSON_Parse("{\"width_cm\":30,\"length_cm\":30}"),cJSON_Delete);
  assert(exhibit.configureExhibition(area.get())==200&&Preferences::data["demoV1"]==unchanged);
  cJSON_AddNumberToObject(area.get(),"width_cm",30);assert(exhibit.configureExhibition(area.get())==400);cJSON_DeleteItemFromObject(area.get(),"width_cm");
  const auto priorAcquires=acquires,priorCommands=commands;
  assert(exhibit.startExhibition(true)==202);advance(exhibit,20000);assert(exhibit.snapshot().planner.visits>3&&commands==priorCommands&&acquires==priorAcquires);
  assert(exhibit.stop(exhibit.snapshot().run)==200);
  assert(exhibit.centerExhibition()==200);const auto centered=exhibit.snapshot();assert(centered.exhibitionCenter&&centered.planner.pose.x==0&&centered.planner.pose.y==0&&centered.planner.pose.heading==0&&centered.planner.traceCount==0);
  assert(commands==priorCommands&&acquires==priorAcquires);logic.stop("global_stop");assert(!exhibit.snapshot().exhibitionCenter&&exhibit.startExhibition()==409);
  assert(exhibit.centerExhibition()==200);assert(logic.acquire(Owner::Program,99,fakeNow,4));assert(exhibit.centerExhibition()==409&&exhibit.startExhibition()==409);logic.stop("foreign_released");assert(!exhibit.snapshot().exhibitionCenter);
  for(DemoPhase phase:{DemoPhase::Choose,DemoPhase::Turn,DemoPhase::Drive,DemoPhase::Brake,DemoPhase::Gesture}){
    logic.output.servoEnabled=false;assert(exhibit.centerExhibition()==200&&exhibit.startExhibition()==202);
    assert(exhibit.centerExhibition()==409&&exhibit.configureExhibition(area.get())==409);
    unsigned count=0;while(exhibit.snapshot().planner.phase!=phase&&count++<20000)advance(exhibit);
    assert(exhibit.snapshot().planner.phase==phase);const auto run=exhibit.snapshot().run;
    assert(exhibit.stop(run+1)==409&&exhibit.snapshot().running);assert(exhibit.stop(run)==200);
    assert(!exhibit.snapshot().exhibitionCenter&&logic.owner==Owner::None&&logic.output.left==0&&logic.output.right==0&&!logic.demoEye);
    assert(exhibit.startExhibition()==409);
  }
  logic.output.servoEnabled=false;assert(exhibit.centerExhibition()==200&&exhibit.startExhibition()==202);
  logic.stop("replace");assert(logic.acquire(Owner::Program,99,fakeNow,5));advance(exhibit);assert(!exhibit.snapshot().running&&logic.session==99);logic.stop("test");
  assert(exhibit.centerExhibition()==200&&exhibit.startExhibition()==202);fakeNow+=301;DemoHarness::tick(exhibit);assert(!exhibit.snapshot().running&&logic.owner==Owner::None);
  assert(exhibit.centerExhibition()==200&&exhibit.startExhibition()==202);logic.beginMaintenance();advance(exhibit);assert(!exhibit.snapshot().running&&!exhibit.snapshot().exhibitionCenter);logic.endMaintenance();
  assert(exhibit.centerExhibition()==200&&exhibit.startExhibition()==202);WiFi.gotIp();WiFi.disconnect();advance(exhibit);assert(!exhibit.snapshot().running&&logic.owner==Owner::None);
  assert(Preferences::data["demoV1"]==unchanged);
  DemoService exhibitReboot(control,exhibitNetwork);assert(exhibitReboot.begin());assert(!exhibitReboot.snapshot().exhibitionCenter&&!exhibitReboot.snapshot().running&&exhibitReboot.snapshot().config.calibration==7);
  NetworkService freshNetwork;freshNetwork.begin(control);DemoService linkTest(control,freshNetwork);assert(linkTest.begin());
  j=request(c);assert(linkTest.configure(j.get())==200);logic.output.servoEnabled=false;
  assert(linkTest.zero()==200&&linkTest.calibrationStart(1)==202);advance(linkTest,50);assert(linkTest.snapshot().samplePending);
  WiFi.gotIp();WiFi.disconnect();assert(!linkTest.snapshot().samplePending);
  Preferences::data["demoV1"]="bad";DemoService corrupt(control,network);assert(corrupt.begin());assert(!corrupt.snapshot().storage&&Preferences::data["demoV1"]=="bad");
}
