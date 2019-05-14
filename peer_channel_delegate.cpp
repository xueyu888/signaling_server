#include "peer_channel_delegate.h"

void peer_channal_delegate::on_read(std::shared_ptr<class sender> sender, const boost::system::error_code& ec, std::shared_ptr<std::string> buffer) {
  if (ec) 
	return;
  std::istringstream iss(*buffer.get());
  std::string signal;
  boost::property_tree::ptree tree;

  if (!iss)
	  printf("buffer data is error: %s\n", buffer->c_str());
  try {
	boost::property_tree::json_parser::read_json(iss, tree);
	signal = tree.get<std::string>("signal");
  }
  catch (boost::property_tree::ptree_error & ec) {
	  printf("%s\n", ec.what());
	  return;
  }

  if (signal == "sign-in") {
    std::string value = tree.get<std::string>("name");
    std::string type = tree.get<std::string>("type");
    clients_.AddMember(sender, 
                       value, 
                       type == "server" ? true : false);
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
  printf("%s sender.use_count() %d \n", __func__, sender.use_count());
}

void peer_channal_delegate::on_error(std::shared_ptr<class sender> sender, const boost::system::error_code& ec) {

}

