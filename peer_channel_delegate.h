#pragma once
#include <boost/asio.hpp>
#include "session/sender.h"
#include "peer_channel.h"

class peer_channal_delegate : public session_delegate
{
 
public:
  peer_channal_delegate() = default;
  ~peer_channal_delegate() = default;

  void on_read(std::shared_ptr<class sender> sender, const boost::system::error_code& ec, std::shared_ptr<std::string> msg);
  void on_error(std::shared_ptr<class sender> sender, const boost::system::error_code& ec);
  void on_close(std::shared_ptr<class sender> sender);

private:
  PeerChannel clients_;
};
