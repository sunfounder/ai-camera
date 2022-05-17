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
#define StrAppend(str, suffix) uint32_t len=strlen(str); str[len] = suffix; str[len+1] = '\0'
#define StrClear(str) str[0] = 0
#define SERIAL_TIMEOUT 100
#define WS_BUFFER_SIZE 100

#ifdef AI_CAM_DEBUG_CUSTOM
#define DateSerial dSerial
#define DebugSerial Serial
#else
#define DateSerial Serial
#define DebugSerial Serial
#endif

void (*__on_receive__)(char*, char*);
char name[32];
char type[32];

char readBuffer[WS_BUFFER_SIZE + strlen(WS_HEADER)];

AiCamera::AiCamera(const char* _name, const char* _type) {
  strcpy(name, _name);
  strcpy(type, _type);
}

void AiCamera::begin(const char* ssid, const char* password, const char* wifiMode, const char* wsPort, const char* cameraMode) {
  #ifdef AI_CAM_DEBUG
  DateSerial.begin(115200);
  #endif
  char ip[15];
  this->set("RESET");
  this->set("NAME", name);
  this->set("TYPE", type);
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
// void AiCamera::readIntoUnblock(char* buffer) {
  bool finished = false;
  char incomingChar;
  StrClear(buffer);
  // DebugSerial.print("Buffer length: ");
  // DebugSerial.println(strlen(buffer));

  while (DateSerial.available()) {
    incomingChar = (char)DateSerial.read();
    if (incomingChar == '\n') {
      finished = true;
      break;
    } else if (incomingChar == '\r') {
      continue;
    } else if ((int)incomingChar > 31 && (int)incomingChar < 127) {
      StrAppend(readBuffer, incomingChar);
    }
  }
  if (finished) {
    // DebugSerial.println("Read into unblock finished");
    if (IsStartWith(readBuffer, "[DEBUG] ")) {
      #ifdef AI_CAM_DEBUG
      String tempString = String(readBuffer);
      tempString.replace("[DEBUG]", "[AI_CAMERA]");
      DebugSerial.println(tempString);
      #endif
      StrClear(buffer);
    } else {
      // strncpy(buffer, readBuffer, strlen(readBuffer));
      strcpy(buffer, readBuffer);
    }
    StrClear(readBuffer);
  }
}

