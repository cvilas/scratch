Spin Buffers
by Prashanth Hirematada

Listing One

public class RingBuffer {
    private final static int SIZE = 3000000;
    private Object m_bfr[] = new Object[SIZE];
    private int m_rPtr=0;
    private int m_wPtr=0;
    private int m_size=0;

    /** Creates a new instance of RingBuffer */
    public RingBuffer() {
    }
    private synchronized void updateSize(int by) {
        m_size += by;
    }
    public boolean put(Object value) {
        if ( m_size == SIZE )
            return false; // buffer is full
        m_bfr[m_wPtr] = value;
        m_wPtr = (m_wPtr+1)%SIZE;
        updateSize(1);
        return true;
    }
    public Object get() {
        if ( m_size == 0 )
            return null; // buffer is empty
        Object ret = m_bfr[m_rPtr];
        m_bfr[m_rPtr] = null;
        m_rPtr = (m_rPtr+1)%SIZE;
        updateSize(-1);
        return ret;
   }
}



Listing Two

public class SpinBuffer {
    private static final int MAX_SIZE = 1000000;
    private Object[][] m_bfr = new Object[3][MAX_SIZE];

    private boolean[] m_busy = new boolean[3];
    private int[] m_count = new int[3];
    private int[] m_ptr = new int[3];

    private int m_pBuf = 0;
    private int m_cBuf = 1;

    /** Creates a new instance of SpinBuffer */
    public SpinBuffer() {
        m_busy[0] = m_busy[1] = true;

        m_busy[2] = false;
        for ( int i=0; i<3; i++ ) {
            m_ptr[i] = m_count[i] = 0;
        }
    }
    public boolean put(Object o) {
        int next = (m_pBuf+1)%3;

        if ( m_ptr[m_pBuf] < MAX_SIZE ) {
            // add to the buffer
            m_bfr[m_pBuf][m_ptr[m_pBuf]] = o;
            m_ptr[m_pBuf]++;
        }
        else
            return false;
        // check if next buffer is free
        if ( !m_busy[next] ) {
            m_count[m_pBuf] = m_ptr[m_pBuf];
            m_ptr[m_pBuf] = 0;
            m_busy[next] = true; // acquire
            m_busy[m_pBuf] = false; // release
            m_pBuf = next;
        }
        return true;
    }
    public Object get() {
        Object o = null;

        if ( m_ptr[m_cBuf] < m_count[m_cBuf]) {
            o = m_bfr[m_cBuf][m_ptr[m_cBuf]];
            // remove the reference
            m_bfr[m_cBuf][m_ptr[m_cBuf]] = null;
            m_ptr[m_cBuf]++;
        }
        else {
            // check if next buffer is free
            int next = (m_cBuf+1)%3;
            if ( !m_busy[next] ) {
                m_busy[next] = true; // acquire
                m_ptr[m_cBuf] = 0;
                m_count[m_cBuf] = 0;
                m_busy[m_cBuf] = false; // release
                m_cBuf = next;
            }
            //else, waiting for consumer
        }
        return o;
    }
}


2
