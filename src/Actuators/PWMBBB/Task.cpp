//***************************************************************************
// Copyright 2007-2015 Universidade do Porto - Faculdade de Engenharia      *
// Laboratório de Sistemas e Tecnologia Subaquática (LSTS)                  *
//***************************************************************************
// This file is part of DUNE: Unified Navigation Environment.               *
//                                                                          *
// Commercial Licence Usage                                                 *
// Licencees holding valid commercial DUNE licences may use this file in    *
// accordance with the commercial licence agreement provided with the       *
// Software or, alternatively, in accordance with the terms contained in a  *
// written agreement between you and Universidade do Porto. For licensing   *
// terms, conditions, and further information contact lsts@fe.up.pt.        *
//                                                                          *
// European Union Public Licence - EUPL v.1.1 Usage                         *
// Alternatively, this file may be used under the terms of the EUPL,        *
// Version 1.1 only (the "Licence"), appearing in the file LICENCE.md       *
// included in the packaging of this file. You may not use this work        *
// except in compliance with the Licence. Unless required by applicable     *
// law or agreed to in writing, software distributed under the Licence is   *
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF     *
// ANY KIND, either express or implied. See the Licence for the specific    *
// language governing permissions and limitations at                        *
// http://ec.europa.eu/idabc/eupl.html.                                     *
//***************************************************************************
// Author: Pedro Gonçalves                                                  *
//***************************************************************************

// ISO C++ 98 headers.
#include <iostream>
#include <pthread.h>

// DUNE headers.
#include <DUNE/DUNE.hpp>

//Local headers
#include "ServoPWM.hpp"

namespace Actuators
{
  namespace PWMBBB
  {
    using DUNE_NAMESPACES;
    struct Task: public DUNE::Tasks::Task
    {
      //!Variables
      struct Arguments
      {
        // - PinOut1
        std::vector<int> portio1;
        // - PinOut2
        std::vector<int> portio2;
      };

      Arguments m_args;
      //Servo 1
      ServoPwm* m_servo1;
      //Servo 2
      ServoPwm* m_servo2;
      //GPIO for signal of servo
      int GPIOPin;
      //state of update msg servo position
      bool updateMsg;
      //Value of servo position in deg
      double valuePos;
      //ID servo
      uint8_t idServo;
 
      //! Constructor.
      //! @param[in] name task name.
      //! @param[in] ctx context.
      Task(const std::string& name, Tasks::Context& ctx):
      DUNE::Tasks::Task(name, ctx),
      m_servo1(NULL)
      {
        param("PinOut1", m_args.portio1)
          .defaultValue("60")
          .description("Port servo1 to use in PWMBBB");

        param("PinOut2", m_args.portio2)
          .defaultValue("48")
          .description("Port servo2 to use in PWMBBB");

        bind<IMC::SetServoPosition>(this);
      }
      
      //! Update internal state with new parameter values.
      void
      onUpdateParameters(void)
      {
      }
      
      //! Reserve entity identifiers.
      void
      onEntityReservation(void)
      {
      }
      
      //! Resolve entity names.
      void
      onEntityResolution(void)
      {
      }
      
      //! Acquire resources.
      void
      onResourceAcquisition(void)
      {
      }
      
      //! Initialize resources.
      void
      onResourceInitialization(void)
      {
        GPIOPin = m_args.portio1[0];
        m_servo1 = new ServoPwm(this, GPIOPin, 1.745329);
        m_servo1->start();
        GPIOPin = m_args.portio2[0];
        m_servo2 = new ServoPwm(this, GPIOPin, 1.745329);
        m_servo2->start();
      }
      
      //! Release resources.
      void
      onResourceRelease(void)
      {
        if (m_servo1 != NULL)
        {
          m_servo1->stopAndJoin();
          delete m_servo1;
          m_servo1 = NULL;
        }
        if (m_servo2 != NULL)
        {
          m_servo2->stopAndJoin();
          delete m_servo2;
          m_servo2 = NULL;
        }
      }
      
      void
      consume(const IMC::SetServoPosition* msg)
      {
        valuePos = msg->value;
        idServo = msg->id;
        if(idServo == 1)
        {
          war("SERVO 1: %f", valuePos);
          m_servo1->SetPwmValue(valuePos);
        }
        else if(idServo == 2)
        {
          war("SERVO 2: %f", valuePos);
          m_servo2->SetPwmValue(valuePos);
        }
          
        updateMsg = true;
      }

      //! Main loop.
      void
      onMain(void)
      {
        IMC::ServoPosition msgServoPos;
        
        while(!m_servo1->CheckGPIOSate() && !stopping())
        {
          setEntityState(IMC::EntityState::ESTA_ERROR, Utils::String::str(DTR("GPIO_SET1")));
          sleep(1);
        }
        while(!m_servo2->CheckGPIOSate() && !stopping())
        {
          setEntityState(IMC::EntityState::ESTA_ERROR, Utils::String::str(DTR("GPIO_SET2")));
          sleep(1);
        }

        while (!stopping())
        {
          if(updateMsg)
          {
            msgServoPos.value = valuePos;
            msgServoPos.id = idServo;
            dispatch(msgServoPos);
            updateMsg = false;
          }

          if(m_servo1->CheckGPIOSate())
            setEntityState(IMC::EntityState::ESTA_NORMAL, Status::CODE_ACTIVE);
          else
            setEntityState(IMC::EntityState::ESTA_ERROR, Utils::String::str(DTR("GPIO_SET1")));

          if(m_servo2->CheckGPIOSate())
            setEntityState(IMC::EntityState::ESTA_NORMAL, Status::CODE_ACTIVE);
          else
            setEntityState(IMC::EntityState::ESTA_ERROR, Utils::String::str(DTR("GPIO_SET2")));

          waitForMessages(1.0);

        }
      }
    };
  }
}
DUNE_TASK
