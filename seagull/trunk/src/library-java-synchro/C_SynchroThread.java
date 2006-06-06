//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// (c)Copyright 2006 Hewlett-Packard Development Company, LP.
//
//
// 
// C_SynchroThread class definition
//

import java.net.* ;  // for Socket
import java.io.* ;   // for IOException, BufferedInputStream
import java.nio.* ;  // for ByteBuffer

import java.util.* ; // for Vector

public class C_SynchroThread extends Thread {

    public final static int STATE_DISCONNECTED = 3 ;
    public final static int STATE_CONNECTED    = 2 ;
    public final static int STATE_INIT         = 1 ;
    public final static int STATE_UNKNOWN      = 0 ;
    
    public final static int MODE_CLIENT        = 2 ;
    public final static int MODE_SERVER        = 1 ;
    public final static int MODE_UNKNOWN       = 0 ;

    public final static int READ_TIMEOUT       = 10 ;

    private Socket               m_socket = null ;
    private ServerSocket         m_ssocket = null ;
    private int                  m_state = STATE_UNKNOWN ;
    private int                  m_mode = MODE_UNKNOWN ;

    private String               m_host = "localhost" ;
    private int                  m_port = 15000 ;
    private BufferedInputStream  m_recv_stream = null ;
    private BufferedOutputStream m_send_stream = null ;
    private byte                 m_recv_buf [] = null ;
    private byte                 m_send_buf [] = null ;
    private int                  m_buf_offset = 0 ;

    private LinkedList           m_msg_recv = null ;
    private LinkedList           m_msg_tosend = null ;
    private ByteBuffer           m_test = ByteBuffer.allocate(2048) ;

    private C_SynchroSemaphore  m_msg_recv_sem = new C_SynchroSemaphore("Message received semaphore"); // Semaphore
    private C_SynchroSemaphore  m_msg_tosend_sem = new C_SynchroSemaphore("Message sent semaphore");   // Semaphore
    private C_SynchroSemaphore  m_cmd_available = new C_SynchroSemaphore("Synchro command available"); // Synchro

    private boolean m_stop = false ;

    // constructor with host (IP address) and port number
    public C_SynchroThread(String P_host, int P_port, int P_mode) {
	if (P_host != "") {
	    m_host = P_host ;
	}
	m_port = P_port ;
	m_mode = P_mode ;
	m_recv_buf = new byte [4096] ;
	m_send_buf = new byte [4096] ;
	m_msg_recv = new LinkedList() ;
	m_msg_tosend = new LinkedList() ;
	m_cmd_available.P(); // => first usage for wait connection
    }
    
    public C_SynchroCmd wait_command () throws E_SynchroLost {
	C_SynchroCmd L_cmd = null ;

	m_cmd_available.P(); // wait for command received event
	if (m_state != STATE_CONNECTED) { throw new E_SynchroLost(); }

	m_msg_recv_sem.P();
	if (m_msg_recv.size() != 0) {
	    L_cmd = (C_SynchroCmd)m_msg_recv.getFirst() ;
	    //L_cmd = m_msg_recv.getFirst() ;
	    m_msg_recv.remove(L_cmd);
	}
	m_msg_recv_sem.V();

	return (L_cmd);
    }

    public void wait_connection () throws E_SynchroLost {
	System.out.println ("C_SynchroThread::wait_connection()");
	m_cmd_available.P(); // ok, let s wait for the connection
	if (m_state != STATE_CONNECTED) { throw new E_SynchroLost(); }

	// now the connection is established
	
	m_cmd_available.V(); 
	m_cmd_available.P(); // ok, let s be ready for the first command received
	                     // end of init synchro
	System.out.println ("C_SynchroThread::wait_connection() established");
    }


    // send_command method
    public void send_command (C_SynchroCmd P_cmd) {

	m_msg_tosend_sem.P();
	if (P_cmd != null) {
	    m_msg_tosend.add(P_cmd) ;
	}
	m_msg_tosend_sem.V();

    }


    // internal: close all stream/buffer/socket
    private void close_all () {
	
	if (m_recv_stream != null) {
	    try {
		m_recv_stream.close() ;
	    }
	    catch (IOException e) {
		// to be done
		System.out.println ("C_SynchroThread::close_all: " + e.toString());
	    }
	}
	if (m_socket != null) {
	    try {
		m_socket.close() ;
	    }
	    catch (IOException e) {
		System.out.println ("C_SynchroThread::close_all: " + e.toString());
	    }
	}
    }


    // internal wait_process method
    private void wait_process (int P_ms) {
	try {
	    sleep (P_ms) ;
	}
	catch (InterruptedException e) {
	    System.out.println ("C_SynchroThread::wait_process: " + e.toString());
	}
    }

