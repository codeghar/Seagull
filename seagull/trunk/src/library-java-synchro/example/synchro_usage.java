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
	C_SynchroThread L_sync = new C_SynchroThread("", 15000) ;


	// create a synchro command CMD_CALLCREATE
        // with two data
	// data 1: size 1, hexa value 0xfe
        // data 2: size 3, hexa value 0x000a01

	C_SynchroCmd L_cmdToSend 
		    = new C_SynchroCmd(C_SynchroCmd.CMD_CALLCREATE, 10, 0);

	L_res = L_cmdToSend.addDataHexa ("fe");
	L_res = L_cmdToSend.addDataHexa ("000a01");

	System.out.println("=>" + L_cmdToSend.toString()) ;


	// start synchro thread
	// the thread is stopped when seagull will be disconnected
	L_sync.start() ;
	
	L_sync.send_command(L_cmdToSend);

    }

}

