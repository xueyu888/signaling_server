#include "peer_dispatch.h"
#include <algorithm>
#include <functional>
#include <stdexcept>

using namespace std::placeholders;

void PeerDispatch::AddClient(int id) {
    Clients_.push_back(id);
	printf("%s %d total %d\n", __func__, id, Clients_.size());
}

void PeerDispatch::AddServer(int id) {
    Servers_.push_back(id);
    printf("%s %d total %d\n", __func__, id, Servers_.size());
}

void PeerDispatch::DeleteMember(int id) {
    auto iter = std::find_if(Servers_.begin(), 
                             Servers_.end(), 
                             [id](int& i){return i == id;});
    if (iter != Servers_.end()) {
        Servers_.erase(iter);
        Map_.erase(id);
    }

    auto iter1 = std::find_if(Clients_.begin(), 
                              Clients_.end(), 
                              [id](int& i){return i == id;});
    if (iter1 != Clients_.end()) {                                
        Clients_.erase(iter1);
        for(auto it = Map_.begin(); it != Map_.end(); ++it) {
          if (it->second == id) {
            Map_.erase(it->first);
            break;
          }
        }
    }
	printf("%s %d total client %d server %d\n", 
			__func__, id, Clients_.size(), Servers_.size());
}

int PeerDispatch::Dispatch(int client_id) {
	if (!Servers_.empty()) {
    for(auto i: Servers_) {
        if (Map_.find(i) == Map_.end()) {
            Map_.insert(std::pair<int, int>(i, client_id));
            printf("%s client %d to server %d\n", 
        __func__, client_id, i);
            return i;
        }
    }
	}
	return 0;
}

  int PeerDispatch::GetPeer(int id)
  {
    auto it = Map_.find(id);
    if ( it != Map_.end())
      return it->second;
    
    for(auto it = Map_.begin(); it != Map_.end(); ++it) {
      if (it->second == id) {
        return it->first;
      }
    }
    return 0;
  }