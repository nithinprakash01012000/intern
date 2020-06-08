#include "ns3/command-line.h"
 #include "ns3/config.h"
#include "ns3/config-store.h"
#include "ns3/core-module.h"
 #include "ns3/string.h"
 #include "ns3/log.h"
 #include "ns3/yans-wifi-helper.h"
#include "ns3/network-module.h"
 #include "ns3/ssid.h"
#include "ns3/internet-module.h"
 #include "ns3/mobility-helper.h"
 #include "ns3/on-off-helper.h"
 #include "ns3/yans-wifi-channel.h"
 #include "ns3/yans-wifi-phy.h"
 #include "ns3/mobility-model.h"
 #include "ns3/packet-sink.h"
#include "ns3/wifi-net-device.h"
 #include "ns3/packet-sink-helper.h"
 #include "ns3/tcp-westwood.h"
 #include "ns3/internet-stack-helper.h"
 #include "ns3/ipv4-address-helper.h"
 #include "ns3/ipv4-global-routing-helper.h"
 
 NS_LOG_COMPONENT_DEFINE ("wifi-tcp");
 
 using namespace ns3;
 
 Ptr<PacketSink> sink;
 Ptr<PacketSink> sink1;                         /* Pointer to the packet sink application */
 uint64_t lastTotalRx = 0;
 uint64_t lastTotalRx1 = 0;                     /* The value of the last total received bytes */
 Ptr<YansWifiPhy>
 GetYansWifiPhyPtr (const NetDeviceContainer &nc)
 {
   Ptr<WifiNetDevice> wnd = nc.Get (0)->GetObject<WifiNetDevice> ();
   Ptr<WifiPhy> wp = wnd->GetPhy ();
   return wp->GetObject<YansWifiPhy> ();
 }
 void
 CalculateThroughput ()
 {
   Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
   double cur = (sink->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;
   double cur1 = (sink1->GetTotalRx () - lastTotalRx1) * (double) 8 / 1e5;     /* Convert Application RX Packets to MBits. */
   std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s"<<"\t"<< cur1 << " Mbit/s" << std::endl;
   lastTotalRx = sink->GetTotalRx ();
   lastTotalRx1 = sink1->GetTotalRx ();
   Simulator::Schedule (MilliSeconds (100), &CalculateThroughput);
 }
 
 int
 main (int argc, char *argv[])
 { uint32_t payloadSize = 1472;
   uint32_t payloadSize1 = 1472;
   uint32_t payloadSize2 = 2944;                       /* Transport layer payload size in bytes. */
   std::string dataRate1 = "100Mbps";
   std::string dataRate2 = "200Mbps";                  /* Application layer datarate. */
   std::string tcpVariant = "TcpNewReno";             /* TCP variant type. */
   std::string phyRate = "HtMcs7";                    /* Physical layer bitrate. */
   double simulationTime = 10;                        /* Simulation time in seconds. */
   bool pcapTracing = false;                          /* PCAP Tracing is enabled or not. */
 
   /* Command line argument parser setup. */
   CommandLine cmd;
   cmd.AddValue ("payloadSize1", "Payload size in bytes", payloadSize1);
   cmd.AddValue ("payloadSize2", "Payload size in bytes", payloadSize2);
   cmd.AddValue ("dataRate1", "Application data ate", dataRate1);
   cmd.AddValue ("dataRate2", "Application data ate", dataRate2);
   cmd.AddValue ("tcpVariant", "Transport protocol to use: TcpNewReno, "
                 "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                 "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat ", tcpVariant);
   cmd.AddValue ("phyRate", "Physical layer bitrate", phyRate);
   cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
   cmd.AddValue ("pcap", "Enable/disable PCAP Tracing", pcapTracing);
   cmd.Parse (argc, argv);
 
   tcpVariant = std::string ("ns3::") + tcpVariant;
   // Select TCP variant
   if (tcpVariant.compare ("ns3::TcpWestwoodPlus") == 0)
     {
       // TcpWestwoodPlus is not an actual TypeId name; we need TcpWestwood here
       Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
       // the default protocol type in ns3::TcpWestwood is WESTWOOD
       Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
     }
   else
     {
       TypeId tcpTid;
       NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (tcpVariant, &tcpTid), "TypeId " << tcpVariant << " not found");
       Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (tcpVariant)));
     }
 
   /* Configure TCP Options */
   Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
 
   WifiMacHelper wifiMac;
   WifiHelper wifiHelper,wifiHelper1;
   wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                       "DataMode", StringValue (phyRate),
                                       "ControlMode", StringValue ("HtMcs0"));
   wifiHelper1.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                       "DataMode", StringValue (phyRate),
                                       "ControlMode", StringValue ("HtMcs0"));
  wifiHelper.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  wifiHelper1.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
   /* Set up Legacy Channel */
   YansWifiChannelHelper wifiChannel;
   wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
   wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));
 
   /* Setup Physical Layer */
   YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
   wifiPhy.SetChannel (wifiChannel.Create ());
   wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
   
 
   NodeContainer networkNodes1,networkNodes2,networkNodes3;
   networkNodes1.Create (1);
   networkNodes2.Create (1);
   networkNodes3.Create(1);
   Ptr<Node> apWifiNode = networkNodes1.Get (0);
   Ptr<Node> staWifiNode1 = networkNodes2.Get (0);
   Ptr<Node> staWifiNode2 = networkNodes3.Get (0);
   
   /* Configure AP */
   Ssid ssid = Ssid ("network");
   wifiMac.SetType ("ns3::ApWifiMac",
                    "Ssid", SsidValue (ssid));
 
   NetDeviceContainer apDevice;
   
   apDevice = wifiHelper1.Install (wifiPhy, wifiMac,networkNodes1.Get (0));
   apDevice = wifiHelper.Install (wifiPhy, wifiMac, networkNodes1.Get (0));
   /* Configure STA */
   wifiMac.SetType ("ns3::StaWifiMac",
                    "Ssid", SsidValue (ssid));
  
  Ptr<YansWifiPhy> phySta;
  Ptr<YansWifiPhy> phySta1;

   NetDeviceContainer staDevices1;
   staDevices1 = wifiHelper.Install (wifiPhy, wifiMac, staWifiNode1);
   phySta = GetYansWifiPhyPtr (staDevices1);
   NS_ASSERT (phySta->GetFrequency () == 5180);
   NetDeviceContainer staDevices2;
   staDevices2 = wifiHelper1.Install (wifiPhy, wifiMac, staWifiNode2);
   phySta1 = GetYansWifiPhyPtr (staDevices2);
   NS_ASSERT (phySta1->GetFrequency () == 2412);
   /* Mobility model */
   MobilityHelper mobility;
   Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
   positionAlloc->Add (Vector (0.0, 0.0, 0.0));
   positionAlloc->Add (Vector (1.0, 1.0, 0.0));
   positionAlloc->Add (Vector (2.0, 1.0, 0.0));
   mobility.SetPositionAllocator (positionAlloc);
   mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
   mobility.Install (apWifiNode);
   mobility.Install (staWifiNode1);
   mobility.Install (staWifiNode2);
 
   /* Internet stack */
   InternetStackHelper stack;
   stack.Install (networkNodes1);
   stack.Install (networkNodes2);
   stack.Install (networkNodes3);
 
   Ipv4AddressHelper address;
   address.SetBase ("10.0.0.0", "255.255.255.0");
   Ipv4InterfaceContainer apInterface;
   apInterface = address.Assign (apDevice);
   Ipv4InterfaceContainer staInterface1,staInterface2;
   staInterface1 = address.Assign (staDevices1);
   staInterface2 = address.Assign (staDevices2);
 
   /* Populate routing table */
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
 
   /* Install TCP Receiver on the access point */
   PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
   ApplicationContainer sinkApp = sinkHelper.Install (apWifiNode);
   sink = StaticCast<PacketSink> (sinkApp.Get (0));

   PacketSinkHelper sinkHelper1 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 10));
   ApplicationContainer sinkApp1 = sinkHelper1.Install (apWifiNode);
   sink1 = StaticCast<PacketSink> (sinkApp1.Get (0)); 
   /* Install TCP/UDP Transmitter on the station */
   OnOffHelper server1 ("ns3::TcpSocketFactory", (InetSocketAddress (apInterface.GetAddress (0), 9)));
   server1.SetAttribute ("PacketSize", UintegerValue (payloadSize1));
   server1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
   server1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
   server1.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate1)));
   ApplicationContainer serverApp1 = server1.Install (staWifiNode1);

   OnOffHelper server2 ("ns3::TcpSocketFactory", (InetSocketAddress (apInterface.GetAddress (0), 10)));
   server2.SetAttribute ("PacketSize", UintegerValue (payloadSize2));
   server2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
   server2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
   server2.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate2)));
   ApplicationContainer serverApp2 = server2.Install (staWifiNode2); 


   /* Start Applications */
   sinkApp.Start (Seconds (0.0));
   sinkApp1.Start (Seconds (0.0));
   serverApp1.Start (Seconds (1.0));
   serverApp2.Start (Seconds (1.0));
   Simulator::Schedule (Seconds (1.1), &CalculateThroughput);
 
   /* Enable Traces */
   if (pcapTracing)
     {
       wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
       wifiPhy.EnablePcap ("AccessPoint", apDevice);
       wifiPhy.EnablePcap ("Station", staDevices1);
       wifiPhy.EnablePcap ("Station", staDevices2);
     }
 
   /* Start Simulation */
   Simulator::Stop (Seconds (simulationTime + 1));
   Simulator::Run ();
 
   double averageThroughput = ((sink->GetTotalRx () * 8) / (1e6 * simulationTime));
   double averageThroughput1 = ((sink1->GetTotalRx () * 8) / (1e6 * simulationTime));
 
   Simulator::Destroy ();
 
   /*if (averageThroughput < 50)
     {
      printf("nithin\n");
       NS_LOG_ERROR ("Obtained throughput is not in the expected boundaries!");
       exit (1);
     }*/
   std::cout << "\nAverage throughput: " << averageThroughput << " Mbit/s"<<"\t"<< averageThroughput1 << " Mbit/s" << std::endl;
   return 0;
 }