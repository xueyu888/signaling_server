#pragma once

#include "boost/asio.hpp"
#include <string>
#include "peer_channel_delegate.h"
//------------------------------------------------------------------------------

using namespace boost::asio::ip;

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::string protocol_;

public:
    listener(
        boost::asio::io_context& ioc,
        tcp::endpoint endpoint,
        std::string& protocol);  
  
    // Start accepting incoming connections
    void run();

    void do_accept();
 
    void on_accept(boost::system::error_code ec);
private:
    std::shared_ptr<peer_channal_delegate> pcd_;
};
    