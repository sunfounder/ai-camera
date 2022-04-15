#include "ai_camera.h"

// #define AI_CAM_DEBUG
// #define AI_CAM_DEBUG_CUSTOM

#ifdef AI_CAM_DEBUG_CUSTOM
#include <SoftwareSerial.h>
SoftwareSerial dSerial(10, 11); // RX, TX
#endif

#define CHECK "SC"
#define OK_FLAG "[OK]"
#define WS_HEADER "WS+"
#define IsStartWith(str, prefix) (strncmp(str, prefix, strlen(prefix)) == 0)
#define StrAppend(str, suffix) int len=strlen(str); str[len] = suffix; str[len+1] = '\0'
#define SERIAL_TIMEOUT 100
#define WS_BUFFER_SIZE 100

#ifdef AI_CAM_DEBUG_CUSTOM
#define DateSerial dSerial
#define DebugSerial Serial
#else
#define DateSerial Serial
#define DebugSerial Serial
#endif

DynamicJsonDocument sendBuffer = DynamicJsonDocument(WS_BUFFER_SIZE);
DynamicJsonDocument recvBuffer = DynamicJsonDocument(WS_BUFFER_SIZE);

AiCamera::AiCamera(const char* name, const char* type) {
  sendBuffer["Name"] = name;
  sendBuffer["Type"] = type;
  sendBuffer["Check"] = CHECK;
}

void AiCamera::begin(const char* ssid, const char* password, const char* wifiMode, const char* wsPort, const char* cameraMode) {
  #ifdef AI_CAM_DEBUG
  DateSerial.begin(115200);
  #endif
  char ip[15];
  this->set("RESET");
  this->set("SSID", ssid);
  this->set("PSK",  password);
  this->set("MODE", wifiMode);
  this->set("PORT", wsPort);
  this->set("CAMERA_MODE", cameraMode);
  this->get("START", ip);
  #ifdef AI_CAM_DEBUG
  DebugSerial.print("[DEBUG] WebServer started on ws://");
  DebugSerial.print(ip);
  DebugSerial.print(":");
  DebugSerial.println(wsPort);
  #endif
}

void AiCamera::readInto(char* buffer) {
  char incomingChar;
  unsigned long timeoutStart = millis();
  buffer[0] = '\0';

  while (DateSerial.available() > 0 || (millis() - timeoutStart) < SERIAL_TIMEOUT) {
    incomingChar = (char)DateSerial.read();
    if (incomingChar == '\n') {
      break;
    } else if (incomingChar == '\r') {
      continue;
    } else if ((int)incomingChar > 31 && (int)incomingChar < 127) {
      StrAppend(buffer, incomingChar);
    }
  }
  if (strlen(buffer) > 0) {
    if (IsStartWith(buffer, "[DEBUG] ")) {
      String tempString = String(buffer);
      tempString.replace("[DEBUG]", "[AI_CAMERA]");
      #ifdef AI_CAM_DEBUG
      DebugSerial.println(tempString);
      #endif
      buffer[0] = "\0";
    }
  }
}

void AiCamera::sendData() {
  uint8_t payload[WS_BUFFER_SIZE];
  serializeJson(sendBuffer, payload);
  DateSerial.print("WS+");
  DateSerial.println((char*)payload);
}

void AiCamera::set(const char* command) {
  char result[10];
  this->command(command, "", result);
}

void AiCamera::set(const char* command, const char* value) {
  char result[10];
  this->command(command, value, result);
}

void AiCamera::get(const char* command, char* result) {
  this->command(command, "", result);
}

void AiCamera::get(const char* command, const char* value, char* result) {
  this->command(command, value, result);
}

void AiCamera::command(const char* command, const char* value, char* result) {
  #ifdef AI_CAM_DEBUG
  DebugSerial.print("AiCamera::command: ");
  DebugSerial.print("SET+");
  DebugSerial.print(command);
  DebugSerial.println(value);
  #endif
  DateSerial.print("SET+");
  DateSerial.print(command);
  DateSerial.println(value);
  while (1) {
    this->readInto(result);
    if (IsStartWith(result, OK_FLAG)){
      #ifdef AI_CAM_DEBUG
      DebugSerial.print("Result: ");
      DebugSerial.println(result);
      #endif
      this->subString(result, strlen(OK_FLAG) + 1); // Add 1 for Space
      break;
    }
  }
}