    // internal connect_control method
    private void connect_control () { // connection or accept in blocking mode 

	if (m_state != STATE_CONNECTED) {
		if (m_mode == MODE_CLIENT) {
			try {
				
				m_socket = new Socket(m_host, m_port);
				
				m_socket.setReuseAddress(true);
				m_socket.setSoLinger(false, 0);
				m_socket.setTcpNoDelay(true);
				m_socket.setSoTimeout(READ_TIMEOUT);
				
				m_recv_stream 
					= new BufferedInputStream(m_socket.getInputStream());
				m_send_stream 
					= new BufferedOutputStream(m_socket.getOutputStream());
				
				m_state = STATE_CONNECTED ;
				m_cmd_available.V(); // for wait_connection
			}
			catch (IOException e) {
				System.out.println ("C_SynchroThread::connect_control: " 
						    + e.toString());
				m_cmd_available.V(); // for wait_connection
			}
		} else if (m_mode == MODE_SERVER) {
			try {
				m_ssocket = new ServerSocket(m_port);
				m_ssocket.setReuseAddress(true);

				m_ssocket.setReuseAddress(true);
				//				m_ssocket.setSoLinger(false, 0);
				//				m_ssocket.setTcpNoDelay(true);
				//				m_ssocket.setSoTimeout(1);

				m_socket = m_ssocket.accept();

				m_socket.setReuseAddress(true);
				m_socket.setSoLinger(false, 0);
				m_socket.setTcpNoDelay(true);
				m_socket.setSoTimeout(READ_TIMEOUT);

				System.out.println ("C_SynchroThread::connect_control: accepted connection from "
						    +m_socket.getInetAddress());
				m_state = STATE_CONNECTED ;
			} catch (IOException e) {
				System.out.println ("C_SynchroThread::connect_control: " 
						    + e.toString());
			}
			if (m_state == STATE_CONNECTED) {
			    m_cmd_available.V(); // for wait_connection
			    try {
				m_recv_stream 
				    = new BufferedInputStream(m_socket.getInputStream());
				m_send_stream 
				    = new BufferedOutputStream(m_socket.getOutputStream());
			    } catch (IOException e) {
				System.out.println ("C_SynchroThread: " + e.toString());
			    }
			}


			//			new C_SynchroThread(m_socket);
			//m_ssocket.close();
		}
	}
    }

    // internal decode method: decode PDU received
    private void decode (int P_size) {

	C_SynchroCmd L_recvCmd        = null ;
	int          L_decodedSize    = 0 ; 
	int          L_sizeToDecode   = P_size ;
	int          L_status         = C_SynchroCmd.DECODE_STATUS_OK ;
	int          L_position       = 0 ;
	int          L_i              = 0 ;

	while (L_sizeToDecode != 0) {

	  L_position += L_decodedSize ;
	  L_recvCmd = new C_SynchroCmd();
	  L_decodedSize 
	    = L_recvCmd.decode(m_recv_buf, L_sizeToDecode, L_position, 
			       L_status);
	  
	  if (L_status == C_SynchroCmd.DECODE_STATUS_OK) {
	      m_msg_recv_sem.P();
	      m_msg_recv.add(L_recvCmd);
	      m_msg_recv_sem.V();
	      L_sizeToDecode -= L_decodedSize ;
	      m_cmd_available.V();
	  } else {
	      L_recvCmd = null ;
	      break ;
	  }
	  
	}

	if (L_status == C_SynchroCmd.DECODE_STATUS_SIZE) {
	    System.out.println ("C_SynchroThread::decode segmentation");
	    // remaining size to decode
	    m_buf_offset = L_sizeToDecode ;
	    for(;L_i<m_buf_offset;L_i++) {
		m_recv_buf[L_i] = m_recv_buf[L_position+L_i] ;
	    }
	}
	
    }

    // internal receive_msg_control method
    private void receive_msg_control () throws E_SynchroLost {
	
	int L_size_read = 0 ;
	try {
	    L_size_read = m_recv_stream.read(m_recv_buf, 
					     m_buf_offset, 
					     m_recv_buf.length);
	    
	    if (L_size_read < 0) {
		m_state = STATE_DISCONNECTED ;
		System.out.println ("C_SynchroThread::receive_msg_control connection lost");
		close_all();
		throw new E_SynchroLost() ;
	    } else {
		L_size_read += m_buf_offset ;
		System.out.println ("C_SynchroThread::receive_msg_control recv="
				    + L_size_read);
		m_buf_offset = 0 ;
		decode(L_size_read) ;
	    }
	}
	catch (SocketTimeoutException e) {
	    //	    System.out.println ("C_SynchroThread::receive_msg_control timeout");
	    // no event => nothing received
	}
	catch (E_SynchroLost e) {
	    throw e ;
	}
	catch (Exception e) {
	    System.out.println ("C_SynchroThread::receive_msg_control: " + e.toString());
	}
	
    }

    public void stop_synchro () {
	m_stop = true ;
    }


    // internal run method: main thread loop
    public void run () {
	m_stop = false ;
	connect_control () ;
	while (m_state == STATE_CONNECTED) {
	    //	    System.out.println ("C_SynchroThread::run : main loop");
	    //	    wait_process(1);
	    try {
		receive_msg_control();
	    } 
	    catch (E_SynchroLost e) {
		System.out.println ("C_SynchroThread::run() exception " + e.toString());
		m_cmd_available.V() ; // for wait_command 
		break ;
	    }
	    send_msg_control();
	    if (m_stop == true) break ;
	    yield();
	}
	close_all();
    }


    // internal send_msg_control method
    private void send_msg_control () {

	C_SynchroCmd L_cmd = null ;
	int          L_size       ;

	m_msg_tosend_sem.P();
	if (m_msg_tosend.size() != 0) {
	    L_cmd = (C_SynchroCmd)m_msg_tosend.getFirst() ;
	    //L_cmd = m_msg_tosend.getFirst() ;
	    m_msg_tosend.remove(L_cmd);
	}
	m_msg_tosend_sem.V();

	if (L_cmd != null) {
	    L_size = L_cmd.encode(m_send_buf);
	    try {
		m_send_stream.write(m_send_buf, 0, L_size);
		m_send_stream.flush();
	    }
	    catch (IOException e) {
	    }
	    L_cmd = null ;
	}
    }

}
