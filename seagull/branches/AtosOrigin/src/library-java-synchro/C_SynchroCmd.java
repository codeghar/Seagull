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
// C_SynchroCmd class definition
//

import java.util.* ; // for LinkedList
import java.nio.* ;  // for ByteBuffer
import java.lang.Integer ;

public class C_SynchroCmd {

    public final static int CMD_INITSYNCHRO    = 0 ;
    public final static int CMD_ENDSYNCHRO     = 1 ;
    public final static int CMD_CALLCREATE     = 2 ;
    public final static int CMD_CALLDATA       = 3 ;
    public final static int CMD_CALLRESULT     = 4 ;
    public final static int CMD_CALLDELETE     = 5 ;
    public final static int CMD_UNKNOWN        = 6 ;

    public final static String[] CMD_NAME_TABLE = {
	"CMD_INITSYNCHRO",
	"CMD_ENDSYNCHRO",
	"CMD_CALLCREATE",
	"CMD_CALLDATA",
	"CMD_CALLRESULT",
	"CMD_CALLDELETE",
	"CMD_UNKNOWN"  } ;

    public final static int TAG_FIELD_BIN_1_DATA = 0 ;
    public final static int TAG_FIELD_BIN_2_DATA = 1 ;
    public final static int TAG_FIELD_BIN_3_DATA = 2 ;
    public final static int TAG_FIELD_BIN_4_DATA = 3 ;
    public final static int TAG_FIELD_INT_1_DATA = 4 ;
    public final static int TAG_FIELD_INT_2_DATA = 5 ;
    public final static int TAG_FIELD_INT_3_DATA = 6 ;
    public final static int TAG_FIELD_INT_4_DATA = 7 ;

    public final static int DECODE_STATUS_OK   = 0 ;
    public final static int DECODE_STATUS_SIZE = 1 ; // temporary KO
    public final static int DECODE_STATUS_KO   = 2 ;
    
    private int m_type            = CMD_UNKNOWN ;
    private int m_user_id_1       = 8 ;
    private int m_user_id_2       = 15 ;
    private LinkedList m_values   = null ;
    private LinkedList m_tags     = null ;

    private final static int HEADER_SIZE       = 16 ;
    private final static int HEADER_FIELD_SIZE = 8  ; // before 4

    // constructor with no paramater
    public C_SynchroCmd () {
	m_type      = CMD_UNKNOWN;
	m_user_id_1 = 0;
	m_user_id_2 = 0;
	m_values = new LinkedList () ;
	m_tags = new LinkedList () ;
    }

    // constructor with cmd type code, user id 1 and user id 2
    public C_SynchroCmd (int P_code, int P_id1, int P_id2) {
	m_type      = P_code;
	m_user_id_1 = P_id1 ;
	m_user_id_2 = P_id2 ;
	m_values = new LinkedList () ;
	m_tags = new LinkedList () ;
    }

    // convertion to a string
    public String toString() {

	int          L_type = m_type ;
	StringBuffer L_buf  = new StringBuffer("C_SyncroCmd") ;
	int          L_int_value  = 0   ;
	String       L_data_value = null ;
	
	if (L_type > CMD_UNKNOWN) { L_type = CMD_UNKNOWN; }
	L_buf.append(" ["+CMD_NAME_TABLE[m_type]+"]");
	L_buf.append(" Id1 ["+Integer.toString(m_user_id_1)+"]");
	L_buf.append(" Id2 ["+Integer.toString(m_user_id_2)+"]");
	L_buf.append(" values ["+Integer.toString(m_values.size())+"]");
	if (m_values.size() != 0) {
	    ListIterator L_it = m_values.listIterator() ;     
	    ListIterator L_it_tag = m_tags.listIterator() ;
	    while (L_it.hasNext()) {
		L_it_tag.hasNext();
		byte[] L_value = (byte[])L_it.next() ;
		Integer L_tag   = (Integer)L_it_tag.next() ;
		int    L_i ;
		L_buf.append(" tag ["+L_tag+"]");
		L_buf.append(" value [");
		for(L_i=0; L_i < L_value.length; L_i++) {
		    L_int_value = L_value[L_i] ;
		    if (L_int_value < 0) { L_int_value += 256; } 
		    L_data_value = Integer.toString(L_int_value,16);
		    if (L_data_value.length() == 1) {
			L_buf.append("0");
		    }
		    L_buf.append(L_data_value);
		}
		L_buf.append("]");
	    }

	}
	
	return (L_buf.toString());
    }

