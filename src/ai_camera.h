#ifndef __AI_CAMERA_H__
#define __AI_CAMERA_H__

#include "Arduino.h"
#include "string.h"

#define DPAD_STOP     0
#define DPAD_FORWARD  1
#define DPAD_BACKWARD 2
#define DPAD_LEFT     3
#define DPAD_RIGHT    4

#define JOYSTICK_X       0
#define JOYSTICK_Y       1
#define JOYSTICK_ANGLE   2
#define JOYSTICK_RADIUS  3

#define WIFI_MODE_NONE "0"
#define WIFI_MODE_STA  "1"
#define WIFI_MODE_AP   "2"

#define CAMERA_MODE_AI     "0"
#define CAMERA_MODE_STREAM "1"
#define CAMERA_MODE_BOTH   "2"

#define REGION_A 0
#define REGION_B 1
#define REGION_C 2
#define REGION_D 3
#define REGION_E 4
#define REGION_F 5
#define REGION_G 6
#define REGION_H 7
#define REGION_I 8
#define REGION_J 9
#define REGION_K 10
#define REGION_L 11
#define REGION_M 12
#define REGION_N 13
#define REGION_O 14
#define REGION_P 15
#define REGION_Q 16
#define REGION_R 17
#define REGION_S 18
#define REGION_T 19
#define REGION_U 20
#define REGION_V 21
#define REGION_W 22
#define REGION_X 23
#define REGION_Y 24
#define REGION_Z 25

class AiCamera {
  public:
    AiCamera(const char* name, const char* type);

    void begin();
    void begin(const char* ssid, const char* password, const char* wifiMode, const char* wsPort, const char* cameraMode);
    void readInto(char* buffer);
    bool read();
    void readIntoUnblock(char* buffer);
    void sendData();
    void set(const char* command);
    void set(const char* command, const char* value);
    void get(const char* command, char* result);
    void get(const char* command, const char* value, char* result);
    void command(const char* command, const char* value, char* result) ;
    void setOnReceived(void (*func)());
    void loop();

    int16_t getSlider(uint8_t region);
    bool getButton(uint8_t region);
    bool getSwitch(uint8_t region);
    int16_t getJoystick(uint8_t region, uint8_t axis);
    uint8_t getDPad(uint8_t region);
    int16_t getThrottle(uint8_t region);
    void setMeter(uint8_t region, double value);
    void setRadar(uint8_t region, int16_t angle, double distance);
    void setGreyscale(uint8_t region, uint16_t value1, uint16_t value2, uint16_t value3);
    void setValue(uint8_t region, double value);

  private:
    void subString(char* str, int16_t start, int16_t end=-1);
    void (*__on_receive__)();
    char name[32];
    char type[32];
    // void getStrOf(char* str, uint8_t index, char* result);
    void getStrOf(char* str, uint8_t index, char* result, char divider);
    // void setStrOf(char* str, uint8_t index, char* value);
    void setStrOf(char* str, uint8_t index, String value);
    int16_t getIntOf(char* str, uint8_t index);
    bool getBoolOf(char* str, uint8_t index);
    double getDoubleOf(char* str, uint8_t index);
};

#endif // __AI_CAMERA_H__