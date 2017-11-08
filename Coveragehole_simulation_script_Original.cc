/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
*/
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/internet-module.h>
#include <ns3/lte-module.h>
#include "ns3/radio-bearer-stats-calculator.h"
#include "ns3/lte-global-pathloss-database.h"
#include <ns3/config-store-module.h>
#include <ns3/buildings-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/applications-module.h>
#include "ns3/address.h"
#include "ns3/netanim-module.h"
#include <ns3/log.h>
#include <iomanip>
#include <ios>
#include <ns3/string.h>
#include <string.h>
#include <vector>
#include <cmath>

#include <iostream>
#include <fstream>
#include <string>
#include <stdint.h>


#include "ns3/node.h"


using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("Test2Flow");



/*******************************************************Global variable for packetsick & throughput functions************************************/
uint32_t ByteCounter = 0;
uint32_t oldByteCounter = 0;
bool fileComplete=false;


/**************************************************Global Vector for initial position*******************************************/

std::vector<Vector> UesPostions;


/*****************************************************Printing Building,enb,ue positions to file******************************/
void 
PrintGnuplottableBuildingListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  uint32_t index = 0;
  for (BuildingList::Iterator it = BuildingList::Begin (); it != BuildingList::End (); ++it)
    {
      ++index;
      Box box = (*it)->GetBoundaries ();
      outFile << "set object " << index
              << " rect from " << box.xMin  << "," << box.yMin
              << " to "   << box.xMax  << "," << box.yMax
              << " front fs empty "<<" border 3 "
              << std::endl;

    }
}


void 
PrintGnuplottableUeListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
          if (uedev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" <<"UE"<<uedev->GetImsi ()
                      << "\" at "<< pos.x << "," << pos.y << " right font \"Helvetica,4\" textcolor rgb \"blue\" front point pt 1 ps 0.3 lc rgb \"grey\" offset 0.2,-0.2"
                      << std::endl;
            }
        }
    }
}

void 
PrintGnuplottableEnbListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          if (enbdev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" <<"eNB"<<enbdev->GetCellId ()
                      << "\" at "<< pos.x << "," << pos.y
                      << " center font \"Helvetica,4\" textcolor rgb \"red\" front  point pt 2 ps 0.3 lc rgb \"white\" offset 0,0"
                      << std::endl;
            }
        }
    }
}


void 
PrintUeListToFile (std::string filename,bool UePosition_FirstWrite)
{

std::ofstream outFile;
std::ofstream ueTrace;

if (UePosition_FirstWrite==true)
           {

            outFile.open(filename.c_str (),std::_S_out);
            ueTrace.open("ueTrace.txt",std::_S_out);
            UePosition_FirstWrite=false;
             }

                   else{

                        outFile.open(filename.c_str (),std::_S_app);
                        ueTrace.open("ueTrace.txt",std::_S_app);
                       
                      }


    if (!outFile.is_open () && !ueTrace.is_open())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  uint64_t Imsi;
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
          if (uedev)
            {
              Imsi=uedev->GetImsi();
              if(Imsi==1)
              {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              Vector vel = node->GetObject<MobilityModel>()->GetVelocity();
              outFile << Simulator::Now ().GetSeconds ()<<" Sec " << uedev->GetImsi ()
                      << " at  "<< pos.x << "," << pos.y << "," <<pos.z<<"  Angle "<<((std::asin(vel.y/16.6667)*180)/3.141592654)<<" Degrees"<<std::endl;
               ueTrace<<pos.x << " " << pos.y<<std::endl;
             }

            }
        }
    }

Simulator::Schedule(Seconds(0.5), &PrintUeListToFile,filename,UePosition_FirstWrite);
 
}
/**************************************Global Values******************************/


