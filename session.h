#pragma once

#include "boost/asio.hpp"
#include "boost/beast.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <string>

namespace pt = boost::property_tree;
using namespace boost::asio::ip;
using namespace boost::asio;

class session : public std::enable_shared_from_this<session> {
public:  
  session(tcp::socket socket);
  ~session() = default;

  void run() {read();}
  void read();
  void on_read(const boost::system::error_code& ec,
                    std::shared_ptr<std::string> buffer);
         
  void send(std::shared_ptr<std::string> buffer);
  void on_send(const boost::system::error_code& ec,
                    std::shared_ptr<std::string> buffer);

  void set_protocol(std::string& protocol);
  void close();
  boost::asio::io_context& get_context();
private:
  tcp::socket socket_;
  std::string protocol_;
};