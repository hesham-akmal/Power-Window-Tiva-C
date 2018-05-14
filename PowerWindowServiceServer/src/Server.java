import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;

public class Server {
    private static final int PORT_NUMBER = 3000;
    private final Socket socket;
    public static ObjectInputStream ois;
    public static ObjectOutputStream oos;

    private Server(Socket socket) {
        this.socket = socket;
        System.out.println("New client connected from " + socket.getInetAddress().getHostAddress());
        run();
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
                        switch (c) {
                            case 'z':
                                TwoWaySerialComm.out.write(c);
                                TwoWaySerialComm.out.flush();
                                break;
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }).start();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        ServerSocket server = null;

        try {

            new TwoWaySerialComm().connect("COM3", 115200); //connect to com port

            server = new ServerSocket(PORT_NUMBER);
            new Server(server.accept());

        } catch (Exception var10) {
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