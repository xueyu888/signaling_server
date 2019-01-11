#include "peer_dispatch.h"
#include <algorithm>
#include <functional>

void PeerDispatch::AddClient(int id) {
    endpoint ep;
    ep.used = false;
    ep.id = id;
    Clients_.push_back(ep);
}

void PeerDispatch::AddServer(int id) {
    endpoint ep;
    ep.used = false;
    ep.id = id;
    Servers_.push_back(ep);
}

void PeerDispatch::DeleteClient(int id) {
    auto iter = std::find_if(Clients_.begin(), Clients_.end(), 
                            std::bind(&FindId, id));
    Clients_.erase(iter);
}

void PeerDispatch::DeleteServer(int id) {
    auto iter = std::find_if(Servers_.begin(), Servers_.end(), 
                            std::bind(&FindId, id));
    Servers_.erase(iter);
}

int PeerDispatch::Dispatch() {
    Servers_.front().used = true;
    return Servers_.front().id;
}

bool PeerDispatch::FindId(endpoint &ep, int id) {
    return (ep.id == id);
}

void PeerDispatch::setUsedFlag(bool if_server, int id, bool used) {
    std::vector<endpoint> &ep = if_server ? Servers_ : Clients_;
    auto iter = std::find_if(ep.begin(), ep.end(), 
                        std::bind(&FindId, id));
    iter->used = used;
}
