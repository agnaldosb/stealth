/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 Georgia Tech Research Corporation, INRIA
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
 * Authors: George F. Riley<riley@ece.gatech.edu>
 *          Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *
 * Modified: Agnaldo de Souza Batista <asbatista@inf.ufpr.br>
 * 			 STEALTH Project (2018)
 *
 */
 
#include "node.h"
#include "node-list.h"
#include "net-device.h"
#include "application.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/object-vector.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/global-value.h"
#include "ns3/boolean.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Node");

NS_OBJECT_ENSURE_REGISTERED (Node);

/**
 * \brief A global switch to enable all checksums for all protocols.
 */
static GlobalValue g_checksumEnabled  = GlobalValue ("ChecksumEnabled",
                                                     "A global switch to enable all checksums for all protocols",
                                                     BooleanValue (false),
                                                     MakeBooleanChecker ());

TypeId 
Node::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Node")
    .SetParent<Object> ()
    .SetGroupName("Network")
    .AddConstructor<Node> ()
    .AddAttribute ("DeviceList", "The list of devices associated to this Node.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&Node::m_devices),
                   MakeObjectVectorChecker<NetDevice> ())
    .AddAttribute ("ApplicationList", "The list of applications associated to this Node.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&Node::m_applications),
                   MakeObjectVectorChecker<Application> ())
    .AddAttribute ("Id", "The id (unique integer) of this Node.",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0),
                   MakeUintegerAccessor (&Node::m_id),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SystemId", "The systemId of this node: a unique integer used for parallel simulations.",
                   TypeId::ATTR_GET | TypeId::ATTR_SET,
                   UintegerValue (0),
                   MakeUintegerAccessor (&Node::m_sid),
                   MakeUintegerChecker<uint32_t> ())
    // Status attribute
    .AddAttribute ("Status", "The status of this node: Emergency (true) or Normal (false).",
				   TypeId::ATTR_GET | TypeId::ATTR_SET,
				   BooleanValue (false),
				   MakeBooleanAccessor (&Node::m_status),
				   MakeBooleanChecker ())
	// Competence attribute
	.AddAttribute ("Competence", "The health competence of this node.",
				   TypeId::ATTR_GET | TypeId::ATTR_SET,
				   StringValue ("other"),
				   MakeStringAccessor (&Node::m_competence),
				   MakeStringChecker ())
    // Service status attribute
	.AddAttribute ("ServiceStatus", "The status of service to this node: Received (true) or Not received (false).",
				   TypeId::ATTR_GET | TypeId::ATTR_SET,
				   BooleanValue (false),
				   MakeBooleanAccessor (&Node::m_servicestatus),
				   MakeBooleanChecker ())
    // Service priority level for this node
    .AddAttribute ("ServicePriority", "Service priority (unique integer) for this Node.",
    			   TypeId::ATTR_GET | TypeId::ATTR_SET,
				   UintegerValue (0),
				   MakeUintegerAccessor (&Node::m_servicepriority),
				   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

Node::Node()
  : m_id (0),
    m_sid (0)
{
  NS_LOG_FUNCTION (this);
  Construct ();
}

Node::Node(uint32_t sid)
  : m_id (0),
    m_sid (sid)
{ 
  NS_LOG_FUNCTION (this << sid);
  Construct ();
}

void
Node::Construct (void)
{
  NS_LOG_FUNCTION (this);
  m_id = NodeList::Add (this);
}

Node::~Node ()
{
  NS_LOG_FUNCTION (this);
}

uint32_t
Node::GetId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_id;
}

Time
Node::GetLocalTime (void) const
{
  NS_LOG_FUNCTION (this);
  return Simulator::Now ();
}

uint32_t
Node::GetSystemId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_sid;
}

uint32_t
Node::AddDevice (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  uint32_t index = m_devices.size ();
  m_devices.push_back (device);
  device->SetNode (this);
  device->SetIfIndex (index);
  device->SetReceiveCallback (MakeCallback (&Node::NonPromiscReceiveFromDevice, this));
  Simulator::ScheduleWithContext (GetId (), Seconds (0.0), 
                                  &NetDevice::Initialize, device);
  NotifyDeviceAdded (device);
  return index;
}
Ptr<NetDevice>
Node::GetDevice (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  NS_ASSERT_MSG (index < m_devices.size (), "Device index " << index <<
                 " is out of range (only have " << m_devices.size () << " devices).");
  return m_devices[index];
}

