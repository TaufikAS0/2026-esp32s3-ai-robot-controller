#include "tcp_service.h"
#include "tcp_peer.h"
#include "firmware_version.h"
#include <fcntl.h>
#include <lwip/tcp.h>
#include <new>

namespace {
bool nonblocking(int fd) {
  const int flags=fcntl(fd,F_GETFL,0);
  return flags>=0 && fcntl(fd,F_SETFL,flags|O_NONBLOCK)==0;
}
}
bool TcpService::begin() {
  listener_=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  if(listener_<0)return false;
  const int yes=1;setsockopt(listener_,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
  sockaddr_in address{};address.sin_family=AF_INET;address.sin_port=htons(port);address.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(listener_,reinterpret_cast<sockaddr*>(&address),sizeof(address))<0 || listen(listener_,2)<0 || !nonblocking(listener_) ||
     xTaskCreatePinnedToCore(task,"tcp_control",8192,this,2,nullptr,1)!=pdPASS) {
    ::close(listener_);listener_=-1;return false;
  }
  ready_=true;return true;
}
void TcpService::task(void* context) {
  auto& self=*static_cast<TcpService*>(context);
  std::unique_ptr<TcpPeer<ControlService>> peer;
  uint32_t connection=esp_random();
  for(;;) {
    if(peer && !peer->connected())peer.reset();
    const int incoming=accept(self.listener_,nullptr,nullptr);
    if(incoming>=0) {
      if(peer || !nonblocking(incoming))::close(incoming);
      else {
        const int yes=1;setsockopt(incoming,IPPROTO_TCP,TCP_NODELAY,&yes,sizeof(yes));
        if(!++connection)++connection;
        auto next=new(std::nothrow) TcpPeer<ControlService>(incoming,self.control_,connection,kFirmwareVersion,kFirmwareBuildUtc,addNetwork,&self);
        if(next)peer.reset(next);else ::close(incoming);
      }
    }
    if(peer)peer->step(millis());
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}
void TcpService::addNetwork(cJSON* reply,void* context) {
  auto data=ApiJson::item(reply,"data");if(!ApiJson::item(data,"owner"))return;
  auto& self=*static_cast<TcpService*>(context);
  cJSON_AddNumberToObject(data,"uptime_ms",millis());
  cJSON_AddBoolToObject(data,"sta_connected",WiFi.status()==WL_CONNECTED);
  cJSON_AddStringToObject(data,"sta_ip",WiFi.localIP().toString().c_str());
  cJSON_AddStringToObject(data,"sta_mac",WiFi.macAddress().c_str());
  cJSON_AddNumberToObject(data,"wifi_disconnect_count",self.network_.disconnectCount());
  cJSON_AddNumberToObject(data,"wifi_connect_count",self.network_.connectCount());
  cJSON_AddNumberToObject(data,"wifi_disconnect_reason",self.network_.disconnectReason());
}
