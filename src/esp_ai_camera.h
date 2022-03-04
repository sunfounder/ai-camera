
#include <ArduinoJson.h>

#define DPAD_STOP 0
#define DPAD_FORWARD 1
#define DPAD_BACKWARD 2
#define DPAD_LEFT 3
#define DPAD_RIGHT 4
#define JOYSTICK_X 0
#define JOYSTICK_Y 1

#define NONE 0
#define STA 1
#define AP 2

class ESP_AI_Camera {
  public:
    ESP_AI_Camera();

    DynamicJsonDocument send_buffer = DynamicJsonDocument(1024);
    DynamicJsonDocument recv_buffer = DynamicJsonDocument(1024);
    String name;
    String type;

    void begin(String _ssid, String _password, int _mode, int _port);
    String read();
    void write(String value);
    void send_data();
    void set(String command, String value);
    void set(String command);
    void set_on_received(void (*func)());
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
    void _command(String mode, String command, String value);
    void _command(String mode, String command);
    void (*__on_receive__)();
};