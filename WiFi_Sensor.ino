#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


//library bmp180

const char* ssid = "****";
const char* password = "****";

struct sensor_inputs{
float pressure;
float temp_p;
float humidity;
float temp_h;
uint16_t light;
} mySensor;

void mySensor_Refresh()
{
  mySensor.pressure = readBMP_pressure();
  mySensor.temp_p=readBMP_temp();
  mySensor.humidity =readDHT_humidity();
  mySensor.temp_h = readDHT_temp();
  mySensor.light = readAPDS_ambient();
}
 
//int ledPin = 13; // GPIO13
WiFiServer server(80);
 
void setup() {
  Serial.begin(115200);
  delay(50);
  
  setupMQTT();
  
  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid); 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {    delay(500);    Serial.print(".");  }
  Serial.println("WiFi connected");

  connectMQTT();
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  
  delay(500);

  
  setupAPDS();
  setupBMP();
  setupDHT();

  //setupAlarms();

}


 
int loop_counter = 0;

void loop() {
  if (loop_counter > 300){
    alarmedPublishMQTT();
    loop_counter = 0;
    }
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    delay(100);
    loop_counter++;
    return;
  }
  loop_counter = 0;
 
  // Wait until the client sends some data
  Serial.println("New Web Client");
  while(!client.available()){
    delay(10);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Match the request
 /*
  int value = LOW;
  if (request.indexOf("/LED=ON") != -1)  {
    //digitalWrite(ledPin, HIGH);
    value = HIGH;
  }
  if (request.indexOf("/LED=OFF") != -1)  {
    //digitalWrite(ledPin, LOW);
    value = LOW;
  }
  */
  mySensor_Refresh();
  //publishMQTT(mySensor);

 
  // Match the request
 
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head><title>Dimitrijs IoT Sensor V0.1</title>");
  client.println("<meta http-equiv=\"refresh\" content=\"60\"></head>");
 
  client.println("<br>");

  client.println("<table BORDER=10 BORDERCOLOR=BLACK><caption>Dimitrijs IoT Sensor V0.1</caption><tr><th>Sensor</th><th>Value</th></tr>");
  client.println("<tr><td>relative Humidity:<br>DHT22</td><td>");
  client.println(mySensor.humidity);client.println("%</td></tr>");

  client.println("<tr><td>Temperature:<br>DHT22</td><td>");
  client.println(mySensor.temp_h);client.println(" degC</td></tr>");
  
  client.println("<tr><td>Atmospheric Pressure:<br>BMP180</td><td>");
  client.println(mySensor.pressure);client.println(" hPa</td></tr>");

  client.println("<tr><td>Temperature:<br>BMP180</td><td>");
  client.println(mySensor.temp_p);client.println(" degC</td></tr>");

    client.println("<tr><td>Ambient Light:<br>APDS-9960</td><td>");
  client.println(mySensor.light);client.println("<br>(0-1024)</td></tr>");
  
  client.println("</table>"),
  client.println("<br><br>");
  client.println("<a href=\"/REF=ON\"\"><button>Refresh Once</button></a>");
  //client.println("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />");  
  client.println("</body></html>");
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
  publishMQTT(mySensor);
  //readAPDS();
}
 