void AiCamera::sendData(char* sendBuffer) {
  DateSerial.print("WS+");
  DateSerial.println(sendBuffer);
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

void AiCamera::setOnReceived(void (*func)(char*, char*)) { __on_receive__ = func; }

void AiCamera::loop() {
  char recvBuffer[WS_BUFFER_SIZE + strlen(WS_HEADER)];
  char sendBuffer[WS_BUFFER_SIZE] = ";;;;;;;;;;;;;;;;;;;;;;;;;";
  this->readInto(recvBuffer);
  if (strlen(recvBuffer) != 0) {
    if (IsStartWith(recvBuffer, "[CONNECTED]")) {
      #ifdef AI_CAM_DEBUG
      this->subString(recvBuffer, 11);
      DebugSerial.print("Connected from ");
      DebugSerial.println(recvBuffer);
      #endif
    } else if (IsStartWith(recvBuffer, "[DISCONNECTED]")) {
      #ifdef AI_CAM_DEBUG
      this->subString(recvBuffer, 14);
      DebugSerial.println("Disconnected from ");
      DebugSerial.println(recvBuffer);
      #endif
      return;
    } else {
      this->subString(recvBuffer, strlen(WS_HEADER));
      #ifdef AI_CAM_DEBUG
      DebugSerial.print("[AI_CAMERA] Received: ");
      DebugSerial.println(recvBuffer);
      #endif
      if (__on_receive__ != NULL) {
        __on_receive__(recvBuffer, sendBuffer);
      }
    }
  this->sendData(sendBuffer);
  }
}

int16_t AiCamera::getSlider(char* buf, uint8_t region) {
  int16_t value = getIntOf(buf, region);
  return value;
}

bool AiCamera::getButton(char* buf, uint8_t region) {
  bool value = getBoolOf(buf, region);
  return value;
}

bool AiCamera::getSwitch(char* buf, uint8_t region) {
  bool value = getBoolOf(buf, region);
  return value;
}

int16_t AiCamera::getJoystick(char* buf, uint8_t region, uint8_t axis) {
  char valueStr[10];
  char xStr[4];
  char yStr[4];
  int16_t x, y, angle, radius;
  getStrOf(buf, region, valueStr, ';');
  x = getIntOf(valueStr, 0, ',');
  y = getIntOf(valueStr, 1, ',');
  angle = atan2(x, y) * 180.0 / PI;
  radius = sqrt(y * y + x * x);
  switch (axis) {
    case JOYSTICK_X: return x;
    case JOYSTICK_Y: return y;
    case JOYSTICK_ANGLE: return angle;
    case JOYSTICK_RADIUS: return radius;
    default: return 0;
  }
}

uint8_t AiCamera::getDPad(char* buf, uint8_t region) {
  char value[10];
  getStrOf(buf, region, value, ';');
  uint8_t result;
  if ((String)value == (String)"forward") result = DPAD_FORWARD;
  else if ((String)value == (String)"backward") result = DPAD_BACKWARD;
  else if ((String)value == (String)"left") result = DPAD_LEFT;
  else if ((String)value == (String)"right") result = DPAD_RIGHT;
  else if ((String)value == (String)"stop") result = DPAD_STOP;
  return result;
}

int16_t AiCamera::getThrottle(char* buf, uint8_t region) {
  int16_t value = getIntOf(buf, region);
  return value;
}

void AiCamera::setMeter(char* buf, uint8_t region, double value) {
  setStrOf(buf, region, String(value));
}

void AiCamera::setRadar(char* buf, uint8_t region, int16_t angle, double distance) {
  setStrOf(buf, region, String(angle) + "," + String(distance));
}

void AiCamera::setGreyscale(char* buf, uint8_t region, uint16_t value1, uint16_t value2, uint16_t value3) {
  setStrOf(buf, region, String(value1) + "," + String(value2) + "," + String(value3));
}

void AiCamera::setValue(char* buf, uint8_t region, double value) {
  setStrOf(buf, region, String(value));
}

void AiCamera::subString(char* str, int16_t start, int16_t end) {
  uint8_t length = strlen(str);
  if (end == -1) {
    end = length;
  }
  for (uint8_t i = 0; i < end; i++) {
    if (i + start < end) {
      str[i] = str[i + start];
    } else {
      str[i] = '\0';
    }
  }
}

void AiCamera::getStrOf(char* str, uint8_t index, char* result, char divider) {
  uint8_t start, end;
  uint8_t length = strlen(str);
  uint8_t i, j;
  // Get start index
  if (index == 0) {
    start = 0;
  } else {
    for (start = 0, j = 1; start < length; start++) {
      if (str[start] == divider) {
        if (index == j) {
          start++;
          break;
        }
        j++;
      }
    }
  }
  // Get end index
  for (end = start, j = 0; end < length; end++) {
    if (str[end] == divider) {
      break;
    }
  }
  // Copy result
  for (i = start, j = 0; i < end; i++, j++) {
    result[j] = str[i];
  }
  result[j] = '\0';
}

void AiCamera::setStrOf(char* str, uint8_t index, String value) {
  uint8_t start, end;
  uint8_t length = strlen(str);
  uint8_t i, j;
  // Get start index
  if (index == 0) {
    start = 0;
  } else {
    for (start = 0, j = 1; start < length; start++) {
      if (str[start] == ';') {
        if (index == j) {
          start++;
          break;
        }
        j++;
      }
    }
  }
  // Get end index
  for (end = start, j = 0; end < length; end++) {
    if (str[end] == ';') {
      break;
    }
  }
  String strString = str;
  String strValue = strString.substring(0, start) + value + strString.substring(end);
  strcpy(str, strValue.c_str());
}

int16_t AiCamera::getIntOf(char* str, uint8_t index, char divider=';') {
  int16_t result;
  char strResult[6];
  getStrOf(str, index, strResult, divider);
  result = String(strResult).toInt();
  return result;
}

bool AiCamera::getBoolOf(char* str, uint8_t index) {
  char strResult[5];
  getStrOf(str, index, strResult, ';');
  return String(strResult).toInt();
}

double AiCamera::getDoubleOf(char* str, uint8_t index) {
  double result;
  char strResult[20];
  getStrOf(str, index, strResult, ';');
  result = String(strResult).toDouble();
  return result;
}
