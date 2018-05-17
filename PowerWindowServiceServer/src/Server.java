import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;

public class Server extends Thread {
    private static final int PORT_NUMBER = 10001;
    private final Socket socket;
    public static ObjectInputStream ois;
    public static ObjectOutputStream oos;

    private Server(Socket socket) {
        this.socket = socket;
        System.out.println("New client connected from " + socket.getInetAddress().getHostAddress());
    }

    public void run() {

        try {
            //Creating ObjectOutputStream before ObjectInputStream IS A MUST to avoid blocking
            oos = new ObjectOutputStream(this.socket.getOutputStream());
            ois = new ObjectInputStream(this.socket.getInputStream());

            new Thread(() ->
            {
                while (true) {
                    //Read from android
                    try {
                        char c = (char) ois.readObject();
                        System.out.println("Command read: " + c);
                        TwoWaySerialComm.out.write(c);
                        TwoWaySerialComm.out.flush();
                    } catch (Exception e) {
                        System.out.println("APP DISCONNECT, CLOSING THREAD");
                        try {
                            ois.close();
                            oos.close();
                        } catch (IOException e1) {
                            e1.printStackTrace();
                        }

                        break;
                    }
                }
            }).start();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {

        String COMNAME = "COM3";
        int Buad = 115200;

        ServerSocket server = null;

        System.out.println("Connecting to TIVA on " + COMNAME
        + " with Baud Rate: " + Buad);

        while (true) {
            try {
                new TwoWaySerialComm().connect(COMNAME, Buad); //connect to com port
                break;
            } catch (Exception ex) {
            }
        }

        try {
            server = new ServerSocket(PORT_NUMBER);

            while (true)
                new Server(server.accept()).start();

        } catch (Exception x) {
            System.out.println("Unable to start server.");
        } finally {
            try {
                if (server != null) {
                    server.close();
                }
            } catch (IOException var9) {
                var9.printStackTrace();
            }
        }

    }
}
