#include "session_delegate.h"
#include <iostream>

session_delegate::session_delegate(){

}
session_delegate::~session_delegate(){

}

void session_delegate::on_connected(std::shared_ptr<session> session) {
  if (delegate_)
  {
    delegate_->on_connected(session);
  }
}

void session_delegate::on_read(std::shared_ptr<session> session, const boost::system::error_code& e, std::shared_ptr<std::string> msg) {
  if (delegate_)
  {
    delegate_->on_read(session, e, msg);
  }
}

void session_delegate::on_send(std::shared_ptr<session> session, const boost::system::error_code& e, std::shared_ptr<std::string> msg) {
  if (delegate_)
  {
    delegate_->on_send(session, e, msg);
  }
}
void session_delegate::on_error(std::shared_ptr<session> session, const boost::system::error_code& e) {
  if (delegate_)
  {
    delegate_->on_error(session, e);
  }
}
void session_delegate::on_close(std::shared_ptr<class session> session) {
  if (delegate_)
  {
    delegate_->on_close(session);
  }
}

void session_delegate::set_delegate(pointer d)
{
  delegate_ = d;
}
