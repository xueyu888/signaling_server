#pragma once

#include <vector>
#include <map>

class PeerDispatch {
  public:
    PeerDispatch() = default;
    ~PeerDispatch() = default;

    void AddClient(int id);
    void AddServer(int id);
    void DeleteMember(int id);
    int Dispatch(int client_id);
    void setUsedFlag(bool if_server, int id, bool used);
  private:
    std::vector<int> Clients_;
    std::vector<int> Servers_;
    std::map<int, int> Map_; //<client_id, server_id>
 };
