#pragma once

#include "boost/asio.hpp"
#include <string>
#include <list>
#include <queue>
#include "session/sender.h"
#include "peer_dispatch.h"


using namespace boost::asio::ip;
#define KEEPALIVE_TIMEOUT  30000

//Represents a single peer connected to the server.
class ChannelMember : public std::enable_shared_from_this<ChannelMember> {
 public:
  explicit ChannelMember(std::shared_ptr<sender> sender,
                         std::string& name);
  ~ChannelMember();

  bool connected() const { return connected_; }
  int id() const { return id_; }
  void set_disconnected() { 
	  connected_ = false; 
  }
  const std::string& name() const { return name_; }
  std::shared_ptr<class sender> sender() { 
    return sender_.lock(); 
  }


  void Close();
  void KeepAlive();
  void OnTimeout(const boost::system::error_code& e);

  void Send(std::shared_ptr<std::string> buffer);

 protected:
  int id_;
  bool connected_;
  boost::asio::steady_timer timer_;
  std::string name_;
  static unsigned int s_member_id_;
  std::weak_ptr<class sender> sender_;
  boost::asio::strand<
	  boost::asio::io_context::executor_type> strand_;
};

class PeerChannel {
 public: 
  typedef std::list<std::shared_ptr<ChannelMember> > Members;

  PeerChannel() {}
  ~PeerChannel() { DeleteAll(); }

  std::shared_ptr<ChannelMember> Lookup(unsigned int id) const;
  std::shared_ptr<ChannelMember> Lookup(std::shared_ptr<sender> sesion) const;


  // Adds a new ChannelMember instance to the list of connected peers and
  // associates it with the socket.
  void AddMember(std::shared_ptr<sender> sender, 
                 std::string name,
                 bool is_server);

  void DeleteMember(std::shared_ptr<sender> sender); 

  // Cloes all connection and sends a "shutting down" message to all 
  // connected peers.
  void CloseAll();

  // Called when a socket was determined to be closing by the peer (or if the
  // connection went dead).
  void OnClosing(tcp::socket& ds);

  void HandleKeepAlive(std::shared_ptr<sender> sesion);

  void ForwardRequestToPeer(unsigned int peer_id, 
                            std::shared_ptr<std::string> buffer);
 protected:
  void DeleteAll();

  // Builds a simple list of "name,id\n" entries for each member.
  std::shared_ptr<std::string> BuildResponseForNewMember(const std::shared_ptr<ChannelMember>& member, int server_id);
 protected:
  Members members_;
  PeerDispatch peer_dispatch_;
  std::recursive_mutex member_lock_;
};