static ns3::GlobalValue g_nRun           ("nRun",
                                          "Run value for simulatio",
                                          ns3::DoubleValue (2),
                                          ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_nMacroEnbSites ("nMacroEnbSites",
                                          "Number macro sites",
                                          ns3::UintegerValue (3),
                                          ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_nUes           ("nUes",
                                          "Number Ues",
                                          ns3::UintegerValue (3),
                                          ns3::MakeUintegerChecker<uint32_t> ());


static ns3::GlobalValue g_interSiteDistance ("interSiteDistance",
                                          "Distance between macro sites",
                                          ns3::DoubleValue (500),
                                          ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_xLimits          ("xLimit",
                                            "X-axis limit of the scenario",
                                          ns3::DoubleValue (2000),
                                          ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_yLimits          ("yLimit",
                                            "Y-axis limit of the scenario",
                                          ns3::DoubleValue (2000),
                                          ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_macroEnbTxPowerDbm("macroEnbTxPowerDbm",
                                            "Tx Power [dBm]used by macro sites",
                                           ns3::DoubleValue (46),
                                          ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_macroEnbDlEarfcn ("macroEnbDlEarfcn",
                                            "DL EARFCN used by macro eNBs",
                                            ns3::UintegerValue (100),
                                            ns3::MakeUintegerChecker<uint16_t> ());

static ns3::GlobalValue g_macroEnbBandwidth ("macroEnbBandwidth",
                                             "bandwidth [num RBs] used by macro eNBs",
                                             ns3::UintegerValue (25),
                                             ns3::MakeUintegerChecker<uint16_t> ());


static ns3::GlobalValue g_epcDl ("epcDl",
                                 "if true, will activate data flows in the downlink when EPC is being used. "
                                 "If false, downlink flows won't be activated. "
                                 "If EPC is not used, this parameter will be ignored.",
                                 ns3::BooleanValue (true),
                                 ns3::MakeBooleanChecker ());
static ns3::GlobalValue g_epcUl ("epcUl",
                                 "if true, will activate data flows in the uplink when EPC is being used. "
                                 "If false, uplink flows won't be activated. "
                                 "If EPC is not used, this parameter will be ignored.",
                                 ns3::BooleanValue (false),
                                 ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_useUdp ("useTcp",
                                  "If true, the BulkSend application will be used over a TCP connection. "
                                  "otherwise, the UdpClient application will be used. ",
                                  ns3::BooleanValue (true),
                                  ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_simTime          ("simTime",
                                             "Total duration of the simulation [s]",
                                              ns3::DoubleValue (10),
                                              ns3::MakeDoubleChecker<double> ());

static ns3::GlobalValue g_generateRem ("generateRem",
                                       "if true, will generate a REM and then abort the simulation;"
                                       "if false, will run the simulation normally (without generating any REM)",
                                       ns3::BooleanValue (false),
                                       ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_numBearersPerUe ("numBearersPerUe",
                                           "How many bearers per UE there are in the simulation",
                                           ns3::UintegerValue (1),
                                           ns3::MakeUintegerChecker<uint16_t> ());

static ns3::GlobalValue g_srsPeriodicity ("srsPeriodicity",
                                          "SRS Periodicity (has to be at least "
                                          "greater than the number of UEs per eNB)",
                                          ns3::UintegerValue (80),
                                          ns3::MakeUintegerChecker<uint16_t> ());


static ns3::GlobalValue g_maxTxBufferSize ("maxTxBufferSize",
                                          "Maximum Size of the Transmission Buffer (in Bytes)",
                                          ns3::UintegerValue (60*1024), 
                                          ns3::MakeUintegerChecker<uint32_t> ());

static ns3::GlobalValue g_remRbId        ("remRbId", "Resource Block Id, for which REM will be generated,"
                                          "default value is -1, what means REM will be averaged from all RBs",
                                          ns3::IntegerValue (-1),  
                                          ns3::MakeIntegerChecker<int32_t> ());

static ns3::GlobalValue g_targetCellId    ("targetCellId",
                                          "Target Cell Id for Handover",
                                          ns3::DoubleValue (2),
                                          ns3::MakeDoubleChecker<double> ());





/*static ns3::GlobalValue g_Speed          ("Speed",
                                             "speed value of macor UE [m/s].",
                                             ns3::DoubleValue (16.6667),//16.6667
                                             ns3::MakeDoubleChecker<double> ());*/




/*********************************************Trace*******************************/

void
NotifyConnectionEstablishedUe (std::string context,
                               uint64_t imsi,
                               uint16_t cellid,
                               uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " UE IMSI " << imsi
            << ": connected to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl; 
}

void
NotifyHandoverStartUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti,
                       uint16_t targetCellId)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " UE IMSI " << imsi
            << ": previously connected to CellId " << cellid
            << " with RNTI " << rnti
            << ", doing handover to CellId " << targetCellId
            << std::endl; 
}

void
NotifyHandoverEndOkUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " UE IMSI " << imsi
            << ": successful handover to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl; 
}

void
NotifyConnectionEstablishedEnb (std::string context,
                                uint64_t imsi,
                                uint16_t cellid,
                                uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " eNB CellId " << cellid
            << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl; 
}

void
NotifyHandoverStartEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti,
                        uint16_t targetCellId)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " eNB CellId " << cellid
            << ": start handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << " to CellId " << targetCellId
            << std::endl; 
}

void
NotifyHandoverEndOkEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " eNB CellId " << cellid
            << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl; 
}

void
StoreInitialPos (Vector position)
{

 UesPostions.push_back(position);
 }

Vector
GetInitialPos(int index)
 {
  
 return UesPostions[index];
 
 }


/******************************************************file complete function*************************************/
void 
filecomp ( NodeContainer ues, ApplicationContainer serverApps ,uint32_t Filesize ,std::vector<Ipv4Address> &myVector, bool & Chk_FirstWrite)

{

BooleanValue booleanValue;
DoubleValue  doubleValue;
GlobalValue::GetValueByName ("epcDl", booleanValue);
bool epcDl = booleanValue.Get ();
GlobalValue::GetValueByName ("epcUl", booleanValue);
bool epcUl = booleanValue.Get ();
GlobalValue::GetValueByName ("targetCellId", doubleValue);
double targetCellId  = doubleValue.Get();
GlobalValue::GetValueByName ("nRun" , doubleValue );
double RunValue = doubleValue.Get();

std::ofstream filecompl;
std::ofstream Angle1;
std::ofstream Angle2;
std::ofstream Angle3;



if (epcDl && epcUl)
{

for (uint32_t f = 0; f < (2*ues.GetN ()); ++f)
{


 if (Chk_FirstWrite==true)
           {

           filecompl.open("filecomplete.txt",std::_S_out);
           filecompl << "Writting File First Time "<<Simulator::Now().GetSeconds()<<std::endl;
           Chk_FirstWrite=false;
             }

                   else{

                        filecompl.open("filecomplete.txt",std::_S_app);
                       // filecompl << "Writting File Time "<<Simulator::Now().GetSeconds()<<std::endl;
                      }




if((f%2)==0)
  {

   Address to;

  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (serverApps.Get (f));
  Ptr<Socket> Lisn_soc = sink1->GetListeningSocket ();
  Lisn_soc->GetSockName (to);
  InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (to);
 
Ipv4Address searchaddr=iaddr.GetIpv4();

if ((sink1->GetTotalRx())==Filesize)
    {


           if(std::find(myVector.begin(), myVector.end(),searchaddr) != myVector.end())
             {
                //filecompl << "Already Added "<<std::endl;
            }

                else
                      {
                          myVector.push_back(searchaddr);
                          filecompl<< "Total Bytes Received (Byte): " <<sink1->GetTotalRx()<< " and file size is "<<Filesize<<" Bytes"<<std::endl;
                          filecompl << "Download Complete for " << iaddr.GetIpv4() << " At " <<Simulator::Now ().GetSeconds ()<< " Seconds"<<std::endl;
                          fileComplete=true;
                          filecompl.close();
                          Simulator::Stop ();

                        }

                      } // end of if ((sink1->GetTotalRx())==Filesize)

    } //end of if((f%2)==0)

  filecompl.close();
 } //endof for loop

}// end of if (epcDl && epcUl)

else {

for (uint32_t f = 0; f < (ues.GetN ()); ++f)
{


 if (Chk_FirstWrite==true)
           {

                        filecompl.open("filecomplete.txt",std::_S_app);
                        Angle1.open("Angle_Range1.txt",std::_S_app);
      Angle1 << "% time\tAngle(degree)\tUe1_Initial_position\ttargetCell_id\tRunValue";
      Angle1 << std::endl;
                        Angle2.open("Angle_Range2.txt",std::_S_app);
      Angle2 << "% time\tAngle(degree)\tUe1_Initial_position\ttargetCell_id\tRunValue";
      Angle2 << std::endl;
                        Angle3.open("Angle_Range3.txt",std::_S_app);
      Angle3 << "% time\tAngle(degree)\tUe1_Initial_position\ttargetCell_id\tRunValue";
      Angle3 << std::endl;
 
           
           Chk_FirstWrite=false;
             }

                   else{

                        filecompl.open("filecomplete.txt",std::_S_app);
                        Angle1.open("Angle_Range1.txt",std::_S_app);
                        Angle2.open("Angle_Range2.txt",std::_S_app);
                        Angle3.open("Angle_Range3.txt",std::_S_app);
                        
                       
                      }



  Address to;
  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (serverApps.Get (f));
  Ptr<Socket> Lisn_soc = sink1->GetListeningSocket ();
  Lisn_soc->GetSockName (to);
  InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (to);

if(iaddr.GetIpv4()=="7.0.0.2")
 {


Ipv4Address searchaddr=iaddr.GetIpv4();

if ((sink1->GetTotalRx())>=Filesize)
    {


           if(std::find(myVector.begin(), myVector.end(),searchaddr) != myVector.end())
             {
               // filecompl << "Already Added "<<std::endl;
            }

                else
                      {
                          myVector.push_back(searchaddr);
                          uint64_t UeImsi;
                          double Angle;
                          for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
                             {
                                  Ptr<Node> node = *it;
                                  int nDevs = node->GetNDevices ();
                                     for (int j = 0; j < nDevs; j++)
                                          {
                                             Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
                                             if (uedev)
                                                {
                                                   UeImsi=uedev->GetImsi();
                                                    if(UeImsi==1)
                                                            {
                                                              
                                                              Vector Initial_pos=GetInitialPos(0);
                                                              Vector vel = node->GetObject<MobilityModel>()->GetVelocity();
                                                              std::cout<<"Velocity "<<vel<<std::endl;
                                                              Angle=((std::asin(vel.y/16.6667)*180)/3.141592654);
                                                              filecompl<< "Download Complete for " << iaddr.GetIpv4() << " At " << Simulator::Now ().GetSeconds ()<<" Sec"
<<" Angle " <<Angle<< " Initial Postion "<< Initial_pos.x << "," << Initial_pos.y << "," <<Initial_pos.z<<" Target Cell Id="<<targetCellId<<" nRun="<<RunValue<<std::endl; //-20º(0.3490658504),+30º(0.5235987756),-60(-1.0471975512)
                                                                 if(Angle<=30&&Angle>=0)
                                                                    {
                                                                      Angle1<< Simulator::Now ().GetSeconds ()<<"\t" <<(Angle)
                                                                      << "\t"<< Initial_pos.x << "," << Initial_pos.y << "," <<Initial_pos.z<<"\t"<<targetCellId<<"\t"<<RunValue<<std::endl;
                                                                     }
                                                                     if(Angle<0&&Angle>=-30)
                                                                    {
                                                                      Angle2<< Simulator::Now ().GetSeconds ()<<"\t" <<(Angle)
                                                                      << "\t"<< Initial_pos.x << "," << Initial_pos.y << "," <<Initial_pos.z<<"\t"<<targetCellId<<"\t"<<RunValue<<std::endl;
                                                                     }
                                                                      if(Angle<-30&&Angle>=-60)
                                                                    {
                                                                       Angle3<< Simulator::Now ().GetSeconds ()<<"\t" <<(Angle)
                                                                      << "\t"<< Initial_pos.x << "," << Initial_pos.y << "," <<Initial_pos.z<<"\t"<<targetCellId<<"\t"<<RunValue<<std::endl;
                                                                     }
                                                              
                                       
                                                              }

                                                 }
                                           }
                                }
                          fileComplete=true;
                          filecompl.close();
                          Angle1.close();
                          Angle2.close();
                          Angle3.close();
                          Simulator::Stop ();

                        } // end of else

                      } // end of if ((sink1->GetTotalRx())==Filesize)


filecompl.close();
   } //end of for loop

}

}// end of else


Simulator::Schedule(Seconds(0.0001), &filecomp, ues,serverApps,Filesize,myVector,Chk_FirstWrite);

}//end of filecomp Function





/******************************************************Packet Sink trace and throughput calculation*************************************/

void ReceivePacket (Ptr<const Packet> packet, const Address &) 
{ 
//NS_LOG_INFO ("Received one packet!");
ByteCounter +=packet->GetSize();
}

void Throughput(bool Thput_FileChk2)
{
  //double packetSize=536;


std::ofstream thrputB;


      if (Thput_FileChk2==true)
           {

           thrputB.open("Bytes_Throughput.txt",std::_S_out);
           //thrputB << "Writting File First Time "<<Simulator::Now().GetSeconds()<<std::endl;
           Thput_FileChk2=false;

             }

                   else{

                        thrputB.open("Bytes_Throughput.txt",std::_S_app);
                       
                      }

 
  double  throughput = (ByteCounter - oldByteCounter)*8/0.5/1024/1024;
  //NS_LOG_UNCOND ("Throughput (Mbits/sec) = " << throughput);
  thrputB <<Simulator::Now().GetSeconds()<<" "<<throughput<<std::endl;
  oldByteCounter = ByteCounter; 
  Simulator::Schedule(Seconds(0.5), &Throughput,Thput_FileChk2);

}





/***************************************************** main starts here ************************************************/




int main (int argc, char *argv[])
{

LogComponentEnable ("Test2Flow", LOG_LEVEL_LOGIC);





// change some default attributes so that they are reasonable for
  // this scenario, but do this before processing command line
  // arguments, so that the user is allowed to override these settings 

 
 Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));
 Config::SetDefault ("ns3::LteHelper::UsePdschForCqiGeneration", BooleanValue (true));
 Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::MiErrorModel));
 Config::SetDefault ("ns3::MacStatsCalculator::DlOutputFilename", StringValue ("DlMacStats_Coverage_hole.txt"));
 Config::SetDefault ("ns3::MacStatsCalculator::UlOutputFilename", StringValue ("UlMacStats_Coverage_hole.txt"));
 Config::SetDefault ("ns3::LteEnbRrc::HandoverJoiningTimeoutDuration", TimeValue (MilliSeconds(2000)));
 Config::SetDefault ("ns3::LteEnbRrc::HandoverLeavingTimeoutDuration", TimeValue (MilliSeconds (2000)));
 Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
 Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (true));

