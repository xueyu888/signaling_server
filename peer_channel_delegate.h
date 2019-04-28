#pragma once
#include <boost/asio.hpp>
#include "session/session.h"
#include "peer_channel.h"

//signaling
#define SIGNALING_SIGN_IN     "sign-in"
#define SIGNALING_SIGN_OUT    "sign-out"
#define SIGNALING_KEEP_ALIVE  "keep-alive"
#define SIGNALING_MESSGAE     "message"

static const char* protocol[] = {
  "tcp", "http", "websocket"
};

typedef enum {
	e_tcp,
	e_http,
	e_ws
}protocol_index;

class peer_channal_delegate : public session_delegate
{
 
public:
  peer_channal_delegate() = default;
  ~peer_channal_delegate() = default;

  void on_read(std::shared_ptr<class session> session, const boost::system::error_code& ec, std::shared_ptr<std::string> msg);
  void on_error(std::shared_ptr<class session> session, const boost::system::error_code& ec);
  void on_close(std::shared_ptr<class session> session);

private:
  PeerChannel clients_;
};
