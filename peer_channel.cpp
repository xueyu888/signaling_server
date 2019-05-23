#include "peer_channel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include <boost/bind.hpp>

namespace pt = boost::property_tree;

// Set to the peer id of the originator when messages are being
// exchanged between peers,  but set to the id of the receiving peer
// itself when notifications are sent from the server about the state
// of other peers.

const size_t kMaxNameLength = 512;

//
// ChannelMember
//
unsigned int ChannelMember::s_member_id_ = 0;

ChannelMember::ChannelMember(std::shared_ptr<class sender> sender,
							               std::string& name)
    : id_(++s_member_id_),
      sender_(sender),
      connected_(true),
      timer_(sender->get_context()),
	  strand_(timer_.get_executor()) {

  name_ = name;
  if (name_.empty())
    name_ = "peer_" + std::to_string(id_);
  else if (name_.length() > kMaxNameLength)
    name_.resize(kMaxNameLength);
  
  printf("%s connected %d\n", __func__, connected());
  std::replace(name_.begin(), name_.end(), ',', '_');
}

ChannelMember::~ChannelMember() 
{
  Close();
  printf("%s\n", __func__);
}

void ChannelMember::Close() {
  if (connected()) {
    set_disconnected();
    timer_.cancel();
		printf("%s channelmember id %d \n", __func__, id());

    if (auto s = sender_.lock())
      s->close();
  }
}

void ChannelMember::Send(std::shared_ptr<std::string> buffer) {
  if (auto s = sender_.lock())
    s->send(buffer);
}

void ChannelMember::KeepAlive() {
  pt::ptree tree;
  std::ostringstream oss;
 
  tree.put("signal", "keep-alive");
  write_json(oss, tree);
  auto msg = std::make_shared<std::string>(oss.str());
  Send(msg);


  timer_.expires_after(std::chrono::milliseconds(KEEPALIVE_TIMEOUT));
  OnTimeout({});
}

void ChannelMember::OnTimeout(const boost::system::error_code& ec) {
  if (ec == boost::asio::error::operation_aborted)
		return;

  if(ec && ec != boost::asio::error::operation_aborted) {
      printf("%s timer error: %s code %d \n", __func__, ec.message().c_str(), ec.value());
      Close();
      return;
  }

  // Verify that the timer really expired since the deadline may have moved.
  if(timer_.expiry() <= std::chrono::steady_clock::now())
  {
      // Closing the socket cancels all outstanding operations. They
      // will complete with boost::asio::error::operation_aborted
      printf("%s timer out \n", __func__);
      Close();
      return;
  }

  // Wait on the timer
  timer_.async_wait(
	  boost::asio::bind_executor(
		  strand_, boost::bind(&ChannelMember::OnTimeout,
							                 shared_from_this(),
							                 boost::asio::placeholders::error)));
}

//
//PeerChannel
//


std::shared_ptr<ChannelMember> PeerChannel::Lookup(unsigned int id) const {
  Members::const_iterator iter = members_.begin();
  for (; iter != members_.end(); ++iter) {
    if (id == (*iter)->id()) {
      return *iter;
    }
  }
  return NULL;
}

std::shared_ptr<ChannelMember> PeerChannel::Lookup(std::shared_ptr<sender> sender) const {
  for (auto i : members_) {
    if (i->sender() == sender) {
      return i;
    }
  }
  return NULL;
}

void PeerChannel::ForwardRequestToPeer(unsigned int peer_id, 
                                       std::shared_ptr<std::string> buffer) {
  std::istringstream iss(*buffer.get());
  boost::property_tree::ptree tree;
  boost::property_tree::read_json(iss, tree);

  std::unique_lock<std::recursive_mutex> lock(member_lock_);
  auto peer = Lookup(peer_id);
  int id = 0;

  if (!peer) {
    printf("%s Can not found peer. id %u\n", __func__, peer_id);
	return;
  }
  id = peer_dispatch_.GetPeer((int)peer_id);
  if (id) {
    tree.erase("peer_id");
    tree.put("peer_id", id);
    std::ostringstream oss;
    pt::write_json(oss, tree);
    auto response = std::make_shared<std::string>(oss.str());

    peer->Send(response);
  } else {
    printf("%s Can not found id in Map. id %u\n", __func__, peer_id);
  }
}

void PeerChannel::AddMember(std::shared_ptr<sender> sender,
                            std::string name,
                            bool is_server) {
  auto new_guy = std::shared_ptr<ChannelMember> (new ChannelMember(sender, name));
  int server_id = 0;
  std::unique_lock<std::recursive_mutex> lock(member_lock_);

  members_.push_back(new_guy);

  if (is_server) {
    peer_dispatch_.AddServer(new_guy->id());
  } else {
    peer_dispatch_.AddClient(new_guy->id());
    server_id = peer_dispatch_.Dispatch(new_guy->id());
  }

  printf("New member added (total=%u): %s %d\n",
         members_.size(), new_guy->name().c_str(), new_guy->id());

  HandleKeepAlive(sender);
  // Let the newly connected peer know about other members of the channel.
  auto response = BuildResponseForNewMember(new_guy, server_id);
  new_guy->Send(response);
}

void PeerChannel::DeleteMember(std::shared_ptr<sender> sender) {
  std::unique_lock<std::recursive_mutex> lock(member_lock_);
  auto member = Lookup(sender);
  if (member) {
    member->Close();
    for (Members::iterator i = members_.begin(); i != members_.end(); ++i) {
      if (member == *i) {
        members_.erase(i);
        peer_dispatch_.DeleteMember(member->id());
        break;
      }
    }
  }
  printf("%s use %d\n", __func__, member.use_count());
}

void PeerChannel::HandleKeepAlive(std::shared_ptr<sender> sender) {
  std::unique_lock<std::recursive_mutex> lock(member_lock_);
  if (auto member = Lookup(sender))
	member->KeepAlive();
}


void PeerChannel::CloseAll() {
  std::unique_lock<std::recursive_mutex> lock(member_lock_);

  Members::const_iterator i = members_.begin();
  for (; i != members_.end(); ++i) {
    //TO DO
  }
  DeleteAll();
}

void PeerChannel::DeleteAll() {
  std::unique_lock<std::recursive_mutex> lock(member_lock_);

  members_.clear();
}


std::shared_ptr<std::string> PeerChannel::BuildResponseForNewMember(
                             const std::shared_ptr<ChannelMember>& member,
                             int server_id) {
  pt::ptree tree;
  pt::ptree children;

  std::ostringstream oss;
  tree.put("signal", "success");
  tree.put("my_id", member->id());
  if (server_id)
    tree.put("peer_id", server_id);

  pt::write_json(oss, tree);
  auto response = std::make_shared<std::string> (oss.str());
  return response;
}




