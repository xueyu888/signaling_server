#include "peer_channel_delegate.h"

void peer_channal_delegate::on_read(std::shared_ptr<class sender> sender, const boost::system::error_code& ec, std::shared_ptr<std::string> buffer) {
  std::istringstream iss(*buffer.get());
  boost::property_tree::ptree tree;
  boost::property_tree::read_json(iss, tree);
  std::string signal = tree.get<std::string>("signal");

  if (signal == "sign-in") {
    std::string value = tree.get<std::string>("name");
    clients_.AddMember(sender, value);
  }
  else if (signal == "message") {
    std::string value = tree.get<std::string>("peer_id");
    clients_.ForwardRequestToPeer(std::stoi(value), buffer);
  }
  else if (signal == "sign-out") {
    clients_.DeleteMember(sender);
  }
  else if (signal == "keep-alive") {
    clients_.HandleKeepAlive(sender);
  }
  else {
    printf("%s UNKNOWN SIGNALING %s\n", __func__, signal.c_str());
  }
}

void peer_channal_delegate::on_close(std::shared_ptr<class sender> sender) {
  clients_.DeleteMember(sender);
}

void peer_channal_delegate::on_error(std::shared_ptr<class sender> sender, const boost::system::error_code& ec) {

}

