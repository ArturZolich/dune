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
// Author: Joel Cardoso                                                     *
// Author: Eduardo Marques                                                  *
// Author: Ricardo Martins                                                  *
// Author: Joao Fortuna                                                     *
//***************************************************************************

// ISO C++ 98 headers.
#include <vector>
#include <cmath>
#include <queue>
#include <iostream>

// DUNE headers.
#include <DUNE/DUNE.hpp>

// MAVLink headers.
#include <mavlink/ardupilotmega/mavlink.h>

namespace KTH
{
    namespace Ardupilot
    {
        using DUNE_NAMESPACES;

        //! APM Type specifier
        enum APM_Vehicle {
            //! Unset or unknown vehicle type
                    VEHICLE_UNKNOWN = 0,
            //! Fixed wing types
                    VEHICLE_FIXEDWING,
            //! Copter types (quad, hexa, etc)
                    VEHICLE_COPTER,
            // ASV
                    VEHICLE_ASV // Elias
        };

        //! List of arducopter modes
        enum APM_copterModes {
            CP_MODE_STABILIZE=0,                     // hold level position
            CP_MODE_ACRO=1,                          // rate control
            CP_MODE_ALT_HOLD=2,                      // AUTO control
            CP_MODE_AUTO=3,                          // AUTO control
            CP_MODE_GUIDED=4,                        // AUTO control
            CP_MODE_LOITER=5,                        // Hold a single location
            CP_MODE_RTL=6,                           // AUTO control
            CP_MODE_CIRCLE=7,                        // AUTO control
            CP_MODE_POSITION=8,                      // AUTO control
            CP_MODE_LAND=9,                          // AUTO control
            CP_MODE_OF_LOITER=10,                    // Hold a single location using optical flow sensor
            CP_MODE_DRIFT=11,                        // DRIFT mode (Note: 12 is no longer used)
            CP_MODE_DUNE=12,
            CP_MODE_SPORT=13                         // earth frame rate control
        };

        //! List of ArduPlane modes
        enum APM_planeModes {
            PL_MODE_AUTO=10,
            PL_MODE_GUIDED=15
        };
        // List of AVS modes /ELIAS
        enum APM_SurfaceBoatModes{
            ASV_MODE_AUTO=11,
            ASV_MODE_MANUAL=10
        };

        struct RadioChannel
        {
            //! PWM range
            int pwm_min;
            int pwm_max;
            //! Value range
            float val_max;
            float val_min;
            //! Channel reverse
            bool reverse;
        };

        //! %Task arguments.
        struct Arguments
        {
            //! Communications timeout
            uint8_t comm_timeout;
            //! Use Ardupilot's waypoint tracker
            bool ardu_tracker;
            //! TCP Port
            uint16_t TCP_port;
            //! TCP Address
            Address TCP_addr;
            //! Telemetry Rate
            uint8_t trate;
            //! Default Altitude
            float alt;
            //! Default Speed
            float speed;
            //! GPS is uBlox
            bool ublox;
            //! LoiterHere (default) radius
            float lradius;
            //! Loitering tolerance
            int ltolerance;
            //! Has Power Module
            bool pwrm;
            //! WP seconds before reach
            int secs;
            //! WP Copter: Minimum wp switch radius
            float cp_wp_radius;
            //! RC setup
            RadioChannel rc1;
            RadioChannel rc2;
            RadioChannel rc3;
            //! HITL
            bool hitl;
            //! Formation Flight
            bool form_fl;
            std::string form_fl_ent;
        };

        struct Task: public DUNE::Tasks::Task
        {
            //! Task arguments.
            Arguments m_args;
            //! Arduino packet handling
            typedef void (Task::* PktHandler)(const mavlink_message_t* msg);
            typedef std::map<int, PktHandler> PktHandlerMap;
            PktHandlerMap m_mlh;
            double m_last_pkt_time;
            uint8_t m_buf[512];

            //! Estimated state message.
            IMC::EstimatedState m_estate;
            //! Battery messages
            IMC::Voltage m_volt;
            IMC::Current m_curr;
            IMC::FuelLevel m_fuel;
            //! Pressure message
            IMC::Pressure m_pressure;
            //! Temperature message
            IMC::Temperature m_temp;
            //! GPS Fix message
            IMC::GpsFix m_fix;
            //! Wind message
            IMC::EstimatedStreamVelocity m_stream;
            //! IMU messages
            IMC::AngularVelocity m_ang_vel;
            IMC::Acceleration m_accel;
            IMC::MagneticField m_mag_field;
            //! Servo PWM message
            IMC::SetServoPosition m_servo;
            //! Path Control State
            IMC::PathControlState m_pcs;
            //! DesiredPath message
            IMC::DesiredPath m_dpath;
            //! DesiredPath message
            IMC::ControlLoops m_controllps;
            //! Reference Lat and Lon and Hei to measure displacement
            fp64_t ref_lat, ref_lon;
            fp32_t ref_hei;
            //! TCP socket
            TCPSocket* m_TCP_sock;
            Address m_TCP_addr;
            uint16_t m_TCP_port;
            //! System ID
            uint8_t m_sysid;
            //! Last received position
            double m_lat, m_lon;
            float m_alt;
            //! External control
            bool m_external;
            //! Current waypoint
            int m_current_wp;
            //! Critical WP
            bool m_critical;
            //! Bitfield of enabled control loops.
            uint32_t m_cloops;
            //! Parser Variables
            mavlink_message_t m_msg;
            int m_desired_radius;
            int m_gnd_speed;
            int m_mode;
            bool m_changing_wp;
            bool m_error_missing, m_esta_ext;
            //! Setup rate hack
            bool m_has_setup_rate;
            //! Vehicle is on ground
            bool m_ground;
            //! Desired control
            float m_droll, m_dclimb, m_dspeed;
            //! Type of system to be controlled
            APM_Vehicle m_vehicle_type;
            //! Check if is in service
            bool m_service;
            //! Time since last waypoint was sent
            float m_last_wp;
            //! Mission items queue
            std::queue<mavlink_message_t> m_mission_items;

