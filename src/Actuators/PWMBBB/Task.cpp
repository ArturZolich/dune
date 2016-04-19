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
        // - PinOut
        std::vector<int> portio;
      };
      Arguments m_args;
      //GPIO for signal of servo
      int GPIOPin;
      //Handle of servo pinout
      FILE *myOutputHandle;
      //Mode in/out of pinout
      char setValue[4];
      char GPIODirection[64];
      //Name of pin to use
      char GPIOString[4];
      //Value to put in pinout
      char GPIOValue[64];
      //state of update msg servo position
      bool updateMsg;
      //Value of servo position in deg
      double valuePos;
 
      //! Constructor.
      //! @param[in] name task name.
      //! @param[in] ctx context.
      Task(const std::string& name, Tasks::Context& ctx):
      DUNE::Tasks::Task(name, ctx)
      {
        param("PinOut", m_args.portio)
          .defaultValue("60")
          .description("Port to use in PWMBBB");

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
      }
      
      //! Release resources.
      void
      onResourceRelease(void)
      {
      }
      
      void
      consume(const IMC::SetServoPosition* msg)
      {
        setAngleServomotor(msg->value);
      }

      //!Inic of config to pinout of servomotor
      bool
      inicServo(void)
      {
        updateMsg = false;
        valuePos = 0;
        GPIOPin=m_args.portio[0]; /* GPIO1_28 or pin 12 on the P9 header */ 
        sprintf(GPIOString, "%d", GPIOPin);
        sprintf(GPIOValue, "/sys/class/gpio/gpio%d/value", GPIOPin);
        sprintf(GPIODirection, "/sys/class/gpio/gpio%d/direction", GPIOPin);
        // Export the pin
        if ((myOutputHandle = fopen("/sys/class/gpio/export", "ab")) == NULL)
        {
          err(DTR("Unable to export GPIO pin"));
          return false;
        }
        strcpy(setValue, GPIOString);
        fwrite(&setValue, sizeof(char), 2, myOutputHandle);
        fclose(myOutputHandle);
        // Set direction of the pin to an output
        if ((myOutputHandle = fopen(GPIODirection, "rb+")) == NULL)
        {
          err(DTR("Unable to open direction handle"));
          return false;
        }
        strcpy(setValue,"out");
        fwrite(&setValue, sizeof(char), 3, myOutputHandle);
        fclose(myOutputHandle);

        return true;
      }

      //!Set 0º to servomotor
      bool
      setAngleServomotor( double angle )
      {
        bool resultState = true;
        valuePos = angle;
        int cntRefreshservo = 0;
        int degAngle = DUNE::Math::Angles::degrees(std::abs(angle));
        if(degAngle < 0)
          degAngle = 0;
        if(degAngle > 180)
          degAngle = 180;

        int valueUP = (10 * degAngle) + 600;

        while(cntRefreshservo < 20 && resultState)
        {
          if ((myOutputHandle = fopen(GPIOValue, "rb+")) == NULL)
          {
            resultState = false;
            err(DTR("Unable to open value handle"));
          }
          strcpy(setValue, "1"); // Set value high
          fwrite(&setValue, sizeof(char), 1, myOutputHandle);
          fclose(myOutputHandle);
          usleep (valueUP);
          // Set output to low
          if ((myOutputHandle = fopen(GPIOValue, "rb+")) == NULL)
          {
            err(DTR("Unable to open value handle"));
            resultState = false;
          }
          strcpy(setValue, "0"); // Set value low
          fwrite(&setValue, sizeof(char), 1, myOutputHandle);
          fclose(myOutputHandle);;
          usleep (20000 - valueUP);

          cntRefreshservo++;
        }

        if(resultState)
          updateMsg = true;

        return resultState;
      }

      //!Close PinOut config
      bool
      closeConfigServo(void)
      {
        // Unexport the pin
        if ((myOutputHandle = fopen("/sys/class/gpio/unexport", "ab")) == NULL)
        {
          err(DTR("Unable to unexport GPIO pin"));
          return false;
        }
        strcpy(setValue, GPIOString);
        fwrite(&setValue, sizeof(char), 2, myOutputHandle);
        fclose(myOutputHandle);

        return true;
      }

      //! Main loop.
      void
      onMain(void)
      {
        IMC::ServoPosition msgServoPos;
        while(!inicServo())
        {
          waitForMessages(1.0);
        }
        while (!stopping())
        {
          waitForMessages(0.1);
          if(updateMsg)
          {
            msgServoPos.value = valuePos;
            dispatch(msgServoPos);
            updateMsg = false;
          }
        }
        closeConfigServo();
      }
    };
  }
}
DUNE_TASK
