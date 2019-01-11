#ifndef SIGNALING_SERVER_PEER_DISPATCH_H_
#define SIGNALING_SERVER_PEER_DISPATCH_H_

#include <vector>

typedef struct endpoint {
  int id;
  bool used;
}endpoint;

class PeerDispatch {
  public:
    PeerDispatch() = default;
    ~PeerDispatch() = default;

    void AddClient(int id);
    void AddServer(int id);
    void DeleteClient(int id);
    void DeleteServer(int id);
    int Dispatch();
    void setUsedFlag(bool if_server, int id, bool used);
  private:
    std::vector<endpoint> Clients_;
    std::vector<endpoint> Servers_;
 };

#endif