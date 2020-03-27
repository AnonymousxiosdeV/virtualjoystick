#include <ESP8266WebServer.h>
#include <FS.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

//Set Wifi ssid and password
char ssid[] = "Nacho WiFi";
char pass[] = "turborx7";

Servo servoX,servoY

ESP8266WebServer server (80);

//This function takes the parameters passed in the URL(the x and y coordinates of the joystick)
//and sets the motor speed based on those parameters. 
void handleJSData(){
  int x = server.arg(0).toInt() * 2;
  int y = server.arg(1).toInt() * 2;
  int pitch = abs(y);
  int yaw = abs(x);

  int aPitch = map(pitch,-100,100, 0, 180);
  int bYaw = map(yaw,-100,100, 0, 180);



  //return an HTTP 200
  server.send(200, "text/plain", "");   
}

void setup()
{
delay(100);
Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //set the pins as outputs
servoX.attach(D7);
servoY.attach(D3);
  // Debug console
  
  //initialize SPIFFS to be able to serve up the static HTML files. 
  if (!SPIFFS.begin()){
    Serial.println("SPIFFS Mount failed");
  } 
  else {
    Serial.println("SPIFFS Mount succesfull");
  }
  //set the static pages on SPIFFS for the html and js
  server.serveStatic("/", SPIFFS, "/joystick.html"); 
  server.serveStatic("/virtualjoystick.js", SPIFFS, "/virtualjoystick.js");
  //call handleJSData function when this URL is accessed by the js in the html file
  server.on("/jsData.html", handleJSData);  
  server.begin();
    if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
}

void loop()
{
  server.handleClient();
  if (Client.connected)
  servoX.write(aPitch);
  servoY.write(aYaw);
}