    // add a data on the data part from an int representation
    public int addDataInt32(int P_tag, int P_value) {
	int    L_ret = 0 ;
	byte[] L_byte = new byte[4] ;
	ByteBuffer L_buf = ByteBuffer.wrap(L_byte);
	L_buf.putInt(P_value);


	m_tags.add(new Integer(P_tag));
	m_values.add(L_byte);

	return (L_ret);
    }

    // add a data on the data part from a byte[] representation
    public int addData (int P_tag, byte[] P_buf) {
	int    L_ret = 0 ;
	byte[] L_byte = P_buf ;
	
	m_tags.add(new Integer(P_tag));
	m_values.add(L_byte);

	return (L_ret);
    }

    // add a data to the data part from a hexadecimal string buffer
    // representation
    public int addDataHexa (int P_tag, String P_buf) {

	int L_ret = 0 ;
	int L_i   = 0 ;
	int L_j   = 0 ;
	int L_length = P_buf.length() ;
	int L_byte_length = L_length / 2 ;

	if ((L_length % 2) == 0) {
	    
	    byte[] L_bbuf = new byte [L_byte_length] ;
	    L_j = 0 ;

	    for(L_i=0; L_i < L_byte_length; L_i++) {
		String L_value = P_buf.substring(L_j, L_j+2);
		L_bbuf[L_i] = (byte) Integer.parseInt(L_value, 16);
		L_j += 2 ;
	    }
	    m_values.add(L_bbuf);
	    m_tags.add(new Integer(P_tag));

	} else {
	    L_ret = 1 ;
	}

	return(L_ret);
	
    }

    // internal for encoding 
    // the synchro command to a PDU
    public int encode (byte[] P_buf) {

	ByteBuffer   L_buf      = ByteBuffer.wrap(P_buf);
	int          L_size     = 0 ;
	int          L_size_pos     ;
	ListIterator L_it       = m_values.listIterator() ;     
	ListIterator L_it_tag   = m_tags.listIterator() ;     

	L_buf.rewind() ;
	L_buf.putInt(m_type) ;
	L_size_pos = L_buf.position() ;
	L_buf.putInt(L_size) ;
	L_buf.putInt(m_user_id_1) ;
	L_buf.putInt(m_user_id_2) ;

	while (L_it.hasNext()) {
	    L_it_tag.hasNext();
	    byte[] L_value = (byte[]) L_it.next() ;
	    Integer L_tag   = (Integer)    L_it_tag.next();
	    L_buf.putInt(L_tag.intValue());
	    L_buf.putInt(L_value.length+HEADER_FIELD_SIZE);
	    L_buf.put(L_value);
	}

	L_size = L_buf.position() ;
	L_buf.putInt(L_size_pos, L_size);

	return (L_size);
    }

    // utils
    // convert a byte[] buffer to a string hexadecimal
    public String byteToString(byte[] P_buf, int P_size) {

	int L_i ;
	StringBuffer L_buf  = new StringBuffer("[") ;
	String       L_value = null ;
	int          L_int_value ;

	for(L_i=0; L_i < P_size; L_i++) {
	    L_int_value = P_buf[L_i] ;
	    if (L_int_value < 0) { L_int_value += 256; }
	    L_value = Integer.toString(L_int_value,16);
	    if (L_value.length() == 1) {
		L_buf.append("0");
	    }
	    L_buf.append(L_value);
	}
	L_buf.append("]");
	return(L_buf.toString());
    }

