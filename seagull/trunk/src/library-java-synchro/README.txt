//
//This program is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//(c)Copyright 2006 Hewlett-Packard Development Company, LP.
//


     Seagull Java Synchronization classes

---------------------------------------

0. Compiling

1. Description 

2. Usage (example)

3. Synchronization protocol with seagull (version 1.2.2)

   See examples in EXAMPLE directory
---------------------------------------

O. Compiling

  a) java version is at least 1.4.2
  b) java is in your user path by typing
     type java
        java is /usr/java/j2sdk1.4.2_02/bin/java
     else find the full_path_to_java and type : 
        export PATH=/full_path_to_java/bin:$PATH

  Also set the CLASSPATH environment variable.
     export CLASSPATH=$CLASSPATH:.



---------------------------------------

1. Description

   Two classes are used for the synchronization
   mechanism with an external tool:

   C_SynchroThread : for the management of the 
                     synchronization 
   C_SynchroCmd    : for the command (sent or received) 
                     for synchronization

   A C_SynchroThread instance must be created, initialized and 
   used for sending or receiving instances of C_SynchroCmd class.

   One thread is created for managing the Sent/Received synchronization
   commands.
    
   The Synchronization command is sent/received with the following
   binary format:

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |              Synchronization command type                     |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |              Synchronization command size                     |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                   User idendifier 1                           |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                   User idendifier 2                           |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    | Data Section (not mandatory) ....                             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   The data section has the following format for each data:

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                          Tag                                  |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                          Data size                            |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    | Data ...                                                      |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

---------------------------------------

