/*
 *  Copyright 2011 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "peer_channel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <algorithm>

#include "data_socket.h"
#include "utils.h"
#include "base/stringencode.h"


// Set to the peer id of the originator when messages are being
// exchanged between peers,  but set to the id of the receiving peer
// itself when notifications are sent from the server about the state
// of other peers.
//
// WORKAROUND: Since support for CORS varies greatly from one browser to the
// next, we don't use a custom name for our peer-id header (originally it was
// "X-Peer-Id: ").  Instead, we use a "simple header", "Pragma" which should
// always be exposed to CORS requests.  There is a special CORS header devoted
// to exposing proprietary headers (Access-Control-Expose-Headers), however
// at this point it is not working correctly in some popular browsers.
static const char kPeerIdHeader[] = "Pragma: ";

static const char* kRequestPaths[] = {
  "/wait", 
  "sign_out", 
  "/message", 
  "/client", 
  "/server",
  "/keepalive"
};

enum RequestPathIndex {
  kWait,
  kSignOut,
  kMessage,
  kClient,
  kServer,
  kKeepalive
};

const size_t kMaxNameLength = 512;

//
// ChannelMember
//

int ChannelMember::s_member_id_ = 0;

ChannelMember::ChannelMember(DataSocket* socket)
    : waiting_socket_(NULL),
      id_(++s_member_id_),
      connected_(true),
      timestamp_(time(NULL)),
      p2p_client_socket_(NULL),
      p2p_server_id_(0),
      time_keepalive_(time(NULL)) {
  assert(socket);
  assert(socket->method() == DataSocket::GET);
  assert(socket->PathEquals("/sign_in"));
  name_ = base::s_url_decode(socket->request_arguments()); 
  if (name_.empty())
    name_ = "peer_" + std::to_string(id_);
  else if (name_.length() > kMaxNameLength)
    name_.resize(kMaxNameLength);
  
  std::replace(name_.begin(), name_.end(), ',', '_');
}

ChannelMember::~ChannelMember() {}

bool ChannelMember::is_wait_request(DataSocket* ds) const {
  return ds && ds->PathEquals(kRequestPaths[kWait]);
}

bool ChannelMember::is_keep_alive(DataSocket* ds) const {
  return ds && ds->PathEquals(kRequestPaths[kKeepalive]);
}

bool ChannelMember::is_client_request(DataSocket* ds) const {
  return ds && ds->PathEquals(kRequestPaths[kClient]);
}

bool ChannelMember::is_server_request(DataSocket* ds) const {
  return ds && ds->PathEquals(kRequestPaths[kServer]);
}

void ChannelMember::keepalive() {
  time_keepalive_ = time(NULL);
}

bool ChannelMember::TimedOut() {
  return (time(NULL) - time_keepalive_) > 15;
}

std::string ChannelMember::GetPeerIdHeader() const {
  std::string ret(kPeerIdHeader + std::to_string(id_) + "\r\n");
  return ret;
}

bool ChannelMember::NotifyOfOtherMember(const ChannelMember& other) {
  assert(&other != this);
  QueueResponse("200 OK", "text/plain", GetPeerIdHeader(), other.GetEntry());
  return true;
}

bool ChannelMember::NotifyServerIdToClient(const ChannelMember* other) {
  assert(other != this);

  if (p2p_client_socket_) {
    assert(p2p_client_socket_->method() == DataSocket::GET);
    bool ok = 
      p2p_client_socket_->Send("200 OK", true, "text/plain", GetPeerIdHeader(), 
                                other ? 
                                other->GetEntryHaveServerId():
                                GetEntryNotServerId() );
    if (!ok) {
      printf("Failed to deliver data to p2p client socket\n");
    }
    p2p_client_socket_ = NULL;
    return true;
  }
  return false;
}


bool ChannelMember::NotifyClientCloseToServer(const ChannelMember* other) {
  assert(other != this);
  QueueResponse("200 OK", "text/plain", GetPeerIdHeader(), 
                other->GetEntryClientClose());
  printf("%s %d", __func__, id_);                
  return true;
}


// Returns a string in the form "name,id,connected\n"
std::string ChannelMember::GetEntry() const {
  assert(name_.length() <= kMaxNameLength);

  //name, 11-digit int, 1-digit bool, newline, null
  char entry[kMaxNameLength + 15];
  snprintf(entry, sizeof(entry), "%s,%d,%d\n",
          name_.substr(0, kMaxNameLength).c_str(), id_, connected_);
  return entry;
}

// Returns a string in the form "name,id,connected\n"
std::string ChannelMember::GetEntryClientClose() const {
  assert(name_.length() <= kMaxNameLength);

  //name, 11-digit int, 1-digit bool, newline, null
  char entry[kMaxNameLength + 15];
  snprintf(entry, sizeof(entry), "%s,%d,2\n",
          name_.substr(0, kMaxNameLength).c_str(), id_);
  return entry;
}

// Returns a string in the form "name,id,connected,#\n". 
//# Indicates that the client needs to connect to the server
std::string ChannelMember::GetEntryHaveServerId() const {
  assert(name_.length() <= kMaxNameLength);

  //name, 11-digit int, 1-digit bool, newline, null
  char entry[kMaxNameLength + 15];
  snprintf(entry, sizeof(entry), "%s,%d,0,#\n",
          name_.substr(0, kMaxNameLength).c_str(), id_);
  return entry;
}

// Returns a string in the form "name,id,connected,#\n". 
//# Indicates that the client needs to connect to the server
std::string ChannelMember::GetEntryNotServerId() const {
  assert(name_.length() <= kMaxNameLength);

  //name, 11-digit int, 1-digit bool, newline, null
  char entry[kMaxNameLength + 15];
  snprintf(entry, sizeof(entry), "%s,0,0,#\n",
          name_.substr(0, kMaxNameLength).c_str());
  return entry;
}

void ChannelMember::ForwardRequestToPeer(DataSocket* ds, ChannelMember* peer) {
  assert(peer);
  assert(ds);

  std::string extra_headers(GetPeerIdHeader());

  if (peer == this) {
    ds->Send("200 OK", true, ds->content_type(), extra_headers, ds->data());
  } else {
    printf("Client %s sending to %s\n", name_.c_str(), peer->name().c_str());
    peer->QueueResponse("200 OK", ds->content_type(), extra_headers,
                        ds->data());
    ds->Send("200 OK", true, "text/plain", "", "");
  }
}

void ChannelMember::OnClosing(DataSocket* ds) {
  if (ds == waiting_socket_) {
    waiting_socket_ = NULL;
    timestamp_ = time(NULL);
  }

  if (ds == p2p_client_socket_) 
    p2p_client_socket_ = NULL;
 }

void ChannelMember::QueueResponse(const std::string& status,
                                  const std::string& content_type,
                                  const std::string& extra_headers,
                                  const std::string& data) {
  if (waiting_socket_) {
    assert(queue_.size() == 0);
    assert(waiting_socket_->method() == DataSocket::GET);
    bool ok = 
      waiting_socket_->Send(status, true, content_type, extra_headers, data);
    if (!ok) {
      printf("Failed to deliver data to waiting socket\n");
    }
    waiting_socket_ = NULL;
    timestamp_ = time(NULL);  
  } else {
    QueuedResponse qr;
    qr.status = status;
    qr.content_type = content_type;
    qr.extra_headers = extra_headers;
    qr.data = data;
    queue_.push(qr);
  }                                
}



void ChannelMember::SetWaitingSocket(DataSocket* ds) {
  assert(ds->method() == DataSocket::GET);
  if (ds && !queue_.empty()) {
    assert(waiting_socket_ == NULL);
    const QueuedResponse& response = queue_.front();
    ds->Send(response.status, true, response.content_type,
             response.extra_headers, response.data);
    queue_.pop();
  } else {
    waiting_socket_ = ds;
  }
}

//For p2p client
void ChannelMember::SetP2pClientSocket(DataSocket* ds) {
  assert(ds->method() == DataSocket::GET);
  p2p_client_socket_ = ds;
}

int ChannelMember::get_p2p_server_id() {
  return p2p_server_id_;
}

void ChannelMember::set_p2p_server_id(int id) {
  p2p_server_id_ = id;
}

//
//PeerChannel
//

// static
bool PeerChannel::IsPeerConnection(const DataSocket* ds) {
  assert(ds);
  return (ds->method() == DataSocket::POST && ds->content_length() > 0) ||
         (ds->method() == DataSocket::GET && ds->PathEquals("/sign_in"));
}

ChannelMember* PeerChannel::Lookup(DataSocket* ds) const {
  assert(ds);

  if (ds->method() != DataSocket::GET && ds->method() != DataSocket::POST)
    return NULL;
  
  size_t i = 0;
  for (; i < ARRAYSIZE(kRequestPaths); ++i) {
    if (ds->PathEquals(kRequestPaths[i]))
      break;
  }

  if (i == ARRAYSIZE(kRequestPaths))
    return NULL;
  
  std::string args(ds->request_arguments());
  static const char kPeerId[] = "peer_id=";
  size_t found = args.find(kPeerId);
  if (found == std::string::npos)
    return NULL;

  int id = atoi(&args[found + ARRAYSIZE(kPeerId) - 1]);
  Members::const_iterator iter = members_.begin();
  for (; iter != members_.end(); ++iter) {
    if (id == (*iter)->id()) {
      if (i == kWait) {
        printf("%s id %d wait\n", __func__, id);
        (*iter)->SetWaitingSocket(ds);
      } 
      if (i == kClient) {
        printf("%s id %d client\n", __func__, id);
        (*iter)->SetP2pClientSocket(ds);
      }
      if (i == kSignOut)
        (*iter)->set_disconnected();
      return *iter;
    }
  }
  return NULL;
}

ChannelMember* PeerChannel::Lookup(int id) const {
  Members::const_iterator iter = members_.begin();
  for (; iter != members_.end(); ++iter) {
    if (id == (*iter)->id()) {
      return *iter;
    }
  }
  return NULL;
}

ChannelMember* PeerChannel::IsTargetedRequest(const DataSocket* ds) const {
  assert(ds);
  //Regardliess of GET or POST, we look for the peer_id parameter
  //only in the request_path
  const std::string& path = ds->request_path();
  size_t args = path.find('?');
  if (args == std::string::npos)
    return NULL;
  size_t found;
  const char kTargetPeerIdParam[] = "to=";
  do {
    found = path.find(kTargetPeerIdParam, args);
    if (found == std::string::npos)
      return NULL;
    if (found == (args + 1) || path[found - 1] == '&') {
      found += ARRAYSIZE(kTargetPeerIdParam) - 1;
      break;
    }
    args = found + ARRAYSIZE(kTargetPeerIdParam) - 1;
  }while(true);

  int id = atoi(&path[found]);
  Members::const_iterator i = members_.begin();
  for (; i != members_.end(); ++i) {
    if ((*i)->id() == id) {
      return *i;
    }
  }
  return NULL;
}

bool PeerChannel::AddMember(DataSocket* ds) {
  assert(IsPeerConnection(ds));
  ChannelMember* new_guy = new ChannelMember(ds);
  Members failures;
  BroadcastChangedState(*new_guy, &failures);
  HandleDeliveryFailures(&failures);
  members_.push_back(new_guy);

  printf("New member added (total=%s): %s\n",
         std::to_string(members_.size()).c_str(), new_guy->name().c_str());
  
  // Let the newly connected peer know about other members of the channel.
  std::string content_type;
  std::string response = BuildResponseForNewMember(*new_guy, &content_type);
  ds->Send("200 Added", true, content_type, new_guy->GetPeerIdHeader(),
           response);
  return true;
}

void PeerChannel::CloseAll() {
  Members::const_iterator i = members_.begin();
  for (; i != members_.end(); ++i) {
    (*i)->QueueResponse("200 OK", "text/plain", "", "Server shutting down");
  }
  DeleteAll();
}

void PeerChannel::OnClosing(DataSocket* ds, PeerDispatch& peerdispatch) {
  for (Members::iterator i = members_.begin(); i != members_.end(); ++i) {
    ChannelMember* m = (*i);
    m->OnClosing(ds);
    if (!m->connected()) {
        //if p2p client close, set server to free, delete client 
      if (m->get_p2p_server_id()) {
        peerdispatch.setUsedFlag(true, m->get_p2p_server_id(), false);
        ChannelMember* p2p_server = Lookup( m->get_p2p_server_id());
        if (p2p_server)
          p2p_server->NotifyClientCloseToServer(m);
        else 
          printf("%s no server found", __func__ );
        peerdispatch.DeleteClient(m->id());
      } else {
        peerdispatch.DeleteServer(m->id());
        m->set_p2p_server_id(0);
      } //if p2p server close, delete server

      i = members_.erase(i);
      Members failures;
      BroadcastChangedState(*m, &failures);
      HandleDeliveryFailures(&failures);
      delete m;
      if (i == members_.end())
        break;
    }
  }
  printf("Total connected: %s\n", std::to_string(members_.size()).c_str());
}

void PeerChannel::CheckForTimeout(PeerDispatch& peerdispatch) {
  for (Members::iterator i = members_.begin(); i != members_.end(); ++i) {
    ChannelMember* m = (*i);
    if (m->TimedOut()) {
      printf("Timeout: %s %d\n", m->name().c_str(), m->id());

        //if p2p client close, set server to free, delete client 
      if (m->get_p2p_server_id()) {
        peerdispatch.setUsedFlag(true, m->get_p2p_server_id(), false);
        ChannelMember* p2p_server = Lookup( m->get_p2p_server_id());
        if (p2p_server)
          p2p_server->NotifyClientCloseToServer(m);
        else 
          printf("%s no server found", __func__ );
        peerdispatch.DeleteClient(m->id());
      } else {
        peerdispatch.DeleteServer(m->id());
        m->set_p2p_server_id(0);
      } //if p2p server close, delete server
    
      m->set_disconnected();
      i = members_.erase(i);
      Members failures;
      BroadcastChangedState(*m, &failures);
      HandleDeliveryFailures(&failures);
      delete m;
      if (i == members_.end())
        break;
    }
  }
}

void PeerChannel::DeleteAll() {
  for (Members::iterator i = members_.begin(); i != members_.end(); ++i)
    delete (*i);
  members_.clear();
}

void PeerChannel::BroadcastChangedState(const ChannelMember& member,
                                        Members* delivery_failures) {
  // This function should be called prior to DataSocket::Close().
  assert(delivery_failures);

  if (!member.connected()) {
    printf("Member disconnected: %s\n", member.name().c_str());
  }

  Members::iterator i = members_.begin();
  for (; i != members_.end(); ++i) {
    if (&member != (*i)) {
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
    ChannelMember* member = *i;
    assert(!member->connected());
    failures->erase(i);
    BroadcastChangedState(*member, failures);
    delete member;
  }
}

// Builds a simple list of "name,id\n" entries for each member.
std::string PeerChannel::BuildResponseForNewMember(const ChannelMember& member,
                                                   std::string* content_type) {
  assert(content_type);

  *content_type = "text/plain";
  // The peer itself will always be the first entry.
  std::string response(member.GetEntry());
  for (Members::iterator i = members_.begin(); i != members_.end(); ++i) {
    if (member.id() != (*i)->id()) {
      assert((*i)->connected());
      response += (*i)->GetEntry();
    }
  }

  return response;
}




