import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;
import java.io.InputStream;
import java.io.OutputStream;

public class TwoWaySerialComm {

    public static OutputStream out;

    public TwoWaySerialComm() {
        super();
    }

    void connect(String portName, int Speed) throws Exception {
        CommPortIdentifier portIdentifier = CommPortIdentifier.getPortIdentifier(portName);
        if (portIdentifier.isCurrentlyOwned()) {
            System.out.println("Error: Port is currently in use");
        } else {
            CommPort commPort = portIdentifier.open(this.getClass().getName(), 1000);

            if (commPort instanceof SerialPort) {
                SerialPort serialPort = (SerialPort) commPort;
                serialPort.setSerialPortParams(Speed, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

                InputStream in = serialPort.getInputStream();
                out = serialPort.getOutputStream();

                new Thread(new SerialReader(in)).start();

            } else {
                System.out.println("Error: Only serial ports are handled by this example.");
            }
        }
    }

    /** */
    public static class SerialReader implements Runnable {
        InputStream in;

        public SerialReader(InputStream in) {
            System.out.println("Listening to tiva..");
            this.in = in;
        }

        public void run() {
            String completeString = "";
            byte[] buffer = new byte[1024];
            int len = -1;
            try {
                while ((len = this.in.read(buffer)) > -1) {
                    //Thread.sleep(100);
                    String stringRead = new String(buffer, 0, len);

                    if (stringRead.equalsIgnoreCase(""))
                        continue;

                    completeString += stringRead;

                    for (int i = 0; i < stringRead.length(); i++) {

                        char c = stringRead.charAt(i);
                        if (c == '\n') {
                            System.out.println(completeString);
                            try {
                                if (Server.oos != null) {
                                    Server.oos.writeObject(completeString);
                                    Server.oos.flush();
                                }
                                completeString = "";
                            } catch (Exception ex) {
                            }
                        }
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

}