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
#define WS_BUFFER_SIZE 200

#ifdef AI_CAM_DEBUG_CUSTOM
#define DateSerial dSerial
#define DebugSerial Serial
#else
#define DateSerial Serial
#define DebugSerial Serial
#endif

char sendBuffer[WS_BUFFER_SIZE] = ";;;;;;;;;;;;;;;;;;;;;;;;;";
char recvBuffer[WS_BUFFER_SIZE];

char readBuffer[WS_BUFFER_SIZE + strlen(WS_HEADER)];

AiCamera::AiCamera(const char* name, const char* type) {
  strcpy(this->name, name);
  strcpy(this->type, type);
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

void AiCamera::sendData() {
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

void AiCamera::setOnReceived(void (*func)()) { __on_receive__ = func; }

void AiCamera::loop() {
  char receive[WS_BUFFER_SIZE + strlen(WS_HEADER)];
  this->readInto(receive);
  // this->readIntoUnblock(receive);
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
    strcpy(recvBuffer, receive);
    if (__on_receive__ != NULL) {
      __on_receive__();
    }
  }
  this->sendData();
}

int16_t AiCamera::getSlider(uint8_t region) {
  int16_t value = getIntOf(recvBuffer, region);
  return value;
}

bool AiCamera::getButton(uint8_t region) {
  bool value = getBoolOf(recvBuffer, region);
  return value;
}

bool AiCamera::getSwitch(uint8_t region) {
  bool value = getBoolOf(recvBuffer, region);
  return value;
}

int16_t AiCamera::getJoystick(uint8_t region, uint8_t axis) {
  char valueStr[10];
  char xStr[4];
  char yStr[4];
  int16_t x, y, angle, radius;
  getStrOf(recvBuffer, region, valueStr, ';');
  getStrOf(valueStr, 0, xStr, ",");
  getStrOf(valueStr, 1, yStr, ",");
  x = (int16_t)String(xStr).toInt();
  y = (int16_t)String(yStr).toInt();
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

uint8_t AiCamera::getDPad(uint8_t region) {
  int16_t value = getIntOf(recvBuffer, region);
  return value;
}

int16_t AiCamera::getThrottle(uint8_t region) {
  int16_t value = getIntOf(recvBuffer, region);
  return value;
}

void AiCamera::setMeter(uint8_t region, double value) {
  setStrOf(sendBuffer, region, String(value));
}

void AiCamera::setRadar(uint8_t region, int16_t angle, double distance) {
  setStrOf(sendBuffer, region, String(angle) + "," + String(distance));
}

void AiCamera::setGreyscale(uint8_t region, uint16_t value1, uint16_t value2, uint16_t value3) {
  setStrOf(sendBuffer, region, String(value1) + "," + String(value2) + "," + String(value3));
}

void AiCamera::setValue(uint8_t region, double value) {
  setStrOf(sendBuffer, region, String(value));
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

// void AiCamera::getStrOf(char* str, uint8_t index, char* result) {
//   getStrOf(str, index, result, ';');
// }

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
    // Serial.println((int)str[end]);
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

// void AiCamera::setStrOf(char* str, uint8_t index, char* value) {
//   setStrOf(str, index, value, ';');
// }

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
  String strValue = String(str).substring(0, start) + value + String(str).substring(end);
  strcpy(str, strValue.c_str());
}

int16_t AiCamera::getIntOf(char* str, uint8_t index) {
  int16_t result;
  char strResult[20];
  getStrOf(str, index, strResult, ';');
  result = String(strResult).toInt();
  return result;
}

bool AiCamera::getBoolOf(char* str, uint8_t index) {
  char strResult[20];
  getStrOf(str, index, strResult, ';');
  return String(strResult) == "true";
}

double AiCamera::getDoubleOf(char* str, uint8_t index) {
  double result;
  char strResult[20];
  getStrOf(str, index, strResult, ';');
  result = String(strResult).toDouble();
  return result;
}