uint32_t 
Node::GetNDevices (void) const
{
  NS_LOG_FUNCTION (this);
  return m_devices.size ();
}

uint32_t 
Node::AddApplication (Ptr<Application> application)
{
  NS_LOG_FUNCTION (this << application);
  uint32_t index = m_applications.size ();
  m_applications.push_back (application);
  application->SetNode (this);
  Simulator::ScheduleWithContext (GetId (), Seconds (0.0), 
                                  &Application::Initialize, application);
  return index;
}
Ptr<Application> 
Node::GetApplication (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  NS_ASSERT_MSG (index < m_applications.size (), "Application index " << index <<
                 " is out of range (only have " << m_applications.size () << " applications).");
  return m_applications[index];
}
uint32_t 
Node::GetNApplications (void) const
{
  NS_LOG_FUNCTION (this);
  return m_applications.size ();
}

void 
Node::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_deviceAdditionListeners.clear ();
  m_handlers.clear ();
  for (std::vector<Ptr<NetDevice> >::iterator i = m_devices.begin ();
       i != m_devices.end (); i++)
    {
      Ptr<NetDevice> device = *i;
      device->Dispose ();
      *i = 0;
    }
  m_devices.clear ();
  for (std::vector<Ptr<Application> >::iterator i = m_applications.begin ();
       i != m_applications.end (); i++)
    {
      Ptr<Application> application = *i;
      application->Dispose ();
      *i = 0;
    }
  m_applications.clear ();
  Object::DoDispose ();
}

void 
Node::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  for (std::vector<Ptr<NetDevice> >::iterator i = m_devices.begin ();
       i != m_devices.end (); i++)
    {
      Ptr<NetDevice> device = *i;
      device->Initialize ();
    }
  for (std::vector<Ptr<Application> >::iterator i = m_applications.begin ();
       i != m_applications.end (); i++)
    {
      Ptr<Application> application = *i;
      application->Initialize ();
    }

  Object::DoInitialize ();
}

void
Node::RegisterProtocolHandler (ProtocolHandler handler, 
                               uint16_t protocolType,
                               Ptr<NetDevice> device,
                               bool promiscuous)
{
  NS_LOG_FUNCTION (this << &handler << protocolType << device << promiscuous);
  struct Node::ProtocolHandlerEntry entry;
  entry.handler = handler;
  entry.protocol = protocolType;
  entry.device = device;
  entry.promiscuous = promiscuous;

  // On demand enable promiscuous mode in netdevices
  if (promiscuous)
    {
      if (device == 0)
        {
          for (std::vector<Ptr<NetDevice> >::iterator i = m_devices.begin ();
               i != m_devices.end (); i++)
            {
              Ptr<NetDevice> dev = *i;
              dev->SetPromiscReceiveCallback (MakeCallback (&Node::PromiscReceiveFromDevice, this));
            }
        }
      else
        {
          device->SetPromiscReceiveCallback (MakeCallback (&Node::PromiscReceiveFromDevice, this));
        }
    }

  m_handlers.push_back (entry);
}

void
Node::UnregisterProtocolHandler (ProtocolHandler handler)
{
  NS_LOG_FUNCTION (this << &handler);
  for (ProtocolHandlerList::iterator i = m_handlers.begin ();
       i != m_handlers.end (); i++)
    {
      if (i->handler.IsEqual (handler))
        {
          m_handlers.erase (i);
          break;
        }
    }
}

bool
Node::ChecksumEnabled (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  BooleanValue val;
  g_checksumEnabled.GetValue (val);
  return val.Get ();
}

bool
Node::PromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                                const Address &from, const Address &to, NetDevice::PacketType packetType)
{
  NS_LOG_FUNCTION (this << device << packet << protocol << &from << &to << packetType);
  return ReceiveFromDevice (device, packet, protocol, from, to, packetType, true);
}

