#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/frr-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/csma-module.h"
#include "frr-utils.h"
#include "vftgen-utils.h"
#include <vector>

// https://gitlab.com/uniroma3/compunet/networks/data-center-tools/fat-tree-generator

using namespace ns3;

int main (int argc, char *argv[])
{  
  // Delete old pcaps

  system ("rm -f *.pcap");

  // Default values

  int k_leaf = 2;
  int k_top = 2;
  int redundancy = 1;
  int n_pods = 0;
  int servers_per_rack = 1;
  bool no_zebra = false;

  CommandLine cmd;
  cmd.AddValue ("k_leaf", "The number of ports of a leaf node pointing north or south", k_leaf);
  cmd.AddValue ("k_top", "The number of ports of a spine node pointing north or south", k_top);
  cmd.AddValue ("redundancy", "The number of connections from a ToF node to a PoD", redundancy);
  cmd.AddValue ("n_pods", "Used to specify the number of pods in the fabric (if not specified the maximum number of pods is used)", n_pods);
  cmd.AddValue ("servers_per_rack", "Used to specify the number of servers for each leaf (considered as Top of Rack)", servers_per_rack);
  cmd.AddValue ("no_zebra", "Set to only run bgpd without the zebra daemon", no_zebra);
  cmd.Parse (argc,argv);

  // Create Fat Tree

  struct Config config = init_config (k_leaf, k_top, redundancy, n_pods, servers_per_rack);
  FatTree* fat_tree = new FatTree ();
  fat_tree->Create (config);

  // Create collisions domains

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  CreateCollisionsDomains (fat_tree, config, csma, pointToPoint);

  // Config network and frr

  FrrHelper frr;
  SetNetworkStack (fat_tree, config);
  IpConfig (fat_tree, config, !no_zebra);
  ConfigFrr (fat_tree, config, frr, no_zebra);

  // Example link failure
  RunIp (fat_tree->pods->at (0)->leaves->at(0)->node, Seconds (30.0), "link set sim0 down");

  // Enable Captures
  
  pointToPoint.EnablePcapAll ("vftgen");

  // Go

  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();
  Simulator::Destroy ();

  RenameFolders (fat_tree, config);
  RenamePcapFiles (fat_tree, config);
  GenerateGraphImg (fat_tree, config);

}