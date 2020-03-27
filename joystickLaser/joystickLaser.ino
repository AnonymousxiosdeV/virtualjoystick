#include <ESP8266WebServer.h>
#include <FS.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

//Set Wifi ssid and password
char ssid[] = "Nacho WiFi";
char pass[] = "turborx7";

int aPitch = 90;
int bYaw = 90;

#define PIN_LED 2
#define LASER D6

Servo servoX, servoY;

ESP8266WebServer server(80);

void blink_led(int interval, int count) {
  for (int i = 0; i < count; i++) {
    builtinledSetStatus(true);
    delay(interval);
    builtinledSetStatus(false);
    delay(interval);
  }
}

//This function takes the parameters passed in the URL(the x and y coordinates of the joystick)
//and sets the motor speed based on those parameters.
void handleJSData() {
  int yaw = server.arg(0).toInt();
  int pitch = server.arg(1).toInt();

  int changePitch = map(pitch, -100, 100, 0, 180);
  int changeYaw = map(yaw, -100, 100, 0, 180);

  aPitch = changePitch;
  bYaw = changeYaw;

  //return an HTTP 200
  server.send(200, "text/plain", "");
}

void setup()
{
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  pinMode(LASER, OUTPUT);
  digitalWrite(LASER, LOW);

  Serial.setRxBufferSize(32);
  Serial.setDebugOutput(false);


  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.disconnect(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    blink_led(200, 1);
  }
  printf("\n");
  printf("SketchSize: %d B\n", ESP.getSketchSize());
  printf("FreeSketchSpace: %d B\n", ESP.getFreeSketchSpace());
  printf("FlashChipSize: %d B\n", ESP.getFlashChipSize());
  printf("FlashChipRealSize: %d B\n", ESP.getFlashChipRealSize());
  printf("FlashChipSpeed: %d\n", ESP.getFlashChipSpeed());
  printf("SdkVersion: %s\n", ESP.getSdkVersion());
  printf("FullVersion: %s\n", ESP.getFullVersion().c_str());
  printf("CpuFreq: %dMHz\n", ESP.getCpuFreqMHz());
  printf("FreeHeap: %d B\n", ESP.getFreeHeap());
  printf("ResetInfo: %s\n", ESP.getResetInfo().c_str());
  printf("ResetReason: %s\n", ESP.getResetReason().c_str());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");

  } else {
    Serial.println("Error setting up MDNS responder!");
  }
  //set the pins as outputs
  servoX.attach(D7);
  servoY.attach(D3);
  // Debug console

  //initialize SPIFFS to be able to serve up the static HTML files.
  if (!SPIFFS.begin()) {
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
  //  digitalWrite(PIN_LED, HIGH);
}

void loop()
{

  WiFiClient client;
  server.handleClient();
  if (client.connected()) {
    digitalWrite(LASER, HIGH);
    servoX.write(aPitch);
    servoY.write(bYaw);
  } else {
    digitalWrite(LASER, LOW);
  }
  delay(5);
}

void builtinledSetStatus(bool on) {
  digitalWrite(PIN_LED, on ? LOW : HIGH);
}