bool
Node::NonPromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                                   const Address &from)
{
  NS_LOG_FUNCTION (this << device << packet << protocol << &from);
  return ReceiveFromDevice (device, packet, protocol, from, device->GetAddress (), NetDevice::PacketType (0), false);
}

bool
Node::ReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                         const Address &from, const Address &to, NetDevice::PacketType packetType, bool promiscuous)
{
  NS_LOG_FUNCTION (this << device << packet << protocol << &from << &to << packetType << promiscuous);
  NS_ASSERT_MSG (Simulator::GetContext () == GetId (), "Received packet with erroneous context ; " <<
                 "make sure the channels in use are correctly updating events context " <<
                 "when transfering events from one node to another.");
  NS_LOG_DEBUG ("Node " << GetId () << " ReceiveFromDevice:  dev "
                        << device->GetIfIndex () << " (type=" << device->GetInstanceTypeId ().GetName ()
                        << ") Packet UID " << packet->GetUid ());
  bool found = false;

  for (ProtocolHandlerList::iterator i = m_handlers.begin ();
       i != m_handlers.end (); i++)
    {
      if (i->device == 0 ||
          (i->device != 0 && i->device == device))
        {
          if (i->protocol == 0 || 
              i->protocol == protocol)
            {
              if (promiscuous == i->promiscuous)
                {
                  i->handler (device, packet, protocol, from, to, packetType);
                  found = true;
                }
            }
        }
    }
  return found;
}

void 
Node::RegisterDeviceAdditionListener (DeviceAdditionListener listener)
{
  NS_LOG_FUNCTION (this << &listener);
  m_deviceAdditionListeners.push_back (listener);
  // and, then, notify the new listener about all existing devices.
  for (std::vector<Ptr<NetDevice> >::const_iterator i = m_devices.begin ();
       i != m_devices.end (); ++i)
    {
      listener (*i);
    }
}

void 
Node::UnregisterDeviceAdditionListener (DeviceAdditionListener listener)
{
  NS_LOG_FUNCTION (this << &listener);
  for (DeviceAdditionListenerList::iterator i = m_deviceAdditionListeners.begin ();
       i != m_deviceAdditionListeners.end (); i++)
    {
      if ((*i).IsEqual (listener))
        {
          m_deviceAdditionListeners.erase (i);
          break;
         }
    }
}
 
void 
Node::NotifyDeviceAdded (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  for (DeviceAdditionListenerList::iterator i = m_deviceAdditionListeners.begin ();
       i != m_deviceAdditionListeners.end (); i++)
    {
      (*i) (device);
    }  
}
 

/* Modifications by Agnaldo de Souza Batista <asbatista@inf.ufpr.br>
 * STEALTH Project (2018)
 *
 * - Create a list to store node's neighbors and methods to access it
 * - Inserts and gets new node's attributes
 */

/* Get node's status
 *
 * Inputs: NIL
 *
 * Output:
 * m_status: Node's health status
 * 			 Emergency: true
 * 			 Normal:	false)
 */

bool
Node::GetStatus (void)
{
  NS_LOG_FUNCTION (this);
  return m_status;
}

/* Get node's competence
 *
 * Inputs: NIL
 *
 * Output:
 * m_competence: Node's competence
 */

std::string
Node::GetCompetence (void)
{
  NS_LOG_FUNCTION (this);
  return m_competence;
}


/* Set node's competence
 *
 * Inputs:
 * competence: Node's competence to be used to
 *
 * Output: NIL
 */

void
Node::SetCompetence (std::string competence)
{
  NS_LOG_FUNCTION (this);
  m_competence = competence;
}

//
// Return true if equal

/* Checks if a node has the same competence of this one
 *
 * Inputs:
 * competence: Competence of a node to be compared to this one
 *
 * Output:
 * true: 	Same competence
 * false: 	Different competences
 */

bool
Node::HasEqualCompetence (std::string competence)
{
  if (m_competence.compare(competence) == 0)
     return true;
  else
	 return false;
}


/* Sets node interests
 *
 * Inputs:
 * interests: string vector with interests
 *
 * Output: NIL
 */

