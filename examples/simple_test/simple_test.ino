#include "ai_camera.h"


// Uncomment the following line to use STA mode
// #define mode = WIFI_MODE_STA;
// #define SSID = "***********";
// #define PASSWORD = "***********";

// Uncomment the following line to use AP mode
#define WIFI_MODE WIFI_MODE_AP
#define SSID "AiCamera"
#define PASSWORD "12345678"


#define CAMERA_MODE CAMERA_MODE_STREAM
#define PORT "8765"

AiCamera aiCam = AiCamera("aiCam", "aiCam");

void on_receive() {
  Serial.print("Slider D: ");Serial.println(aiCam.getSwitch("D"));
  Serial.print("Switch J: ");Serial.println(aiCam.getSwitch("J"));
  Serial.print("Button I: ");Serial.println(aiCam.getButton("I"));
  Serial.print("Joystick K X: ");Serial.println(aiCam.getJoystick("K", JOYSTICK_X));
  Serial.print("Joystick K Y: ");Serial.println(aiCam.getJoystick("K", JOYSTICK_Y));
  Serial.print("DPad Q: ");Serial.println(aiCam.getDPad("Q"));
  Serial.print("Throttle Q: ");Serial.println(aiCam.getThrottle("Q"));
  aiCam.setMeter("H", 46);
  aiCam.setRadar("B", 20, 30);
  aiCam.setGreyscale("A", 300, 234, 678);
  aiCam.setValue("C", 498);
}

void setup() {
  Serial.begin(115200);
  Serial.println("aiCam begin");
  // aiCam.setSSID(SSID);
  // aiCam.setPassword(PASSWORD);
  // aiCam.setWiFiMode(WIFI_MODE);
  // aiCam.setWSPort(PORT);
  // aiCam.setCameraMode(CAMERA_MODE);
  // aiCam.begin();
  aiCam.begin(SSID, PASSWORD, WIFI_MODE, PORT, CAMERA_MODE);
}

void loop() {
  aiCam.loop();
  delay(10);
}