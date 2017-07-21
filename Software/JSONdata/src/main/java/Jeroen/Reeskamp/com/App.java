package Jeroen.Reeskamp.com;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import javax.net.ssl.*;
import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.X509Certificate;
import java.util.Vector;

public class App {

    private static Vector<JsonObject> jsonObjects;
    private static Vector<Coordinate> coordinates;

    private static ButtonLayout buttonLayout;

    public static void main( String[] args ) throws IOException {
        buttonLayout = new ButtonLayout();
        disableSslVerification();

        jsonObjects = new Vector<>();
            update();
    }

    public static Vector<String> readFromUrl(String u, String method) {
        Vector<String> strings = new Vector<>();
        try {
            URL url = new URL("https://192.168.179.132:443/api/" + u);
            HttpURLConnection httpURLConnection = (HttpURLConnection)url.openConnection();
            httpURLConnection.setRequestMethod(method);
            if (method.equals("POST")) {
                for(int i = 0; i < coordinates.size(); i++) {
                    httpURLConnection.setRequestProperty("coor",Integer.toString(i) + ": " +
                                                            Integer.toString(coordinates.elementAt(i).getX()) +
                                                            "," + Integer.toString(coordinates.elementAt(i).getY()) +
                                                            "," + Integer.toString(coordinates.elementAt(i).getZ()));
                }
                httpURLConnection.setDoInput(true);
                httpURLConnection.setDoOutput(true);
            }

            httpURLConnection.connect();
            BufferedReader in = new BufferedReader(new InputStreamReader((httpURLConnection.getInputStream())));

            String inputLine;
            while ((inputLine = in.readLine()) != null) {
                strings.add(inputLine);
            }
            in.close();
            return  strings;
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static void extractJson(String url) {
        Vector<String> json = readFromUrl(url, "GET");
        JsonParser parser = new JsonParser();
        for(int i = 0; i < json.size(); i++) {
            jsonObjects.add(parser.parse(json.elementAt(i)).getAsJsonObject());
        }
    }

    public static void setPath() {
        coordinates = new Vector<>();
        for (int i = 0; i < 5; i++) {
            Coordinate coor = new Coordinate();
            coor.setX(i);
            coor.setY(i);
            coor.setZ(i);
            coordinates.add(coor);
        }


        Vector<String> response = readFromUrl("setpath", "POST");
        System.out.println(response);

    }

    public static void update() {
        new Thread(() -> {
            while(true) {
                setPath();

                extractJson("position");
                JsonObject latest = jsonObjects.elementAt(jsonObjects.size() - 1).get("position").getAsJsonObject();

                int[] coordinates = {   Integer.parseInt(latest.get("Xaxis").toString()) * (DrawPanel.X / 18),
                                        Integer.parseInt(latest.get("Yaxis").toString()) * (DrawPanel.Y / 11),
                                        Integer.parseInt(latest.get("Zaxis").toString())
                                    };

                buttonLayout.setLabelX(Integer.toString(coordinates[0]));
                buttonLayout.setLabelY(Integer.toString(coordinates[1]));
                buttonLayout.setLabelZ(Integer.toString(coordinates[2]));
                buttonLayout.updateDrawPanel(coordinates);

                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

            }
        }).start();
    }

    private static void disableSslVerification() {
        try
        {
            // Create a trust manager that does not validate certificate chains
            TrustManager[] trustAllCerts = new TrustManager[] {new X509TrustManager() {
                public java.security.cert.X509Certificate[] getAcceptedIssuers() {
                    return null;
                }
                public void checkClientTrusted(X509Certificate[] certs, String authType) {
                }
                public void checkServerTrusted(X509Certificate[] certs, String authType) {
                }
            }
            };

            // Install the all-trusting trust manager
            SSLContext sc = SSLContext.getInstance("SSL");
            sc.init(null, trustAllCerts, new java.security.SecureRandom());
            HttpsURLConnection.setDefaultSSLSocketFactory(sc.getSocketFactory());

            // Create all-trusting host name verifier
            HostnameVerifier allHostsValid = new HostnameVerifier() {
                public boolean verify(String hostname, SSLSession session) {
                    return true;
                }
            };

            // Install the all-trusting host verifier
            HttpsURLConnection.setDefaultHostnameVerifier(allHostsValid);
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        } catch (KeyManagementException e) {
            e.printStackTrace();
        }
    }


}
