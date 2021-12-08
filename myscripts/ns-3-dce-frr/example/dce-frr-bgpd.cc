#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/frr-helper.h"
#include "ns3/point-to-point-helper.h"
#include "frr-utils.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  // Parameters
  uint32_t nNodes = 2;
  uint32_t stopTime = 6000;

  //
  //  Step 0
  //  Node Basic Configuration
  //

  CommandLine cmd;
  cmd.AddValue ("nNodes", "Number of Router nodes", nNodes);
  cmd.AddValue ("stopTime", "Time to stop(seconds)", stopTime);
  cmd.Parse (argc,argv);

  //
  //  Step 1
  //  Node Basic Configuration
  //
  NodeContainer nodes;
  nodes.Create (nNodes);

  // Address conf In virtual topology
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);


  DceManagerHelper processManager;

  // processManager.SetLoader ("ns3::DlmLoaderFactory");
  processManager.SetTaskManagerAttribute ("FiberManagerType",
                                          EnumValue (0));
  processManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                                  "Library", StringValue ("liblinux.so"));
  processManager.Install (nodes);

  // IP address configuration
  AddAddress (nodes.Get (0), Seconds (0.1), "sim0", "10.0.0.1/24");
  RunIp (nodes.Get (0), Seconds (0.11), "link set lo up");
  RunIp (nodes.Get (0), Seconds (0.11), "link set sim0 up");

  AddAddress (nodes.Get (1), Seconds (0.1), "sim0", "10.0.0.2/24");
  RunIp (nodes.Get (1), Seconds (0.11), "link set lo up");
  RunIp (nodes.Get (1), Seconds (0.11), "link set sim0 up");

  FrrHelper frr;
  frr.EnableZebraDebug (nodes);
  frr.EnableBgp (nodes);
  //frr.BgpAddNeighbor (nodes.Get (0), "10.0.0.2", frr.GetAsn (nodes.Get (1)));
  //frr.BgpAddNeighbor (nodes.Get (1), "10.0.0.1", frr.GetAsn (nodes.Get (0)));
  frr.Install (nodes);

  pointToPoint.EnablePcapAll ("dce-frr-bgpd");

  //
  // Now It's ready to GO!
  //
  if (stopTime != 0)
    {
      Simulator::Stop (Seconds (stopTime));
    }
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