// TCP parameters

 Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1448));

  CommandLine cmd;
  cmd.Parse (argc, argv);
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override input file default values via command line
  cmd.Parse (argc, argv); 


static bool Chk_FirstWrite=true;
static bool Thput_FileChk2=true;
static bool UePosition_FirstWrite=true;


uint32_t Filesize=15728640; //15728640(15MB)
std::ofstream thrput;
std::ofstream filecompl;
std::ofstream Angle1;
std::ofstream Angle2;
std::ofstream Angle3;

thrput.open ("thrput_dual.txt",std::_S_out);

// the scenario parameters get their values from the global attributes defined above
  UintegerValue uintegerValue;
  DoubleValue doubleValue;
  BooleanValue booleanValue;
  StringValue stringValue;
  IntegerValue integerValue;



GlobalValue::GetValueByName ("nRun" , doubleValue );
double RunValue = doubleValue.Get();
GlobalValue::GetValueByName ("nMacroEnbSites", uintegerValue);
uint32_t nMacroEnbSites=uintegerValue.Get();
GlobalValue::GetValueByName ("nUes", uintegerValue);
uint32_t nUes = uintegerValue.Get();
GlobalValue::GetValueByName ("interSiteDistance", doubleValue);
double interSiteDistance = doubleValue.Get();
GlobalValue::GetValueByName ("xLimit" , doubleValue );
double xLimit = doubleValue.Get();
GlobalValue::GetValueByName ("yLimit" , doubleValue);
double yLimit = doubleValue.Get();
GlobalValue::GetValueByName ("macroEnbTxPowerDbm" , doubleValue);
double macroEnbTxPowerDbm = doubleValue.Get();
GlobalValue::GetValueByName ("macroEnbDlEarfcn" , uintegerValue);
uint16_t macroEnbDlEarfcn = uintegerValue.Get();
GlobalValue::GetValueByName ("macroEnbBandwidth" , uintegerValue);
uint16_t macroEnbBandwidth = uintegerValue.Get();
GlobalValue::GetValueByName ("simTime" , doubleValue);
double simTime = doubleValue.Get();
GlobalValue::GetValueByName ("generateRem", booleanValue);
bool generateRem = booleanValue.Get();
GlobalValue::GetValueByName ("numBearersPerUe" , uintegerValue);
uint16_t numBearersPerUe = uintegerValue.Get();
GlobalValue::GetValueByName ("srsPeriodicity" , uintegerValue);
uint16_t srsPeriodicity = uintegerValue.Get();
GlobalValue::GetValueByName ("maxTxBufferSize" , uintegerValue);
uint16_t maxTxBufferSize = uintegerValue.Get();
GlobalValue::GetValueByName ("epcDl", booleanValue);
bool epcDl = booleanValue.Get ();
GlobalValue::GetValueByName ("epcUl", booleanValue);
bool epcUl = booleanValue.Get ();
GlobalValue::GetValueByName ("useTcp", booleanValue);
bool useTcp = booleanValue.Get ();
GlobalValue::GetValueByName ("remRbId", integerValue);
int32_t remRbId = integerValue.Get ();
GlobalValue::GetValueByName ("targetCellId", doubleValue);
double targetCellId = doubleValue.Get();



