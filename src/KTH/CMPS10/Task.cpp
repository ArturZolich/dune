//***************************************************************************
// Copyright 2007-2014 Universidade do Porto - Faculdade de Engenharia      *
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
// https://www.lsts.pt/dune/licence.                                        *
//***************************************************************************
// Author: Elias                                                            *
//***************************************************************************

#include <stdio.h>
#include <stdlib.h>
// DUNE headers.
#include <DUNE/DUNE.hpp>

namespace KTH
{
  namespace CMPS10
  {
    using DUNE_NAMESPACES;

    struct Arguments
    {
        // Serial port device.
        std::string uart_dev;
        // Serial port baud rate.
        unsigned uart_baud;
    };

    struct Task: public DUNE::Tasks::Task
    {

      // Serial port handle.
      SerialPort* m_uart;
      // Task arguments.
      Arguments m_args;
      // IMC message
      IMC::EntityParameter m_compass;

      Task(const std::string& name, Tasks::Context& ctx):
        DUNE::Tasks::Task(name, ctx),
		m_uart(NULL)
      {
          // Define configuration parameters.
          param("Serial Port - Device", m_args.uart_dev)
          .defaultValue("")
          .description("Serial port device used to communicate with the sensor");

          param("Serial Port - Baud Rate", m_args.uart_baud)
          .defaultValue("9600")
          .description("Serial port baud rate");

          m_compass.name = "Heading";
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
    	  inf("Starting: %s", resolveEntity(getEntityId()).c_str());
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
    	  m_uart = new SerialPort(m_args.uart_dev, m_args.uart_baud,
    			  	  	  	  	  SerialPort::SP_PARITY_NONE,
								  SerialPort::SP_STOPBITS_2);
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
          Memory::clear(m_uart);
      }

      ~Task(void)
      {
        onResourceRelease();
      }

      void
	  sendCommand(void)
      {
    	  static uint8_t cmd = 0x13;
    	  uint8_t buf[1];
    	  buf[0] = cmd;
    	  unsigned int len = 1;
    	  m_uart->write(buf,len);
      }

      //! Main loop.
      void
      onMain(void)
      {
        while (!stopping())
        {
          sendCommand();
    	  Delay::wait(0.1);
    	  static uint8_t readbuf[2];
          m_uart->read(readbuf,2); // size_t n_bytes_read =
          uint16_t heading = (readbuf[0] << 8 | readbuf[1]); // [deg*10]
          std::cout << "CMPS10 Read: " << heading << std::endl;

          // Put the heading in a string
          char val[17];
          snprintf(val,16,"%d",heading);
          //std::cout << "COUT SNPRINTF: " << a << std::endl;
          m_compass.value = val;
          m_compass.setSourceEntity(getEntityId());
          dispatch(m_compass);
        }
      }
    };
  }
}

DUNE_TASK
