
public static void main(String[] args) {
        SpinBuffer bfr = new SpinBuffer();

        Thread t = new Thread(new BenzTest(true, bfr));
        t.start();Š
        t = new Thread(new BenzTest(false, bfr));
        t.start();
   }
   
   
class SpinTest implements Runnable {

    private static boolean s_done = false;

    private boolean m_isProd;
    private SpinBuffer m_bfr;

    public SpinTest(boolean isProd, SpinBuffer bfr) {
        m_isProd = isProd;
        m_bfr = bfr;
    }

    public void run() {
        int pass = 3000000;
        long start=0;
        Object o;
        int counter = 0;
        if ( !m_isProd )
            start = System.currentTimeMillis();
        o = new Integer(counter);
        while(!s_done) {
            if ( m_isProd ) {
                m_bfr.put(o);
                counter++;
                if ( counter == pass )
                    System.out.println("P DONE Count: " + counter);
            }
            else {
                o = m_bfr.get();
                if ( o != null ) {
                    counter++;
                }
                if ( counter == pass) {
                    long now = System.currentTimeMillis();
                    System.out.println("Count: " + counter +
                                       " in " + (now - start));
                    s_done = true;
                }
            }
        }
    }

}
