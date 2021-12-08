#ifndef FRR_HELPER_H
#define FRR_HELPER_H

#include "ns3/dce-manager-helper.h"
#include "ns3/dce-application-helper.h"

namespace ns3 {

class FrrHelper
{
public:
  FrrHelper ();

  ApplicationContainer Install (NodeContainer nodes);
  ApplicationContainer Install (Ptr<Node> node);
  ApplicationContainer Install (std::string nodeName);

  void EnableZebraDebug (NodeContainer nodes);
  void UseManualZebraConfig (NodeContainer nodes);

  void EnableBgp (NodeContainer nodes);
  void RunBgpWithoutZebra (NodeContainer nodes);
  void UseGivenBgpConfig (Ptr<Node> node, std::string file);
  void FatTreeBgpConfig (Ptr<Node> node, int asn, int num_tor, int num_fabric, 
                        std::string network, std::vector<std::string> *ips);
  void BgpNodeFailure (Ptr<Node> node, Time at);
  void BgpDelayedStart (Ptr<Node> node, Time at);

  void EnableOspf (NodeContainer nodes, const char *network);
  void SetOspfRouterId (Ptr<Node> node, const char *routerid);
  void EnableOspfDebug (NodeContainer nodes);

private:
  ApplicationContainer InstallPriv (Ptr<Node> node);
  void GenerateConfigBasic (Ptr<Node> node);
  void GenerateConfigZebra (Ptr<Node> node);
  void GenerateConfigBgp (Ptr<Node> node);
  void GenerateConfigOspf (Ptr<Node> node);
  
};

} // namespace ns3

#endif /* FRR_HELPER_H */
