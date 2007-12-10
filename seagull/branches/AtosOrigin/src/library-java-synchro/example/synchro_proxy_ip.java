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
// synchro_proxy_ip
// C_SynchroThread usage sample

// main entry point class definition
// two thread (slave mode) connected to two seagulls (master mode)
// the synchro messages received from the first seagull (first
// thread) are sended by the second second thread to the other seagull



public class synchro_proxy_ip {


    public static void main (String[] args) {
	
	// first synchro thread, connected to seagull (master mode, 
        // master port=15000)
     
	C_SynchroThread L_sync = new C_SynchroThread("", 15000) ;
	C_SynchroCmd    L_cmd_recv = null ;
	C_SynchroCmd    L_cmd_resp = null ;
	int             L_nb_cmd = 0 ;

	// second synchro thread, connected to seagull (master mode, 
        // master port=15001)
	C_SynchroThread L_sync2 = new C_SynchroThread("", 15001) ;

	// start synchro thread 1 loop
	L_sync.start() ;

	try { Thread.sleep(100); }
	catch (InterruptedException e) { System.out.println (e.toString()); }

	// start synchro thread 2 loop
	L_sync2.start() ;

	// main loop
	while (true) {

	    try { Thread.sleep(1) ; } catch (InterruptedException e) {
		System.out.println (e.toString());
	    }

	    // test number of cmd messages received by thread 1
	    L_nb_cmd = L_sync.command_available() ;
	    while (L_nb_cmd > 0) {
		L_cmd_recv = L_sync.get_command() ;
		System.out.println("CMD RECV: " + L_cmd_recv.toString());

		// use thread 2 to send the cmd message
		L_sync2.send_command(L_cmd_recv);
		System.out.println("CMD SENT: " + L_cmd_recv.toString());
		L_cmd_recv = null ;

		L_nb_cmd -- ;
	    }

	}


    }

}

