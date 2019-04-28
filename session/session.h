#pragma once

#include "boost/asio.hpp"
#include "boost/beast.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <string>
#include "session_delegate.h"

using namespace boost::asio::ip;
using namespace boost::asio;

class session {
public:  
  session() = default;
  virtual ~session() = default;

  virtual void run() = 0;
  virtual void read() = 0;
  virtual void on_read(const boost::system::error_code& ec,
                       std::shared_ptr<std::string> buffer) = 0;
         
  virtual void send(std::shared_ptr<std::string> buffer) = 0;
  virtual void on_send(const boost::system::error_code& ec,
                       std::shared_ptr<std::string> buffer) = 0;

  virtual void set_protocol(std::string& protocol) = 0;
  virtual void close() = 0;
  virtual boost::asio::io_context& get_context() = 0;

};