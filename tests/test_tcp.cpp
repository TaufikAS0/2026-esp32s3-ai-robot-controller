#include <cassert>
#include <fcntl.h>
#include <string>
#include <limits>
#include "../02_Firmware/robot_controller/tcp_peer.h"

struct FakeControl {
  ControlLogic logic;
  uint32_t now=0,next=100;
  struct Snapshot { ControlLogic logic; struct { bool ready=true,writeOk=true; } pwm; };
  Snapshot snapshot() { logic.expire(now);return {logic,{}}; }
  bool acquire(Owner owner,uint32_t& session,uint32_t transport) { session=++next;return logic.acquire(owner,session,now,transport); }
  bool setMode(ControlMode mode,bool idle) { return logic.setMode(mode,idle); }
  bool command(const Command& c,uint32_t transport) { bool ok=logic.command(c,now,transport);logic.tick(now);return ok; }
  bool release(uint32_t session,uint32_t transport) { return logic.release(session,transport); }
  bool stop(const char* why) { logic.stop(why);return true; }
};
bool ok(const ApiJson::Json& r) { return cJSON_IsTrue(ApiJson::item(r.get(),"ok")); }
std::string command(uint32_t session,uint32_t sequence=1) {
  return "{\"v\":1,\"id\":7,\"type\":\"command\",\"session\":"+std::to_string(session)+",\"sequence\":"+std::to_string(sequence)+",\"left\":0.5,\"right\":0.5,\"arm_left\":42,\"arm_right\":79,\"laser\":true}";
}
void protocolTests() {
  FakeControl c; TcpProtocol<FakeControl> p(c,22,"test","utc");
  assert(!ok(p.handle("{\"v\":2,\"id\":1,\"type\":\"hello\"}")));
  assert(!ok(p.handle("{\"v\":1,\"id\":1,\"id\":2,\"type\":\"hello\"}")));
  assert(!ok(p.handle("{\"v\":1,\"id\":true,\"type\":\"hello\"}")));
  assert(!ok(p.handle("{\"v\":1,\"id\":1,\"type\":\"stop\\u0000x\"}")));
  assert(!ok(p.handle((std::string(100,'[')+"0"+std::string(100,']')).c_str())));
  assert(!ok(p.handle("{\"v\":1,\"id\":1,\"type\":\"acquire\"}"))); // Manual boot
  assert(c.logic.acquire(Owner::Manual,44,0));
  assert(!ok(p.handle("{\"v\":1,\"id\":1,\"type\":\"mode\",\"mode\":\"auto\"}")));
  assert(c.logic.session==44);c.logic.stop("test");
  assert(ok(p.handle("{\"v\":1,\"id\":1,\"type\":\"mode\",\"mode\":\"auto\"}")));
  assert(ok(p.handle("{\"v\":1,\"id\":2,\"type\":\"acquire\"}")));
  auto session=c.logic.session;
  assert(ok(p.handle(command(session).c_str())));
  assert(c.logic.output.laser && c.logic.output.left>0);
  Command http{session,2,0,0,42,79,false};
  assert(!c.logic.command(http,1)); // HTTP cannot use TCP lease, even with correct public ID.
  assert(!c.logic.release(session));
  TcpProtocol<FakeControl> other(c,23,"test","utc");
  assert(!ok(other.handle(command(session,2).c_str())));
  assert(!ok(p.handle(command(session).c_str()))); // duplicate sequence
  auto bad=command(session,2);bad.replace(bad.find("0.5"),3,"2.0");
  c.now=100;assert(!ok(p.handle(bad.c_str())));assert(c.logic.lastCommandMs==0);
  c.logic.armPolicy.leftMax=40;assert(!ok(p.handle(command(session,2).c_str())));assert(c.logic.lastCommandMs==0);c.logic.armPolicy.leftMax=180;
  bad=command(session,2);bad.replace(bad.find("true"),4,"1");assert(!ok(p.handle(bad.c_str())));
  assert(ok(p.handle("{\"v\":1,\"id\":9,\"type\":\"status\"}")));
  c.now=500;c.logic.tick(c.now);assert(c.logic.owner==Owner::None && !c.logic.output.laser && c.logic.output.left==0);
  assert(!ok(p.handle(command(session,2).c_str())));
  assert(c.logic.setMode(ControlMode::Manual));assert(c.logic.acquire(Owner::Manual,99,501));
  p.close();assert(c.logic.session==99); // Old disconnect must not release a new HTTP owner.
  c.logic.beginMaintenance();assert(!ok(other.handle("{\"v\":1,\"id\":2,\"type\":\"acquire\"}")));
  assert(!c.logic.output.servoEnabled && !c.logic.output.laser);
  c.logic.endMaintenance();assert(c.logic.setMode(ControlMode::Auto));
  assert(c.logic.acquire(Owner::Program,888,600)); // Existing HTTP program lease
  assert(!c.logic.command(Command{888,1,0,0,42,79,false},601,22));
  assert(c.logic.command(Command{888,1,0,0,42,79,false},601));
}
struct PeerTest {
  FakeControl control;
  int pair[2];
  std::unique_ptr<TcpPeer<FakeControl>> peer;
  PeerTest() {
    assert(socketpair(AF_UNIX,SOCK_STREAM,0,pair)==0);
    for(int fd:pair)assert(fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0)|O_NONBLOCK)==0);
    peer.reset(new TcpPeer<FakeControl>(pair[0],control,11,"test","utc"));
  }
  ~PeerTest(){peer.reset();::close(pair[1]);}
  void send(const std::string& text){assert(::send(pair[1],text.data(),text.size(),0)==int(text.size()));}
  void step(unsigned n=1){while(n--){++control.now;control.logic.tick(control.now);peer->step(control.now);}}
  std::string read(){char bytes[4096];int n=recv(pair[1],bytes,sizeof(bytes),0);return n>0?std::string(bytes,n):"";}
  std::string call(const std::string& text){send(text+"\n");std::string r;for(unsigned n=0;n<200 && r.find('\n')==std::string::npos;++n){step();r+=read();}assert(!r.empty());return r;}
};
int main() {
  protocolTests();
  TcpFramer f;
  assert(f.push('{',UINT32_MAX-10)==TcpFramer::Result::More);
  assert(f.expired(240)); // wrap-safe, anchored to first byte
  assert(TcpFramer::validUtf8("abc",3));assert(!TcpFramer::validUtf8("\xc0\x80",2));
  assert(!TcpFramer::validUtf8("\xed\xa0\x80",3));assert(!TcpFramer::validUtf8("\xf4\x90\x80\x80",4));
  assert(!TcpFramer::validUtf8("\xe0",1));
  {
    PeerTest t;t.send("{\"v\":1,");t.step(30);assert(t.read().empty());
    t.send("\"id\":1,\"type\":\"hello\"}\n{\"v\":1,\"id\":2,\"type\":\"status\"}\n");
    t.step(20);auto text=t.read();assert(text.find("\"id\":1")!=text.npos && text.find("\"id\":2")!=text.npos);
    assert(t.call("{bad}").find("\"ok\":false")!=std::string::npos);
    assert(t.call("{\"v\":1,\"id\":3,\"type\":\"mode\",\"mode\":\"auto\"}").find("\"ok\":true")!=std::string::npos);
    t.call("{\"v\":1,\"id\":4,\"type\":\"acquire\"}");
    auto session=t.control.logic.session;t.call(command(session));assert(t.control.logic.output.laser);
    t.send("{");t.step(251);assert(!t.peer->connected());assert(t.control.logic.owner==Owner::None && !t.control.logic.output.laser);
  }
  {PeerTest t;t.send(std::string(1025,'x'));t.step(20);assert(!t.peer->connected());}
  {PeerTest t;std::string line="{\"v\":1,\"id\":1,\"type\":\"hello\"}";line.resize(1024,' ');
    assert(t.call(line).find("\"ok\":true")!=std::string::npos);assert(t.peer->connected());}
  {PeerTest t;t.send(std::string("{\0}",3));t.step();assert(!t.peer->connected());}
  {PeerTest t;t.send("\xc0\x80\n");t.step();assert(!t.peer->connected());}
  {PeerTest t;t.control.logic.setMode(ControlMode::Auto);t.call("{\"v\":1,\"id\":1,\"type\":\"acquire\"}");
    auto session=t.control.logic.session;t.send(command(session)+"\n"+command(session,2)+"\n");
    t.step(20);assert(!t.peer->connected());assert(!t.control.logic.hasCommand && !t.control.logic.output.servoEnabled);}
  {PeerTest t;t.control.logic.setMode(ControlMode::Auto);t.call("{\"v\":1,\"id\":1,\"type\":\"acquire\"}");
    t.call(command(t.control.logic.session));assert(t.control.logic.output.laser);
    ::shutdown(t.pair[1],SHUT_RDWR);t.step();assert(!t.peer->connected());assert(!t.control.logic.output.laser);}
  {PeerTest t;const int size=1024;setsockopt(t.pair[0],SOL_SOCKET,SO_SNDBUF,&size,sizeof(size));
    std::string batch;for(int i=0;i<60;++i)batch+="{\"v\":1,\"id\":1,\"type\":\"status\"}\n";
    t.send(batch);t.step(1000);assert(!t.peer->connected());}
}
