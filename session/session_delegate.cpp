#include "session_delegate.h"
#include <iostream>

session_delegate::session_delegate(){

}
session_delegate::~session_delegate(){

}

void session_delegate::on_connected(std::shared_ptr<sender> sender) {
  if (delegate_)
  {
    delegate_->on_connected(sender);
  }
}

void session_delegate::on_read(std::shared_ptr<sender> sender, const boost::system::error_code& e, std::shared_ptr<std::string> msg) {
  if (delegate_)
  {
    delegate_->on_read(sender, e, msg);
  }
}

void session_delegate::on_send(std::shared_ptr<sender> sender, const boost::system::error_code& e, std::shared_ptr<std::string> msg) {
  if (delegate_)
  {
    delegate_->on_send(sender, e, msg);
  }
}
void session_delegate::on_error(std::shared_ptr<sender> sender, const boost::system::error_code& e) {
  if (delegate_)
  {
    delegate_->on_error(sender, e);
  }
}
void session_delegate::on_close(std::shared_ptr<class sender> sender) {
  if (delegate_)
  {
    delegate_->on_close(sender);
  }
}

void session_delegate::set_delegate(pointer d)
{
  delegate_ = d;
}
