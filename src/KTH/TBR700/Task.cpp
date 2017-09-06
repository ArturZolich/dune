//***************************************************************************
// Copyright 2007-2016 Universidade do Porto - Faculdade de Engenharia      *
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
// Author: Artur Zolich                                                     *
//***************************************************************************

// DUNE headers.
#include <DUNE/DUNE.hpp>

namespace
{
  namespace KTH
  {
    namespace TBR700
    {
      using DUNE_NAMESPACES;

      static const unsigned int c_max_buffer = 256;

      struct Arguments
      {
        // Serial port device.
        std::string uart_dev;
        // Serial port baud rate.
        unsigned uart_baud;
        // Send fake message - for simulation purposes
        std::string fake_msg;
      };

      struct Task: public DUNE::Tasks::Task
      {
        // Device protocol handler.
        SerialPort* m_uart;

        //! Task arguments.
        Arguments m_args;

        // I/O Multiplexer.
        Poll m_poll;

        //! Scratch buffer.
        char m_buffer[c_max_buffer];

        std::string m_msg;

        //! Constructor.
        //! @param[in] name task name.
        //! @param[in] ctx context.
        Task(const std::string& name, Tasks::Context& ctx):
          DUNE::Tasks::Task(name, ctx)
        {
          debug(DTR("CONSTRUCTOR!!!"));

          param("Serial Port - Device", m_args.uart_dev)
                              .defaultValue("/dev/ttyUSB0")
                              .description("Serial port device (used to communicate with the actuator)");

          param("Serial Port - Baud Rate", m_args.uart_baud)
          .defaultValue("9600")
          .description("Serial port baud rate");

          param("Fake message", m_args.fake_msg)
          .defaultValue("")
          .description("Sends fake message for simulation purposes");

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
          debug(DTR("Resource Initialization!!!"));
          //setEntityState(IMC::EntityState::ESTA_NORMAL, Status::CODE_ACTIVE);

          if (m_args.fake_msg.empty())
          {
            m_uart = new SerialPort(m_args.uart_dev, m_args.uart_baud);
            m_poll.add(*m_uart);

            debug(DTR("Serial port initialized"));
          }
        }

        //! Release resources.
        void
        onResourceRelease(void)
        {
          // if(m_args.fake_msg.empty())
          // {
          //   if (m_uart != NULL)
          //   {
          //     m_poll.remove(*m_uart);
          //     delete m_uart;
          //     //m_uart = NULL;
          //   }
          // }
        }

        bool
        joinMessage(char* part, std::string* msg)
        {
          std::string data(part);

          if(data.find('\r') != std::string::npos)
          {
            msg->append(data);
            debug(DTR("full msg: %s"), msg->c_str());
            return true;
          }
          else
          {
            msg->append(data);
          }

          return false;
        }

        //! Main loop.
        void
        onMain(void)
        {
          for (int i = 0; i++; i < sizeof(m_buffer)) {
            m_buffer[i] = 0;
          }

          while (!stopping())
          {
            if(m_args.fake_msg.empty())
            {
              m_poll.poll(1.0);
              if (m_poll.wasTriggered(*m_uart))
              {
                int rv = 0;

                try
                {
                  rv = m_uart->readString(m_buffer, sizeof(m_buffer));

                  debug(DTR("Serial data received: %s"), m_buffer);

                }
                catch (std::exception& e)
                {
                  err(DTR("read error: %s"), e.what());
                }

                if (rv <= 0)
                {
                  err(DTR("unknown read error"));

                }
                else
                {

                  if (joinMessage(m_buffer, &m_msg))
                  {
                  IMC::DevDataText m_fishtelemetry;
                  m_fishtelemetry.value = m_msg;
                  dispatch(m_fishtelemetry);
                  m_msg.clear();
                  }

                  for (int i = 0; i++; i < sizeof(m_buffer)) {
                    m_buffer[i] = 0;
                  }
                }
              }
            }
            else
            {

              IMC::DevDataText m_fishtelemetry;
              m_fishtelemetry.value = m_args.fake_msg;
              dispatch(m_fishtelemetry);
              inf(DTR("SENDING FAKE MESSAGE"));
              Delay::wait(5);
              //waitForMessages(1.0);
            }

          }
        }
      };
    }
  }
}

DUNE_TASK