            Task(const std::string& name, Tasks::Context& ctx):
                    Tasks::Task(name, ctx),
                    ref_lat(0.0),
                    ref_lon(0.0),
                    ref_hei(0.0),
                    m_TCP_sock(NULL),
                    m_TCP_port(0),
                    m_sysid(1),
                    m_lat(0.0),
                    m_lon(0.0),
                    m_alt(0.0),
                    m_external(true),
                    m_current_wp(0),
                    m_critical(false),
                    m_cloops(0),
                    m_desired_radius(0),
                    m_gnd_speed(0),
                    m_mode(0),
                    m_changing_wp(false),
                    m_error_missing(false),
                    m_esta_ext(false),
                    m_has_setup_rate(false),
                    m_ground(true),
                    m_droll(0),
                    m_dclimb(0),
                    m_dspeed(20),
                    m_vehicle_type(VEHICLE_UNKNOWN),
                    m_service(false),
                    m_last_wp(0)
            {
                param("Communications Timeout", m_args.comm_timeout)
                        .minimumValue("1")
                        .maximumValue("60")
                        .defaultValue("10")
                        .units(Units::Second)
                        .description("Ardupilot communications timeout");

                param("Ardupilot Tracker", m_args.ardu_tracker)
                        .visibility(Tasks::Parameter::VISIBILITY_USER)
                        .scope(Tasks::Parameter::SCOPE_MANEUVER)
                        .defaultValue("false")
                        .description("Use Ardupilot's waypoint tracker");

                param("TCP - Port", m_args.TCP_port)
                        .defaultValue("5760")
                        .description("Port for connection to Ardupilot");

                param("TCP - Address", m_args.TCP_addr)
                        .defaultValue("127.0.0.1")
                        .description("Address for connection to Ardupilot");

                param("Telemetry Rate", m_args.trate)
                        .defaultValue("10")
                        .units(Units::Hertz)
                        .description("Telemetry output rate from Ardupilot");

                param("Default altitude", m_args.alt)
                        .defaultValue("200.0")
                        .units(Units::Meter)
                        .description("Altitude to be used if desired Z has no units");

                param("Default speed", m_args.speed)
                        .defaultValue("18.0")
                        .units(Units::MeterPerSecond)
                        .description("Speed to be used if desired speed is not specified");

                param("uBlox GPS", m_args.ublox)
                        .defaultValue("false")
                        .description("The installed GPS is uBlox");

                param("Default loiter radius", m_args.lradius)
                        .defaultValue("-150.0")
                        .units(Units::Meter)
                        .description("Loiter radius used in LoiterHere (idle)");

                param("Loitering tolerance", m_args.ltolerance)
                        .defaultValue("10")
                        .units(Units::Meter)
                        .description("Distance to consider loitering (radius + tolerance)");

                param("Power Module", m_args.pwrm)
                        .defaultValue("true")
                        .description("There is a Power Module installed");

                param("Seconds before Waypoint", m_args.secs)
                        .defaultValue("4")
                        .units(Units::Second)
                        .description("Seconds before actually reaching Waypoint that it is considered as reached");

                param("Copter - Minimum WP switch radius", m_args.cp_wp_radius)
                        .defaultValue("0.5")
                        .units(Units::Meter)
                        .description("Used for waypointswitching at copters. Copters uses the seconds before reaching wp, but will also switch if this distance is met.");

                param("RC 1 PWM MIN", m_args.rc1.pwm_min)
                        .defaultValue("1000")
                        .units(Units::Microsecond)
                        .description("Min PWM value for Roll channel");

                param("RC 1 PWM MAX", m_args.rc1.pwm_max)
                        .defaultValue("2000")
                        .units(Units::Microsecond)
                        .description("Max PWM value for Roll channel");

                param("RC 1 MAX", m_args.rc1.val_max)
                        .defaultValue("30.0")
                        .units(Units::Degree)
                        .description("Max Roll");

                param("RC 1 REV", m_args.rc1.reverse)
                        .defaultValue("False")
                        .description("Reverse Roll Channel");

                param("RC 2 PWM MIN", m_args.rc2.pwm_min)
                        .defaultValue("1000")
                        .units(Units::Microsecond)
                        .description("Min PWM value for Climb Rate channel");

                param("RC 2 PWM MAX", m_args.rc2.pwm_max)
                        .defaultValue("2000")
                        .units(Units::Microsecond)
                        .description("Max PWM value for Climb Rate channel");

                param("RC 2 MAX", m_args.rc2.val_max)
                        .defaultValue("2.0")
                        .units(Units::MeterPerSecond)
                        .description("Max Climb Rate");

                param("RC 2 REV", m_args.rc2.reverse)
                        .defaultValue("False")
                        .description("Reverse Pitch Channel");

                param("RC 3 PWM MIN", m_args.rc3.pwm_min)
                        .defaultValue("1000")
                        .units(Units::Microsecond)
                        .description("Min PWM value for Air Speed channel");

                param("RC 3 PWM MAX", m_args.rc3.pwm_max)
                        .defaultValue("2000")
                        .units(Units::Microsecond)
                        .description("Max PWM value for Air Speed channel");

                param("RC 3 MIN", m_args.rc3.val_min)
                        .defaultValue("10.0")
                        .units(Units::MeterPerSecond)
                        .description("Min Air Speed");

                param("RC 3 MAX", m_args.rc3.val_max)
                        .defaultValue("30.0")
                        .units(Units::MeterPerSecond)
                        .description("Max Air Speed");

                param("RC 3 REV", m_args.rc3.reverse)
                        .defaultValue("False")
                        .description("Reverse Speed Channel");

                param("HITL", m_args.hitl)
                        .defaultValue("false")
                        .description("Hardware in the loop");

                param("Formation Flight", m_args.form_fl)
                        .visibility(Tasks::Parameter::VISIBILITY_USER)
                        .scope(Tasks::Parameter::SCOPE_MANEUVER)
                        .defaultValue("false")
                        .description("Receive control references from Formation Flight controller");

                param("Formation Flight Entity", m_args.form_fl_ent)
                        .defaultValue("Formation Control")
                        .description("Entity that sends Formation Flight control references");

                // Setup packet handlers
                // IMPORTANT: set up function to handle each type of MAVLINK packet here
                m_mlh[MAVLINK_MSG_ID_ATTITUDE] = &Task::handleAttitudePacket;
                m_mlh[MAVLINK_MSG_ID_GLOBAL_POSITION_INT] = &Task::handlePositionPacket;
                m_mlh[MAVLINK_MSG_ID_HWSTATUS] = &Task::handleHWStatusPacket;
                m_mlh[MAVLINK_MSG_ID_SCALED_PRESSURE] = &Task::handleScaledPressurePacket;
                m_mlh[MAVLINK_MSG_ID_GPS_RAW_INT] = &Task::handleRawGPSPacket;
                m_mlh[MAVLINK_MSG_ID_WIND] = &Task::handleWindPacket;
                m_mlh[MAVLINK_MSG_ID_COMMAND_ACK] = &Task::handleCmdAckPacket;
                m_mlh[MAVLINK_MSG_ID_MISSION_ACK] = &Task::handleMissionAckPacket;
                m_mlh[MAVLINK_MSG_ID_MISSION_CURRENT] = &Task::handleMissionCurrentPacket;
                m_mlh[MAVLINK_MSG_ID_STATUSTEXT] = &Task::handleStatusTextPacket; // Used for debug
                m_mlh[MAVLINK_MSG_ID_HEARTBEAT] = &Task::handleHeartbeatPacket;
                m_mlh[MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT] = &Task::handleNavControllerPacket;
                m_mlh[MAVLINK_MSG_ID_MISSION_ITEM] = &Task::handleMissionItemPacket;
                m_mlh[MAVLINK_MSG_ID_SYS_STATUS] = &Task::handleSystemStatusPacket;
                m_mlh[MAVLINK_MSG_ID_VFR_HUD] = &Task::handleHUDPacket;
                m_mlh[MAVLINK_MSG_ID_SYSTEM_TIME] = &Task::handleSystemTimePacket;
                m_mlh[MAVLINK_MSG_ID_MISSION_REQUEST] = &Task::handleMissionRequestPacket;
                // Some additional messages for debug purposes (Added by Elias)
                m_mlh[MAVLINK_MSG_ID_DEBUG_VECT] = &Task::handleDebugVectPacket; // Elias
                m_mlh[MAVLINK_MSG_ID_NAMED_VALUE_INT] = &Task::handleNamedValueIntPacket; // Elias
                m_mlh[MAVLINK_MSG_ID_NAMED_VALUE_FLOAT] = &Task::handleNamedValueFloatPacket; // Elias


                // Setup processing of IMC messages
                bind<DesiredPath>(this);
                bind<DesiredRoll>(this);
                bind<DesiredZ>(this);
                bind<DesiredVerticalRate>(this);
                bind<DesiredSpeed>(this);
                bind<IdleManeuver>(this);
                bind<ControlLoops>(this);
                bind<PowerChannelControl>(this);
                bind<VehicleMedium>(this);
                bind<VehicleState>(this);
                bind<SimulatedState>(this);
                bind<EntityParameter>(this); // Elias

                //! Misc. initialization
                m_last_pkt_time = 0; //! time of last packet from Ardupilot
                m_estate.clear();
            }

            void
            onResourceRelease(void)
            {
                Memory::clear(m_TCP_sock);
            }

            void
            onResourceAcquisition(void)
            {
                m_TCP_addr = m_args.TCP_addr;
                m_TCP_port = m_args.TCP_port;
                openConnection();
            }

            void
            onUpdateParameters(void)
            {
                //! Minimum value for bank (RC1) and vertical rate (R2)
                //! are symmetrical to maximum values, no need to input them manually
                m_args.rc1.val_min = -m_args.rc1.val_max;
                m_args.rc2.val_min = -m_args.rc2.val_max;
            }

            void
            openConnection(void)
            {
                try
                {
                    m_TCP_sock = new TCPSocket;
                    m_TCP_sock->connect(m_TCP_addr, m_TCP_port);
                    setupRate(m_args.trate);
                    inf(DTR("Ardupilot interface initialized"));
                }
                catch (...)
                {
                    Memory::clear(m_TCP_sock);
                    war(DTR("Connection failed, retrying..."));
                    setEntityState(IMC::EntityState::ESTA_NORMAL, Status::CODE_COM_ERROR);
                }
            }

            void
            setupRate(uint8_t rate)
            {
                uint8_t buf[512];
                mavlink_message_t msg;

                //! ATTITUDE and SIMSTATE messages
                mavlink_msg_request_data_stream_pack(255, 0, &msg,
                                                     m_sysid,
                                                     0,
                                                     MAV_DATA_STREAM_EXTRA1,
                                                     rate,
                                                     1);

                uint16_t n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);
                spew("ATTITUDE Stream setup to %d Hertz", rate);

                //! VFR_HUD message
                mavlink_msg_request_data_stream_pack(255, 0, &msg,
                                                     m_sysid,
                                                     0,
                                                     MAV_DATA_STREAM_EXTRA2,
                                                     rate,
                                                     1);

                n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);
                spew("VFR Stream setup to %d Hertz", rate);

                //! GLOBAL_POSITION_INT message
                mavlink_msg_request_data_stream_pack(255, 0, &msg,
                                                     m_sysid,
                                                     0,
                                                     MAV_DATA_STREAM_POSITION,
                                                     rate,
                                                     1);

                n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);
                spew("POSITION Stream setup to %d Hertz", rate);

                //! SYS_STATUS, POWER_STATUS, MEMINFO, MISSION_CURRENT,
                //! GPS_RAW_INT, NAV_CONTROLLER_OUTPUT and FENCE_STATUS messages
                mavlink_msg_request_data_stream_pack(255, 0, &msg,
                                                     m_sysid,
                                                     0,
                                                     MAV_DATA_STREAM_EXTENDED_STATUS,
                                                     (int)(rate/5),
                                                     1);