void AiCamera::setOnReceived(void (*func)()) { __on_receive__ = func; }

void AiCamera::loop() {
  char receive[WS_BUFFER_SIZE];
  this->readInto(receive);
  #ifdef AI_CAM_DEBUG
  DebugSerial.println(receive);
  #endif
  if (strlen(receive) == 0) {
    return;
  }
  if (IsStartWith(receive, "[CONNECTED]")) {
    #ifdef AI_CAM_DEBUG
    this->subString(receive, 11);
    DebugSerial.print("Connected from ");
    DebugSerial.println(receive);
    #endif
  } else if (IsStartWith(receive, "[DISCONNECTED]")) {
    #ifdef AI_CAM_DEBUG
    this->subString(receive, 14);
    DebugSerial.println("Disconnected from ");
    DebugSerial.println(receive);
    #endif
    return;
  } else {
    this->subString(receive, strlen(WS_HEADER));
    // #ifdef AI_CAM_DEBUG
    // DebugSerial.print("[AI_CAMERA] Received: ");
    // DebugSerial.println(receive);
    // #endif
    recvBuffer.clear();
    DeserializationError err = deserializeJson(recvBuffer, receive);
    // if (err != DeserializationError::Ok) {
    //   DebugSerial.print("[ERROR] Received failed: ");
    //   DebugSerial.println(err.c_str());
    //   recvBuffer.clear();
    // } else {
      if (__on_receive__ != NULL) {
        __on_receive__();
      }
    // }
  }
  this->sendData();
}

int16_t AiCamera::getSlider(const char* region) {
  int value = (int)recvBuffer[region];
  return value;
}

bool AiCamera::getButton(const char* region) {
  bool value = (bool)recvBuffer[region];
  return value;
}

bool AiCamera::getSwitch(const char* region) {
  bool value = (bool)recvBuffer[region];
  return value;
}

int16_t AiCamera::getJoystick(const char* region, uint8_t axis) {
  int16_t x, y, angle, radius;
  x = (int16_t)recvBuffer[region][0];
  y = (int16_t)recvBuffer[region][1];
  angle = atan2(x, y) * 180 / PI;
  radius = sqrt(y * y + x * x);
  switch (axis) {
    case JOYSTICK_X: return x;
    case JOYSTICK_Y: return y;
    case JOYSTICK_ANGLE: return angle;
    case JOYSTICK_RADIUS: return radius;
    default: return 0;
  }
}

uint8_t AiCamera::getDPad(const char* region) {
  const char* value = (const char*)recvBuffer[region];
  uint8_t result;
  if ((String)value == (String)"forward") result = DPAD_FORWARD;
  else if ((String)value == (String)"backward") result = DPAD_BACKWARD;
  else if ((String)value == (String)"left") result = DPAD_LEFT;
  else if ((String)value == (String)"right") result = DPAD_RIGHT;
  else if ((String)value == (String)"stop") result = DPAD_STOP;
  else result = -1;
  return result;
}

int16_t AiCamera::getThrottle(const char* region) {
  int16_t value = (int16_t)recvBuffer[region];
  return value;
}

void AiCamera::setMeter(const char* region, double value) {
  sendBuffer[region] = value;
}

void AiCamera::setRadar(const char* region, int16_t angle, double distance) {
  if (sendBuffer.containsKey(region)) {
    sendBuffer[region][0] = angle;
    sendBuffer[region][1] = distance;
  } else {
    JsonArray data = sendBuffer.createNestedArray(region);
    data.add(angle);
    data.add(distance);
  }
}

void AiCamera::setGreyscale(const char* region, uint16_t value1, uint16_t value2, uint16_t value3) {
  JsonArray data = sendBuffer.createNestedArray(region);
  data.add(value1);
  data.add(value2);
  data.add(value3);
}

void AiCamera::setValue(const char* region, double value) {
  sendBuffer[region] = value;
}

void AiCamera::setVideo(const char* url) {
  sendBuffer["video"] = url;
}

void AiCamera::subString(char* str, uint8_t start) {
  uint8_t length = strlen(str);
  for (uint8_t i = 0; i < length; i++) {
    if (i + start < length) {
      str[i] = str[i + start];
    } else {
      str[i] = '\0';
    }
  }
}
