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
  nodes.Create (28);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NodeContainer cable1;
  cable1.Add (nodes.Get (20));
  cable1.Add (nodes.Get (12));
  csma.Install (cable1);

  NodeContainer cable2;
  cable2.Add (nodes.Get (21));
  cable2.Add (nodes.Get (13));
  csma.Install (cable2);

  NodeContainer cable3;
  cable3.Add (nodes.Get (22));
  cable3.Add (nodes.Get (14));
  csma.Install (cable3);

  NodeContainer cable4;
  cable4.Add (nodes.Get (23));
  cable4.Add (nodes.Get (15));
  csma.Install (cable4);

  NodeContainer cable5;
  cable5.Add (nodes.Get (24));
  cable5.Add (nodes.Get (16));
  csma.Install (cable5);

  NodeContainer cable6;
  cable6.Add (nodes.Get (25));
  cable6.Add (nodes.Get (17));
  csma.Install (cable6);

  NodeContainer cable7;
  cable7.Add (nodes.Get (26));
  cable7.Add (nodes.Get (18));
  csma.Install (cable7);

  NodeContainer cable8;
  cable8.Add (nodes.Get (27));
  cable8.Add (nodes.Get (19));
  csma.Install (cable8);

  //

  NodeContainer cable9;
  cable9.Add (nodes.Get (12));
  cable9.Add (nodes.Get (4));
  pointToPoint.Install (cable9);

  NodeContainer cable10;
  cable10.Add (nodes.Get (12));
  cable10.Add (nodes.Get (5));
  pointToPoint.Install (cable10);

  NodeContainer cable11;
  cable11.Add (nodes.Get (13));
  cable11.Add (nodes.Get (4));
  pointToPoint.Install (cable11);

  NodeContainer cable12;
  cable12.Add (nodes.Get (13));
  cable12.Add (nodes.Get (5));
  pointToPoint.Install (cable12);

  NodeContainer cable13;
  cable13.Add (nodes.Get (14));
  cable13.Add (nodes.Get (6));
  pointToPoint.Install (cable13);

  NodeContainer cable14;
  cable14.Add (nodes.Get (14));
  cable14.Add (nodes.Get (7));
  pointToPoint.Install (cable14);

  NodeContainer cable15;
  cable15.Add (nodes.Get (15));
  cable15.Add (nodes.Get (6));
  pointToPoint.Install (cable15);

  NodeContainer cable16;
  cable16.Add (nodes.Get (15));
  cable16.Add (nodes.Get (7));
  pointToPoint.Install (cable16);

  NodeContainer cable17;
  cable17.Add (nodes.Get (16));
  cable17.Add (nodes.Get (8));
  pointToPoint.Install (cable17);

  NodeContainer cable18;
  cable18.Add (nodes.Get (16));
  cable18.Add (nodes.Get (9));
  pointToPoint.Install (cable18);

  NodeContainer cable19;
  cable19.Add (nodes.Get (17));
  cable19.Add (nodes.Get (8));
  pointToPoint.Install (cable19);

  NodeContainer cable20;
  cable20.Add (nodes.Get (17));
  cable20.Add (nodes.Get (9));
  pointToPoint.Install (cable20);

  NodeContainer cable21;
  cable21.Add (nodes.Get (18));
  cable21.Add (nodes.Get (10));
  pointToPoint.Install (cable21);

  NodeContainer cable22;
  cable22.Add (nodes.Get (18));
  cable22.Add (nodes.Get (11));
  pointToPoint.Install (cable22);

  NodeContainer cable23;
  cable23.Add (nodes.Get (19));
  cable23.Add (nodes.Get (10));
  pointToPoint.Install (cable23);

  NodeContainer cable24;
  cable24.Add (nodes.Get (19));
  cable24.Add (nodes.Get (11));
  pointToPoint.Install (cable24);

  //

  NodeContainer cable25;
  cable25.Add (nodes.Get (4));
  cable25.Add (nodes.Get (0));
  pointToPoint.Install (cable25);

  NodeContainer cable26;
  cable26.Add (nodes.Get (4));
  cable26.Add (nodes.Get (1));
  pointToPoint.Install (cable26);

  NodeContainer cable27;
  cable27.Add (nodes.Get (6));
  cable27.Add (nodes.Get (0));
  pointToPoint.Install (cable27);

  NodeContainer cable28;
  cable28.Add (nodes.Get (6));
  cable28.Add (nodes.Get (1));
  pointToPoint.Install (cable28);

  NodeContainer cable29;
  cable29.Add (nodes.Get (5));
  cable29.Add (nodes.Get (2));
  pointToPoint.Install (cable29);

  NodeContainer cable30;
  cable30.Add (nodes.Get (5));
  cable30.Add (nodes.Get (3));
  pointToPoint.Install (cable30);  

  NodeContainer cable31;
  cable31.Add (nodes.Get (7));
  cable31.Add (nodes.Get (2));
  pointToPoint.Install (cable31);

  NodeContainer cable32;
  cable32.Add (nodes.Get (7));
  cable32.Add (nodes.Get (3));
  pointToPoint.Install (cable32);

  NodeContainer cable33;
  cable33.Add (nodes.Get (8));
  cable33.Add (nodes.Get (0));
  pointToPoint.Install (cable33);

  NodeContainer cable34;
  cable34.Add (nodes.Get (8));
  cable34.Add (nodes.Get (1));
  pointToPoint.Install (cable34);

  NodeContainer cable35;
  cable35.Add (nodes.Get (10));
  cable35.Add (nodes.Get (0));
  pointToPoint.Install (cable35);

  NodeContainer cable36;
  cable36.Add (nodes.Get (10));
  cable36.Add (nodes.Get (1));
  pointToPoint.Install (cable36);

  NodeContainer cable37;
  cable37.Add (nodes.Get (9));
  cable37.Add (nodes.Get (2));
  pointToPoint.Install (cable37);

  NodeContainer cable38;
  cable38.Add (nodes.Get (9));
  cable38.Add (nodes.Get (3));
  pointToPoint.Install (cable38);

  NodeContainer cable39;
  cable39.Add (nodes.Get (11));
  cable39.Add (nodes.Get (2));
  pointToPoint.Install (cable39);

  NodeContainer cable40;
  cable40.Add (nodes.Get (11));
  cable40.Add (nodes.Get (3));
  pointToPoint.Install (cable40);


  // Set network stack

  DceManagerHelper dce;
  
  dce.SetTaskManagerAttribute ("FiberManagerType", EnumValue (0));
  dce.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));
  dce.Install (nodes);
  
  // IP address configuration

  AddAdressesList (nodes.Get (0), {"10.0.0.18/30", "10.0.0.50/30", "10.0.0.82/30", "10.0.0.114/30"}, true);
  AddAdressesList (nodes.Get (1), {"10.0.0.22/30", "10.0.0.54/30", "10.0.0.86/30", "10.0.0.118/30"}, true);
  AddAdressesList (nodes.Get (2), {"10.0.0.26/30", "10.0.0.58/30", "10.0.0.90/30", "10.0.0.122/30"}, true);
  AddAdressesList (nodes.Get (3), {"10.0.0.30/30", "10.0.0.62/30", "10.0.0.94/30", "10.0.0.126/30"}, true);
  AddAdressesList (nodes.Get (4), {"10.0.0.2/30", "10.0.0.10/30", "10.0.0.17/30", "10.0.0.21/30"}, true);
  AddAdressesList (nodes.Get (5), {"10.0.0.6/30", "10.0.0.14/30", "10.0.0.25/30", "10.0.0.29/30"}, true);
  AddAdressesList (nodes.Get (6), {"10.0.0.34/30", "10.0.0.42/30", "10.0.0.49/30", "10.0.0.53/30"}, true);
  AddAdressesList (nodes.Get (7), {"10.0.0.38/30", "10.0.0.46/30", "10.0.0.57/30", "10.0.0.61/30"}, true);
  AddAdressesList (nodes.Get (8), {"10.0.0.66/30", "10.0.0.74/30", "10.0.0.81/30", "10.0.0.85/30"}, true);
  AddAdressesList (nodes.Get (9), {"10.0.0.70/30", "10.0.0.78/30", "10.0.0.89/30", "10.0.0.93/30"}, true);
  AddAdressesList (nodes.Get (10), {"10.0.0.98/30", "10.0.0.106/30", "10.0.0.113/30", "10.0.0.117/30"}, true);
  AddAdressesList (nodes.Get (11), {"10.0.0.102/30", "10.0.0.110/30", "10.0.0.121/30", "10.0.0.125/30"}, true);
  AddAdressesList (nodes.Get (12), {"200.0.0.1/24", "10.0.0.1/30", "10.0.0.5/30"}, true);
  AddAdressesList (nodes.Get (13), {"200.0.1.1/24", "10.0.0.9/30", "10.0.0.13/30"}, true);
  AddAdressesList (nodes.Get (14), {"200.0.2.1/24", "10.0.0.33/30", "10.0.0.37/30"}, true);
  AddAdressesList (nodes.Get (15), {"200.0.3.1/24", "10.0.0.41/30", "10.0.0.45/30"}, true);
  AddAdressesList (nodes.Get (16), {"200.0.4.1/24", "10.0.0.65/30", "10.0.0.69/30"}, true);
  AddAdressesList (nodes.Get (17), {"200.0.5.1/24", "10.0.0.73/30", "10.0.0.77/30"}, true);
  AddAdressesList (nodes.Get (18), {"200.0.6.1/24", "10.0.0.97/30", "10.0.0.101/30"}, true);
  AddAdressesList (nodes.Get (19), {"200.0.7.1/24", "10.0.0.105/30", "10.0.0.109/30"}, true);  
  
  AddAdressesList (nodes.Get (20), {"200.0.0.2/24"}, true);
  RunIp (nodes.Get (20), Seconds (0.20), "route add default via 200.0.0.1 dev sim0");
  AddAdressesList (nodes.Get (21), {"200.0.1.2/24"}, true);
  RunIp (nodes.Get (21), Seconds (0.20), "route add default via 200.0.1.1 dev sim0");
  AddAdressesList (nodes.Get (22), {"200.0.2.2/24"}, true);
  RunIp (nodes.Get (22), Seconds (0.20), "route add default via 200.0.2.1 dev sim0");
  AddAdressesList (nodes.Get (23), {"200.0.3.2/24"}, true);
  RunIp (nodes.Get (23), Seconds (0.20), "route add default via 200.0.3.1 dev sim0");
  AddAdressesList (nodes.Get (24), {"200.0.4.2/24"}, true);
  RunIp (nodes.Get (24), Seconds (0.20), "route add default via 200.0.4.1 dev sim0");
  AddAdressesList (nodes.Get (25), {"200.0.5.2/24"}, true);
  RunIp (nodes.Get (25), Seconds (0.20), "route add default via 200.0.5.1 dev sim0");
  AddAdressesList (nodes.Get (26), {"200.0.6.2/24"}, true);
  RunIp (nodes.Get (26), Seconds (0.20), "route add default via 200.0.6.1 dev sim0");
  AddAdressesList (nodes.Get (27), {"200.0.7.2/24"}, true);
  RunIp (nodes.Get (27), Seconds (0.20), "route add default via 200.0.7.1 dev sim0");
  
  // Config frr
  
  FrrHelper frr;
  for (int i=0; i<20; i++) {
      frr.EnableBgp (nodes.Get (i));
      frr.UseGivenBgpConfig (nodes.Get (i), "myscripts/ns-3-dce-frr/example/config-files/fat_tree/" + std::to_string(i) + std::string(".conf"));
      frr.Install (nodes.Get (i));
  }

  // Print route tables
  
  for (int i=0; i<28; i++) {
    RunIp (nodes.Get (i), Seconds (59.0), "route list");
  }
  
  // Ping

  RunPing (nodes.Get (20), Seconds (45.0), "200.0.7.2");

  
  // Enable Captures
  
  pointToPoint.EnablePcapAll ("fat_tree");
  csma.EnablePcapAll ("fat_tree");

  // Go

  Simulator::Stop (Seconds (60.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
