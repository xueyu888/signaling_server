#include "peer_channel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

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

ChannelMember::ChannelMember(std::shared_ptr<class session> session,
							               std::string& name)
    : id_(++s_member_id_),
      session_(session),
      connected_(true),
      timer_(session->get_context())  {

  name_ = name;
  if (name_.empty())
    name_ = "peer_" + std::to_string(id_);
  else if (name_.length() > kMaxNameLength)
    name_.resize(kMaxNameLength);
  
  std::replace(name_.begin(), name_.end(), ',', '_');
}

ChannelMember::~ChannelMember() {}


bool ChannelMember::NotifyOfOtherMember(const std::shared_ptr<ChannelMember> member) {
  pt::ptree tree;
  pt::ptree child;
  pt::ptree children;
  std::ostringstream oss;

  tree.put("signal", "notity");
  child.put("name", member->name());
  child.put("id", member->id());
  child.put("connected", member->connected());
  children.push_back(std::make_pair("", child));
  tree.add_child("peer", children);

  write_json(oss, tree);
  auto msg = std::make_shared<std::string>(oss.str());

  Send(msg);

  return true;
}


void ChannelMember::Close() {
  if (connected()) {
    set_disconnected();
    session_->close();
  }
}


void ChannelMember::Send(std::shared_ptr<std::string> buffer) {
  session_->send(buffer);
}

void ChannelMember::RecvKeepAlive() {
  timer_.expires_after(std::chrono::seconds(KEEPALIVE_TIMEOUT));
  timer_.async_wait(std::bind(&ChannelMember::OnTimeout,
							                shared_from_this(),
							                std::placeholders::_1));
}

void ChannelMember::OnTimeout(const boost::system::error_code& ec) {
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
      Close();
      return;
  }

  // Wait on the timer
  timer_.async_wait(std::bind(&ChannelMember::OnTimeout,
							                shared_from_this(),
							                std::placeholders::_1));
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

std::shared_ptr<ChannelMember> PeerChannel::Lookup(std::shared_ptr<session> session) const {
  for (auto i : members_) {
    if (i->session() == session) {
      return i;
    }
  }
  return NULL;
}

void PeerChannel::ForwardRequestToPeer(unsigned int peer_id, 
                                       std::shared_ptr<std::string> buffer) {
  auto peer = Lookup(peer_id);
  if (!peer) {
    printf("%s Can not found peer. id %u\n", __func__, peer_id);
  }

  peer->Send(buffer);
}

void PeerChannel::AddMember(std::shared_ptr<session> session,
                            std::string name) {
  auto new_guy = std::make_shared<ChannelMember>(session, name);
  Members failures;
  BroadcastChangedState(new_guy, &failures);
  HandleDeliveryFailures(&failures);
  members_.push_back(new_guy);

  printf("New member added (total=%s): %s\n",
         std::to_string(members_.size()).c_str(), new_guy->name().c_str());
  
  // Let the newly connected peer know about other members of the channel.
  std::string content_type;
  auto response = BuildResponseForNewMember(new_guy);
  new_guy->Send(response);
}

void PeerChannel::DeleteMember(std::shared_ptr<session> session) {
  auto member = Lookup(session);
  if (member) {
    member->Close();
    for (Members::iterator i = members_.begin(); i != members_.end(); ++i) {
      if (member == (*i)) {
        Members failures;
        BroadcastChangedState(member, &failures);
        HandleDeliveryFailures(&failures);
        members_.erase(i);
        break;
      }
    }
  }
}

void PeerChannel::HandleKeepAlive(std::shared_ptr<session> session) {
  auto member = Lookup(session);
  member->RecvKeepAlive();
}


void PeerChannel::CloseAll() {
  Members::const_iterator i = members_.begin();
  for (; i != members_.end(); ++i) {
    //TO DO
  }
  DeleteAll();
}

void PeerChannel::DeleteAll() {
  members_.clear();
}

void PeerChannel::BroadcastChangedState(std::shared_ptr<ChannelMember> member,
                                        Members* delivery_failures) {
  assert(delivery_failures);

  if (!member->connected()) {
    printf("Member disconnected: %s\n", member->name().c_str());
  }

  Members::iterator i = members_.begin();
  for (; i != members_.end(); ++i) {
    if (member != (*i)) {
      if (!(*i)->NotifyOfOtherMember(member)) {
        (*i)->set_disconnected();
        delivery_failures->push_back(*i);
        i = members_.erase(i);
        if (i == members_.end())
          break;
      }
    }
  }
}

void PeerChannel::HandleDeliveryFailures(Members* failures) {
  assert(failures);

  while (!failures->empty()) {
    Members::iterator i = failures->begin();
    auto member = *i;
    assert(!member->connected());
    failures->erase(i);
    BroadcastChangedState(member, failures);
  }
}

std::shared_ptr<std::string> PeerChannel::BuildResponseForNewMember(const std::shared_ptr<ChannelMember> member) {
  pt::ptree tree;
  pt::ptree children;

  std::ostringstream oss;
  tree.put("signal", "success");

  for (Members::iterator i = members_.begin(); i != members_.end(); ++i) {
      if (member->id() != (*i)->id()) {
        assert((*i)->connected());
      pt::ptree child;
      child.put("name", (*i)->name());
      child.put("id", (*i)->id());
      child.put("connected", (*i)->connected());
      children.push_back(std::make_pair("", child));
    }
  }

  tree.add_child("client", children);
  pt::write_json(oss, tree);
  auto response = std::make_shared<std::string> (oss.str());
  return response;
}




