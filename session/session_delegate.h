#pragma once
#include <boost/asio.hpp>
#include "sender.h"

class session_delegate
{
public:
  typedef std::shared_ptr<session_delegate> pointer;
  session_delegate();
  virtual ~session_delegate();

  void set_delegate(pointer d);

  virtual void on_connected(std::shared_ptr<class sender> sender);
  virtual void on_read(std::shared_ptr<class sender> sender, const boost::system::error_code& ec, std::shared_ptr<std::string> msg);
  virtual void on_send(std::shared_ptr<class sender> sender, const boost::system::error_code& ec, std::shared_ptr<std::string> msg);

  virtual void on_error(std::shared_ptr<class sender> sender, const boost::system::error_code& ec);
  virtual void on_close(std::shared_ptr<class sender> sender);

protected:
  pointer delegate_;
};