    // internal for decoding the synchro cmd PDU
    public int decode (byte[] P_buf, 
		       int P_size, int P_position, 
		       int P_status) {

	ByteBuffer   L_buf      = ByteBuffer.wrap(P_buf)  ;


	int          L_size     = 0                       ;
	int          L_size_pos                           ;
	ListIterator L_it       = m_values.listIterator() ;
	ListIterator L_it_tag   = m_tags.listIterator() ;
	int          L_decoded_size = 0 ;
	
	int          L_data_size = 0 ;
	byte[]       L_data_value = null ;
	int          L_i = 0 ;


        int      L_decodedSize = 0      ;
        int      L_totalSizeToDecode ;
        int      L_bodySizeToDecode  ;
        int      L_remainingBufferSize = P_size ;
	Integer  L_tag ;

	L_buf.position(P_position);
	if (L_remainingBufferSize >= HEADER_SIZE) {

	  m_type = L_buf.getInt() ;
	  L_totalSizeToDecode = L_buf.getInt() ;
	  m_user_id_1 = L_buf.getInt() ;
	  m_user_id_2 = L_buf.getInt() ;
	  
	  if (L_totalSizeToDecode >= HEADER_SIZE) {

	    L_decodedSize = HEADER_SIZE ;

	    if (L_remainingBufferSize >= L_totalSizeToDecode) {

	      L_remainingBufferSize -= HEADER_SIZE ;
	      L_bodySizeToDecode = L_totalSizeToDecode - HEADER_SIZE ;
	
	      if (L_bodySizeToDecode > 0) {

		P_status = DECODE_STATUS_OK ;
		L_decodedSize = L_totalSizeToDecode ;

		while (L_bodySizeToDecode != 0) {
		  if (L_bodySizeToDecode >= HEADER_FIELD_SIZE) {
		    L_tag = new Integer(L_buf.getInt());
		    L_data_size = L_buf.getInt();
		    L_data_size -= HEADER_FIELD_SIZE ;
		    L_bodySizeToDecode -= HEADER_FIELD_SIZE ;
		    if (L_bodySizeToDecode >= L_data_size) {
		      L_data_value = new byte[L_data_size] ;
		      for(L_i=0; L_i<L_data_size; L_i++) {
			L_data_value[L_i] = L_buf.get();
		      }
		      m_values.add (L_data_value);
		      m_tags.add(L_tag);
		      L_bodySizeToDecode -= L_data_size ;
		    } else {
		      P_status = DECODE_STATUS_KO ;
		      break ;
		    }
		  } else {
		    P_status = DECODE_STATUS_KO ;
		    break ;
		  }
		} // while
		
	      } else { // decode with no body
		
		P_status = DECODE_STATUS_OK ;
		
	      }

	    } else {
	      
	      P_status = DECODE_STATUS_SIZE ;

	    }

	  } else {
	    P_status = DECODE_STATUS_KO ;
	  }

	} else {
	    P_status = DECODE_STATUS_SIZE ;
	}

	return (L_decodedSize);
    }


    // set/get field methods

    public int getId1() {
	return (m_user_id_1);
    }

    public int getId2() {
	return (m_user_id_2);
    }

    public int getType () {
	return (m_type) ;
    }

    // there is no setNBData: the number of add is updated
    // by the addData method
    public int getNbData () {
	return (m_values.size());
    }

    public byte[] getByteData (int P_n) {
	byte[] L_ret = null ;
	if (P_n < m_values.size()) {
	    L_ret = (byte[]) m_values.get(P_n) ;
	}
	return (L_ret);
    }

    public int getInt32Data (int P_n) {
	byte[] L_bin = getByteData(P_n) ;
	ByteBuffer   L_buf      = ByteBuffer.wrap(L_bin)  ;
	return(L_buf.getInt());
    }

    public String getStringHexaData (int P_n) {
	String L_ret = null ;
	byte[] L_bin = getByteData(P_n) ;
	int    L_i   = 0 ;
	String L_value = null ;
	int    L_int_value = 0 ;

	if (L_bin != null) {
	    StringBuffer L_buf = new StringBuffer("");
	    for(L_i = 0; L_i < L_bin.length; L_i++) {
		L_int_value = L_bin[L_i] ;
		if (L_int_value < 0) { L_int_value += 256; }
		L_value = Integer.toString(L_int_value, 16);
		if (L_value.length() == 1) {
		    L_buf.append("0");
		}
		L_buf.append(L_value);
	    }
	    L_ret=L_buf.toString();
	}
	return (L_ret);
    }

    public String getBCDData (int P_n) {
	String L_orig      = getStringHexaData(P_n);
	int    L_i         = 0;
	String L_ret       = null;
	StringBuffer L_buf = new StringBuffer("");
	
	for (L_i = 0; L_i < L_orig.length(); L_i+=2) {
	    L_buf.append(L_orig.charAt(L_i+1));
	    L_buf.append(L_orig.charAt(L_i));
	}
	L_ret = L_buf.toString();
	return (L_ret);
    }
    

}










