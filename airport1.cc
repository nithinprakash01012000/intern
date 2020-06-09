/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016
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
 * Author: Sebastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/yans-wifi-channel.h"

// This is a simple example in order to show how to configure an IEEE 802.11n Wi-Fi network
// with multiple TOS. It outputs the aggregated UDP throughput, which depends on the number of
// stations, the HT MCS value (0 to 7), the channel width (20 or 40 MHz) and the guard interval
// (long or short). The user can also specify the distance between the access point and the
// stations (in meters), and can specify whether RTS/CTS is used or not.

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiMultiTos");

int main (int argc, char *argv[])
{
  uint32_t nWifi = 10;
   uint32_t payloadSize1 = 1472;
   uint32_t payloadSize2 = 2944;                       /* Transport layer payload size in bytes. */
   std::string dataRate1 = "100Mbps";
   std::string dataRate2 = "200Mbps";  
  double simulationTime = 5; //seconds
  double distance = 1.0; //meters
  uint16_t mcs = 7;
  uint8_t channelWidth = 20; //MHz
  bool useShortGuardInterval = false;
  bool useRts = false;

  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of stations", nWifi);
  cmd.AddValue ("distance", "Distance in meters between the stations and the access point", distance);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("useRts", "Enable/disable RTS/CTS", useRts);
  cmd.AddValue ("mcs", "MCS value (0 - 7)", mcs);
  cmd.AddValue ("channelWidth", "Channel width in MHz", channelWidth);
  cmd.AddValue ("useShortGuardInterval", "Enable/disable short guard interval", useShortGuardInterval);
  cmd.Parse (argc,argv);

  NodeContainer wifiStaNodes,wifiStaNodes1;
  wifiStaNodes.Create (nWifi);
  wifiStaNodes1.Create (nWifi);
  NodeContainer wifiApNode;
  wifiApNode.Create (1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiMacHelper mac;
  WifiHelper wifi,wifi1;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi1.SetStandard(WIFI_PHY_STANDARD_80211n_2_4GHZ);
  std::ostringstream oss;
  oss << "HtMcs" << mcs;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue (oss.str ()),
                                "ControlMode", StringValue (oss.str ()),
                                "RtsCtsThreshold", UintegerValue (useRts ? 0 : 999999));
  wifi1.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue (oss.str ()),
                                "ControlMode", StringValue (oss.str ()),
                                "RtsCtsThreshold", UintegerValue (useRts ? 0 : 999999));

  Ssid ssid = Ssid ("ns3-80211n");

  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer staDevices,staDevices1;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);
  staDevices1 = wifi1.Install (phy, mac, wifiStaNodes1);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevice;
  apDevice = wifi1.Install (phy, mac, wifiApNode);
  apDevice = wifi.Install (phy, mac, wifiApNode);



  // Set channel width
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (channelWidth));

  // Set guard interval
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/HtConfiguration/ShortGuardIntervalSupported", BooleanValue (useShortGuardInterval));

  // mobility
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  for (uint32_t i = 0; i < 2*nWifi; i++)
    {
      positionAlloc->Add (Vector (distance, 0.0, 0.0));
    }
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  mobility.Install (wifiStaNodes);
  mobility.Install (wifiStaNodes1);

  // Internet stack
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);
  stack.Install (wifiStaNodes1);
  Ipv4AddressHelper address;

  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staNodeInterfaces, apNodeInterface,staNodeInterfaces1;

  staNodeInterfaces = address.Assign (staDevices);
  staNodeInterfaces1 = address.Assign (staDevices1);
  apNodeInterface = address.Assign (apDevice);

  // Setting applications
  ApplicationContainer sourceApplications, sinkApplications;
 // std::vector<uint8_t> tosValues = {0x70, 0x28, 0xb8, 0xc0}; //AC_BE, AC_BK, AC_VI, AC_VO
  uint32_t portNumber = 9;
  
  for (uint32_t index = 0; index < nWifi; ++index)
    {
      
          auto ipv4 = wifiApNode.Get (0)->GetObject<Ipv4> ();
          const auto address = ipv4->GetAddress (1, 0).GetLocal ();
          InetSocketAddress sinkSocket (address, portNumber++);
          //sinkSocket.SetTos (tosValue);
          OnOffHelper onOffHelper ("ns3::UdpSocketFactory", sinkSocket);
          onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
          onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
          onOffHelper.SetAttribute ("DataRate", DataRateValue (dataRate1));
          onOffHelper.SetAttribute ("PacketSize", UintegerValue (payloadSize1)); //bytes
          sourceApplications.Add (onOffHelper.Install (wifiStaNodes.Get (index)));
          PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", sinkSocket);
          sinkApplications.Add (packetSinkHelper.Install (wifiApNode.Get (0)));
          
        
    }
    //std::string s=
    

    for (uint32_t index = 0; index < nWifi; ++index)
    {
      
          auto ipv4 = wifiApNode.Get (0)->GetObject<Ipv4> ();
          const auto address = ipv4->GetAddress (1, 0).GetLocal ();
          InetSocketAddress sinkSocket (address, portNumber++);
          //sinkSocket.SetTos (tosValue);
          OnOffHelper onOffHelper ("ns3::UdpSocketFactory", sinkSocket);
          onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
          onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
          onOffHelper.SetAttribute ("DataRate", DataRateValue (dataRate2 ));
          onOffHelper.SetAttribute ("PacketSize", UintegerValue (payloadSize2)); //bytes
          sourceApplications.Add (onOffHelper.Install (wifiStaNodes1.Get (index)));
          PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", sinkSocket);
          sinkApplications.Add (packetSinkHelper.Install (wifiApNode.Get (0)));
      
    }

  sinkApplications.Start (Seconds (0.0));
  sinkApplications.Stop (Seconds (simulationTime + 1));
  sourceApplications.Start (Seconds (1.0));
  sourceApplications.Stop (Seconds (simulationTime + 1));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();
  std::vector<double>  throughput_values;
  double throughput = 0;
  for (uint32_t index = 0; index < 2*nWifi; ++index)
    {
      uint64_t totalPacketsThrough = DynamicCast<PacketSink> (sinkApplications.Get (index))->GetTotalRx ();
      throughput = ((totalPacketsThrough * 8) / (simulationTime * 1000000.0)); //Mbit/s
      throughput_values.push_back(throughput);
    }
  Simulator::Destroy ();
  for (uint32_t index = 0; index < 2*nWifi; ++index)
  {
    std::cout << "throughput of device "<<index+1 <<": "<< throughput_values[index] << " Mbit/s" << std::endl;
  }
  

  return 0;
}
