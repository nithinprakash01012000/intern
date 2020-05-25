#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/config.h"
#include "ns3/boolean.h"
#include "ns3/command-line.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/uinteger.h"
 #include "ns3/string.h"
#include "ns3/internet-module.h"
 #include "ns3/yans-wifi-phy.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/wifi-net-device.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");
Ptr<YansWifiPhy>
 GetYansWifiPhyPtr (const NetDeviceContainer &nc)
 {
   Ptr<WifiNetDevice> wnd = nc.Get (0)->GetObject<WifiNetDevice> ();
   Ptr<WifiPhy> wp = wnd->GetPhy ();
   return wp->GetObject<YansWifiPhy> ();
 }
 void
 PrintAttributesIfEnabled (bool enabled)
 {
   if (enabled)
     {
       ConfigStore outputConfig;
       outputConfig.ConfigureAttributes ();
     }
 }

 int 
main (int argc, char *argv[])
{
	bool verbose = true;
  
  uint32_t nWifi = 5;

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer wifiStaNodes;
  NodeContainer wifiStaNodes1;
  wifiStaNodes.Create (nWifi);
  wifiStaNodes1.Create (nWifi);
  NodeContainer wifiApNode;
  wifiApNode.Create(1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());


  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::IdealWifiManager");

  WifiHelper wifi1;
  wifi1.SetRemoteStationManager ("ns3::IdealWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));
  WifiMacHelper macAp;
   macAp.SetType ("ns3::ApWifiMac",
                  "Ssid", SsidValue (ssid),
                  "BeaconInterval", TimeValue (MicroSeconds (102400)),
                  "BeaconGeneration", BooleanValue (true));
  
  NetDeviceContainer staDevices;
  NetDeviceContainer staDevices1;
  NetDeviceContainer apDevices;

  Ptr<YansWifiPhy> phySta;
  Ptr<YansWifiPhy> phySta1;
  wifi1.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
  staDevices = wifi.Install (phy, mac, wifiStaNodes);
  phySta = GetYansWifiPhyPtr (staDevices);
  //NS_ASSERT (phySta->GetFrequency () == 5180);

  //wifi1.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
  staDevices1 = wifi1.Install (phy, mac, wifiStaNodes1);
  phySta1 = GetYansWifiPhyPtr (staDevices1);
  // NS_ASSERT (phySta1->GetFrequency () == 2412);

   apDevices = wifi.Install (phy, macAp, wifiApNode.Get (0));






   MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);
  mobility.Install (wifiStaNodes1);


  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);


  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);
  stack.Install (wifiStaNodes1);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (staDevices1);
  Ipv4InterfaceContainer chumma;
  chumma=address.Assign (apDevices);

  UdpEchoServerHelper echoServer (3456);

  ApplicationContainer serverApps = echoServer.Install (wifiApNode.Get(0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (12.0));


  UdpEchoClientHelper echoClient1 (chumma.GetAddress(0), 3456);
  echoClient1.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps1 = echoClient1.Install (wifiStaNodes.Get (nWifi - 5));
  clientApps1.Start (Seconds (2.0));
  clientApps1.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient2 (chumma.GetAddress(0), 3456);
  echoClient2.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient2.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps2 = echoClient2.Install (wifiStaNodes.Get (nWifi - 4));
  clientApps2.Start (Seconds (2.0));
  clientApps2.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient3 (chumma.GetAddress(0), 3456);
  echoClient3.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient3.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient3.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps3 = echoClient3.Install (wifiStaNodes.Get (nWifi - 3));
  clientApps3.Start (Seconds (2.0));
  clientApps3.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient4 (chumma.GetAddress(0), 3456);
  echoClient4.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient4.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient4.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps4 = echoClient4.Install (wifiStaNodes.Get (nWifi - 2));
  clientApps4.Start (Seconds (2.0));
  clientApps4.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient5 (chumma.GetAddress(0), 3456);
  echoClient5.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient5.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient5.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps5 = echoClient5.Install (wifiStaNodes.Get (nWifi - 1));
  clientApps5.Start (Seconds (2.0));
  clientApps5.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient6 (chumma.GetAddress(0), 3456);
  echoClient6.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient6.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient6.SetAttribute ("PacketSize", UintegerValue (1024));
  
  ApplicationContainer clientApps6 = echoClient6.Install (wifiStaNodes1.Get (nWifi - 5));
  clientApps6.Start (Seconds (9.0));
  clientApps6.Stop (Seconds (10.0)); 


  UdpEchoClientHelper echoClient7 (chumma.GetAddress(0), 3456);
  echoClient7.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient7.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient7.SetAttribute ("PacketSize", UintegerValue (1024));
  
  ApplicationContainer clientApps7 = echoClient7.Install (wifiStaNodes1.Get (nWifi - 4));
  clientApps7.Start (Seconds (9.0));
  clientApps7.Stop (Seconds (10.0)); 

    UdpEchoClientHelper echoClient8 (chumma.GetAddress(0), 3456);
  echoClient8.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient8.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient8.SetAttribute ("PacketSize", UintegerValue (1024));
  
  ApplicationContainer clientApps8 = echoClient8.Install (wifiStaNodes1.Get (nWifi - 3));
  clientApps8.Start (Seconds (2.0));
  clientApps8.Stop (Seconds (10.0)); 

  UdpEchoClientHelper echoClient9 (chumma.GetAddress(0), 3456);
  echoClient9.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient9.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient9.SetAttribute ("PacketSize", UintegerValue (1024));
  
  ApplicationContainer clientApps9 = echoClient9.Install (wifiStaNodes1.Get (nWifi - 2));
  clientApps9.Start (Seconds (9.0));
  clientApps9.Stop (Seconds (10.0));

    UdpEchoClientHelper echoClient10 (chumma.GetAddress(0), 3456);
  echoClient10.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient10.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient10.SetAttribute ("PacketSize", UintegerValue (1024));
  
  ApplicationContainer clientApps10 = echoClient10.Install (wifiStaNodes1.Get (nWifi - 1));
  clientApps10.Start (Seconds (9.0));
  clientApps10.Stop (Seconds (10.0)); 

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));
  
  AnimationInterface anim ("anim1.xml");
  
  
  anim.SetConstantPosition(wifiApNode.Get(0), 10.0,10.0);
  printf("nithin");
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}