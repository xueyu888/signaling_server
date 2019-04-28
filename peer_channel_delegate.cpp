#include "peer_channel_delegate.h"

void peer_channal_delegate::on_read(std::shared_ptr<class session> session, const boost::system::error_code& ec, std::shared_ptr<std::string> buffer) {
  std::istringstream iss(*buffer.get());
  boost::property_tree::ptree tree;
  boost::property_tree::read_json(iss, tree);
  std::string signal = tree.get<std::string>("signal");

  if (signal == SIGNALING_SIGN_IN) {
    std::string value = tree.get<std::string>("name");
    clients_.AddMember(session, value);
  }
  else if (signal == SIGNALING_MESSGAE) {
    std::string value = tree.get<std::string>("peer_id");
    clients_.ForwardRequestToPeer(std::stoi(value), buffer);
  }
  else if (signal == SIGNALING_SIGN_OUT) {
    clients_.DeleteMember(session);
  }
  else if (signal == SIGNALING_KEEP_ALIVE) {
    clients_.HandleKeepAlive(session);
  }
  else {
    printf("%s UNKNOWN SIGNALING %s\n", __func__, signal.c_str());
  }
}

void peer_channal_delegate::on_close(std::shared_ptr<class session> session) {
  clients_.DeleteMember(session);
}

void peer_channal_delegate::on_error(std::shared_ptr<class session> session, const boost::system::error_code& ec) {

}

