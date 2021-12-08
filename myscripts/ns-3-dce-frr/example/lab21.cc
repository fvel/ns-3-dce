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

  NodeContainer lan0;
  lan0.Add (routers.Get (0));
  lan0.Create (1);

  NodeContainer lan1;
  lan1.Add (routers.Get (1));
  lan1.Create (1);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer lan0devices;
  lan0devices = csma.Install (lan0);
  NetDeviceContainer lan1devices;
  lan1devices = csma.Install (lan1);

  // Set network stack

  DceManagerHelper dce;
  
  dce.SetTaskManagerAttribute ("FiberManagerType", EnumValue (0));
  dce.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));
  dce.Install (lan0);
  dce.Install (lan1);
  
  // IP address configuration
  
  AddAddress (routers.Get (0), Seconds (0.1), "sim0", "193.10.11.1/24");
  AddAddress (routers.Get (0), Seconds (0.1), "sim1", "195.11.14.1/24");
  RunIp (routers.Get (0), Seconds (0.11), "link set lo up");
  RunIp (routers.Get (0), Seconds (0.11), "link set sim0 up");
  RunIp (routers.Get (0), Seconds (0.11), "link set sim1 up");

  AddAddress (routers.Get (1), Seconds (0.1), "sim0", "193.10.11.2/24");
  AddAddress (routers.Get (1), Seconds (0.1), "sim1", "200.1.1.1/24");
  RunIp (routers.Get (1), Seconds (0.11), "link set lo up");
  RunIp (routers.Get (1), Seconds (0.11), "link set sim0 up");
  RunIp (routers.Get (1), Seconds (0.11), "link set sim1 up");

  AddAddress (lan0.Get (1), Seconds (0.1), "sim0", "195.11.14.100/24");
  RunIp (lan0.Get (1), Seconds (0.11), "link set lo up");
  RunIp (lan0.Get (1), Seconds (0.11), "link set sim0 up");
  RunIp (lan0.Get (1), Seconds (0.12), "route add default via 195.11.14.1 dev sim0");

  AddAddress (lan1.Get (1), Seconds (0.1), "sim0", "200.1.1.100/24");
  RunIp (lan1.Get (1), Seconds (0.11), "link set lo up");
  RunIp (lan1.Get (1), Seconds (0.11), "link set sim0 up");
  RunIp (lan1.Get (1), Seconds (0.12), "route add default via 200.1.1.1 dev sim0");

  // Config frr
  
  FrrHelper frr;
  frr.EnableBgp (routers);
  frr.UseGivenBgpConfig (routers.Get (0), "myscripts/ns-3-dce-frr/example/config-files/lab21-0.conf");
  frr.UseGivenBgpConfig (routers.Get (1), "myscripts/ns-3-dce-frr/example/config-files/lab21-1.conf");
  frr.Install (routers);

  // Print route tables
  
  RunIp (lan0.Get (0), Seconds (30.0), "route list");
  RunIp (lan0.Get (1), Seconds (30.0), "route list");
  RunIp (lan1.Get (0), Seconds (30.0), "route list");
  RunIp (lan1.Get (1), Seconds (30.0), "route list");

  // Ping

  RunPing (lan1.Get (1), Seconds (50.0), "195.11.14.100");                                     

  // Enable Captures
  
  pointToPoint.EnablePcapAll ("lab21");

  // Go

  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
