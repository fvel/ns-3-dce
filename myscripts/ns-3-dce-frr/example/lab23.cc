#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/frr-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/csma-module.h"
#include "frr-utils.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc,argv);

  // Create routers nodes

  NodeContainer routers;
  routers.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer rdevices;
  rdevices = pointToPoint.Install (routers);

  // Create LAN nodes

  NodeContainer lanB;
  lanB.Add (routers.Get (0));
  lanB.Create (1);

  NodeContainer lanC;
  lanC.Add (routers.Get (0));
  lanC.Create (1);

  NodeContainer lanD;
  lanD.Add (routers.Get (1));
  lanD.Create (1);

  NodeContainer lanE;
  lanE.Add (routers.Get (1));
  lanE.Create (1);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer lanBdevices = csma.Install (lanB);
  NetDeviceContainer lanCdevices = csma.Install (lanC);
  NetDeviceContainer lanDdevices = csma.Install (lanD);
  NetDeviceContainer lanEdevices = csma.Install (lanE);

  // Set network stack

  DceManagerHelper dce;
  
  dce.SetTaskManagerAttribute ("FiberManagerType", EnumValue (0));
  dce.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));
  dce.Install (lanB);
  dce.Install (lanC.Get (1));
  dce.Install (lanD);
  dce.Install (lanE.Get (1));

  // IP address configuration
  
  AddAddress (routers.Get (0), Seconds (0.1), "sim0", "193.10.11.1/24");
  AddAddress (routers.Get (0), Seconds (0.1), "sim1", "195.11.14.1/24");
  AddAddress (routers.Get (0), Seconds (0.1), "sim2", "195.11.15.1/24");
  RunIp (routers.Get (0), Seconds (0.11), "link set lo up");
  RunIp (routers.Get (0), Seconds (0.11), "link set sim0 up");
  RunIp (routers.Get (0), Seconds (0.11), "link set sim1 up");
  RunIp (routers.Get (0), Seconds (0.11), "link set sim2 up");

  AddAddress (routers.Get (1), Seconds (0.1), "sim0", "193.10.11.2/24");
  AddAddress (routers.Get (1), Seconds (0.1), "sim1", "200.1.1.1/24");
  AddAddress (routers.Get (1), Seconds (0.1), "sim2", "200.1.2.1/24");
  RunIp (routers.Get (1), Seconds (0.11), "link set lo up");
  RunIp (routers.Get (1), Seconds (0.11), "link set sim0 up");
  RunIp (routers.Get (1), Seconds (0.11), "link set sim1 up");
  RunIp (routers.Get (1), Seconds (0.11), "link set sim2 up");

  AddAddress (lanB.Get (1), Seconds (0.1), "sim0", "195.11.14.100/24");
  RunIp (lanB.Get (1), Seconds (0.11), "link set lo up");
  RunIp (lanB.Get (1), Seconds (0.11), "link set sim0 up");
  RunIp (lanB.Get (1), Seconds (0.12), "route add default via 195.11.14.1 dev sim0");

  AddAddress (lanC.Get (1), Seconds (0.1), "sim0", "195.11.15.100/24");
  RunIp (lanC.Get (1), Seconds (0.11), "link set lo up");
  RunIp (lanC.Get (1), Seconds (0.11), "link set sim0 up");
  RunIp (lanC.Get (1), Seconds (0.12), "route add default via 195.11.15.1 dev sim0");

  AddAddress (lanD.Get (1), Seconds (0.1), "sim0", "200.1.1.100/24");
  RunIp (lanD.Get (1), Seconds (0.11), "link set lo up");
  RunIp (lanD.Get (1), Seconds (0.11), "link set sim0 up");
  RunIp (lanD.Get (1), Seconds (0.12), "route add default via 200.1.1.1 dev sim0");

  AddAddress (lanE.Get (1), Seconds (0.1), "sim0", "200.1.2.100/24");
  RunIp (lanE.Get (1), Seconds (0.11), "link set lo up");
  RunIp (lanE.Get (1), Seconds (0.11), "link set sim0 up");
  RunIp (lanE.Get (1), Seconds (0.12), "route add default via 200.1.2.1 dev sim0");

  // Config frr
  
  FrrHelper frr;
  frr.EnableBgp (routers);
  frr.UseGivenBgpConfig (routers.Get (0), "myscripts/ns-3-dce-frr/example/config-files/lab23-0.conf");
  frr.UseGivenBgpConfig (routers.Get (1), "myscripts/ns-3-dce-frr/example/config-files/lab23-1.conf");
  frr.Install (routers);

  // Print route tables
  
  RunIp (routers.Get (0), Seconds (30.0), "route list");
  RunIp (routers.Get (1), Seconds (30.0), "route list");
  
  // Ping

  RunPing (lanB.Get (1), Seconds (45.0), "200.1.1.100");
  RunPing (lanB.Get (1), Seconds (55.0), "200.1.2.100");                                      
  RunPing (lanC.Get (1), Seconds (50.0), "200.1.2.100");                                     

  // Enable Captures
  
  pointToPoint.EnablePcapAll ("lab23");

  // Go

  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