                n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);
                spew("STATUS Stream setup to %d Hertz", (int)(rate/5));

                //! AHRS, HWSTATUS, WIND, RANGEFINDER and SYSTEM_TIME messages
                mavlink_msg_request_data_stream_pack(255, 0, &msg,
                                                     m_sysid,
                                                     0,
                                                     MAV_DATA_STREAM_EXTRA3,
                                                     1,
                                                     1);

                n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);
                spew("AHRS-HWSTATUS-WIND Stream setup to 1 Hertz");

                //! RAW_IMU, SCALED_PRESSURE and SENSOR_OFFSETS messages
                mavlink_msg_request_data_stream_pack(255, 0, &msg,
                                                     m_sysid,
                                                     0,
                                                     MAV_DATA_STREAM_RAW_SENSORS,
                                                     1,
                                                     1);

                n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);
                spew("SENSORS Stream setup to 1 Hertz");

                //! RC_CHANNELS_RAW and SERVO_OUTPUT_RAW messages
                mavlink_msg_request_data_stream_pack(255, 0, &msg,
                                                     m_sysid,
                                                     0,
                                                     MAV_DATA_STREAM_RC_CHANNELS,
                                                     1,
                                                     0);

                n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);
                spew("RC Stream disabled");
            }

            //! Prints to terminal and log when control loops are activated and deactivated
            void
            info(uint32_t was, uint32_t is, uint32_t loop, const char* desc)
            {
                was &= loop;
                is &= loop;

                if (was && !is)
                    inf("%s - deactivating", desc);
                else if (!was && is)
                    inf("%s - activating", desc);
            }

            // Elias. Receive heading from CMPS10 task and send as MAVlink message to APM
            void
            consume(const IMC::EntityParameter* ep)
            {
                //static std::string str ("Heading");
                //if (str.compare(ep->name))
                //{

                // Now extract that information from a and print again
                // The C++-way of doing it
                static uint16_t hdg; // The variable the heading is put in
                std::stringstream convert(ep->value);
                convert >> hdg;

                // The C-way of doing it
                //sscanf(ep->value,"%d",&scan); // Finds an integer in the a string and puts it in scan
                //std::cout << "SCANNED VALUE:" << scan << std::endl;

                mavlink_message_t msg;
                uint8_t buf[512];

                char name[10] = "Heading";
                mavlink_msg_named_value_int_pack(0,0,&msg,0,name,hdg);

                uint16_t len = mavlink_msg_to_send_buffer(buf,&msg);
                sendData(buf,len);


                //}
            }

            void
            consume(const IMC::ControlLoops* cloops)
            {
                /*if (m_external && cloops->enable)
                {
                  m_controllps = *cloops;
                  inf(DTR("ArduPilot is in Manual mode, saving control loops."));
                  //! Control loops will be enabled when AUTO mode is activated
                  return;
                }*/

                uint32_t prev = m_cloops;

                if (cloops->enable)
                {
                    m_cloops |= cloops->mask;

                    /*if ((!m_args.ardu_tracker) && (cloops->mask & IMC::CL_PATH))
                    {
                      inf("Ardupilot tracker is NOT enabled");
                      m_cloops &= ~IMC::CL_PATH;
                    }*/

                    if (!(m_args.ardu_tracker) && (cloops->mask & IMC::CL_ROLL))
                    {
                        onUpdateParameters();
                        activateFBW();
                    }
                }
                else
                {
                    m_cloops &= ~cloops->mask;

                    if ((cloops->mask & IMC::CL_ROLL) && !m_ground)
                    {
                        mavlink_message_t msg;
                        uint8_t buf[512];

                        //! Sending value 0 disables RC override for that channel
                        mavlink_msg_rc_channels_override_pack(255, 0, &msg,
                                                              1,
                                                              1,
                                                              0, //! RC Channel 1 (roll)
                                                              0, //! RC Channel 2 (vertical rate)
                                                              0, //! RC Channel 3 (speed)
                                                              0, //! RC Channel 4 (rudder)
                                                              0, //! RC Channel 5 (not used)
                                                              0, //! RC Channel 6 (not used)
                                                              0, //! RC Channel 7 (not used)
                                                              0);//! RC Channel 8 (mode)
                        uint16_t n = mavlink_msg_to_send_buffer(buf, &msg);
                        sendData(buf, n);
                    }
                }

                info(prev, m_cloops, IMC::CL_SPEED, "speed control");
                info(prev, m_cloops, IMC::CL_ALTITUDE, "altitude control");
                info(prev, m_cloops, IMC::CL_VERTICAL_RATE, "vertical rate control");
                info(prev, m_cloops, IMC::CL_ROLL, "bank control");
                info(prev, m_cloops, IMC::CL_YAW, "heading control");
                info(prev, m_cloops, IMC::CL_PATH, "path control");
            }

            //! Messages for FBWB control (using DUNE's controllers)
            void
            activateFBW(void)
            {
                uint8_t buf[512];
                mavlink_message_t msg;

                mavlink_msg_set_mode_pack(255, 0, &msg,
                                          m_sysid,
                                          1,
                                          6); //! FBWB is mode 6

                uint16_t n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);
            }

            void
            consume(const IMC::DesiredRoll* d_roll)
            {
                //! If in Manual mode, Taking-off, Landing or on Ground do not send
                //! control references to ArduPilot
                if (m_external || m_critical || m_ground)
                {
                    debug("external: %d, critical: %d, ground: %d", (int)m_external, (int)m_critical, (int)m_ground);
                    return;
                }

                if (!(m_cloops & IMC::CL_ROLL))
                {
                    debug("bank control is NOT active");
                    return;
                }

                //! If in formation flight only keep bank references coming from Formation Control entity
                if (m_args.form_fl && resolveEntity(m_args.form_fl_ent) != d_roll->getSourceEntity())
                    return;

                m_droll = Angles::degrees(d_roll->value);

                //! Convert references to PWM and send all

                int pwm_roll = map2PWM(m_args.rc1.pwm_min, m_args.rc1.pwm_max,
                                       m_args.rc1.val_min, m_args.rc1.val_max,
                                       m_droll, m_args.rc1.reverse);

                int pwm_climb = map2PWM(m_args.rc2.pwm_min, m_args.rc2.pwm_max,
                                        m_args.rc2.val_min, m_args.rc2.val_max,
                                        m_dclimb, m_args.rc2.reverse);

                int pwm_speed = map2PWM(m_args.rc3.pwm_min, m_args.rc3.pwm_max,
                                        m_args.rc3.val_min, m_args.rc3.val_max,
                                        m_dspeed, m_args.rc3.reverse);

                debug("V1: %f, V2: %f, V3: %f", m_droll, m_dclimb, m_dspeed);
                debug("RC1: %d, RC2: %d, RC3: %d", pwm_roll, pwm_climb, pwm_speed);

                uint8_t buf[512];

                mavlink_message_t msg;
                mavlink_msg_rc_channels_override_pack(255, 0, &msg,
                                                      1,
                                                      1,
                                                      pwm_roll, //! RC Channel 1 (roll)
                                                      pwm_climb, //! RC Channel 2 (vertical rate)
                                                      pwm_speed, //! RC Channel 3 (speed)
                                                      1500, //! RC Channel 4 (rudder)
                                                      0, //! RC Channel 5 (not used)
                                                      0, //! RC Channel 6 (not used)
                                                      0, //! RC Channel 7 (not used)
                                                      0);//! RC Channel 8 (mode - do not override)
                uint16_t n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);
            }

            void
            consume(const IMC::DesiredZ* d_z)
            {
                if (!(m_cloops & IMC::CL_ALTITUDE))
                {
                    debug("altitude control is NOT active");
                    return;
                }

                (void) d_z;
            }

            void
            consume(const IMC::DesiredVerticalRate* d_vr)
            {
                if (!(m_cloops & IMC::CL_VERTICAL_RATE))
                {
                    debug("vertical rate control is NOT active");
                    return;
                }

                //! Saving value
                m_dclimb = d_vr->value;
            }

            void
            consume(const IMC::DesiredSpeed* d_speed)
            {
                if (!(m_cloops & IMC::CL_SPEED))
                {
                    inf(DTR("speed control is NOT active"));
                    return;
                }

                //! Saving value
                m_dspeed = d_speed->value;
            }

            //! Converts value in range min_value:max_value to a value_pwm in range min_pwm:max_pwm
            int
            map2PWM(int min_pwm, int max_pwm, float min_value, float max_value, float value, bool reverse)
            {
                int value_pwm;

                if (reverse)
                    value_pwm = (int) max_pwm - ((max_pwm - min_pwm) * (value - min_value) / (max_value - min_value));
                else
                    value_pwm = (int) ((max_pwm - min_pwm) * (value - min_value) / (max_value - min_value)) + min_pwm;

                return trimValue(value_pwm, min_pwm, max_pwm);
            }

            //! Message for GUIDED/AUTO control (using Ardupilot's controllers)
            void
            consume(const IMC::DesiredPath* path)
            {
                /*if (m_external)
                {
                  m_dpath = *path;
                  inf(DTR("ArduPilot is in Manual mode, saving desired path."));
                  return;
                }*/

                /*if ((m_vehicle_type == VEHICLE_COPTER) || (m_vehicle_type ==  VEHICLE_FIXEDWING))
                  {
                    if (!((m_cloops & IMC::CL_PATH) && m_args.ardu_tracker))
                  {
                    inf(DTR("path control is NOT active"));
                    return;
                  }
            } */

                if (m_ground && ((m_vehicle_type == VEHICLE_COPTER) || (m_vehicle_type == VEHICLE_FIXEDWING))) // Always returns false?
                {

                    inf(DTR("ArduPilot in Auto mode but still in ground, performing takeoff first."));
                    if (m_vehicle_type == VEHICLE_COPTER)
                    {
                        takeoff_copter(path);
                    }
                    else if (m_vehicle_type == VEHICLE_FIXEDWING){
                        takeoff_plane(path);
                    }
                    return;
                }

                if(m_vehicle_type == VEHICLE_COPTER)
                {
                    // Copters must first be set to guided as of AC 3.2
                    // Disabled, as this is not
                    uint8_t buf[512];
                    mavlink_message_t* msg = new mavlink_message_t;

                    mavlink_msg_set_mode_pack(255, 0, msg,
                                              m_sysid,
                                              1,
                                              CP_MODE_GUIDED); //! DUNE mode on arducopter is 12

                    uint16_t n = mavlink_msg_to_send_buffer(buf, msg);
                    sendData(buf, n);
                    debug("Guided MODE on ardupilot is set");
                }

                uint8_t buf[512];

                mavlink_message_t msg;

                //! Setting airspeed parameter
                if (m_vehicle_type == VEHICLE_COPTER)
                {
                    mavlink_msg_param_set_pack(255, 0, &msg,
                                               m_sysid, //! target_system System ID
                                               0, //! target_component Component ID
                                               "WPNAV_SPEED", //! Parameter name
                                               (int)(path->speed * 100), //! Parameter value
                                               MAV_PARAM_TYPE_INT16); //! Parameter type
                    int n = mavlink_msg_to_send_buffer(buf, &msg);
                    sendData(buf, n);
                }
                else if (m_vehicle_type == VEHICLE_FIXEDWING)
                {
                    mavlink_msg_param_set_pack(255, 0, &msg,
                                               m_sysid, //! target_system System ID
                                               0, //! target_component Component ID
                                               "TRIM_ARSPD_CM", //! Parameter name
                                               (int)(path->speed * 100), //! Parameter value
                                               MAV_PARAM_TYPE_INT16); //! Parameter type
                    int n = mavlink_msg_to_send_buffer(buf, &msg);
                    sendData(buf, n);
                }

                //! Setting loiter radius parameter
                if (m_vehicle_type == VEHICLE_FIXEDWING)
                {
                    mavlink_msg_param_set_pack(255, 0, &msg,
                                               m_sysid, //! target_system System ID
                                               0, //! target_component Component ID
                                               "WP_LOITER_RAD", //! Parameter name
                                               path->flags & DesiredPath::FL_CCLOCKW ? (-1 * path->lradius) : (path->lradius), //! Parameter value
                                               MAV_PARAM_TYPE_INT16); //! Parameter type

                    int n = mavlink_msg_to_send_buffer(buf, &msg);
                    sendData(buf, n);
                }
                m_desired_radius = (uint16_t) path->lradius;

                float alt = (path->end_z_units & IMC::Z_NONE) ? m_args.alt : (float)path->end_z;

                //! Destination

                if (m_vehicle_type == VEHICLE_COPTER)
                {
                    mavlink_msg_mission_item_pack(255, 0, &msg,
                                                  m_sysid, //! target_system System ID
                                                  0, //! target_component Component ID
                                                  1, //! seq Sequence
                                                  MAV_FRAME_GLOBAL, //! frame The coordinate system of the MISSION. see MAV_FRAME in mavlink_types.h
                                                  MAV_CMD_NAV_WAYPOINT, //! command The scheduled action for the MISSION. see MAV_CMD in ardupilotmega.h
                                                  2, //! current false:0, true:1, guided mode:2
                                                  0, //! autocontinue to next wp
                                                  0, //! p1 - Radius of acceptance? Think not used.
                                                  0, //! Not used
                                                  0, // direction does not matter for copter.
                                                  0, //! Not used
                                                  (float)Angles::degrees(path->end_lat), //! x PARAM5 / local: x position, global: latitude
                                                  (float)Angles::degrees(path->end_lon), //! y PARAM6 / y position: global: longitude
                                                  alt);//! z PARAM7 / z position: global: altitude
                    int n = mavlink_msg_to_send_buffer(buf, &msg);
                    sendData(buf, n);
                }
                else if (m_vehicle_type == VEHICLE_ASV)
                {
                    std::cout << (float)Angles::degrees(path->end_lat) << ", " << (float)Angles::degrees(path->end_lon) << std::endl;

                    mavlink_msg_mission_item_pack(255, 0, &msg,
                                                  m_sysid, //! target_system System ID
                                                  0, //! target_component Component ID
                                                  1, //! seq Sequence
                                                  MAV_FRAME_GLOBAL, //! frame The coordinate system of the MISSION. see MAV_FRAME in mavlink_types.h
                                                  MAV_CMD_NAV_WAYPOINT, //! command The scheduled action for the MISSION. see MAV_CMD in ardupilotmega.h
                                                  2, //! current false:0, true:1, guided mode:2
                                                  0, //! autocontinue to next wp
                                                  0, //! p1 - Radius of acceptance? Think not used.
                                                  0, //! Not used
                                                  0, // direction does not matter for copter.
                                                  0, //! Not used
                                                  (float)Angles::degrees(path->end_lat), //! x PARAM5 / local: x position, global: latitude
                                                  (float)Angles::degrees(path->end_lon), //! y PARAM6 / y position: global: longitude
                                                  0);//! z PARAM7 / z position: global: altitude
                    int n = mavlink_msg_to_send_buffer(buf, &msg);
                    sendData(buf, n);

                    // mavlink_msg_command_long_pack(255, 0, &msg, m_sysid, 0, cmd, 0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
                    mavlink_msg_mission_item_pack(255, 0, &msg,
                                                  m_sysid, //! target_system System ID
                                                  0, //! target_component Component ID
                                                  1, //! seq Sequence
                                                  MAV_FRAME_GLOBAL, //! frame The coordinate system of the MISSION. see MAV_FRAME in mavlink_types.h
                                                  MAV_CMD_DO_CHANGE_SPEED, //! command The scheduled action for the MISSION. see MAV_CMD in ardupilotmega.h
                                                  1, //! current false:0, true:1, guided mode:2
                                                  0, //! autocontinue to next wp
                                                  0, //! p1 - Radius of acceptance? Think not used.
                                                  path->speed, //!
                                                  0, // direction does not matter for copter.
                                                  0, //! Not used
                                                  0, //! x PARAM5 / local: x position, global: latitude
                                                  0, //! y PARAM6 / y position: global: longitude
                                                  0);//! z PARAM7 / z position: global: altitude
                    n = mavlink_msg_to_send_buffer(buf, &msg);
                    sendData(buf, n);
                }
                else if (m_vehicle_type == VEHICLE_FIXEDWING)
                {
                    //! Because this is a GUIDED waypoint, MISSION_COUNT and WRITE_PARTIAL_LIST messages should not be sent
                    mavlink_msg_mission_item_pack(255, 0, &msg,
                                                  m_sysid, //! target_system System ID
                                                  0, //! target_component Component ID
                                                  1, //! seq Sequence
                                                  MAV_FRAME_GLOBAL, //! frame The coordinate system of the MISSION. see MAV_FRAME in mavlink_types.h
                                                  MAV_CMD_NAV_LOITER_UNLIM, //! command The scheduled action for the MISSION. see MAV_CMD in ardupilotmega.h
                                                  2, //! current false:0, true:1, guided mode:2
                                                  0, //! autocontinue to next wp
                                                  0, //! Not used
                                                  0, //! Not used
                                                  path->flags & DesiredPath::FL_CCLOCKW ? -1 : 0, //! If <0, then CCW loiter
                                                  0, //! Not used
                                                  (float)Angles::degrees(path->end_lat), //! x PARAM5 / local: x position, global: latitude
                                                  (float)Angles::degrees(path->end_lon), //! y PARAM6 / y position: global: longitude
                                                  alt);//! z PARAM7 / z position: global: altitude
                    int n = mavlink_msg_to_send_buffer(buf, &msg);
                    sendData(buf, n);

                }

                m_changing_wp = true;

                m_pcs.start_lat = Angles::radians(m_lat);
                m_pcs.start_lon = Angles::radians(m_lon);
                m_pcs.start_z = m_alt;
                m_pcs.start_z_units = IMC::Z_HEIGHT;

                m_pcs.end_lat = path->end_lat;
                m_pcs.end_lon = path->end_lon;
                m_pcs.end_z = alt;
                m_pcs.end_z_units = IMC::Z_HEIGHT;
                m_pcs.flags = PathControlState::FL_3DTRACK | PathControlState::FL_CCLOCKW;
                m_pcs.flags &= path->flags;
                m_pcs.lradius = path->lradius;
                m_pcs.path_ref = path->path_ref;

                dispatch(m_pcs);
                m_dpath = *path;
                m_last_wp = Clock::get();

                debug("Waypoint packet sent to Ardupilot");
            }

            void
            takeoff_copter(const IMC::DesiredPath* dpath)
            {

                uint8_t buf[512];
                int seq = 1;

                mavlink_message_t msg;

                mavlink_msg_param_set_pack(255, 0, &msg,
                                           m_sysid, //! target_system System ID
                                           0, //! target_component Component ID
                                           "WP_LOITER_RAD", //! Parameter name
                                           dpath->flags & DesiredPath::FL_CCLOCKW ? (-1 * dpath->lradius) : (dpath->lradius), //! Parameter value
                                           MAV_PARAM_TYPE_INT16); //! Parameter type

                uint16_t n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);

                mavlink_msg_mission_count_pack(255, 0, &msg,
                                               m_sysid, //! target_system System ID
                                               0, //! target_component Component ID
                                               3); //! size of Mission

                n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);

                mavlink_msg_mission_write_partial_list_pack(255, 0, &msg,
                                                            m_sysid, //! target_system System ID
                                                            0, //! target_component Component ID
                                                            seq, //! start_index Start index, 0 by default and smaller / equal to the largest index of the current onboard list
                                                            seq+1); //! end_index End index, equal or greater than start index

                m_mission_items.push(msg);

                sendMissionItem(false);

                float alt = (dpath->end_z_units & IMC::Z_NONE) ? m_args.alt : (float)dpath->end_z;

                //! Current position
                mavlink_msg_mission_item_pack(255, 0, &msg,
                                              m_sysid, //! target_system System ID
                                              0, //! target_component Component ID
                                              seq++, //! seq Sequence
                                              MAV_FRAME_GLOBAL, //! frame The coordinate system of the MISSION. see MAV_FRAME in mavlink_types.h
                                              MAV_CMD_NAV_TAKEOFF, //! command The scheduled action for the MISSION. see MAV_CMD in ardupilotmega.h
                                              1, //! current false:0, true:1
                                              1, //! autocontinue autocontinue to next wp
                                              5, //! Pitch
                                              0, //! Altitude
                                              0, //! Not used
                                              0, //! Not used
                                              0, //! x PARAM5 / local: x position, global: latitude
                                              0, //! y PARAM6 / y position: global: longitude
                                              alt);//! z PARAM7 / z position: global: altitude

                m_mission_items.push(msg);

                //! Desired speed
