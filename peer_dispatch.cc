#include "peer_dispatch.h"
#include <algorithm>
#include <functional>

using namespace std::placeholders;

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
                            [id](endpoint& ep){return ep.id == id;});
    Clients_.erase(iter);
}

void PeerDispatch::DeleteServer(int id) {
    auto iter = std::find_if(Servers_.begin(), Servers_.end(), 
                            [id](endpoint& ep){return ep.id == id;});
    Servers_.erase(iter);
}

int PeerDispatch::Dispatch() {
	if (!Servers_.empty()) {
		Servers_.front().used = true;
		return Servers_.front().id;
	}
	else {
		return 0;
	}
}

void PeerDispatch::setUsedFlag(bool if_server, int id, bool used) {
    std::vector<endpoint> &ep = if_server ? Servers_ : Clients_;
    auto iter = std::find_if(ep.begin(), ep.end(), 
                        [id](endpoint& ep){return ep.id == id;});
    iter->used = used;
}
