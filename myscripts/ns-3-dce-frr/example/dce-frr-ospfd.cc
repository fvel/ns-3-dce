/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Hajime Tazaki, NICT
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Hajime Tazaki <tazaki@nict.go.jp>
 */

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
  CommandLine cmd;
  cmd.Parse (argc,argv);

  //
  //  Step 1
  //  Node Basic Configuration
  //
  NodeContainer nodes;
  nodes.Create (2);

  NodeContainer nodes2;
  nodes2.Add (nodes.Get (1));
  nodes2.Create (1);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);
  NetDeviceContainer devices2;
  devices2 = pointToPoint.Install (nodes2);
  DceManagerHelper processManager;
  //
  // Address Configuration
  //
  
  // processManager.SetLoader ("ns3::DlmLoaderFactory");
  processManager.SetTaskManagerAttribute ("FiberManagerType",
                                          EnumValue (0));
  processManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                                  "Library", StringValue ("liblinux.so"));
  processManager.Install (nodes);
  processManager.Install (nodes2.Get (1));

  // IP address configuration
  AddAddress (nodes.Get (0), Seconds (0.1), "sim0", "10.0.0.1/24");
  RunIp (nodes.Get (0), Seconds (0.11), "link set lo up");
  RunIp (nodes.Get (0), Seconds (0.11), "link set sim0 up");

  AddAddress (nodes.Get (1), Seconds (0.1), "sim0", "10.0.0.2/24");
  AddAddress (nodes.Get (1), Seconds (0.1), "sim1", "10.0.1.1/24");
  RunIp (nodes.Get (1), Seconds (0.11), "link set lo up");
  RunIp (nodes.Get (1), Seconds (0.11), "link set sim0 up");
  RunIp (nodes.Get (1), Seconds (0.11), "link set sim1 up");

  AddAddress (nodes2.Get (1), Seconds (0.1), "sim0", "10.0.1.2/24");
  RunIp (nodes2.Get (1), Seconds (0.11), "link set lo up");
  RunIp (nodes2.Get (1), Seconds (0.11), "link set sim0 up");

  FrrHelper frr;
  frr.EnableOspf (nodes, "10.0.0.0/16");
  frr.EnableOspfDebug (nodes);
  frr.EnableZebraDebug (nodes);
  frr.Install (nodes);

  frr.EnableOspf (nodes2.Get (1), "10.0.0.0/16");
  frr.EnableOspfDebug (nodes2.Get (1));
  frr.EnableZebraDebug (nodes2.Get (1));
  frr.Install (nodes2.Get (1));

  for (int i=0; i<2; i++) {
    RunIp (nodes.Get (i), Seconds (59.0), "route list");
  }

  RunIp (nodes2.Get (1), Seconds (59.0), "route list");
  RunPing (nodes.Get (0), Seconds (58.0), "10.0.1.2");
  pointToPoint.EnablePcapAll ("dce-frr-ospfd");

  //
  // Step 9
  // Now It's ready to GO!
  //
  Simulator::Stop (Seconds (60));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