//          mavlink_msg_mission_item_pack(255, 0, &msg,
//                                        m_sysid, //! target_system System ID
//                                        0, //! target_component Component ID
//                                        seq++, //! seq Sequence
//                                        MAV_FRAME_GLOBAL, //! frame The coordinate system of the MISSION. see MAV_FRAME in mavlink_types.h
//                                        MAV_CMD_DO_CHANGE_SPEED, //! command The scheduled action for the MISSION. see MAV_CMD in common.xml MAVLink specs
//                                        0, //! current false:0, true:1
//                                        1, //! autocontinue autocontinue to next wp
//                                        0, //! Speed type (0=Airspeed, 1=Ground Speed)
//                                        (float)(dpath->speed_units == IMC::SUNITS_METERS_PS ? dpath->speed : -1), //! Speed  (m/s, -1 indicates no change)
//                                        (float)(dpath->speed_units == IMC::SUNITS_PERCENTAGE ? dpath->speed : -1), //! Throttle  ( Percent, -1 indicates no change)
//                                        0, //! Not used
//                                        0, //! Not used
//                                        0, //! Not used
//                                        0);//! Not used

                //m_mission_items.push(msg);

                //! Destination
                mavlink_msg_mission_item_pack(255, 0, &msg,
                                              m_sysid, //! target_system System ID
                                              0, //! target_component Component ID
                                              seq++, //! seq Sequence
                                              MAV_FRAME_GLOBAL, //! frame The coordinate system of the MISSION. see MAV_FRAME in mavlink_types.h
                                              MAV_CMD_NAV_WAYPOINT, //! command The scheduled action for the MISSION. see MAV_CMD in ardupilotmega.h
                                              0, //! current false:0, true:1
                                              0, //! autocontinue autocontinue to next wp
                                              0, //! Not used
                                              0, //! Not used
                                              0, //! Not used
                                              0, //! Not used
                                              (float)Angles::degrees(dpath->end_lat), //! x PARAM5 / local: x position, global: latitude
                                              (float)Angles::degrees(dpath->end_lon), //! y PARAM6 / y position: global: longitude
                                              (float)(dpath->end_z));//! z PARAM7 / z position: global: altitude

                m_mission_items.push(msg);

                mavlink_msg_mission_set_current_pack(255, 0, &msg,
                                                     m_sysid,
                                                     0,
                                                     1);

                n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);

                m_pcs.start_lat = Angles::radians(m_lat);
                m_pcs.start_lon = Angles::radians(m_lon);
                m_pcs.start_z = m_alt;
                m_pcs.start_z_units = IMC::Z_HEIGHT;

                m_pcs.end_lat = dpath->end_lat;
                m_pcs.end_lon = dpath->end_lon;

                //float alt = (dpath->end_z_units & IMC::Z_NONE) ? m_args.alt : (float)dpath->end_z;

                m_pcs.end_z = alt;
                m_pcs.end_z_units = IMC::Z_HEIGHT;
                m_pcs.flags = PathControlState::FL_3DTRACK | PathControlState::FL_CCLOCKW;
                m_pcs.flags &= dpath->flags;
                m_pcs.lradius = dpath->lradius;
                m_pcs.path_ref = dpath->path_ref;

                dispatch(m_pcs);

                debug("Waypoint packet sent to Ardupilot");
            }

            void
            takeoff_plane(const IMC::DesiredPath* dpath)
            {
                uint8_t buf[512];
                int seq = 1;

                mavlink_message_t msg;

                mavlink_msg_param_set_pack(255, 0, &msg,
                                           m_sysid, //! target_system System ID
                                           0, //! target_component Component ID
                                           "WP_LOITER_RAD", //! Parameter name
                                           dpath->flags & DesiredPath::FL_CCLOCKW ? (-1 * dpath->lradius) : (dpath->lradius), //! Parameter value
                                           MAV_PARAM_TYPE_INT16); //! Parameter type

                uint16_t n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);

                mavlink_msg_mission_count_pack(255, 0, &msg,
                                               m_sysid, //! target_system System ID
                                               0, //! target_component Component ID
                                               4); //! size of Mission

                n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);

                mavlink_msg_mission_write_partial_list_pack(255, 0, &msg,
                                                            m_sysid, //! target_system System ID
                                                            0, //! target_component Component ID
                                                            seq, //! start_index Start index, 0 by default and smaller / equal to the largest index of the current onboard list
                                                            seq+2); //! end_index End index, equal or greater than start index

                m_mission_items.push(msg);

                sendMissionItem(false);

                //! Current position
                mavlink_msg_mission_item_pack(255, 0, &msg,
                                              m_sysid, //! target_system System ID
                                              0, //! target_component Component ID
                                              seq++, //! seq Sequence
                                              MAV_FRAME_GLOBAL, //! frame The coordinate system of the MISSION. see MAV_FRAME in mavlink_types.h
                                              MAV_CMD_NAV_TAKEOFF, //! command The scheduled action for the MISSION. see MAV_CMD in ardupilotmega.h
                                              1, //! current false:0, true:1
                                              1, //! autocontinue autocontinue to next wp
                                              5, //! Pitch
                                              0, //! Altitude
                                              0, //! Not used
                                              0, //! Not used
                                              0, //! x PARAM5 / local: x position, global: latitude
                                              0, //! y PARAM6 / y position: global: longitude
                                              m_alt + 10);//! z PARAM7 / z position: global: altitude

                m_mission_items.push(msg);

                //! Desired speed
                mavlink_msg_mission_item_pack(255, 0, &msg,
                                              m_sysid, //! target_system System ID
                                              0, //! target_component Component ID
                                              seq++, //! seq Sequence
                                              MAV_FRAME_GLOBAL, //! frame The coordinate system of the MISSION. see MAV_FRAME in mavlink_types.h
                                              MAV_CMD_DO_CHANGE_SPEED, //! command The scheduled action for the MISSION. see MAV_CMD in common.xml MAVLink specs
                                              0, //! current false:0, true:1
                                              1, //! autocontinue autocontinue to next wp
                                              0, //! Speed type (0=Airspeed, 1=Ground Speed)
                                              (float)(dpath->speed_units == IMC::SUNITS_METERS_PS ? dpath->speed : -1), //! Speed  (m/s, -1 indicates no change)
                                              (float)(dpath->speed_units == IMC::SUNITS_PERCENTAGE ? dpath->speed : -1), //! Throttle  ( Percent, -1 indicates no change)
                                              0, //! Not used
                                              0, //! Not used
                                              0, //! Not used
                                              0);//! Not used

                m_mission_items.push(msg);

                //! Destination
                mavlink_msg_mission_item_pack(255, 0, &msg,
                                              m_sysid, //! target_system System ID
                                              0, //! target_component Component ID
                                              seq++, //! seq Sequence
                                              MAV_FRAME_GLOBAL, //! frame The coordinate system of the MISSION. see MAV_FRAME in mavlink_types.h
                                              (dpath->lradius ? MAV_CMD_NAV_LOITER_UNLIM : MAV_CMD_NAV_WAYPOINT), //! command The scheduled action for the MISSION. see MAV_CMD in ardupilotmega.h
                                              0, //! current false:0, true:1
                                              0, //! autocontinue autocontinue to next wp
                                              0, //! Not used
                                              0, //! Not used
                                              0, //! Not used
                                              0, //! Not used
                                              (float)Angles::degrees(dpath->end_lat), //! x PARAM5 / local: x position, global: latitude
                                              (float)Angles::degrees(dpath->end_lon), //! y PARAM6 / y position: global: longitude
                                              (float)(dpath->end_z));//! z PARAM7 / z position: global: altitude

                m_mission_items.push(msg);

                mavlink_msg_mission_set_current_pack(255, 0, &msg,
                                                     m_sysid,
                                                     0,
                                                     1);

                n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);

                m_pcs.start_lat = Angles::radians(m_lat);
                m_pcs.start_lon = Angles::radians(m_lon);
                m_pcs.start_z = m_alt;
                m_pcs.start_z_units = IMC::Z_HEIGHT;

                m_pcs.end_lat = dpath->end_lat;
                m_pcs.end_lon = dpath->end_lon;

                float alt = (dpath->end_z_units & IMC::Z_NONE) ? m_args.alt : (float)dpath->end_z;

                m_pcs.end_z = alt;
                m_pcs.end_z_units = IMC::Z_HEIGHT;
                m_pcs.flags = PathControlState::FL_3DTRACK | PathControlState::FL_CCLOCKW;
                m_pcs.flags &= dpath->flags;
                m_pcs.lradius = dpath->lradius;
                m_pcs.path_ref = dpath->path_ref;

                dispatch(m_pcs);

                debug("Waypoint packet sent to Ardupilot");
            }

            void
            loiterHere(void)
            {

                if ((getEntityState() != IMC::EntityState::ESTA_NORMAL) || m_external)
                    return;

                if (m_vehicle_type != VEHICLE_ASV && m_ground) return;

                /*mavlink_message_t msg;
                uint8_t buf[512];

                mavlink_msg_param_set_pack(255, 0, &msg,
                                           m_sysid, //! target_system System ID
                                           0, //! target_component Component ID
                                           "WP_LOITER_RAD", //! Parameter name
                                           m_args.lradius, //! Parameter value
                                           MAV_PARAM_TYPE_INT16); //! Parameter type

                uint16_t n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);
      */

                sendCommandPacket(MAV_CMD_NAV_LOITER_UNLIM);

                debug("Sent LOITER packet to Ardupilot");

                m_pcs.start_lat = m_fix.lat;
                m_pcs.start_lon = m_fix.lon;
                m_pcs.start_z = m_fix.height;

                m_pcs.end_lat = m_fix.lat;
                m_pcs.end_lon = m_fix.lon;
                m_pcs.end_z = m_fix.height;

                if( m_vehicle_type == VEHICLE_COPTER )
                {
                    // Calculate the prev. absolute position
                    double lat = m_estate.lat;
                    double lon = m_estate.lon;
                    WGS84::displace(m_estate.x, m_estate.y, &lat, &lon);
                    m_pcs.end_lat = lat;
                    m_pcs.end_lon = lon;
                }

                m_pcs.flags = PathControlState::FL_LOITERING | PathControlState::FL_NEAR | (m_args.lradius < 0 ? PathControlState::FL_CCLOCKW : 0);
                m_pcs.lradius = m_args.lradius * (m_args.lradius < 0 ? -1 : 1);

                dispatch(m_pcs);
            }

            void
            consume(const IMC::IdleManeuver* idle)
            {
                debug("IDLE MANEUVER RECEIVED");
                (void)idle;
                m_dpath.clear();

                loiterHere();
            }

            //! Trigger relay in Ardupilot, used to shoot pictures with photo camera
            void
            consume(const IMC::PowerChannelControl* pcc)
            {
                trace("Trigger Request Received");

                if (pcc->op & IMC::PowerChannelControl::PCC_OP_TURN_ON)
                    sendCommandPacket(MAV_CMD_DO_SET_RELAY, 0, 1);
                else
                    sendCommandPacket(MAV_CMD_DO_SET_RELAY, 0, 0);
            }

            void
            consume(const IMC::VehicleMedium* vm)
            {
                m_ground = (vm->medium == IMC::VehicleMedium::VM_GROUND);
            }

            void
            consume(const IMC::VehicleState* msg)
            {
                m_service = (msg->op_mode == IMC::VehicleState::VS_SERVICE);
            }

            //! Used for HITL simulations
            void
            consume(const IMC::SimulatedState* sim_state)
            {
                if (!m_ctx.profiles.isSelected("HITL"))
                    return;

                mavlink_message_t msg;
                uint8_t buf[512];

                double lat_ref = sim_state->lat;
                double lon_ref = sim_state->lon;
                float hei_ref = sim_state->height;

                int lat, lon, alt;

                WGS84::displace(sim_state->x, sim_state->y, sim_state->z,
                                &lat_ref, &lon_ref, &hei_ref);

                lat = (int) (Angles::degrees(lat_ref) * 1E7);
                lon = (int) (Angles::degrees(lon_ref) * 1E7);
                alt = (int) (hei_ref * 1000);

                float vx, vy, vz;

                Coordinates::BodyFixedFrame::toInertialFrame(sim_state->phi, sim_state->theta, sim_state->psi,
                                                             sim_state->u, sim_state->v, sim_state->w,
                                                             &vx, &vy, &vz);

                mavlink_msg_hil_state_pack(255, 0, &msg,
                                           (unsigned long int) (Clock::getSinceEpochNsec() / 1000), //! Timestamp (microseconds since UNIX epoch or microseconds since system boot)
                                           sim_state->phi, //! Roll angle (rad)
                                           sim_state->theta, //! Pitch angle (rad)
                                           sim_state->psi, //! Yaw angle (rad)
                                           sim_state->p, //! Roll angular speed (rad/s)
                                           sim_state->q, //! Pitch angular speed (rad/s)
                                           sim_state->r, //! Yaw angular speed (rad/s)
                                           lat, //! Latitude, expressed as * 1E7
                                           lon, //! Longitude, expressed as * 1E7
                                           alt, //! Altitude in meters, expressed as * 1000 (millimeters)
                                           (short int) (vx * 100), //! Ground X Speed (Latitude), expressed as m/s * 100
                                           (short int) (vy * 100), //! Ground Y Speed (Longitude), expressed as m/s * 100
                                           (short int) (vz * 100), //! Ground Z Speed (Altitude), expressed as m/s * 100
                                           0, //! X acceleration (mg)
                                           0, //! Y acceleration (mg)
                                           0); //! Z acceleration (mg)

                uint16_t n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);
            }

            void
            sendCommandPacket(uint16_t cmd, float arg1=0, float arg2=0, float arg3=0, float arg4=0, float arg5=0, float arg6=0, float arg7=0)
            {
                uint8_t buf[512];

                mavlink_message_t msg;

                trace("%0.2f %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f", arg1, arg2, arg3, arg4, arg5, arg6, arg7);

                mavlink_msg_command_long_pack(255, 0, &msg, m_sysid, 0, cmd, 0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);

                uint16_t n = mavlink_msg_to_send_buffer(buf, &msg);
                sendData(buf, n);
            }

            void
            sendMissionItem(bool next)
            {
                if (next && !m_mission_items.empty())
                    m_mission_items.pop();

                if (m_mission_items.empty())
                {
                    debug("Mission Item queue is empty.");
                    return;
                }

                uint8_t buf[512];

                uint16_t n = mavlink_msg_to_send_buffer(buf, &m_mission_items.front());
                sendData(buf, n);
            }

            void
            onMain(void)
            {
                while (!stopping())
                {
                    // Handle data
                    if (m_TCP_sock)
                    {
                        handleArdupilotData();
                    }
                    else
                    {
                        Time::Delay::wait(0.5);
                        openConnection();
                    }

                    if (!m_error_missing)
                    {
                        if (m_external)
                        {
                            if (!m_esta_ext)
                            {
                                setEntityState(IMC::EntityState::ESTA_NORMAL, "External Control");
                                m_esta_ext = true;
                            }
                        }
                        else// if (getEntityState() != IMC::EntityState::ESTA_NORMAL)
                        {
                            setEntityState(IMC::EntityState::ESTA_NORMAL, Status::CODE_ACTIVE);
                            m_esta_ext = false;
                        }
                    }

                    // Handle IMC messages from bus
                    consumeMessages();
                }
            }

            bool
            poll(double timeout)
            {
                if (m_TCP_sock != NULL)
                    return Poll::poll(*m_TCP_sock, timeout);

                return false;
            }

            int
            sendData(uint8_t* bfr, int size)
            {
                if (m_TCP_sock)
                {
                    trace("Sending something");
                    return m_TCP_sock->write((char*)bfr, size);
                }
                return 0;
            }

            int
            receiveData(uint8_t* buf, size_t blen)
            {
                if (m_TCP_sock)
                {
                    try
                    {
                        return m_TCP_sock->read(buf, blen);
                    }
                    catch (std::runtime_error& e)
                    {
                        err("%s", e.what());
                        war(DTR("Connection lost, retrying..."));
                        Memory::clear(m_TCP_sock);

                        m_TCP_sock = new Network::TCPSocket;
                        m_TCP_sock->connect(m_TCP_addr, m_TCP_port);
                        return 0;
                    }
                }
                return 0;
            }

            void
            handleArdupilotData(void)
            {
                mavlink_status_t status;

                double now = Clock::get();
                int counter = 0;

                while (poll(0.01) && counter < 100)
                {
                    counter++;

                    int n = receiveData(m_buf, sizeof(m_buf));

                    if (n < 0)
                    {
                        debug("Receive error");
                        break;
                    }

                    now = Clock::get();


                    for (int i = 0; i < n; i++)
                    {
                        int rv = mavlink_parse_char(MAVLINK_COMM_0, m_buf[i], &m_msg, &status);
                        if (status.packet_rx_drop_count)
                        {
                            switch(status.parse_state)
                            {
                                case MAVLINK_PARSE_STATE_IDLE:
                                    spew("failed at state IDLE");
                                    break;
                                case MAVLINK_PARSE_STATE_GOT_STX:
                                    spew("failed at state GOT_STX");
                                    break;
                                case MAVLINK_PARSE_STATE_GOT_LENGTH:
                                    spew("failed at state GOT_LENGTH");
                                    break;
                                case MAVLINK_PARSE_STATE_GOT_SYSID:
                                    spew("failed at state GOT_SYSID");
                                    break;
                                case MAVLINK_PARSE_STATE_GOT_COMPID:
                                    spew("failed at state GOT_COMPID");
                                    break;
                                case MAVLINK_PARSE_STATE_GOT_MSGID:
                                    spew("failed at state GOT_MSGID");
                                    break;
                                case MAVLINK_PARSE_STATE_GOT_PAYLOAD:
                                    spew("failed at state GOT_PAYLOAD");
                                    break;
                                case MAVLINK_PARSE_STATE_GOT_CRC1:
                                    spew("failed at state GOT_CRC1");
                                    break;
                                default:
                                    spew("failed OTHER");
                            }
                        }
                        if (rv)
                        {
                            switch ((int)m_msg.msgid)
                            {
                                default:
                                    debug("UNDEF: %u", m_msg.msgid);
                                    break;
                                case MAVLINK_MSG_ID_HEARTBEAT:
                                    trace("HEARTBEAT");
                                    break;
                                case MAVLINK_MSG_ID_SYS_STATUS:
                                    trace("SYS_STATUS");
                                    break;
                                case MAVLINK_MSG_ID_SYSTEM_TIME:
                                    trace("SYSTEM_TIME");
                                    break;
                                case 22:
                                    trace("PARAM_VALUE");
                                    break;
                                case MAVLINK_MSG_ID_GPS_RAW_INT:
                                    spew("GPS_RAW");
                                    break;
                                case 27:
                                    trace("IMU_RAW");
                                    break;
                                case MAVLINK_MSG_ID_SCALED_PRESSURE:
                                    spew("SCALED_PRESSURE");
                                    break;
                                case MAVLINK_MSG_ID_ATTITUDE:
                                    spew("ATTITUDE");
                                    break;
                                case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
                                    spew("GLOBAL_POSITION_INT");
                                    break;
                                case 34:
                                    trace("RC_CHANNELS_SCALED");
                                    break;
                                case 35:
                                    trace("RC_CHANNELS_RAW");
                                    break;
                                case MAVLINK_MSG_ID_MISSION_ITEM:
                                    trace("MISSION_ITEM");
                                    break;
                                case MAVLINK_MSG_ID_MISSION_REQUEST:
                                    trace("MISSION_REQUEST");
                                    break;
                                case MAVLINK_MSG_ID_MISSION_CURRENT:
                                    trace("MISSION_CURRENT");
                                    break;
                                case MAVLINK_MSG_ID_MISSION_ACK:
                                    spew("MISSION_ACK");
                                    break;
                                case MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:
                                    trace("NAV_CONTROLLER_OUTPUT");
                                    break;
                                case MAVLINK_MSG_ID_VFR_HUD:
                                    trace("VFR_HUD");
                                    break;
                                case MAVLINK_MSG_ID_COMMAND_ACK:
                                    spew("CMD_ACK");
                                    break;
                                case MAVLINK_MSG_ID_BATTERY_STATUS:
                                    spew("BATTERY_STAT");
                                    break;
                                case 150:
                                    trace("SENSOR_OFFSETS");
                                    break;
                                case 152:
                                    trace("MEMINFO");
                                    break;
                                case 162:
                                    trace("FENCE_STATUS");
                                    break;
                                case 163:
                                    trace("AHRS");
                                    break;
                                case 164:
                                    trace("SIM_STATE");
                                    break;
                                case MAVLINK_MSG_ID_HWSTATUS:
                                    spew("HW_STATUS");
                                    break;
                                case MAVLINK_MSG_ID_WIND:
                                    spew("WIND");
                                    break;
                                case MAVLINK_MSG_ID_STATUSTEXT:
                                    trace("STATUSTEXT");
                                    break;
                                case MAVLINK_MSG_ID_DEBUG_VECT:
                                    trace("DEBUG_VECT");
                                    break;
                                case MAVLINK_MSG_ID_NAMED_VALUE_INT:
                                    trace("NAMED_VALUE_INT");
                                    break;
                                case MAVLINK_MSG_ID_NAMED_VALUE_FLOAT:
                                    trace("NAMED_VALUE_FLOAT");
                                    break;
                            }

                            PktHandler h = m_mlh[m_msg.msgid];

                            if (!h)
                                continue;  // Ignore this packet (no handler for it)

                            // Call handler
                            (this->*h)(&m_msg);
                            m_sysid = m_msg.sysid;

                            m_last_pkt_time = now;
                        }
                    }
                }

                if (now - m_last_pkt_time >= m_args.comm_timeout)
                {
                    if (!m_error_missing)
                    {
                        setEntityState(IMC::EntityState::ESTA_ERROR, Status::CODE_MISSING_DATA);
                        m_error_missing = true;
                        m_esta_ext = false;
                    }
                }
                else
                    m_error_missing = false;
            }

            void
            handleAttitudePacket(const mavlink_message_t* msg)
            {
                mavlink_attitude_t att;

                mavlink_msg_attitude_decode(msg, &att);
                m_estate.phi = att.roll;
                m_estate.theta = att.pitch;
                m_estate.psi = att.yaw;
                m_estate.p = att.rollspeed;
                m_estate.q = att.pitchspeed;
                m_estate.r = att.yawspeed;

                dispatch(m_estate);
            }

            void
            handlePositionPacket(const mavlink_message_t* msg)
            {
                mavlink_global_position_int_t gp;
                mavlink_msg_global_position_int_decode(msg, &gp);

                double lat = Angles::radians((double)gp.lat * 1e-07);
                double lon = Angles::radians((double)gp.lon * 1e-07);
                float hei = m_alt;

                m_lat = (double)gp.lat * 1e-07;
                m_lon = (double)gp.lon * 1e-07;

                double distance_to_ref = WGS84::distance(ref_lat,ref_lon,ref_hei,
                                                         lat,lon,hei);

                if(true) //distance_to_ref>1000)
                {
                    m_estate.lat = lat;
                    m_estate.lon = lon;
                    m_estate.height = hei;

                    m_estate.x = 0;
                    m_estate.y = 0;
                    m_estate.z = 0;

                    ref_lat = lat;
                    ref_lon = lon;
                    ref_hei = hei;
                }
                else
                {
                    WGS84::displacement(ref_lat, ref_lon, ref_hei,
                                        lat, lon, hei,
                                        &m_estate.x, &m_estate.y, &m_estate.z);

                    m_estate.lat = ref_lat;
                    m_estate.lon = ref_lon;
                    m_estate.height = ref_hei;
                }

                m_estate.vx = 1e-02 * gp.vx;
                m_estate.vy = 1e-02 * gp.vy;
                m_estate.vz = -1e-02 * gp.vz;

                // Note: the following will yield body-fixed *ground* velocity
                // Maybe this can be fixed w/IAS readings (anyway not too important)
                BodyFixedFrame::toBodyFrame(m_estate.phi, m_estate.theta, m_estate.psi,
                                            m_estate.vx, m_estate.vy, m_estate.vz,
                                            &m_estate.u, &m_estate.v, &m_estate.w);

                m_estate.depth = -1;
                m_estate.alt = -1;

                dispatch(m_estate);

                //std::cout << "DUNE: Global Position Int received" << std::endl;
            }

            void
            handleHWStatusPacket(const mavlink_message_t* msg)
            {
                if (m_args.pwrm)
                {
                    (void) msg;
                    return;
                }

                mavlink_hwstatus_t hw_status;

                mavlink_msg_hwstatus_decode(msg, &hw_status);

                m_volt.value = 0.001 * hw_status.Vcc;

                dispatch(m_volt);

                // std::cout << "DUNE: HWstatus Packet received. VOLT = " << m_volt.value << std::endl;
            }

            void
            handleSystemStatusPacket(const mavlink_message_t* msg)
            {
                if (!m_args.pwrm)
                {
                    (void) msg;
                    return;
                }
                mavlink_sys_status_t sys_status;

                mavlink_msg_sys_status_decode(msg, &sys_status);

                m_volt.value = 0.001 * (float)sys_status.voltage_battery;
                m_curr.value = 0.01 * (float)sys_status.current_battery;
                m_fuel.value = (float)sys_status.battery_remaining;

                dispatch(m_volt);
                dispatch(m_curr);
                dispatch(m_fuel);

                /*
                std::cout << "DUNE: System Status Packet received" << std::endl;
                std::cout << "VOLT = " << m_volt.value << " CURRENT = " << m_curr.value <<
                        " BATTERY = " << m_fuel.value << std::endl;
                */
            }

            void
            handleScaledPressurePacket(const mavlink_message_t* msg)
            {
                mavlink_scaled_pressure_t sc_press;

                mavlink_msg_scaled_pressure_decode(msg, &sc_press);

                m_pressure.value = sc_press.press_abs;
                m_temp.value = 0.01 * sc_press.temperature;

                dispatch(m_pressure);
                dispatch(m_temp);
            }

            void
            handleRawGPSPacket(const mavlink_message_t* msg)
            {
                mavlink_gps_raw_int_t gps_raw;

                mavlink_msg_gps_raw_int_decode(msg, &gps_raw);

                m_fix.cog = Angles::radians((double)gps_raw.cog * 0.01);
                m_fix.sog = (float)gps_raw.vel * 0.01;
                m_fix.hdop = (float)gps_raw.eph * 0.01;
                m_fix.vdop = (float)gps_raw.epv * 0.01;
                m_fix.lat = Angles::radians((double)gps_raw.lat * 1e-07);
                m_fix.lon = Angles::radians((double)gps_raw.lon * 1e-07);
                m_fix.height = (double)gps_raw.alt * 0.001;
                m_fix.satellites = gps_raw.satellites_visible;

                std::cout << "m_fix.lat: " << std::setprecision(7) << m_fix.lat << " m_fix.lon: " << m_fix.lon << std::endl;

                m_fix.validity = 0;
                if (gps_raw.fix_type>=1)
                    m_fix.validity |= IMC::GpsFix::GFV_VALID_POS;
                if (m_fix.utc_year>2012)
                    m_fix.validity |= (IMC::GpsFix::GFV_VALID_TIME | IMC::GpsFix::GFV_VALID_DATE);

                dispatch(m_fix); // Elias
            }


            void
            handleWindPacket(const mavlink_message_t* msg)
            {
                mavlink_wind_t wind;

                mavlink_msg_wind_decode(msg, &wind);

                double wind_dir_rad = wind.direction * Math::c_pi / 180;

                m_stream.x = -std::cos(wind_dir_rad) * wind.speed;
                m_stream.y = -std::sin(wind_dir_rad) * wind.speed;
                m_stream.z = wind.speed_z;

                dispatch(m_stream);
            }

            void
            handleCmdAckPacket(const mavlink_message_t* msg)
            {
                mavlink_command_ack_t cmd_ack;

                mavlink_msg_command_ack_decode(msg, &cmd_ack);
                debug("Command %d was received, result is %d", cmd_ack.command, cmd_ack.result);
                m_changing_wp = false;
                m_last_wp = 0;
            }

            void
            handleMissionAckPacket(const mavlink_message_t* msg)
            {
                mavlink_mission_ack_t miss_ack;

                mavlink_msg_mission_ack_decode(msg, &miss_ack);
                debug("Mission was received, result is %d", miss_ack.type);
                m_changing_wp = false;
                m_last_wp = 0;
                std::cout << "DUNE: Mission ACK received" << std::endl;
            }

            void
            handleMissionCurrentPacket(const mavlink_message_t* msg)
            {
                mavlink_mission_current_t miss_curr;

                mavlink_msg_mission_current_decode(msg, &miss_curr);
                m_current_wp = miss_curr.seq;
                trace("Current mission item: %d", miss_curr.seq);

                uint8_t buf[512];

                mavlink_message_t msg_out;

                mavlink_msg_mission_request_pack(255, 0, &msg_out,
                                                 m_sysid, //! target_system System ID
                                                 0, //! target_component Component ID
                                                 m_current_wp); //! Mission item to request

                uint16_t n = mavlink_msg_to_send_buffer(buf, &msg_out);
                sendData(buf, n);
                std::cout << "DUNE: Mission Current received. WP = " << miss_curr.seq << std::endl;
            }

            void
            handleStatusTextPacket(const mavlink_message_t* msg)
            {
                mavlink_statustext_t stat_tex;
                mavlink_msg_statustext_decode(msg, &stat_tex);
                debug("Status: %s", stat_tex.text);
                std::cout << "STATUSTEXT: " << stat_tex.text << std::endl;
            }

            void
            handleHeartbeatPacket(const mavlink_message_t* msg)
            {
                if (!m_has_setup_rate)
                {
                    m_has_setup_rate = true;
                    setupRate(m_args.trate);
                    debug("Rates setup second time.");
                }

                mavlink_heartbeat_t hbt;
                mavlink_msg_heartbeat_decode(msg, &hbt);

                IMC::AutopilotMode mode;

                // Update vehicle type if applicable
                if (m_vehicle_type == VEHICLE_UNKNOWN)
                {
                    MAV_TYPE mav_type = static_cast<MAV_TYPE>(hbt.type);
                    switch (mav_type)
                    {
                        default:
                            err(DTR("Controlling an unknown vehicle type."));
                            break;
                        case MAV_TYPE_FIXED_WING:
                            m_vehicle_type = VEHICLE_FIXEDWING;
                            debug("Controlling a fixed-wing vehicle.");
                            break;
                        case MAV_TYPE_QUADROTOR:
                        case MAV_TYPE_HEXAROTOR:
                        case MAV_TYPE_OCTOROTOR:
                        case MAV_TYPE_TRICOPTER:
                            m_vehicle_type = VEHICLE_COPTER;
                            debug("Controlling a multicopter.");
                            break;
                        case MAV_TYPE_SURFACE_BOAT:
                            m_vehicle_type = VEHICLE_ASV;
                            debug("Controlling an ASV!");
                            break;
                    }
                }

                m_mode = hbt.custom_mode;
                if (m_vehicle_type == VEHICLE_COPTER)
                {
                    switch(m_mode)
                    {
                        default:
                            mode.autonomy = IMC::AutopilotMode::AL_MANUAL;
                            mode.mode = "MANUAL";
                            m_external = true;
                            break;
                        case CP_MODE_AUTO:
                            mode.autonomy = IMC::AutopilotMode::AL_AUTO;
                            mode.mode = "AUTO";
                            trace("AUTO");
                            m_external = false;
                            break;
                        case CP_MODE_LOITER:
                            mode.autonomy = IMC::AutopilotMode::AL_AUTO;
                            mode.mode = "LOITER";
                            trace("LOITER");
                            m_external = false;
                            break;
                        case CP_MODE_DUNE:
                            mode.autonomy = IMC::AutopilotMode::AL_AUTO;
                            mode.mode = "DUNE";
                            trace("DUNE");
                            m_external = false;
                            break;
                        case CP_MODE_GUIDED:
                            mode.autonomy = IMC::AutopilotMode::AL_AUTO;
                            mode.mode = "GUIDED";
                            trace("GUIDED");
                            m_external = false;
                            break;
                    }
                }
                else if (m_vehicle_type == VEHICLE_FIXEDWING)
                {
                    switch(m_mode)
                    {
                        default:
                            m_external = true;
                            mode.autonomy = IMC::AutopilotMode::AL_MANUAL;
                            mode.mode = "MANUAL";
                            m_critical = false;
                            if (m_mode == 2)
                            {
                                mode.autonomy = IMC::AutopilotMode::AL_ASSISTED;
                                mode.mode = "STABILIZE";
                            }
                            {
                                IMC::ControlLoops cl;
                                cl.enable = IMC::ControlLoops::CL_DISABLE;
                                cl.mask = IMC::CL_ALL;
                                receive(&cl);
                            }
                            break;
                        case 10:
                            mode.autonomy = IMC::AutopilotMode::AL_AUTO;
                            mode.mode = "AUTO";
                            trace("AUTO");
                            if (m_external)
                            {
                                m_external = false;
                                if (m_dpath.end_lat)
                                {
                                    receive(&m_controllps);
                                    receive(&m_dpath);
                                }
                            }
                            if (m_service)
                                loiterHere();
                            break;
                        case 12:
                            mode.autonomy = IMC::AutopilotMode::AL_AUTO;
                            mode.mode = "LOITER";
                            trace("LOITER");
                            m_external = false;
                            m_critical = false;
                            break;
                        case 6:
                            mode.autonomy = IMC::AutopilotMode::AL_AUTO;
                            mode.mode = "FBWB";
                            trace("FBWB");
                            m_external = false;
                            m_critical = false;
                            break;
                        case 15:
                            mode.autonomy = IMC::AutopilotMode::AL_AUTO;
                            mode.mode = "GUIDED";
                            trace("GUIDED");
                            m_external = false;
                            m_critical = false;
                            break;
                    }
                }
                else if (m_vehicle_type == VEHICLE_ASV)
                {
                    switch(m_mode)
                    {
                        default:
                            m_external = true;
                            mode.autonomy = IMC::AutopilotMode::AL_MANUAL;
                            mode.mode = "MANUAL";
                            m_critical = false;
                            break;
                        case 11:
                            mode.autonomy = IMC::AutopilotMode::AL_AUTO;
                            mode.mode = "AUTO";
                            m_critical = false;
                            m_external = false;
                            break;
                        case 10:
                            mode.autonomy = IMC::AutopilotMode::AL_MANUAL;
                            mode.mode = "MANUAL";
                            m_critical = false;
                            m_external = true;
                            break;
                    }
                }

                dispatch(mode);
            }

            void
            handleNavControllerPacket(const mavlink_message_t* msg)
            {
                mavlink_nav_controller_output_t nav_out;
                mavlink_msg_nav_controller_output_decode(msg, &nav_out);
                trace("WP Dist: %d", nav_out.wp_dist);

                bool is_valid_mode = false;

                if (m_mode == 11) {is_valid_mode = true;}

                float since_last_wp = Clock::get() - m_last_wp;
                bool is_near = ((nav_out.wp_dist <= 5)
                                && (is_valid_mode)
                                && (since_last_wp > 1.0));
                /*
                          && (nav_out.wp_dist <= m_desired_radius + m_args.secs * m_fix.sog)
                          && (nav_out.wp_dist >= m_desired_radius - m_args.secs * m_fix.sog)
                */
                if (is_near)
                {
                    m_pcs.flags |= PathControlState::FL_NEAR;
                }

                if (m_last_wp && since_last_wp > 1.5)
                    receive(&m_dpath);

                if (m_gnd_speed)
                    m_pcs.eta = nav_out.wp_dist / m_fix.sog;

                dispatch(m_pcs);
            }

            void
            handleMissionItemPacket(const mavlink_message_t* msg)
            {
                mavlink_mission_item_t miss_item;
                mavlink_msg_mission_item_decode(msg, &miss_item);
                trace("Mission type: %d", miss_item.command);

                switch(miss_item.command)
                {
                    default:
                        m_critical = false;
                        break;
                    case MAV_CMD_NAV_TAKEOFF:
                        m_critical = true;
                        break;
                    case MAV_CMD_NAV_LAND:
                        m_critical = true;
                        break;
                }
                std::cout << "DUNE: Mission Item Packet received" << std::endl;
            }

            void
            handleHUDPacket(const mavlink_message_t* msg)
            {
                mavlink_vfr_hud_t vfr_hud;
                mavlink_msg_vfr_hud_decode(msg, &vfr_hud);

                //IMC::IndicatedSpeed ias;
                IMC::TrueSpeed gs;


                //ias.value = (fp64_t)vfr_hud.airspeed;
                gs.value = (fp64_t)vfr_hud.groundspeed;
                m_gnd_speed = (int)vfr_hud.groundspeed;
                m_alt = vfr_hud.alt;

                //dispatch(ias);
                dispatch(gs);
            }

            void
            handleSystemTimePacket(const mavlink_message_t* msg)
            {
                inf("Got System Time Packet: \n");
                mavlink_system_time_t sys_time;
                mavlink_msg_system_time_decode(msg, &sys_time);

                time_t t = sys_time.time_unix_usec / 1000000;
                struct tm* utc;
                utc = gmtime(&t);

                m_fix.utc_time = utc->tm_hour * 3600 + utc->tm_min * 60 + utc->tm_sec + (sys_time.time_unix_usec % 1000000) * 1e-6;
                m_fix.utc_year = utc->tm_year + 1900;
                m_fix.utc_month = utc->tm_mon +1;
                m_fix.utc_day = utc->tm_mday;

                std::cout << "UTC: " << sys_time.time_unix_usec << std::endl;
                std::cout << "H: " << utc->tm_hour << std::endl;
                std::cout << "M: " << utc->tm_min << std::endl;
                std::cout << "S: " << utc->tm_sec << std::endl;

                if (m_fix.utc_year>2014)
                    m_fix.validity |= (IMC::GpsFix::GFV_VALID_TIME | IMC::GpsFix::GFV_VALID_DATE);

                if (!m_args.hitl)
                    dispatch(m_fix);
            }

            void
            handleMissionRequestPacket(const mavlink_message_t* msg)
            {
                mavlink_mission_request_t mission_request;
                mavlink_msg_mission_request_decode(msg, &mission_request);

                debug("Requesting item #%d", mission_request.seq);

                sendMissionItem(true);
            }

            void
            handleDebugVectPacket(const mavlink_message_t* msg)
            {
                mavlink_debug_vect_t debug_vect;
                mavlink_msg_debug_vect_decode(msg,&debug_vect);

                //std::cout << debug_vect.name << ":" << std::endl;
                printf("%s: X(Lat) = %.4f Y(Lon) = %.4f Z(Alt) = %.4f \n",
                       debug_vect.name,debug_vect.x,debug_vect.y,debug_vect.z);
                /*
                std::cout << "X (Lat): " << debug_vect.x << std::endl;
                std::cout << "Y (Lon): " << debug_vect.y << std::endl;
                std::cout << "Z (Alt): " << debug_vect.z << std::endl;
                */

            }

            void
            handleNamedValueIntPacket(const mavlink_message_t* msg)
            {
                mavlink_named_value_int_t m_int;
                mavlink_msg_named_value_int_decode(msg,&m_int);

                std::cout << m_int.name << ": " << m_int.value << std::endl;
            }

            void
            handleNamedValueFloatPacket(const mavlink_message_t* msg)
            {
                mavlink_named_value_float_t m_float;
                mavlink_msg_named_value_float_decode(msg,&m_float);

                printf("%s: %.4f \n",m_float.name,m_float.value);
/*Funderade på att göra någon smart switchgrejj för schysst format, men bättre att skicka
 * rätt direkt från ardupilot.
        	switch (m_float.name)
        	{
        	default:
        		printf("%s: %.4f \n",m_float.name,m_float.value);
        		break;
        	case "Heading":
        		printf("%s [deg]: %.4f \n",m_float.name,Angles::degrees(m_float.value));
        		break;
        	case "CC":
        		printf("%s [deg]: %.4f \n",m_float.name,Angles::degrees(m_float.value));
        		break;

        	}
*/
            }
        };
    }
}

DUNE_TASK