#include <Arduino.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

#include <arduino_homekit_server.h>
#include "ButtonDebounce.h"
#include "ButtonHandler.h"

#define SIMPLE_INFO(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);
#define laser D6
#define PIN_LED 16

//D0 16 //led
//D3  0 //flash button
//D4  2 //led

int aPitch = 90;
int bYaw = 90;

bool Running = true;
bool Attach = true;

ESP8266WebServer server(80);

Servo servoX;
Servo servoY;

const char *ssid = "Nacho WiFi";
const char *password = "turborx7";

void blink_led(int interval, int count) {
  for (int i = 0; i < count; i++) {
    builtinledSetStatus(true);
    delay(interval);
    builtinledSetStatus(false);
    delay(interval);
  }
}

void handleJSData() {
  int yaw = server.arg(0).toInt();
  int pitch = server.arg(1).toInt();

  int changePitch = map(pitch, -100, 100, 0, 180);
  int changeYaw = map(yaw, -100, 100, 180, 0);

  Serial.println("Pitch is: ");
  Serial.print(changePitch);

  Serial.println("Yaw is: ");
  Serial.print(changeYaw);


  aPitch = changePitch;
  bYaw = changeYaw;
  ledCheck();

  server.send(200, "text/plain", "");
}

void ledCheck()
{

  byte dCheck = digitalRead(D4);

  if ( dCheck != 1 ) {
    Attach = true;
    digitalWrite(laser, HIGH);
  } else if (dCheck == 1) {
    Attach = false;
    digitalWrite(laser, LOW);
  }

}

void setup() {
  Serial.begin(115200);
  Serial.setRxBufferSize(32);
  Serial.setDebugOutput(false);
  pinMode(PIN_LED, OUTPUT);
  pinMode(laser, OUTPUT);
  digitalWrite(laser, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.disconnect(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);

  SIMPLE_INFO("");
  SIMPLE_INFO("SketchSize: %d B\n", ESP.getSketchSize());
  SIMPLE_INFO("FreeSketchSpace: %d B\n", ESP.getFreeSketchSpace());
  SIMPLE_INFO("FlashChipSize: %d B\n", ESP.getFlashChipSize());
  SIMPLE_INFO("FlashChipRealSize: %d B\n", ESP.getFlashChipRealSize());
  SIMPLE_INFO("FlashChipSpeed: %d\n", ESP.getFlashChipSpeed());
  SIMPLE_INFO("SdkVersion: %s\n", ESP.getSdkVersion());
  SIMPLE_INFO("FullVersion: %s\n", ESP.getFullVersion().c_str());
  SIMPLE_INFO("CpuFreq: %dMHz\n", ESP.getCpuFreqMHz());
  SIMPLE_INFO("FreeHeap: %d B\n", ESP.getFreeHeap());
  SIMPLE_INFO("ResetInfo: %s\n", ESP.getResetInfo().c_str());
  SIMPLE_INFO("ResetReason: %s\n", ESP.getResetReason().c_str());
  DEBUG_HEAP();
  homekit_setup();
  DEBUG_HEAP();
  blink_led(200, 3);
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount failed");
  }
  else {
    Serial.println("SPIFFS Mount succesfull");
  }
  server.serveStatic("/", SPIFFS, "/joystick.html");
  server.serveStatic("/virtualjoystick.js", SPIFFS, "/virtualjoystick.js");
  server.on("/jsData.html", handleJSData);
  server.begin();
  servoX.attach(D7);
  servoY.attach(D2);
}

void loop() {
  homekit_loop();
  server.handleClient();
  if (Attach) {
    if (!Running) {
      servoX.attach(D7);
      servoY.attach(D2);
      Running = true;
    }
  } else if (!Attach) {
    if (Running) {
      servoX.detach();
      servoY.detach();
      Running = false;
    }
  }
  servoX.write(aPitch);
  servoY.write(bYaw);

}

void builtinledSetStatus(bool on) {
  digitalWrite(PIN_LED, on ? LOW : HIGH);
}

//==============================
// Homekit setup and loop
//==============================

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t name;
extern "C" void occupancy_toggle();
extern "C" void led_toggle();
extern "C" void accessory_init();

ButtonDebounce btn(0, INPUT_PULLUP, LOW);
ButtonHandler btnHandler;

void IRAM_ATTR btnInterrupt() {
  btn.update();
}

uint32_t next_heap_millis = 0;

void homekit_setup() {
  accessory_init();
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  int name_len = snprintf(NULL, 0, "%s_%02X%02X%02X",
                          name.value.string_value, mac[3], mac[4], mac[5]);
  char *name_value = (char*)malloc(name_len + 1);
  snprintf(name_value, name_len + 1, "%s_%02X%02X%02X",
           name.value.string_value, mac[3], mac[4], mac[5]);
  name.value = HOMEKIT_STRING_CPP(name_value);

  arduino_homekit_setup(&config);

  btn.setCallback([](const bool down) {
    btnHandler.handleChange(down);
  });
  btn.setInterrupt(btnInterrupt);

  btnHandler.setIsDownFunction([](void) {
    return btn.checkIsDown();
  });
  btnHandler.setCallback([](const button_event e) {
    SIMPLE_INFO("Button Event: ");
    switch (e) {
      case BUTTON_EVENT_SINGLECLICK:
        SIMPLE_INFO("SINGLECLICK");
        led_toggle();
        break;
      case BUTTON_EVENT_DOUBLECLICK:
        SIMPLE_INFO("DOUBLECLICK");
        occupancy_toggle();
        break;
      case BUTTON_EVENT_LONGCLICK:
        SIMPLE_INFO("LONGCLICK");
        homekit_storage_reset();
        system_restart();
        break;
    }
  }	);
}

void homekit_loop() {
  btnHandler.loop();
  arduino_homekit_loop();
  uint32_t time = millis();
  if (time > next_heap_millis) {
    INFO("heap: %d, sockets: %d",
         ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
    next_heap_millis = time + 5000;
  }
}
