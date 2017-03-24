/*
 * OpcUaLWM2MObserver.h
 *
 *  Created on: 6 Mar 2017
 *      Author: osboxes
 */

#ifndef OPCUALWM2MOBSERVER_H_
#define OPCUALWM2MOBSERVER_H_


#include "LWM2MServerObserver.h"
#include "OpcUaStackCore/Base/Log.h"
#include <iostream>


namespace OpcUaLWM2M
{

class OpcUaLWM2MLib;

class OpcUaLWM2MObserver
  : public LWM2MServerObserver
{

public:

  /**
   * \brief   Default Constructor.
   */
  OpcUaLWM2MObserver(OpcUaLWM2MLib& opcualwm2mlib);

  /**
   * \brief   Default destructor.
   */
  virtual ~OpcUaLWM2MObserver()
  {
  }

  /**
   * \brief   Get notifications from LWM2M server
   */
  int8_t notify(const LWM2MDevice* p_dev, const e_lwm2m_serverobserver_event_t ev);


private:

  OpcUaLWM2MLib& opcualwm2mlib_;

};

} /* namespace OpcUaLWM2M */


#endif /* OPCUALWM2MOBSERVER_H_ */
