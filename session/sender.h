#pragma once

#include "boost/asio.hpp"
#include "boost/beast.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <string>
#include "session_delegate.h"

using namespace boost::asio::ip;
using namespace boost::asio;

typedef struct{
	char msg[3000];
}message;

// Class for send message
class sender {
public:  
  sender() = default;
  virtual ~sender() = default;

  virtual void send(std::shared_ptr<std::string> buffer) = 0;
  virtual void close() = 0;
  virtual boost::asio::io_context& get_context() = 0;
};