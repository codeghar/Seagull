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
// synchro_usage
// C_SynchroThread usage sample

// main entry point class definition

public class synchro_usage {

    public static void main (String[] args) {
	
	int             L_res  = 0 ;
	C_SynchroThread L_sync = new 
	    C_SynchroThread("", 15000, C_SynchroThread.MODE_SERVER) ;
	
	C_SynchroCmd    L_cmd_recv = null ; 
	boolean L_connection_error = false ;
	
	// create a synchro command CMD_CALLCREATE
        // with two data
        // data : tag TAG_FIELD_BIN_1_DATA, size 1 octet, hexadecimal value 0xfe
	// data : tag TAG_FIELD_INT_1_DATA, size 4 octet, init value 6554

	C_SynchroCmd L_cmdToSend 
		    = new C_SynchroCmd(C_SynchroCmd.CMD_INITSYNCHRO, 0, 0);

	System.out.println("=>" + L_cmdToSend.toString()) ;


	// start synchro thread
	// the thread is stopped when the seagull-octcap will be disconnected
	// or a bad format of data

	L_sync.start() ;

	try {
	    L_sync.wait_connection();
	}
	catch (E_SynchroLost e){
	    System.out.println ("Error connection not established");
	    L_connection_error = true ;
	}

	if (L_connection_error == false) {
	    while (L_sync.isAlive()) {
		try {
		    L_cmd_recv = L_sync.wait_command();
		    System.out.println ("cmd " + L_cmd_recv.toString());
		    
		    if (L_cmd_recv.getType() == C_SynchroCmd.CMD_INITSYNCHRO) {
			L_cmdToSend 
			    = new C_SynchroCmd(C_SynchroCmd.CMD_INITSYNCHRO, 
					       L_cmd_recv.getId1(), 
					       L_cmd_recv.getId2());
			L_sync.send_command (L_cmdToSend);
			L_cmdToSend = null ;
		    }

		    if (L_cmd_recv.getType() == C_SynchroCmd.CMD_CALLCREATE) {
			
			L_cmdToSend 
			    = new C_SynchroCmd(C_SynchroCmd.CMD_CALLCREATE, 
					       L_cmd_recv.getId1(), 
					       L_cmd_recv.getId2());

  		        L_res = L_cmdToSend.addDataHexa (C_SynchroCmd.TAG_FIELD_BIN_1_DATA,
							 "fe");

		        if (L_res == 1) {
	                    System.out.println ("Bad format of Data");
		            L_sync.stop_synchro() ;
			    break;
			}

  		        L_res = L_cmdToSend.addDataInt32 (C_SynchroCmd.TAG_FIELD_INT_1_DATA,
							  6554);

			L_sync.send_command (L_cmdToSend);
			L_cmdToSend = null ;
		    }
		    
		    L_cmd_recv = null ; // garbage collector
		}
		catch (E_SynchroLost e) {
		    System.out.println ("server exception: " + e.toString());
		    break;
		}
		if (L_res == 1) break ;
	    }
	}
	System.out.println ("end loop");

    }

}