/*GlobalValue::GetValueByName ("Speed", doubleValue);
double Speed = doubleValue.Get();*/


RngSeedManager::SetRun (RunValue); 
Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (srsPeriodicity));
Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (maxTxBufferSize));




/************************************* Creating the Box for Source Enbs and Target Enbs *****************************/
double UeZ= 1.5;
double EnbZ= 30;

double sourceEnb1Box_xmin = 0;
double sourceEnb1Box_xmax = (xLimit/4);
double sourceEnb1Box_ymin = (yLimit/4);
double sourceEnb1Box_ymax = (yLimit/2);

Box sourceEnb1Box (sourceEnb1Box_xmin, sourceEnb1Box_xmax, sourceEnb1Box_ymin, sourceEnb1Box_ymax, UeZ, UeZ );


double targetEnbsBox_xmin = sourceEnb1Box_xmax+100;
double targetEnbsBox_xmax = (xLimit);
double targetEnbsBox_ymin = (0);
double targetEnbsBox_ymax = (yLimit);


Box targetEnbsBox (targetEnbsBox_xmin, targetEnbsBox_xmax, targetEnbsBox_ymin , targetEnbsBox_ymax, UeZ, UeZ );




/************************************************Creating Nodes and setting LTE Attributes*********************************/


  NodeContainer moving_Ue;
  moving_Ue.Create (nUes-(nUes-1));

  NodeContainer stationary_Ues;
  stationary_Ues.Create (nUes-1);
  

  NodeContainer SourceEnb;
  SourceEnb.Create (nMacroEnbSites-(nMacroEnbSites-1));
  NodeContainer TargetEnbs;
  TargetEnbs.Create (nMacroEnbSites-1);


Ptr <LteHelper> lteHelper = CreateObject<LteHelper> ();
Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
lteHelper->SetEpcHelper (epcHelper);


lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::OkumuraHataPropagationLossModel")); 
lteHelper->AddPropagationLoss("ns3::ObstaclesPropagationLossModel","ObstacleLoss", DoubleValue(90)); //To chain Obstacle pathloss model to previously added pathloss model
lteHelper->SetSpectrumChannelType ("ns3::MultiModelSpectrumChannel");


/******************************************Install Mobility Model for eNB**********************************/