void
Node::SetInterests (std::vector<std::string> interests)
{
	NS_LOG_FUNCTION (this);
	m_interests = interests;
}


/* Get node's critical data based on another node competence
 * 06Nov18
 *
 * Inputs:
 * competence: competence of another node
 *
 * Output:
 * criticalData: string with critical information
 */

std::string
Node::GetCriticalInfo(std::string competence)
{
	std::string criticalInfo;

	if (!competence.compare("doctor"))
		criticalInfo = "InfoA";
	else if (!competence.compare("nurse"))
		criticalInfo = "InfoB";
	else if (!competence.compare("caregiver"))
		criticalInfo = "InfoC";
	else
		criticalInfo = "InfoD";

	return criticalInfo;
}


/* Get node's interests
 *
 * Inputs: NIL
 *
 * Output:
 * m_interests: string vector with node's interests
 */

std::vector<std::string>
Node::GetInterests()
{
	return m_interests;
}


/* Register one node as neighbor in node's neighbor list
 *
 * Inputs:
 * ip: Neighbor's node IP address
 * competence: Neighbor's node competence
 * interests: Neighbor's node interests
 * trust: Neighbor's node calculated trust
 *
 * Output: NIL
 */

void
Node::RegisterNeighbor (Address ip,
                        std::string competence,
                        std::vector <std::string> interests,
                        double trust)
{
	NS_LOG_FUNCTION (this);
	struct Node::Neighbor neighbor;

	neighbor.ip = ip;
	neighbor.competence = competence;
	neighbor.interests = interests;
	neighbor.trust = trust;
	neighbor.around = true;
	m_neighborList.push_back (neighbor);
}


/* Get node's neighbors IP addresses
 *
 * Inputs: NIL
 *
 * Output:
 * NeighborIpList: List with node's neighbor IP addresses
 */

std::vector<Address>
Node::GetNeighborIpList ()
{
  NS_LOG_FUNCTION (this);
  std::vector<Address> NeighborIpList;
  for (NeighborHandlerList::iterator i = m_neighborList.begin ();
       i != m_neighborList.end (); i++)
    {
		NeighborIpList.push_back(i->ip);
    }
  return NeighborIpList;
}


/* Turn off live neighbors in the list. This permits
 * to refresh neighbor's list at every broadcast sent
 *
 * Inputs: NIL
 *
 * Output: NIL
 */

void
Node::TurnOffLiveNeighbors ()
{
  NS_LOG_FUNCTION (this);
  for (NeighborHandlerList::iterator i = m_neighborList.begin ();
       i != m_neighborList.end (); i++)
 	  	  i->around = false;
 }


/* Remove a node from nodes' neighbor list
 *
 * Inputs:
 * ip: Neighbor IP address
 *
 * Output: NIL
 */

void
Node::UnregisterNeighbor (Address ip)
{
  NS_LOG_FUNCTION (this);
  if (IsAlreadyNeighbor(ip)) // checks if is already a neighbor
  	{
	  for (NeighborHandlerList::iterator i = m_neighborList.begin ();
       i != m_neighborList.end (); i++)
	  	 {
		  if (i->ip == ip)
		  	  {
			  m_neighborList.erase (i);
			  break;
		  	  }
	  	 }
  	}
}


/* Remove neighbors that is not around a node
 * from its neighbors' list
 *
 * Inputs: NIL
 *
 * Output: NIL
 */

void
Node::UnregisterOffNeighbors ()
{
  NS_LOG_FUNCTION (this);
  for (NeighborHandlerList::iterator i = m_neighborList.begin ();
      i != m_neighborList.end (); )
	  	  if (i->around == false)
	  	  {
	  		  m_neighborList.erase (i);
	  	  }
	  	  else
	  		  ++i;
}


/* Confirm neighbor presence in the node neighbors' list
 *
 * Inputs:
 * ip: Neighbor IP address
 *
 * Output: NIL
 */

void
Node::TurnNeighborOn (Address ip)
{
  NS_LOG_FUNCTION (this);
  for (NeighborHandlerList::iterator i = m_neighborList.begin ();
      i != m_neighborList.end (); i++)
	  	  if (i->ip == ip)
	  	  {
	  		  i->around = true;
	  		  break;
	  	  }
}


