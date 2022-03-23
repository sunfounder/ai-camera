#ifndef __AI_CAMERA_H__
#define __AI_CAMERA_H__

#include <ArduinoJson.h>
#include "Arduino.h"
#include "string.h"

#define DPAD_STOP 0
#define DPAD_FORWARD 1
#define DPAD_BACKWARD 2
#define DPAD_LEFT 3
#define DPAD_RIGHT 4
#define JOYSTICK_X 0
#define JOYSTICK_Y 1

#define WIFI_MODE_NONE "0"
#define WIFI_MODE_STA  "1"
#define WIFI_MODE_AP   "2"

#define CAMERA_MODE_AI     "0"
#define CAMERA_MODE_STREAM "1"
#define CAMERA_MODE_BOTH   "2"

class AiCamera {
  public:
    AiCamera(const char* name, const char* type);

    DynamicJsonDocument sendBuffer = DynamicJsonDocument(1024);
    DynamicJsonDocument recvBuffer = DynamicJsonDocument(1024);
    char name[40];
    char type[40];

    void setSSID(const char* ssid);
    void setPassword(const char* password);
    void setWiFiMode(int wifiMode);
    void setWSPort(int port);
    void setCameraMode(int cameraMode);
    void begin();
    void readInto(char* buffer);
    String read();
    void sendData();
    void set(const char* command);
    void set(const char* command, int value);
    void set(const char* command, const char* value);
    void get(const char* command, char* result);
    void get(const char* command, int value, char* result);
    void get(const char* command, const char* value, char* result);
    void command(const char* command, const char* value, char* result) ;
    void setOnReceived(void (*func)());
    void loop();

    int getSlider(const char* region);
    int getButton(const char* region);
    bool getSwitch(const char* region);
    int getJoystick(const char* region, int axis);
    int getDPad(const char* region);
    int getThrottle(const char* region);
    void setMeter(const char* region, double value);
    void setRadar(const char* region, int angle, double distance);
    void setGreyscale(const char* region, int value1, int value2, int value3);
    void setValue(const char* region, double value);

  private:
    char ssid[30] = "AiCamera";
    char password[30] = "12345678";
    int wifiMode = WIFI_MODE_AP;
    int wsPort = 8765;
    int cameraMode = CAMERA_MODE_STREAM;

    void subString(char* str, int start);
    void (*__on_receive__)();
};

#endif // __AI_CAMERA_H__