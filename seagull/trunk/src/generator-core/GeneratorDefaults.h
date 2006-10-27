/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * (c)Copyright 2006 Hewlett-Packard Development Company, LP.
 *
 */

#ifndef _GENERATOR_DEFAULTS_H
#define _GENERATOR_DEFAULTS_H

#define DEF_OPTION_LOG_FILE                                   NULL
#define DEF_OPTION_CONF_FILE                                  NULL
#define DEF_OPTION_DICO_FILE                                  NULL
#define DEF_OPTION_SCEN_FILE                                  NULL
#define DEF_OPTION_LOG_LEVEL                         LOG_LEVEL_ERR
#define DEF_OPTION_BG_MODE                                   false
#define DEF_CALL_RATE                                            0
#define DEF_BURST_LIMIT                                          0
#define DEF_MAX_SEND                                            10
#define DEF_MAX_RECEIVE                                         10
#define DEF_SELECT_TIMEOUT_MS                                 1000
#define DEF_MAX_SIMULTANEOUS_CALLS                             100
#define DEF_CALL_TIMEOUT                                         0
#define DEF_MSG_BUFFER_SIZE                                   4096
#define DEF_DISPLAY_PERIOD                                       0
#define DEF_LOG_STAT_PERIOD                                      0
#define DEF_LOG_STAT_FILE                                     NULL
#define DEF_RESP_TIME_REPART       (char*)"50,75,100,150,300,5000"
#define DEF_NUMBER_CALLS                                         0
#define DEF_TIMESTAMP_LOG                                     true
#define DEF_CHECK_MSG                                        false
#define DEF_CHECK_LEVEL               E_CHECK_LEVEL_FIELD_PRESENCE
#define DEF_CHECK_BEHAVIOUR              E_CHECK_BEHAVIOUR_WARNING
#define DEF_DATA_LOG_FILE                                     NULL
#define DEF_DATA_LOG_PERIOD                                      1
#define DEF_DATA_LOG_NUMBER                                    200
#define DEF_CALL_RATE_SCALE                                      1
#define DEF_EXTERNAL_DATA_FILE                                NULL
#define DEF_EXTERNAL_DATA_SELECT                                 0
#define DEF_DATA_LOG_RTDISTRIB                                   0
#define DEF_LOG_PROTOCOL_STAT_PERIOD                             0
#define DEF_LOG_PROTOCOL_STAT                                 NULL
#define DEF_LOG_PROTOCOL_STAT_FILE                            NULL
#define DEF_DISPLAY_PROTOCOL_STAT                            false
#define DEF_DISPLAY_SCENARIO_STAT                            false
#define DEF_FILES_NO_TIMESTAMP                               false
#define DEF_CALL_TIMEOUT_BEH_ABRT                            false
#define DEF_OPEN_TIMEOUT                                         0
#define DEF_EXECUTE_CHECK_ACTION                              true
#define DEF_MAX_RETRANS                                          0
#define DEF_RETRANS_ENABLED                                  false
#define DEF_MODEL_TRAFFIC_SELECT           E_MODEL_TRAFFIC_UNIFORM
#define DEF_OPTION_REMOTE_CMD                                 NULL
#define DEF_REMOTE_DICO_PATH (char*)"/usr/local/share/seagull/config"



#endif

