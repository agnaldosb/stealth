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
 */

#ifndef NODE_H
#define NODE_H

#include <vector>
#include <string> // for string use

#include "ns3/object.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/net-device.h"
#include "ns3/string.h"


namespace ns3 {

class Application;
class Packet;
class Address;
class Time;


/**
 * \ingroup network
 *
 * \brief A network Node.
 *
 * This class holds together:
 *   - a list of NetDevice objects which represent the network interfaces
 *     of this node which are connected to other Node instances through
 *     Channel instances.
 *   - a list of Application objects which represent the userspace
 *     traffic generation applications which interact with the Node
 *     through the Socket API.
 *   - a node Id: a unique per-node identifier.
 *   - a system Id: a unique Id used for parallel simulations.
 *
 * Every Node created is added to the NodeList automatically.
 */
class Node : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  Node();
  /**
   * \param systemId a unique integer used for parallel simulations.
   */
  Node(uint32_t systemId);

  virtual ~Node();

  /**
   * \returns the unique id of this node.
   * 
   * This unique id happens to be also the index of the Node into
   * the NodeList. 
   */
  uint32_t GetId (void) const;

  /**
   * In the future, ns3 nodes may have clock that returned a local time
   * different from the virtual time Simulator::Now().
   * This function is currently a placeholder to ease the development of this feature.
   * For now, it is only an alias to Simulator::Now()
   *
   * \return The time as seen by this node
   */
  Time GetLocalTime (void) const;

  /**
   * \returns the system id for parallel simulations associated
   *          to this node.
   */
  uint32_t GetSystemId (void) const;

  /**
   * \brief Associate a NetDevice to this node.
   *
   * \param device NetDevice to associate to this node.
   * \returns the index of the NetDevice into the Node's list of
   *          NetDevice.
   */
  uint32_t AddDevice (Ptr<NetDevice> device);
  /**
   * \brief Retrieve the index-th NetDevice associated to this node.
   *
   * \param index the index of the requested NetDevice
   * \returns the requested NetDevice.
   */
  Ptr<NetDevice> GetDevice (uint32_t index) const;
  /**
   * \returns the number of NetDevice instances associated
   *          to this Node.
   */
  uint32_t GetNDevices (void) const;

  /**
   * \brief Associate an Application to this Node.
   *
   * \param application Application to associate to this node.
   * \returns the index of the Application within the Node's list
   *          of Application.
   */
  uint32_t AddApplication (Ptr<Application> application);
  /**
   * \brief Retrieve the index-th Application associated to this node.
   *
   * \param index the index of the requested Application
   * \returns the requested Application.
   */
  Ptr<Application> GetApplication (uint32_t index) const;

  /**
   * \returns the number of Application instances associated to this Node.
   */
  uint32_t GetNApplications (void) const;

  /**
   * A protocol handler
   *
   * \param device a pointer to the net device which received the packet
   * \param packet the packet received
   * \param protocol the 16 bit protocol number associated with this packet.
   *        This protocol number is expected to be the same protocol number
   *        given to the Send method by the user on the sender side.
   * \param sender the address of the sender
   * \param receiver the address of the receiver; Note: this value is
   *                 only valid for promiscuous mode protocol
   *                 handlers.  Note:  If the L2 protocol does not use L2
   *                 addresses, the address reported here is the value of 
   *                 device->GetAddress().
   * \param packetType type of packet received
   *                   (broadcast/multicast/unicast/otherhost); Note:
   *                   this value is only valid for promiscuous mode
   *                   protocol handlers.
   */
  typedef Callback<void,Ptr<NetDevice>, Ptr<const Packet>,uint16_t,const Address &,
                   const Address &, NetDevice::PacketType> ProtocolHandler;
  /**
   * \param handler the handler to register
   * \param protocolType the type of protocol this handler is 
   *        interested in. This protocol type is a so-called
   *        EtherType, as registered here:
   *        http://standards.ieee.org/regauth/ethertype/eth.txt
   *        the value zero is interpreted as matching all
   *        protocols.
   * \param device the device attached to this handler. If the
   *        value is zero, the handler is attached to all
   *        devices on this node.
   * \param promiscuous whether to register a promiscuous mode handler
   */
  void RegisterProtocolHandler (ProtocolHandler handler, 
                                uint16_t protocolType,
                                Ptr<NetDevice> device,
                                bool promiscuous=false);
  /**
   * \param handler the handler to unregister
   *
   * After this call returns, the input handler will never
   * be invoked anymore.
   */
  void UnregisterProtocolHandler (ProtocolHandler handler);

