#pragma once
#include <string>
#include "session.h"

//signaling
#define SIGNALING_SIGN_IN     "sign-in"
#define SIGNALING_SIGN_OUT    "sign-out"
#define SIGNALING_KEEP_ALIVE  "keep-alive"
#define SIGNALING_MESSGAE     "message"

static const char* protocol[] = {
  "tcp", "http", "websocket"
};

typedef enum  {
  e_tcp,
  e_http,
  e_ws
}protocol_index;


extern void handle_request(std::shared_ptr<class session> session, std::shared_ptr<std::string> buffer);
extern void handle_close(std::shared_ptr<class session> session);