/******************************************Install Mobility Model and setting attributes for source eNB*****************************/

  Ptr<ListPositionAllocator> SenbPositionAlloc = CreateObject<ListPositionAllocator> ();
  MobilityHelper enbMobility;
 double sourceEnb_xPosition=(sourceEnb1Box.xMin+sourceEnb1Box.xMax)/2;
 double sourceEnb_yPosition=(sourceEnb1Box.yMin+sourceEnb1Box.yMax)/2;

  Vector sourceEnbPosition (sourceEnb_xPosition,sourceEnb_yPosition,EnbZ);
  SenbPositionAlloc->Add (sourceEnbPosition);
  enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator (SenbPositionAlloc);
  enbMobility.Install (SourceEnb);
  BuildingsHelper::Install(SourceEnb);





Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (macroEnbTxPowerDbm)); 
lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");
lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (macroEnbDlEarfcn));
lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (macroEnbDlEarfcn + 18000));
lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (macroEnbBandwidth));
lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (macroEnbBandwidth));
lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrpHandoverAlgorithm");
lteHelper->SetHandoverAlgorithmAttribute ("TargetCellid",DoubleValue (targetCellId));
lteHelper->SetFfrAlgorithmType("ns3::LteFrHardAlgorithm");
lteHelper->SetFfrAlgorithmAttribute("FrCellTypeId",UintegerValue(1));

NetDeviceContainer sourceEnbDevs  = lteHelper->InstallEnbDevice (SourceEnb); // Install LTE Devices in  source eNB


std::string handoverAlgorithmType1 = lteHelper->GetHandoverAlgorithmType ();
  NS_LOG_DEBUG ("HandoverAlgorithmType: " << handoverAlgorithmType1);

/******************************************Install Mobility Model and setting attributes for target eNBs****************************/

Ptr<ListPositionAllocator> TenbPositionAlloc = CreateObject<ListPositionAllocator> ();
  
 int targetEnb1_xPosition= 0 ;
 int targetEnb1_yPosition= sourceEnb1Box.yMin;
 
int distancesqr=pow(interSiteDistance,2);
int subtsqr= pow((sourceEnb_yPosition-targetEnb1_yPosition),2);
targetEnb1_xPosition = sqrt(distancesqr-subtsqr) + sourceEnb_xPosition;

for (uint16_t i = 1; i < nMacroEnbSites; i++)
    {
           
     Vector targetEnbPosition (targetEnb1_xPosition,i*targetEnb1_yPosition,EnbZ);
      TenbPositionAlloc->Add (targetEnbPosition);
    }

 
  enbMobility.SetPositionAllocator (TenbPositionAlloc);
  enbMobility.Install (TargetEnbs);
  BuildingsHelper::Install(TargetEnbs);
Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (macroEnbTxPowerDbm)); 
lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");
lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (macroEnbDlEarfcn));
lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (macroEnbDlEarfcn + 18000));
lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (macroEnbBandwidth));
lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (macroEnbBandwidth));
lteHelper->SetHandoverAlgorithmType ("ns3::NoOpHandoverAlgorithm");
lteHelper->SetFfrAlgorithmType("ns3::LteFrHardAlgorithm");
NetDeviceContainer targetEnbDevs;

lteHelper->SetFfrAlgorithmAttribute("FrCellTypeId",UintegerValue(2));
targetEnbDevs.Add(lteHelper->InstallEnbDevice (TargetEnbs.Get(0))); // Install LTE Devices in eNB
lteHelper->SetFfrAlgorithmAttribute("FrCellTypeId",UintegerValue(3));
targetEnbDevs.Add(lteHelper->InstallEnbDevice (TargetEnbs.Get(1))); // Install LTE Devices in eNB


//FR algorithm reconfiguration if needed
  PointerValue tmp;
  sourceEnbDevs.Get (0)->GetAttribute ("LteFfrAlgorithm", tmp);
  Ptr<LteFfrAlgorithm> ffrAlgorithm = DynamicCast<LteFfrAlgorithm> (tmp.GetObject ());
  ffrAlgorithm->SetAttribute ("FrCellTypeId", UintegerValue (1));


std::string handoverAlgorithmType = lteHelper->GetHandoverAlgorithmType ();
  NS_LOG_DEBUG ("HandoverAlgorithmType: " << handoverAlgorithmType);

NodeContainer macroEnbs;
macroEnbs.Add(SourceEnb);
macroEnbs.Add(TargetEnbs);
NetDeviceContainer macroEnbDevs;
macroEnbDevs.Add(sourceEnbDevs);
macroEnbDevs.Add(targetEnbDevs);


// Add X2 interface

lteHelper->AddX2Interface (macroEnbs);




/**************************************Obsatacle Placement********************************************/


Vector enbtoBlock_Pos = macroEnbs.Get (1)->GetObject<MobilityModel>()->GetPosition ();
Box box (enbtoBlock_Pos.x-3,enbtoBlock_Pos.x+11,enbtoBlock_Pos.y+10,enbtoBlock_Pos.y+15,UeZ,EnbZ+5);
Ptr<Building> obstacle = CreateObject<Building>();
obstacle->SetBoundaries (box);




/******************************************Install Mobility Model in Ue**********************************/


MobilityHelper ueMobility;

Ptr<RandomBoxPositionAllocator> ue_PositionAlloc = CreateObject<RandomBoxPositionAllocator> ();
      Ptr<UniformRandomVariable> xVal = CreateObject<UniformRandomVariable> ();
      xVal->SetAttribute ("Min", DoubleValue (350));
      xVal->SetAttribute ("Max", DoubleValue (350));
      ue_PositionAlloc->SetAttribute ("X", PointerValue (xVal));
      Ptr<UniformRandomVariable> yVal = CreateObject<UniformRandomVariable> ();
      yVal->SetAttribute ("Min", DoubleValue (750));
      yVal->SetAttribute ("Max", DoubleValue (750));
      ue_PositionAlloc->SetAttribute ("Y", PointerValue (yVal));
      Ptr<UniformRandomVariable> zVal = CreateObject<UniformRandomVariable> ();
      zVal->SetAttribute ("Min", DoubleValue (sourceEnb1Box.zMin));
      zVal->SetAttribute ("Max", DoubleValue (sourceEnb1Box.zMax));
      ue_PositionAlloc->SetAttribute ("Z", PointerValue (zVal)); 
      ueMobility.SetPositionAllocator (ue_PositionAlloc); 






 ueMobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Mode", StringValue ("Time"),
                             "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=16.6667]"),
                             "Time",TimeValue (Seconds(simTime)),
                             "Distance", DoubleValue (xLimit*2),
                             "Direction", StringValue ("ns3::UniformRandomVariable[Min=-1.0471975512|Max=0.5235987756]"),//-20º(0.3490658504)&+30º(0.5235987756),-60(-1.0471975512) 
                             "Bounds", RectangleValue (Rectangle (sourceEnb1Box_xmin,targetEnbsBox_xmax,targetEnbsBox_ymin,targetEnbsBox_ymax)));




      ueMobility.Install (moving_Ue);
      BuildingsHelper::Install(moving_Ue);
      NetDeviceContainer moving_UeDevs = lteHelper->InstallUeDevice (moving_Ue); // Install LTE Devices in moving Ue
   