  /**
   * A callback invoked whenever a device is added to a node.
   */
  typedef Callback<void,Ptr<NetDevice> > DeviceAdditionListener;
  /**
   * \param listener the listener to add
   *
   * Add a new listener to the list of listeners for the device-added
   * event. When a new listener is added, it is notified of the existence
   * of all already-added devices to make discovery of devices easier.
   */
  void RegisterDeviceAdditionListener (DeviceAdditionListener listener);
  /**
   * \param listener the listener to remove
   *
   * Remove an existing listener from the list of listeners for the 
   * device-added event.
   */
  void UnregisterDeviceAdditionListener (DeviceAdditionListener listener);



  /**
   * \returns true if checksums are enabled, false otherwise.
   */
  static bool ChecksumEnabled (void);

  /* Modifications realized by Agnaldo de Souza Batista <asbatista@inf.ufpr.br>
 * STEALTH Project (2018)
 *
 * - Create a list to store node's neighbors and methods to access it
 * - Insert new node's attributes
*/

  /*
   * \returns the status of this node.
   *
   * Emergency = true
   * Normal = false
   */

   bool 		GetStatus (void);
   std::string 	GetCompetence (void);
   void		 	SetCompetence (std::string competence);
   bool			HasEqualCompetence (std::string competence);
   void			SetInterests (std::vector<std::string> interests);
   void 		RegisterNeighbor (Address ip,
		   	   	   	   	   	   	  std::string competence,
								  std::vector<std::string> interests,
								  double trust);

   void						UnregisterNeighbor (Address ip);
   void						UnregisterOffNeighbors ();
   Address					GetPlusTrustNeighbor (std::vector<std::string> competences);
   void						TurnNeighborOn (Address ip);
   bool						IsThereAnyNeighbor ();
   std::vector<std::string> GetInterests ();
   std::vector<Address> 	GetNeighborIpList ();
   void						TurnOffLiveNeighbors ();
   std::string				GetCriticalInfo(std::string competence);
   bool						IsAlreadyNeighbor (Address ip);
   bool						IsAliveNeighbor(Address ip);
   double					GetNeighborTrust (Address ip);
   std::string				GetNeighborCompetence (Address ip);
   std::vector<std::string> GetNeighborInterests (Address ip);
   int 						GetNNeighbors();
   bool 					GetServiceStatus (void);
   int	 					GetServicePriority (void);

   void			RegisterAttendingCall (Address ip,
   							std::string criticalData,
   							int priority,
   							double attendingCallTime);
   std::vector<Address> 	GetAttendingIpList ();
   int 						GetNPendingAttending();
   void						CloseAttending (Address ip);
   std::string				GetAttendingCriticalData (Address ip);
   int						GetAttendingPriority (Address ip);

protected:
  /**
   * The dispose method. Subclasses must override this method
   * and must chain up to it by calling Node::DoDispose at the
   * end of their own DoDispose method.
   */
  virtual void DoDispose (void);
  virtual void DoInitialize (void);
private:

  /**
   * \brief Notifies all the DeviceAdditionListener about the new device added.
   * \param device the added device to notify.
   */
  void NotifyDeviceAdded (Ptr<NetDevice> device);