/* Verify if node has neighbors
 *
 * Inputs: NIL
 *
 * Output:
 * true:	Node has neighbors
 * false:	Node has no neighbors
 */

bool
Node::IsThereAnyNeighbor()
{
  NS_LOG_FUNCTION (this);
  return !m_neighborList.empty();
}


/* Check node's neighbors' list for the node with the biggest trust
 * based on competences used in simulation. They should be given in
 * order of priority, from 0 to n.
 * 06Nov18
 *
 * Do not check if neighbor list is empty. This must be done
 * before through method Node::IsThereAnyNeighbor
 *
 * Inputs:
 * competences: ompetences used in simulation
 *
 * Output:
 * ip: IP address of the biggest trust neighbor node
 */

Address
Node::GetPlusTrustNeighbor (std::vector<std::string> competences)
{
  double trust = 0.0;
  bool gotTrust = false;
  NeighborHandlerList::iterator n;
  NS_LOG_FUNCTION (this);

  // search for the biggest competence
  for (uint8_t i = 0; i != competences.size(); i++ )
  {
	  // search for the biggest trust with that competence
	  for (NeighborHandlerList::iterator it = m_neighborList.begin ();
			  it != m_neighborList.end (); it++)
	  {
		  if (it->competence == competences[i])
		  {
			  if (it->trust > trust)
			  {
				  trust = it->trust;
				  n = it;
				  gotTrust = true;
			  }
		  }
	  }
	  if (gotTrust)
		  break;
  }
  return n->ip;
}


/* Verify if a node is neighbor of this one
 *
 * Inputs:
 * ip: IP address of a node
 *
 * Output:
 * true:	Node searched is neighbor
 * false:	Node searched is not neighbor
 */

bool
Node::IsAlreadyNeighbor(Address ip)
{
  NS_LOG_FUNCTION (this);
  bool inTheList = false;

  for (NeighborHandlerList::iterator i = m_neighborList.begin ();
       i != m_neighborList.end (); i++)
       if (i->ip == ip)
       {
    	   inTheList = true;
    	   break;
       }
  return inTheList;
}


/* Verify if a node is still in the vicinity
 *
 * Inputs:
 * ip: IP address of a searched node
 *
 * Output:
 * true:	Node is in the vicinity
 * false:	Node is not in the vicinity
 */

bool
Node::IsAliveNeighbor(Address ip)
{
  NS_LOG_FUNCTION (this);
  NeighborHandlerList::iterator i;
  for (i = m_neighborList.begin ();
       i != m_neighborList.end (); i++)
       if (i->ip == ip)
    	   break;
  return i->around;
}


/* Get a neighbor node's trust
 * 30Out18
 *
 * Inputs:
 * ip: IP address of a neighbor node
 *
 * Output:
 * trust: neighbor node's trust
 */

double
Node::GetNeighborTrust (Address ip)
{
  NS_LOG_FUNCTION (this);
  NeighborHandlerList::iterator i;
  for (i = m_neighborList.begin ();
      i != m_neighborList.end (); i++)
	  	  if (i->ip == ip)
	  		  break;
  return i->trust;
}

/* Get a neighbor node's competence
 * 09Nov18
 *
 * Inputs:
 * ip: IP address of a neighbor node
 *
 * Output:
 * competence: neighbor node's competence
 */

std::string
Node::GetNeighborCompetence (Address ip)
{
  NS_LOG_FUNCTION (this);
  NeighborHandlerList::iterator i;
  for (i = m_neighborList.begin ();
      i != m_neighborList.end (); i++)
	  	  if (i->ip == ip)
	  		  break;
  return i->competence;
}

/* Get a neighbor node's interests
 * 21Nov19
 *
 * Inputs:
 * ip: IP address of a neighbor node
 *
 * Output:
 * interests: neighbor node's interests
 */

std::vector<std::string>
Node::GetNeighborInterests (Address ip)
{
  NS_LOG_FUNCTION (this);
  NeighborHandlerList::iterator i;
  for (i = m_neighborList.begin ();
      i != m_neighborList.end (); i++)
	  	  if (i->ip == ip)
	  		  break;
  return i->interests;
}

