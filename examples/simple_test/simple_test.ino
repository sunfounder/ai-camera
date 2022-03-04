#include "esp_ai_cam.h"


// Uncomment the following line to use STA mode
// int mode = STA;
// String ssid = "***********";
// String password = "***********";

// Uncomment the following line to use AP mode
int mode = AP;
String ssid = "ESP_AI_CAMERA";
String password = "12345678";

ESP_AI_Camera ai_cam = ESP_AI_Camera("ai_cam", "ai_cam");

int port = 8765;

void on_receive() {
  Serial.print("Slider D: ");Serial.println(ai_cam.getSwitch("D"));
  Serial.print("Switch J: ");Serial.println(ai_cam.getSwitch("J"));
  Serial.print("Button I: ");Serial.println(ai_cam.getButton("I"));
  Serial.print("Joystick K X: ");Serial.println(ai_cam.getJoystick("K", JOYSTICK_X));
  Serial.print("Joystick K Y: ");Serial.println(ai_cam.getJoystick("K", JOYSTICK_Y));
  Serial.print("DPad Q: ");Serial.println(ai_cam.getDPad("Q"));
  Serial.print("Throttle Q: ");Serial.println(ai_cam.getThrottle("Q"));
  ai_cam.setMeter("H", 46);
  ai_cam.setRadar("B", 20, 30);
  ai_cam.setGreyscale("A", 300, 234, 678);
  ai_cam.setValue("C", 498);
}

void setup():
  Serial.begin(115200);
  ai_cam.begin(ssid, password, mode, port);
}

void loop():
  ai_cam.loop();
}