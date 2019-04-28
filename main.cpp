#include "listener.h"
using namespace boost::asio::ip;

int main(int argc, char* argv[])
{
    std::string potocol("tcp");
    auto const address = address::from_string("0.0.0.0");
    const unsigned short port = 8888;
    auto const threads = 4;

    // The io_context is required for all I/O
    boost::asio::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<listener>(
        ioc,
        tcp::endpoint{address, port},
		potocol)->run();

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
