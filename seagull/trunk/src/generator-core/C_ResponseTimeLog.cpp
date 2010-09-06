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

#include "Utils.hpp"
#include "iostream_t.hpp"
#include "fstream_t.hpp"
#include "C_ResponseTimeLog.hpp"
#include "TimeUtils.hpp"
#include "ProtocolData.hpp"

C_ResponseTimeLog:: C_ResponseTimeLog(string_t  *P_LogFileName, unsigned long L_rsptime_threshold)
{    
    m_rsptime_log_filename = *P_LogFileName;
    m_rsptime_threshold = L_rsptime_threshold;
    m_RspTimeTraceStream =  &iostream_error ;    
}

C_ResponseTimeLog::~C_ResponseTimeLog
(
)
{

    if (m_RspTimeTraceStream != &iostream_error) 
    {
        fstream_output *L_stream = dynamic_cast<fstream_output*>(m_RspTimeTraceStream);
        DELETE_VAR(L_stream);
        m_RspTimeTraceStream = &iostream_error;
    }
}

bool C_ResponseTimeLog::Init
(
)
{
   fstream_output* L_stream ;
   NEW_VAR(L_stream, fstream_output(m_rsptime_log_filename.c_str()));
   if (!L_stream->good()) 
   {
       iostream_error <<  "Unable to open file [" << m_rsptime_log_filename.c_str() << "]" ;
       return false ;
   } 
    
   m_RspTimeTraceStream = L_stream ;            
   return true;
}

void C_ResponseTimeLog::LogRspTimeInfo
(
    T_pCallContext  P_callCtxt,
    C_MessageFrame *P_msg,
    C_MessageFrame *P_ref,
    unsigned long   L_RspTime
)
{
    char L_Req_Timestamp [TIME_STRING_LENGTH] ;
    char L_Rsp_Timestamp[TIME_STRING_LENGTH] ;
    T_pValueData  L_pSessionId;
      

    
    if (L_RspTime > m_rsptime_threshold)    
    {
    
        if (0 == P_msg->logMsgHeaderInfo(m_RspTimeTraceStream))
        {
        
            time_tochar_minus(L_Req_Timestamp, &P_callCtxt->m_start_time);
            time_tochar_minus(L_Rsp_Timestamp, &P_callCtxt->m_current_time);
            L_pSessionId = P_msg->get_session_id(P_callCtxt);

            (*m_RspTimeTraceStream)<<(*L_pSessionId)<<","<<L_Req_Timestamp<<","<<L_Rsp_Timestamp<<","<<L_RspTime<<iostream_endl;
        }
    }
    
    return;
}

