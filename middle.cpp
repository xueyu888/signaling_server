#include "middle.h"
#include <iostream>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "peer_channel.h"

PeerChannel clients;

void handle_request(std::shared_ptr<class session> session, 
                    std::shared_ptr<std::string> buffer) {
  std::istringstream iss(*buffer.get());
  boost::property_tree::ptree tree;
  pt::read_json(iss, tree);
  std::string signal = tree.get<std::string>("signal");
  if (signal == "protocol") {
    std::string value = tree.get<std::string>("type");
    if (value == protocol[e_tcp] 
        || value == protocol[e_http] 
        || value == protocol[e_ws]) {
      session->set_protocol(value);
    } else {
      printf("%s UNKNOWN PROTOCOL %s\n", __func__, value.c_str());  
    }
  } 
  else if (signal == SIGNALING_SIGN_IN) {
    std::string value = tree.get<std::string>("name");
    clients.AddMember(session, value);
  }
  else if (signal == SIGNALING_MESSGAE) {
    std::string value = tree.get<std::string>("peer_id");
    clients.ForwardRequestToPeer(std::stoi(value), buffer);
  }
  else if (signal == SIGNALING_SIGN_OUT) {
    clients.DeleteMember(session);
  }
  else if (signal == SIGNALING_KEEP_ALIVE) {
    clients.HandleKeepAlive(session);
  }
  else {
    printf("%s UNKNOWN SIGNALING %s\n", __func__, signal.c_str());
  }
}


void handle_close(std::shared_ptr<class session> session) {
  clients.DeleteMember(session);
}