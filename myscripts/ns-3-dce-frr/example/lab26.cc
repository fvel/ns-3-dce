#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/frr-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/csma-module.h"
#include "frr-utils.h"
#include <string>

using namespace ns3;

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc,argv);

  // Create nodes

  NodeContainer nodes;
  nodes.Create (6);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NodeContainer as20;
  as20.Add (nodes.Get (0));
  as20.Add (nodes.Get (1));
  csma.Install (as20);

  NodeContainer as100;
  as100.Add (nodes.Get (2));
  as100.Add (nodes.Get (4));
  csma.Install (as100);

  NodeContainer as200;
  as200.Add (nodes.Get (3));
  as200.Add (nodes.Get (5));
  csma.Install (as200);

  NodeContainer p2pE;
  p2pE.Add (nodes.Get (0));
  p2pE.Add (nodes.Get (2));
  pointToPoint.Install (p2pE);

  NodeContainer p2pF;
  p2pF.Add (nodes.Get (1));
  p2pF.Add (nodes.Get (2));
  pointToPoint.Install (p2pF);

  NodeContainer p2pA;
  p2pA.Add (nodes.Get (1));
  p2pA.Add (nodes.Get (3));
  pointToPoint.Install (p2pA);

  // Set network stack

  DceManagerHelper dce;
  
  dce.SetTaskManagerAttribute ("FiberManagerType", EnumValue (0));
  dce.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));
  dce.Install (nodes);
  
  // IP address configuration

  AddAdressesList (nodes.Get (0), {"20.1.1.2/24", "11.0.0.2/30"}, true);
  AddAdressesList (nodes.Get (1), {"20.1.1.1/24", "11.0.0.6/30", "11.0.0.34/30"}, true);
  AddAdressesList (nodes.Get (2), {"100.1.0.1/16", "11.0.0.1/30", "11.0.0.5/30"}, true);
  AddAdressesList (nodes.Get (3), {"200.2.0.1/16", "11.0.0.33/30"}, true);
  AddAdressesList (nodes.Get (4), {"100.1.0.100/16"}, true);
  AddAdressesList (nodes.Get (5), {"200.2.0.100/16"}, true);
  RunIp (nodes.Get (4), Seconds (0.12), "route add default via 100.1.0.1 dev sim0");
  RunIp (nodes.Get (5), Seconds (0.12), "route add default via 200.2.0.1 dev sim0");
  
  // Config frr
  
  FrrHelper frr;
  for (int i=0; i<4; i++) {
      frr.EnableBgp (nodes.Get (i));
      frr.UseGivenBgpConfig (nodes.Get (i), "myscripts/ns-3-dce-frr/example/config-files/lab26-" + std::to_string(i) + std::string(".conf"));
      frr.Install (nodes.Get (i));
  }

  // Print route tables
  
  for (int i=0; i<4; i++) {
    RunIp (nodes.Get (i), Seconds (59.0), "route list");
  }
  
  // Ping

  RunPing (nodes.Get (4), Seconds (45.0), "200.2.0.100");
  RunPing (nodes.Get (0), Seconds (55.0), "200.2.0.100");

  // Enable Captures
  
  pointToPoint.EnablePcapAll ("lab26");
  csma.EnablePcapAll ("lab26");

  // Go

  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
