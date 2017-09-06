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
// Author: Elias Strandell                                                  *
//***************************************************************************

// DUNE headers.
#include <DUNE/DUNE.hpp>

namespace KTH
{
    namespace Fish
    {
        using DUNE_NAMESPACES;

        struct Task: public DUNE::Tasks::Task
        {
            IMC::RemoteSensorInfo remoteSensorInfo;
            IMC::Announce announce;
            IMC::GpsFix m_fix;
            //! Constructor.
            //! @param[in] name task name.
            //! @param[in] ctx context.
            Task(const std::string& name, Tasks::Context& ctx):
                    DUNE::Tasks::Task(name, ctx)
            {
                remoteSensorInfo.id = "fish_module";
                remoteSensorInfo.sensor_class = "Sensor";
                remoteSensorInfo.lat = Angles::radians(59.359267);
                remoteSensorInfo.lon = Angles::radians(18.0525);
                remoteSensorInfo.alt = 0;
                remoteSensorInfo.heading = 0;

                announce.sys_name = "fish_module";
                announce.sys_type = DUNE::IMC::SYSTEMTYPE_MOBILESENSOR;
                announce.lat = remoteSensorInfo.lat;
                announce.lon = remoteSensorInfo.lon;
                announce.height = 0;
                announce.setSource(0x9003);

                bind<IMC::GpsFix>(this);
                bind<IMC::SimulatedState>(this);
                bind<IMC::EstimatedState>(this);
            }


            void consume(const IMC::GpsFix *fix){
                std::cout << "Dummy fish got a fix!" << std::endl;

                if (fix->getSource() != getSystemId())
                {
                    std::cout << "Dummy fish task got GpsFix from wrong source." << std::endl;
                    return;
                }

                std::cout << "Fish dummy task got local fix." << std::endl;
                m_fix = *fix;
                remoteSensorInfo.lat = m_fix.lat;
                remoteSensorInfo.lon = m_fix.lon;
            }

            void consume(const IMC::SimulatedState *esta) {
                if (esta->getSource() != getSystemId())
                {
                    std::cout << "Dummy fish task got SimulatedState from wrong source." << std::endl;
                    return;
                }

                std::cout << "Fish dummy task got local SimulatedState." << std::endl;
                remoteSensorInfo.lat = esta->lat;
                remoteSensorInfo.lon = esta->lon;
            }

            void consume(const IMC::EstimatedState *esta) {
                if (esta->getSource() != getSystemId())
                {
                    std::cout << "Dummy fish task got EstiamtedState from wrong source." << std::endl;
                    return;
                }

                std::cout << "Fish dummy task got local SimulatedState." << std::endl;
                remoteSensorInfo.lat = esta->lat;
                remoteSensorInfo.lon = esta->lon;
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

            //! Main loop.
            void
            onMain(void)
            {
                while (!stopping())
                {
                    consumeMessages();
                    dispatch(remoteSensorInfo);
                    Delay::wait(5.0);
                }
            }
        };
    }
}

DUNE_TASK
