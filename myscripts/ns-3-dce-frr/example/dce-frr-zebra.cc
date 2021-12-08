#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/frr-helper.h"
// example

using namespace ns3;

static void RunIp (Ptr<Node> node, Time at, std::string str)
{
  DceApplicationHelper process;
  ApplicationContainer apps;
  process.SetBinary ("ip");
  process.SetStackSize (1 << 16);
  process.ResetArguments ();
  process.ParseArguments (str.c_str ());
  apps = process.Install (node);
  apps.Start (at);
}

static void AddAddress (Ptr<Node> node, Time at, const char *name, const char *address)
{
  std::ostringstream oss;
  oss << "-f inet addr add " << address << " dev " << name;
  RunIp (node, at, oss.str ());
}

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  NodeContainer node;
  node.Create (1);

  DceManagerHelper dceManager;
  FrrHelper frr;

  dceManager.SetTaskManagerAttribute ("FiberManagerType",
                                      EnumValue (0));
  dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                                      "Library", StringValue ("liblinux.so"));
  dceManager.Install (node);

  AddAddress (node.Get (0), Seconds (0.1), "sim0", "10.0.0.1/24");
  RunIp (node.Get (0), Seconds (0.11), "link set lo up");
  RunIp (node.Get (0), Seconds (0.11), "link set sim0 up");

  frr.EnableZebraDebug (node);
  frr.EnableBgp (node);
  frr.Install (node);

  Simulator::Stop (Seconds (100.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
  
}
