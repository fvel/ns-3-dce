#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include <vector>

namespace ns3 {

  void RunIp (Ptr<Node> node, Time at, std::string str);
  void AddAddress (Ptr<Node> node, Time at, const char *name, const char *address);
  void AddAdressesList (Ptr<Node> node, std::vector<std::string> adresses, bool ipv6);
  void RunPing (Ptr<Node> node, Time at, std::string str);

}