NodeContainer macroUes;
NetDeviceContainer macroUeDevs;
macroUes.Add(moving_Ue);
macroUeDevs.Add(moving_UeDevs);
      
       if(nUes!=1)
 {

         ueMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
         ueMobility.Install (stationary_Ues);

       if (nUes==4)
       {
       stationary_Ues.Get(0)->GetObject<MobilityModel> ()->SetPosition (Vector (270,750,UeZ));
       stationary_Ues.Get(1)->GetObject<MobilityModel> ()->SetPosition (Vector (600,550,UeZ));   //(700,600,UeZ)
       stationary_Ues.Get(2)->GetObject<MobilityModel> ()->SetPosition (Vector (600,1000,UeZ));
        }
         
       else {
       stationary_Ues.Get(0)->GetObject<MobilityModel> ()->SetPosition (Vector (600,500,UeZ));   //(700,600,UeZ)
       stationary_Ues.Get(1)->GetObject<MobilityModel> ()->SetPosition (Vector (600,1000,UeZ));
        }
       BuildingsHelper::Install(stationary_Ues);
      NetDeviceContainer stationary_UeDevs = lteHelper->InstallUeDevice (stationary_Ues); // Install LTE Devices in stationary Ues*/
              
macroUes.Add(stationary_Ues);
macroUeDevs.Add(stationary_UeDevs);
  }                       


for (NodeContainer::Iterator it = macroUes.Begin ();it != macroUes.End ();++it)
        {
          (*it)->Initialize ();
        }

for (uint16_t n = 0; n < macroUes.GetN(); n++)
    {
           
     StoreInitialPos(macroUes.Get(n)->GetObject<MobilityModel> ()->GetPosition ()); 
    }


 Ipv4Address remoteHostAddr;
 Ipv4InterfaceContainer ueIpIfaces;
 Ptr<Node> remoteHost;

NS_LOG_LOGIC ("setting up internet and remote host");

      // Create a single RemoteHost
      NodeContainer remoteHostContainer;
      remoteHostContainer.Create (1);
      remoteHost = remoteHostContainer.Get (0);
      InternetStackHelper internet;
      internet.Install (remoteHostContainer);

      // Create the Internet
      PointToPointHelper p2ph;
      p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
      p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
      p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010))); //0.010
      //p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (0.12))); //0.010
      Ptr<Node> pgw = epcHelper->GetPgwNode ();
      NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
      Ipv4AddressHelper ipv4h;
      ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
      Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
      // in this container, interface 0 is the pgw, 1 is the remoteHost
      remoteHostAddr = internetIpIfaces.GetAddress (1);

      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
      remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

   // Install the IP stack on the UEs
      internet.Install (macroUes);
      ueIpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (macroUeDevs));

      // Attach all UEs to the respective eNodeB

 if(nUes==4)
  { 
  for (uint16_t i = 0; i < nUes; i++)
    {
      if(i==1 || i==0)
        {
           lteHelper->Attach (macroUeDevs.Get (i), macroEnbDevs.Get (i-i));
        }
        else
        {
      lteHelper->Attach (macroUeDevs.Get (i), macroEnbDevs.Get (i-1));
        }
    }
  }

else
   {
        for (uint16_t i = 0; i < nUes; i++)
         {
          
            lteHelper->Attach (macroUeDevs.Get (i), macroEnbDevs.Get (i));        
           }
    }



NS_LOG_LOGIC ("setting up applications");

      // Install and start applications on UEs and remote host
      uint16_t dlPort = 10000;
      uint16_t ulPort = 20000;
     ApplicationContainer clientApps;
     ApplicationContainer serverApps;

      // randomize a bit start times to avoid simulation artifacts
      // (e.g., buffer overflows due to packet transmissions happening
      // exactly at the same time) 
      Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
      if(useTcp)
        {
      // TCP needs to be started late enough so that all UEs are connected
          // otherwise TCP SYN packets will get lost
          startTimeSeconds->SetAttribute ("Min", DoubleValue (0.100)); 
          startTimeSeconds->SetAttribute ("Max", DoubleValue (0.110)); 
          }

        else
           {
              startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
              startTimeSeconds->SetAttribute ("Max", DoubleValue (0.010)); 
           }


