#include "listener.h"
#include <iostream>
#include <memory>
#include "session/tcp_session.h"
#include "session/http_session.h"

// Report a failure
void
fail(boost::system::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

listener::listener(
    boost::asio::io_context& ioc,
    tcp::endpoint endpoint,
    std::string& protocol,
    std::shared_ptr<peer_channal_delegate> pcd)
    : acceptor_(ioc)
    , socket_(ioc)
    , protocol_(protocol)
    , pcd_(pcd)
{
    boost::system::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if(ec)
    {
        fail(ec, "open");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if(ec)
    {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(
        boost::asio::socket_base::max_listen_connections, ec);
    if(ec)
    {
        fail(ec, "listen");
        return;
    }
}

// Start accepting incoming connections
void listener::run() {
    if(! acceptor_.is_open())
        return;
    do_accept();
}

void listener::do_accept() {
    acceptor_.async_accept(
        socket_,
        std::bind(
            &listener::on_accept,
            shared_from_this(),
            std::placeholders::_1));
}

void listener::on_accept(boost::system::error_code ec) {
    if(ec)
    {
        fail(ec, "accept");
    }
    else
    {
		
        if (protocol_ == "tcp") 
            std::make_shared<tcp_session>(std::move(socket_), pcd_)->run();
		if (protocol_ == "http")
			std::make_shared<http_session>(std::move(socket_), pcd_)->run();
    }

    // Accept another connection
    do_accept();
}
