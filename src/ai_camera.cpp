#include "ai_camera.h"

#define CHECK "SunFounder Controller"
#define OK_FLAG "[OK]"
#define IsStartWith(str, prefix) (strncmp(str, prefix, strlen(prefix)) == 0)
#define StrAppend(str, suffix) int len=strlen(str); str[len] = suffix; str[len+1] = '\0'
#define SERIAL_TIMEOUT 200

DynamicJsonDocument sendBuffer = DynamicJsonDocument(1024);
DynamicJsonDocument recvBuffer = DynamicJsonDocument(1024);

AiCamera::AiCamera(const char* name, const char* type) {
  // Serial.begin(115200);
  // Serial.print("AiCamera::AiCamera");
  // Serial.print(" name: ");
  // Serial.print(name);
  // Serial.print(" type: ");
  // Serial.println(type);
  sendBuffer["Name"] = name;
  sendBuffer["Type"] = type;
  sendBuffer["Check"] = CHECK;
  // Serial.println((char*)(sendBuffer["Name"]));
}

void AiCamera::begin(const char* ssid, const char* password, const char* wifiMode, const char* wsPort, const char* cameraMode) {
  // Serial.println("AiCamera::begin");
  char ip[15];
  this->set("SSID", ssid);
  this->set("PSK",  password);
  this->set("MODE", wifiMode);
  this->set("PORT", wsPort);
  this->set("CAMERA_MODE", cameraMode);
  this->get("START", ip);
  Serial.print("[DEBUG] WebServer started on ws://");
  Serial.print(ip);
  Serial.print(":");
  Serial.println(wsPort);
  Serial.println((char*)(sendBuffer["Name"]));
}

void AiCamera::readInto(char* buffer) {
  char incomingChar;
  unsigned long timeoutStart = millis();
  buffer[0] = '\0';

  while (Serial.available() > 0 || (millis() - timeoutStart) < SERIAL_TIMEOUT) {
    incomingChar = (char)Serial.read();
    if (incomingChar == '\n') {
      break;
    } else if (incomingChar == '\r') {
      continue;
    } else if ((int)incomingChar > 31 && (int)incomingChar < 127) {
      StrAppend(buffer, incomingChar);
    }
  }
  if (strlen(buffer) > 0) {
    // Serial.print("AiCamera::readInto buffer: ");
    // Serial.println(strlen(buffer));
    if (IsStartWith(buffer, "[DEBUG] ")) {
      String tempString = String(buffer);
      tempString.replace("[DEBUG]", "[AI_CAMERA]");
      Serial.println(tempString);
      buffer[0] = "\0";
    }
  }
}

void AiCamera::sendData() {
  uint8_t payload[1024];
  // sendBuffer.clear();
  Serial.println((char*)(sendBuffer["Name"]));
  deserializeJson(sendBuffer, payload);
  Serial.print("WS+");
  Serial.println((char*)payload);
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
  Serial.print("AiCamera::command command: ");
  Serial.print(" command: ");
  Serial.print(command);
  Serial.print(" value: ");
  Serial.println(value);
  char data[30] = "SET+";
  strcat(data, command);
  strcat(data, value);
  Serial.println(data);
  while (1) {
    this->readInto(result);
    if (IsStartWith(result, OK_FLAG)){
      Serial.println(result);
      this->subString(result, strlen(OK_FLAG) + 1); // Add 1 for Space
      break;
    }
  }
}

void AiCamera::setOnReceived(void (*func)()) { __on_receive__ = func; }

void AiCamera::loop() {
  Serial.println("AiCamera::loop");
  Serial.println((char*)(sendBuffer["Name"]));
  char receive[1024];
  this->readInto(receive);
  // Serial.println(receive);
  // if (strlen(receive) == 0) {
  // } else if (IsStartWith(receive, "[CONNECTED]")) {
  //   this->subString(receive, 11);
  //   Serial.print("Connected from ");
  //   Serial.println(receive);
  // } else if (IsStartWith(receive, "[DISCONNECTED]")) {
  //   this->subString(receive, 14);
  //   Serial.println("Disconnected from ");
  //   Serial.println(receive);
  //   return;
  // } else {
  //   recvBuffer.clear();
  //   String tempString = String(receive);
  //   serializeJson(recvBuffer, tempString);
  //   if (__on_receive__ != NULL) {
  //     __on_receive__();
  //   }
  // }
  this->sendData();
}

int AiCamera::getSlider(const char* region) {
  int value = recvBuffer[region];
  return value;
}

int AiCamera::getButton(const char* region) {
  int value = recvBuffer[region];
  return value;
}

bool AiCamera::getSwitch(const char* region) {
  bool value = recvBuffer[region];
  return value;
}

int AiCamera::getJoystick(const char* region, int axis) {
  int value = recvBuffer[region][axis];
  return value;
}

int AiCamera::getDPad(const char* region) {
  const char* value = recvBuffer[region];
  int result;
  if ((String)value == (String)"forward") result = DPAD_FORWARD;
  else if ((String)value == (String)"backward") result = DPAD_BACKWARD;
  else if ((String)value == (String)"left") result = DPAD_LEFT;
  else if ((String)value == (String)"right") result = DPAD_RIGHT;
  else if ((String)value == (String)"stop") result = DPAD_STOP;
  else result = -1;
  return result;
}

int AiCamera::getThrottle(const char* region) {
  int value = recvBuffer[region];
  return value;
}

void AiCamera::setMeter(const char* region, double value) {
  sendBuffer[region] = value;
}

void AiCamera::setRadar(const char* region, int angle, double distance) {
  #ifdef MC_DEBUG
  Serial.printf("AiCamera::setRadar(%d, %f)\n", angle, distance);
  #endif
  if (sendBuffer.containsKey(region)) {
    sendBuffer[region][0] = angle;
    sendBuffer[region][1] = distance;
  } else {
    JsonArray data = sendBuffer.createNestedArray(region);
    data.add(angle);
    data.add(distance);
  }
}

void AiCamera::setGreyscale(const char* region, int value1, int value2, int value3) {
  JsonArray data = sendBuffer.createNestedArray(region);
  data.add(value1);
  data.add(value2);
  data.add(value3);
}

void AiCamera::setValue(const char* region, double value) {
  sendBuffer[region] = value;
}

void AiCamera::subString(char* str, int start) {
  int length = strlen(str);
  for (int i = 0; i < length; i++) {
    if (i + start < length) {
      str[i] = str[i + start];
    } else {
      str[i] = '\0';
    }
  }
}