2. Usage	
	
   The Usage of these two classes is explained by an example
   (server mode) .
   
   2.1 Initialization
   
       The information needed for the initialization is:
           the hostname (string) or ip address (string) 
           the port number (integer)
           and mode of connection (master).  

       example: synchronization with a tool running on the same host
                with the port 15000

          C_SynchroThread L_sync = new C_SynchroThread("", 15000,
	                              C_SynchroThread.MODE_SERVER) ;

       	Then, the thread for synchronization can be started with the
       command:
         
	  L_sync.start();

 	Finaly, the thread wait a connection with the command:
	  L_sync.wait_connection();
	
   2.2 Synchronization command creation

       For creating a Synchronization command, the following 
       information is required:
         - the type (integer) of the command: the possible values are
             C_SynchroCmd.CMD_INITSYNCHRO    Init command  
	     C_SynchroCmd.CMD_ENDSYNCHRO     End  command
             C_SynchroCmd.CMD_CALLCREATE     Call create command
             C_SynchroCmd.CMD_CALLDATA	     Call data command
             C_SynchroCmd.CMD_CALLRESULT     Call result command
             C_SynchroCmd.CMD_CALLDELETE     Call delete command
               
         - two user identifiers (two integers)
             these two values are used for identifying the call.
             for a response, the same two values must be used

         - the data information (if needed). 

	example: synchronization command CMD_INITSYNCHRO with user 
             identifiers 10 and 0, and with two data

          first data : tag TAG_FIELD_BIN_1_DATA , 
	               size 1 octet, 
	               hexadecimal value 0xfe

          second data: tag TAG_FIELD_INT_1_DATA,
		       size 4 octet, 
		       init value 6554

  	      Note : the predefined values of tags are:

                     TAG_FIELD_BIN_1_DATA = 0
		     TAG_FIELD_BIN_2_DATA = 1 
		     TAG_FIELD_BIN_3_DATA = 2
		     TAG_FIELD_BIN_4_DATA = 3 
		     TAG_FIELD_INT_1_DATA = 4
		     TAG_FIELD_INT_2_DATA = 5
		     TAG_FIELD_INT_3_DATA = 6
		     TAG_FIELD_INT_4_DATA = 7
  
              These values can be extended.


          C_SynchroCmd L_cmdToSend 
                = new C_SynchroCmd(C_SynchroCmd.INITSYNCHRO, 10, 0);


 	  L_cmdToSend.addDataHexa  (C_SynchroCmd.TAG_FIELD_BIN_1_DATA,
						 "fe");
          L_cmdToSend.addDataInt32 (C_SynchroCmd.TAG_FIELD_INT_1_DATA,
	                                          6554);	
			

          a direct binary data value can also be set, for the
          second value, the code is:
 
            byte[] L_buf = new byte[3] ;
            L_buf[0] = 0x00 ;
            L_buf[1] = 0x0a ;
            L_buf[2] = 0x01 ;
            L_cmdToSend.addData(tag_value,L_buf);
   
       
   2.3 Send process

       A synchronization command can be sent by using the following 
       method:
  	  
          L_sync.send_command(L_cmdToSend) ;

   2.4 Receive process

       The reception of command can be done by polling with the 
       following methods:

       while (...) {
            ....

         int     	    L_nb_command_received ;
         C_SynchroCmd       L_cmdReceived ;

	 // get the number of command received       
         L_nb_command_received = L_sync.command_available() ;

         if (L_nb_command_received != 0) {
            // get a command (return null if no command)
	    L_cmdReceived = L_sync.get_command() ;
         }

	 ...

       }

 

   2.5 Received command 
   
       A synchronization command (received) can be manipulated with
       the following methods :
     
         - String toString()               for pretty printing the command 

         - int getType()                   for the type of the command
	 - int getId1()                    for the user identifier 1
         - int getId2()                    for the user identifier 2
	
         - int getNbData()                 for the number of data contained by 
                                           the command

         - byte[] getByteData(int i)       for the value number i of the 
                                           command directly in a byte[]

	 - String getStringHexaData(int i) for the value number i of the 
                                           command in a String with the 
                                           hexadecimal format

	 - String getBCDData(int i)        for the value number i of the 
                                           command in a String with the 
                                           BCD format

	 - int getInt32Data (int i)        for the value number i of the 
                                           command in an int

        example:

         - print out the command received
            System.out.println(L_cmdReceived.toString());
	
         - managing the data values (Hexadecimal string format) of the command:

            int    L_nb_data = L_cmdReceived.getNbData() ;
            String L_data ;

	    for(int L_i = 0; L_i < L_nb_data; L_i++) {
	       L_data = L_cmdReceived.getStringHexaData(L_i) ;
               .....
	    }

	 - making a response without data :
             
	    C_SynchroCmd L_cmdResponse 
                = new C_SynchroCmd(C_SynchroCmd.CMD_CALLRESULT,
	                           L_cmdReceived.getId1(),	
	                           L_cmdReceived.getId2());
	    L_sync.send_command(L_cmdResponse);

   2.5 Synchronization stoped command 

       The thread for synchronization can be stoped with the
       command: 
           L_sync.stop_synchro() 	    
	 
---------------------------------------

3. Synchronization Protocol for seagull tool

   seagull 1.2.2 (in slave mode) is able to communicate with the 
   java synchro thread class (in master mode). 

   To connect a seagull tool to the C_SynchroThread you add  
   in the configuration file of seagull tool this channel

     <define entity="channel"
        name="channel-1"
        protocol="command-synchro-v1"
        transport="trans-ip-v4"
        open-args="mode=client;dest=@IP_destination:port">
     </define>

   example:
    
   when a command of the scenario file (.xml) of seagull is used:

   <send channel="channel-1">
     <command-synchro name="CMD_CALLCREATE">
         <parameter name="field-bin-1-data" value="0x12999223"></parameter>
         <parameter name="field-int-2-data" value="0x129"></parameter>
     </command-synchro>
   </send>
       
   the C_SynchroThread instance will receive a C_SynchroCmd of type
   CMD_CALLCREATE with user id 1 and user id 2 set by seagull tool
   example : 
        <set-value name="user-id-1" format="$(session-counter)"></set-value>

   the data section of the message contain the data extracted from
   TCAP messages (if <restore name="...></restore> action is used by seagull).

   when the following command of the scenario file is used: 

   <receive channel="channel-1">
     <command-synchro name="CMD_CALLRESULT">
     </command-synchro>
   </receive>

   seagull is waiting for a CALLRESULT synchro command sent by 
   the C_SynchroThread instance.

   Note:  seagull and C_SynchroThread Java use the same masseges type.


----------------------------------------



