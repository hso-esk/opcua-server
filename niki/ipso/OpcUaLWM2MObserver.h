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
 * This file was developed by the Institute of reliable Embedded Systems
 * and Communication Electronics
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
