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
// C_Semaphore class definition
//
// Generic semaphore implementation
// for data Synchronization mecanisms
//
// wait() and notify() definitions

public class C_SynchroSemaphore {

    private String m_name ;
    private int    m_counter ;

    public final static int COUNTER_INIT_VALUE    = 1 ;
    public final static int COUNTER_COMPARE_VALUE = 0 ;

    public C_SynchroSemaphore(String P_name) {
	m_counter = COUNTER_INIT_VALUE ;
	m_name = P_name ;
	System.out.println(Thread.currentThread().getName()
			   +":create semaphore("+m_name+")");
    }

    public C_SynchroSemaphore(int P_init, String P_name) {
	m_counter = P_init ;
	m_name = P_name ;
	System.out.println(Thread.currentThread().getName()
			   +":create semaphore("+m_name+")");    }

    public String getName() {
	return (m_name);
    }

    public synchronized int P() {
	while (m_counter < COUNTER_INIT_VALUE) {
	    try {
		wait();
	    } 
	    catch (InterruptedException e) {}
	}
        m_counter -- ;
	//	System.out.println(Thread.currentThread().getName()
	//			   +":P("+m_name+")="+m_counter);
	return (m_counter);
    }
    
    public synchronized int V() {
	m_counter ++ ;
	notify() ;
	//	System.out.println(Thread.currentThread().getName()
	//			   +":V("+m_name+")="+m_counter);
	return (m_counter);
    }


}