for (uint32_t u = 0; u < macroUes.GetN (); ++u)
        {
          Ptr<Node> ue = macroUes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);


        for (uint32_t b = 0; b < numBearersPerUe; ++b)
            {
              ++dlPort;
              ++ulPort;
               
               if (useTcp)
                 {
                 if (epcDl)
                    {

                     NS_LOG_LOGIC ("installing TCP DL app for UE " << u);
                                       
                      BulkSendHelper dlClientHelper ("ns3::TcpSocketFactory",
                                                     InetSocketAddress (ueIpIfaces.GetAddress (u), dlPort));
                      dlClientHelper.SetAttribute ("SendSize", UintegerValue (1448));
                    
                      if(u==0)
                         {
                           dlClientHelper.SetAttribute ("MaxBytes", UintegerValue (Filesize)); //10485760
                            NS_LOG_LOGIC ("File size for UE " << u <<" "<<Filesize);
                           }
                       else
                         {
                           dlClientHelper.SetAttribute ("MaxBytes", UintegerValue (Filesize*10)); //10485760
                            NS_LOG_LOGIC ("File size for UE " << u <<" "<<Filesize*10);
                           }

                  
                      clientApps.Add (dlClientHelper.Install (remoteHost));

                       PacketSinkHelper dlPacketSinkHelper ("ns3::TcpSocketFactory", 
                                                              Address (InetSocketAddress (ueIpIfaces.GetAddress (u), dlPort)));
                
                      serverApps.Add (dlPacketSinkHelper.Install (ue));
                   }

                   if (epcUl)
                    {

                      NS_LOG_LOGIC ("installing TCP UL app for UE " << u);
                      BulkSendHelper ulClientHelper ("ns3::TcpSocketFactory",
                                                     InetSocketAddress (remoteHostAddr, ulPort));
                      ulClientHelper.SetAttribute ("MaxBytes", UintegerValue (0));
                      clientApps.Add (ulClientHelper.Install (ue));
                      Ptr<Ipv4> ipv4remoteHost = remoteHost->GetObject<Ipv4>();//
	              Ipv4InterfaceAddress ipv4_int_addr_remoteHost = ipv4remoteHost->GetAddress(1,0);//
	              Ipv4Address ip_addr_remoteHost = ipv4_int_addr_remoteHost.GetLocal();//
                      PacketSinkHelper ulPacketSinkHelper ("ns3::TcpSocketFactory", 
                                                           InetSocketAddress (ip_addr_remoteHost, ulPort));//
                   
                      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
                    }
                }

             else //use Udp
                {
                  if (epcDl)
                    {
                      NS_LOG_LOGIC ("installing UDP DL app for UE " << u);

                     OnOffHelper dlClientHelper ("ns3::UdpSocketFactory", 
                                                  Address(InetSocketAddress (ueIpIfaces.GetAddress (u), dlPort)));
                     dlClientHelper.SetConstantRate (DataRate ("100Mb/s"));
                     dlClientHelper.SetAttribute("PacketSize",UintegerValue(1472));
                    // dlClientHelper.SetAttribute("MaxBytes",UintegerValue(Filesize));       
                     clientApps.Add (dlClientHelper.Install(remoteHost));
                        
                      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", 
                                                           Address(InetSocketAddress (ueIpIfaces.GetAddress (u), dlPort)));
                   
                      serverApps.Add (dlPacketSinkHelper.Install (ue));
                    }
                  if (epcUl)
                    {
                      NS_LOG_LOGIC ("installing UDP UL app for UE " << u);
                      UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);
                      clientApps.Add (ulClientHelper.Install (ue));

                       Ptr<Ipv4> ipv4remoteHost = remoteHost->GetObject<Ipv4>();//
	              Ipv4InterfaceAddress ipv4_int_addr_remoteHost = ipv4remoteHost->GetAddress(1,0);//
	              Ipv4Address ip_addr_remoteHost = ipv4_int_addr_remoteHost.GetLocal();//
                      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", 
                                                           InetSocketAddress (ip_addr_remoteHost, ulPort));
                      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
                    }
                }
          
                 
                       
                 Ptr<EpcTft> tft = Create<EpcTft> ();

                if (epcDl)
                 {
                  EpcTft::PacketFilter dlpf;
                  dlpf.localPortStart = dlPort;
                  dlpf.localPortEnd = dlPort;
                  tft->Add (dlpf);              
               }

               if (epcUl)
                  {
                  EpcTft::PacketFilter ulpf;
                  ulpf.remotePortStart = ulPort;
                  ulpf.remotePortEnd = ulPort;
                  tft->Add (ulpf);           
                  }
             
                 if (epcDl || epcUl)
                  {
              
                  
                  EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
                  lteHelper->ActivateDedicatedEpsBearer (macroUeDevs.Get (u), bearer, tft);
                }

              Time startTime = Seconds (startTimeSeconds->GetValue ());
              serverApps.Start (startTime);
              clientApps.Start (startTime);


                }

       }

              
BuildingsHelper::MakeMobilityModelConsistent ();



// connect custom trace sinks for RRC connection establishment and handover notification
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished",
                   MakeCallback (&NotifyConnectionEstablishedEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished",
                   MakeCallback (&NotifyConnectionEstablishedUe));
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart",
                   MakeCallback (&NotifyHandoverStartEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverStart",
                   MakeCallback (&NotifyHandoverStartUe));
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",
                   MakeCallback (&NotifyHandoverEndOkEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverEndOk",
                   MakeCallback (&NotifyHandoverEndOkUe));

// Trace sinks for Packet Sink
  Config::ConnectWithoutContext("/NodeList/0/ApplicationList/0/$ns3::PacketSink/Rx",MakeCallback (&ReceivePacket));
 

  
 static std::vector<Ipv4Address> myVector;
if (!generateRem)
{
Simulator::Schedule(Seconds(1.2), &filecomp, macroUes,serverApps,Filesize,myVector,Chk_FirstWrite);

Simulator::Schedule(Seconds(0), &PrintUeListToFile,"UePosition.txt",UePosition_FirstWrite);

Simulator::Schedule(Seconds(0), &Throughput,Thput_FileChk2);

}

  Ptr<RadioEnvironmentMapHelper> remHelper;
  if (generateRem)
    {
      PrintGnuplottableBuildingListToFile ("buildings.txt");
      PrintGnuplottableEnbListToFile ("enbs.txt");
      PrintGnuplottableUeListToFile ("ues.txt");

      remHelper = CreateObject<RadioEnvironmentMapHelper> ();
      remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
      remHelper->SetAttribute ("OutputFile", StringValue ("Sn1_v2-SINR_ping.rem"));
      remHelper->SetAttribute ("UseDataChannel", BooleanValue (true));
      remHelper->SetAttribute ("RbId", IntegerValue (remRbId));
      remHelper->SetAttribute ("XMin", DoubleValue (sourceEnb1Box.xMin));
      remHelper->SetAttribute ("XMax", DoubleValue (targetEnbsBox.xMax));
      remHelper->SetAttribute ("YMin", DoubleValue (sourceEnb1Box.xMin));
      remHelper->SetAttribute ("YMax", DoubleValue (targetEnbsBox.yMax));
      remHelper->SetAttribute ("Z", DoubleValue (EnbZ));
      remHelper->SetAttribute ("XRes",UintegerValue(600));
      remHelper->SetAttribute ("YRes",UintegerValue(600));
      remHelper->SetAttribute ("MaxPointsPerIteration",UintegerValue (12000));
      remHelper->Install ();
      // simulation will stop right after the REM has been generated
    }
  else 
    {
      Simulator::Stop (Seconds (simTime));
    }



