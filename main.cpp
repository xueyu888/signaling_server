#include "listener.h"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

using namespace boost::asio::ip;

/*
{
  "tcpPort": "8888",
  "httpPort": "6666",
  "tcpListeningIp": "0.0.0.0",
  "HttpListeningIp": "0.0.0.0",
  "threadNumber": "4"
}
*/

int main(int argc, char* argv[])
{
    boost::property_tree::ptree tree;
    boost::property_tree::json_parser::read_json("./configure.json", tree);
    int tcp_port = tree.get<int>("tcpPort");
    std::string tcp_listen_ip = tree.get<std::string>("tcpListeningIp");
    int http_port = tree.get<int>("httpPort");
    std::string http_listen_ip = tree.get<std::string>("HttpListeningIp");
    int thread_num = tree.get<int>("threadNumber");
    
    
    std::string potocol("tcp");
    std::string protocol("http");
    auto const address = address::from_string("0.0.0.0");
    const unsigned short port = 8888;
    const unsigned short httpport = 6666;
    auto const threads = 4;

    // The io_context is required for all I/O
    boost::asio::io_context ioc{threads};

    auto pcd = std::make_shared<peer_channal_delegate>();

    // Create and launch a listening port
    std::make_shared<listener>(
        ioc,
        tcp::endpoint{address, port},
		potocol,
        pcd)->run();

    std::make_shared<listener>(
        ioc,
        tcp::endpoint{address, (unsigned short)http_port},
		protocol,
        pcd)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
        [&ioc]
        {
            ioc.run();
        });
    ioc.run();

    return EXIT_SUCCESS;
}
