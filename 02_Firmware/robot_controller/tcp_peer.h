#pragma once
#ifdef ARDUINO
#include <lwip/sockets.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <cerrno>
#include "tcp_framer.h"
#include "tcp_protocol.h"

// Socket must be nonblocking. Shared by the actual firmware and host socket tests.
template<class Control> class TcpPeer {
 public:
  using Augment = void (*)(cJSON*, void*);
  TcpPeer(int fd, Control& control, uint32_t connection, const char* version, const char* build,
          Augment augment=nullptr, void* context=nullptr)
    : fd_(fd), protocol_(control,connection,version,build), augment_(augment), context_(context) {}
  ~TcpPeer() { close(); }
  TcpPeer(const TcpPeer&)=delete;
  TcpPeer& operator=(const TcpPeer&)=delete;
  bool connected() const { return fd_>=0; }
  void close() {
    if(fd_>=0) { ::close(fd_);fd_=-1;protocol_.close(); }
    if(tx_) { cJSON_free(tx_);tx_=nullptr; }
  }
  void step(uint32_t now) {
    if(fd_<0)return;
    if(framer_.expired(now) || (tx_ && uint32_t(now-txStarted_)>=250)) { close();return; }
    if(tx_) {
      const int n=::send(fd_,tx_+sent_,length_-sent_,0);
      if(n>0)sent_+=size_t(n);
      else if(n==0 || (errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR)) { close();return; }
      if(sent_==length_) { cJSON_free(tx_);tx_=nullptr;if(closeAfterTx_)close(); }
      return; // No further requests processed while a reply is pending.
    }
    for(unsigned budget=0;budget<256;++budget) {
      char c;const int n=::recv(fd_,&c,1,0);
      if(n==0) { close();return; }
      if(n<0) { if(errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR)close();return; }
      const auto result=framer_.push(c,now);
      if(result==TcpFramer::Result::Invalid || result==TcpFramer::Result::TooLarge) { close();return; }
      if(result==TcpFramer::Result::Line) {
        if(!TcpFramer::validUtf8(framer_.line(),framer_.size())) { close();return; }
        char pending;const bool queued=::recv(fd_,&pending,1,MSG_PEEK)>0;
        auto reply=protocol_.handle(framer_.line(),queued);
        auto error=ApiJson::item(reply.get(),"error");
        closeAfterTx_=cJSON_IsString(error) && strcmp(error->valuestring,"command_pipeline_not_allowed")==0;
        if(augment_ && reply)augment_(reply.get(),context_);
        tx_=cJSON_PrintUnformatted(reply.get());framer_.reset();
        if(!tx_) { close();return; }
        length_=strlen(tx_);sent_=0;txStarted_=now;
        // cJSON owns an allocation with room only for its terminator: add newline separately.
        char* framed=static_cast<char*>(cJSON_malloc(length_+2));
        if(!framed) { close();return; }
        memcpy(framed,tx_,length_);framed[length_++]='\n';framed[length_]=0;
        cJSON_free(tx_);tx_=framed;
        return;
      }
    }
  }
 private:
  int fd_;
  TcpProtocol<Control> protocol_;
  TcpFramer framer_;
  char* tx_=nullptr;
  size_t sent_=0,length_=0;
  uint32_t txStarted_=0;
  bool closeAfterTx_=false;
  Augment augment_;
  void* context_;
};
