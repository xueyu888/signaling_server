#pragma once

#include "boost/asio.hpp"
#include <string>
//------------------------------------------------------------------------------

using namespace boost::asio::ip;

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    tcp::acceptor acceptor_;
    tcp::socket socket_;

public:
    listener(
        boost::asio::io_context& ioc,
        tcp::endpoint endpoint);  
  
    // Start accepting incoming connections
    void run();

    void do_accept();
 
    void on_accept(boost::system::error_code ec);
};
    