lteHelper->EnablePhyTraces ();  
lteHelper->EnableMacTraces ();
lteHelper->EnableRlcTraces ();


  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.05))); 
  rlcStats->SetAttribute ("DlRlcOutputFilename", StringValue("DlRlcStats_Coverage_hole.txt"));
  rlcStats->SetAttribute ("UlRlcOutputFilename", StringValue("UlRlcStats_Coverage_hole.txt"));

  

  if (epcDl || epcUl)
    {
      lteHelper->EnablePdcpTraces ();
  Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats ();
  pdcpStats->SetAttribute ("EpochDuration", TimeValue (Seconds (0.05)));
  pdcpStats->SetAttribute ("DlPdcpOutputFilename", StringValue("DlPdcpStats_Coverage_hole.txt"));
  pdcpStats->SetAttribute ("UlPdcpOutputFilename", StringValue("UlPdcpStats_Coverage_hole.txt"));

    }

ns3::GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));


 
/*********************************************************Angle calculation for the case when download is not finish*********************************/
 
 Vector vel = macroUes.Get(0)->GetObject<MobilityModel>()->GetVelocity();
std::cout<<"Velocity "<<vel<<std::endl;
 double Angle=((std::asin(vel.y/16.6667)*180)/3.141592654);

/****************************************************************************************************************************************************/


/*
// keep track of all path loss values in two centralized objects
  DownlinkLteGlobalPathlossDatabase dlPathlossDb;
  UplinkLteGlobalPathlossDatabase ulPathlossDb;
  // we rely on the fact that LteHelper creates the DL channel object first, then the UL channel object,
  // hence the former will have index 0 and the latter 1
  Config::Connect ("/ChannelList/0/PathLoss",
                   MakeCallback (&DownlinkLteGlobalPathlossDatabase::UpdatePathloss, &dlPathlossDb));
  Config::Connect ("/ChannelList/1/PathLoss",
                    MakeCallback (&UplinkLteGlobalPathlossDatabase::UpdatePathloss, &ulPathlossDb)); */



Simulator::Run ();
/*
// print the pathloss values at the end of the simulation
  std::cout << std::endl << "Downlink pathloss:" <<std::endl;
  dlPathlossDb.Print ();
   std::cout<< std::endl << "Uplink pathloss:" <<std::endl;
  ulPathlossDb.Print ();*/

if (!generateRem)
 {

if (epcDl && epcUl)
{

for (uint32_t k = 0; k < (2*macroUes.GetN ()); ++k)
{


if((k%2)==0)
  {

  
  Address to; 
          
  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (serverApps.Get (k));
  Ptr<Socket> Lisn_soc = sink1->GetListeningSocket ();
  Lisn_soc->GetSockName (to);
  InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (to);
  thrput<< iaddr.GetIpv4 () << ":" << iaddr.GetPort () << std::endl;
  thrput<< "Total Bytes Received (Byte): " << sink1->GetTotalRx () << std::endl; 
  thrput<< "Total Throughput (Mbps): " << sink1->GetTotalRx ()*8/simTime/1000000 << std::endl; 
   }
}

}
 else
      {

for (uint32_t k = 0; k < (macroUes.GetN ()); ++k)
{
 Address to; 
          
  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (serverApps.Get (k));
  Ptr<Socket> Lisn_soc = sink1->GetListeningSocket ();
  Lisn_soc->GetSockName (to);
   InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (to);
 
  if (!fileComplete&&(iaddr.GetIpv4()=="7.0.0.2")&&(Simulator::Now ().GetSeconds ()==simTime))
  {
  filecompl.open("filecomplete.txt",std::_S_app);
   Vector pos = GetInitialPos(0); 
filecompl<< "End of Simulation and File Download not Complete for " <<iaddr.GetIpv4 ()<<" Angle "<<Angle<<" "<<"Simulatiom Stop time "<<simTime<<" Sec"<<" Initial Position "<< pos.x << "," << pos.y << "," <<pos.z<<" Target Cell Id="<<targetCellId<<" nRun="<<RunValue<<std::endl;

              
                     if(Angle<=30&&Angle>=0)
                        {
                          Angle1.open("Angle_Range1.txt",std::_S_app);
                          Angle1<<simTime<<"\t" <<(Angle)
                          << "\t"<< pos.x << "," << pos.y << "," <<pos.z<<"\t"<<targetCellId<<"\t"<<RunValue<<std::endl;
                                                                     }
                          if(Angle<0&&Angle>=-30)
                         {
                              Angle2.open("Angle_Range2.txt",std::_S_app);
                              Angle2<<simTime<<"\t" <<(Angle)
                               << "\t"<< pos.x << "," << pos.y << "," <<pos.z<<"\t"<<targetCellId<<"\t"<<RunValue<<std::endl;
                          }      
                       if(Angle<-30&&Angle>=-60)
                        {
                          Angle3.open("Angle_Range3.txt",std::_S_app);
                          Angle3<<simTime<<"\t" <<(Angle)
                          << "\t"<< pos.x << "," << pos.y << "," <<pos.z<<"\t"<<targetCellId<<"\t"<<RunValue<<std::endl;
                        }
                                                                

  filecompl.close();
  Angle1.close();
  Angle2.close();
  Angle3.close();
  }
  thrput<< iaddr.GetIpv4 () << ":" << iaddr.GetPort () << std::endl;
  thrput<< "Total Bytes Received (Byte): " << sink1->GetTotalRx () << std::endl; 
  thrput<< "Total Throughput (Mbps): " << sink1->GetTotalRx ()*8/simTime/1024/1024 << std::endl; 

   }

}
 }


    
  //GtkConfigStore config;
 // config.ConfigureAttributes ();


  lteHelper = 0;
  Simulator::Destroy ();
  thrput.close();
  return 0;
}


















