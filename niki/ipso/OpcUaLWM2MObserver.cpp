/*
 * OpcUaLWM2MObserver.cpp
 *
 *  Created on: 13 Mar 2017
 *      Author: osboxes
 */

#include "OpcUaLWM2MObserver.h"
#include "OpcUaIPSOLib.h"
#include "OpcUaStackCore/Base/Log.h"


namespace OpcUaIPSO
{
 using namespace OpcUaStackCore;

 /**
  * OpcUaLWM2MObserver()
  */
OpcUaLWM2MObserver::OpcUaLWM2MObserver(OpcUaIPSOLib& opcuaipsolib)
  : LWM2MServerObserver()
  , opcuaipsolib_(opcuaipsolib)
{
  Log(Debug, "OpcUaLWM2MObserver::OpcUaLWM2MObserver");
}

/*---------------------------------------------------------------------------*/
/**
 * notify()
 */
int8_t OpcUaLWM2MObserver::notify(const LWM2MDevice* p_dev, const e_lwm2m_serverobserver_event_t ev)
{
  Log(Debug, "OpcUaLWM2MObserver::notify");

  switch (ev)
  {
    case e_lwm2m_serverobserver_event_register:
    {
	  Log(Debug, "Event device registration triggered")
		   .parameter("Device name", p_dev->getName());

	  /* execute onDeviceRegister function */
	  opcuaipsolib_.onDeviceRegister(p_dev);
    }
    break;

    case e_lwm2m_serverobserver_event_update:
    {
	  Log(Debug, "Event device update triggered")
		   .parameter("Device name", p_dev->getName());
	  /** TODO */

	  Log(Warning, "Update event not implemented yet");
    }
    break;

    case e_lwm2m_serverobserver_event_deregister:
    {
	  Log(Debug, "Event device deregistration triggered")
	       .parameter("Device name", p_dev->getName());

	  /* execute onDeviceRegister function */
	  opcuaipsolib_.onDeviceDeregister(p_dev);

	  return -1;
    }
    break;
 }

 return 0;
}

} /* namespace OpcUaIPSO */
