#include "ai_camera.h"

// Uncomment the following line to use STA mode
// #define mode = WIFI_MODE_STA;
// #define SSID = "***********";
// #define PASSWORD = "***********";

// Uncomment the following line to use AP mode
#define WIFI_MODE WIFI_MODE_AP
#define SSID "AIC_Test"
#define PASSWORD "12345678"


#define CAMERA_MODE CAMERA_MODE_STREAM
#define PORT "8765"

AiCamera aiCam = AiCamera("aiCam", "aiCam");

void onReceive() {
  Serial.print("Slider D: ");Serial.println(aiCam.getSlider(REGION_D));
  Serial.print("Switch J: ");Serial.println(aiCam.getSwitch(REGION_J));
  Serial.print("Button I: ");Serial.println(aiCam.getButton(REGION_I));
  Serial.print("Joystick Q X: ");Serial.println(aiCam.getJoystick(REGION_Q, JOYSTICK_X));
  Serial.print("Joystick Q Y: ");Serial.println(aiCam.getJoystick(REGION_Q, JOYSTICK_Y));
  Serial.print("Joystick Q Angle: ");Serial.println(aiCam.getJoystick(REGION_Q, JOYSTICK_ANGLE));
  Serial.print("Joystick Q Radius: ");Serial.println(aiCam.getJoystick(REGION_Q, JOYSTICK_RADIUS));
  Serial.print("Throttle K: ");Serial.println(aiCam.getThrottle(REGION_K));
  aiCam.setMeter(REGION_H, 46);
  aiCam.setRadar(REGION_B, 20, 30);
  aiCam.setGreyscale(REGION_A, 300, 234, 678);
  aiCam.setValue(REGION_C, 498);
}

void setup() {
  Serial.begin(115200);
  Serial.println("aiCam begin");
  aiCam.begin(SSID, PASSWORD, WIFI_MODE, PORT, CAMERA_MODE);
  aiCam.setOnReceived(onReceive);
}

void loop() {
  aiCam.loop();
  delay(100);
}