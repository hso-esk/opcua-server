/*
 * --- License -------------------------------------------------------------- *
 */

/*
 * Copyright 2017 NIKI 4.0 project team
 *
 * NIKI 4.0 was financed by the Baden-Württemberg Stiftung gGmbH (www.bwstiftung.de).
 * Project partners are FZI Forschungszentrum Informatik am Karlsruher
 * Institut für Technologie (www.fzi.de), Hahn-Schickard-Gesellschaft
 * für angewandte Forschung e.V. (www.hahn-schickard.de) and
 * Hochschule Offenburg (www.hs-offenburg.de).
 * This file was developed by [Kofi Atta Nsiah], [Hahn-Schickard-Gesellschaft
 * für angewandte Forschung e.V.]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/


#include "OpcUaLWM2MObserver.h"
#include "OpcUaLWM2MLib.h"
#include "OpcUaStackCore/Base/Log.h"


namespace OpcUaLWM2M
{
 using namespace OpcUaStackCore;

 /**
  * OpcUaLWM2MObserver()
  */
OpcUaLWM2MObserver::OpcUaLWM2MObserver(OpcUaLWM2MLib& opcualwm2mlib)
  : LWM2MServerObserver()
  , opcualwm2mlib_(opcualwm2mlib)
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
      opcualwm2mlib_.onDeviceRegister(p_dev);
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
      opcualwm2mlib_.onDeviceDeregister(p_dev);

      return -1;
    }
    break;
 }

 return 0;
}

} /* namespace OpcUaIPSO */