  /**
   * \brief Receive a packet from a device in non-promiscuous mode.
   * \param device the device
   * \param packet the packet
   * \param protocol the protocol
   * \param from the sender
   * \returns true if the packet has been delivered to a protocol handler.
   */
  bool NonPromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol, const Address &from);
  /**
   * \brief Receive a packet from a device in promiscuous mode.
   * \param device the device
   * \param packet the packet
   * \param protocol the protocol
   * \param from the sender
   * \param to the destination
   * \param packetType the packet type
   * \returns true if the packet has been delivered to a protocol handler.
   */
  bool PromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                                 const Address &from, const Address &to, NetDevice::PacketType packetType);
  /**
   * \brief Receive a packet from a device.
   * \param device the device
   * \param packet the packet
   * \param protocol the protocol
   * \param from the sender
   * \param to the destination
   * \param packetType the packet type
   * \param promisc true if received in promiscuous mode
   * \returns true if the packet has been delivered to a protocol handler.
   */
  bool ReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet>, uint16_t protocol,
                          const Address &from, const Address &to, NetDevice::PacketType packetType, bool promisc);

  /**
   * \brief Finish node's construction by setting the correct node ID.
   */
  void Construct (void);

  /**
   * \brief Protocol handler entry.
   * This structure is used to demultiplex all the protocols.
   */
  struct ProtocolHandlerEntry {
    ProtocolHandler handler; //!< the protocol handler
    Ptr<NetDevice> device;   //!< the NetDevice
    uint16_t protocol;       //!< the protocol number
    bool promiscuous;        //!< true if it is a promiscuous handler
  };

  /// Typedef for protocol handlers container
  typedef std::vector<struct Node::ProtocolHandlerEntry> ProtocolHandlerList;
  /// Typedef for NetDevice addition listeners container
  typedef std::vector<DeviceAdditionListener> DeviceAdditionListenerList;

  uint32_t    m_id;         //!< Node id for this node
  uint32_t    m_sid;        //!< System id for this node
  std::vector<Ptr<NetDevice> > m_devices; //!< Devices associated to this node
  std::vector<Ptr<Application> > m_applications; //!< Applications associated to this node
  ProtocolHandlerList m_handlers; //!< Protocol handlers in the node
  DeviceAdditionListenerList m_deviceAdditionListeners; //!< Device addition listeners in the node


/* Modifications realized by Agnaldo de Souza Batista <asbatista@inf.ufpr.br>
 * STEALTH Project (2018)
 *
 * - Create a list to store node's neighbors and methods to access it
 * - Insert new node's attributes
 */

  /**
   * \brief Neighbor entry.
   * This structure is used to store Neighbors' node.
   */

  struct Neighbor {
    Address ip; 							//!< the neighbor IP address
    std::string competence;   				//!< the neighbor competence
    std::vector <std::string> interests;    //!< the list of neighbor interests
    double trust;        					//!< the neighbor trust value
    bool around;							//!< the neighbor presence
  };

  // Typedef for neighbors handlers container
  typedef std::vector<struct Node::Neighbor> NeighborHandlerList;
  NeighborHandlerList 		m_neighborList; //!< Neighbor list in the node

  struct Attending {
    Address ip; 							//!< the attending IP address
    std::string criticalData;	   			//!< the attending data
    double attendingTime;        			//!< the attending receiving time
    int attendingPriority;					//!< the attending priority (1,2,3)
  };

  // Typedef for attending handlers container
  typedef std::vector<struct Node::Attending> AttendingHandlerList;
  AttendingHandlerList 		m_attendingList; //!< Attending list in the node

  bool						m_status;		//!< Node status (Emergency = true)
  std::string 				m_competence;	//!< Node competence
  std::vector<std::string> 	m_interests; 	//!< Node interests
  bool						m_servicestatus;		//!< Node receive service (receive = true)
  int						m_servicepriority;		//!< Service priority
};

} // namespace ns3

#endif /* NODE_H */