/* Get the number of node's neighbors
 * 16Nov18
 *
 * Output:
 * int: number of node's neighbors
 */

int
Node::GetNNeighbors (void)
{
  NS_LOG_FUNCTION (this);
  return (int)m_neighborList.size ();
}

/* Get node's service status
 * 090119
 * Inputs: NIL
 *
 * Output:
 * m_service: Node's service status
 * 			  Service received: true
 * 			  Service not received:	false
 */

bool
Node::GetServiceStatus (void)
{
  NS_LOG_FUNCTION (this);
  return m_servicestatus;
}

/* Get node's service priority
 * 090119
 * Inputs: NIL
 *
 * Output:
 * m_service: Node's service priority (0,1,2,3)
 *
 */

int
Node::GetServicePriority (void)
{
  NS_LOG_FUNCTION (this);
  return m_servicepriority;
}


/* Register a attending call in node's attending list
 * 30Jan19
 *
 * Inputs:
 * ip: Neighbor's node IP address
 * criticalData: Attending's node critical data
 * priority: Attending's node priority
 * attendingCallTime: Attending's node call time
 *
 * Output: NIL
 */

void
Node::RegisterAttendingCall (Address ip,
							 std::string criticalData,
							 int priority,
							 double attendingCallTime)
{
	NS_LOG_FUNCTION (this);
	struct Node::Attending attending;

	attending.ip = ip;
	attending.criticalData = criticalData;
	attending.attendingPriority = priority;
	attending.attendingTime = attendingCallTime;
	m_attendingList.push_back (attending);
}


/* Get the number of node's pending attending
 * 30Jan19
 *
 * Output:
 * int: number of node's pending attending
 */

int
Node::GetNPendingAttending (void)
{
  NS_LOG_FUNCTION (this);
  return (int)m_attendingList.size ();
}

/* Remove an attending from nodes' attending list
 * 30Jan19
 * Inputs:
 * ip: Neighbor IP address
 *
 * Output: NIL
 */

void
Node::CloseAttending (Address ip)
{
  NS_LOG_FUNCTION (this);
  if (GetNPendingAttending() != 0) // checks there is pending attending
  	{
	  for (AttendingHandlerList::iterator i = m_attendingList.begin ();
       i != m_attendingList.end (); i++)
	  	 {
		  if (i->ip == ip)
		  	  {
			  m_attendingList.erase (i);
			  break;
		  	  }
	  	 }
  	}
}

/* Get node's attending IP addresses
 * 30Jan19
 * Inputs: NIL
 *
 * Output:
 * AttendingIpList: List with node's pending attending IP addresses
 */

std::vector<Address>
Node::GetAttendingIpList ()
{
  NS_LOG_FUNCTION (this);
  std::vector<Address> AttendingIpList;
  for (AttendingHandlerList::iterator i = m_attendingList.begin ();
       i != m_attendingList.end (); i++)
    {
	  	  AttendingIpList.push_back(i->ip);
    }
  return AttendingIpList;
}


/* Get an attending node's critical data
 * 30Jan19
 *
 * Inputs:
 * ip: IP address of an attending node
 *
 * Output:
 * criticalData: attending node's critical data
 */

std::string
Node::GetAttendingCriticalData (Address ip)
{
  NS_LOG_FUNCTION (this);
  AttendingHandlerList::iterator i;
  for (i = m_attendingList.begin ();
      i != m_attendingList.end (); i++)
	  	  if (i->ip == ip)
	  		  break;
  return i->criticalData;
}

/* Get an attending node's priority
 * 30Jan19
 *
 * Inputs:
 * ip: IP address of a neighbor node
 *
 * Output:
 * competence: attending node's priority
 */

int
Node::GetAttendingPriority (Address ip)
{
  NS_LOG_FUNCTION (this);
  AttendingHandlerList::iterator i;
  for (i = m_attendingList.begin ();
      i != m_attendingList.end (); i++)
	  	  if (i->ip == ip)
	  		  break;
  return i->attendingPriority;
}

} // namespace ns3
