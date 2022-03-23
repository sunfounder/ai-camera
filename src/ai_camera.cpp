#include "ai_camera.h"

#define CHECK "SunFounder Controller"
#define OK_FLAG "[OK]"
#define IsStartWith(str, prefix) (strncmp(str, prefix, strlen(prefix)) == 0)
#define Write(str) Serial.println(str)
#define StrAppend(str, suffix) int len=strlen(str); str[len] = suffix; str[len+1] = '\0'
#define SERIAL_TIMEOUT 200

AiCamera::AiCamera(const char* name, const char* type) {
  strcpy(this->name, name);
  strcpy(this->type, type);
  this->sendBuffer["Name"] = this->name;
  this->sendBuffer["Type"] = this->type;
  this->sendBuffer["Check"] = CHECK;
}

void AiCamera::setSSID(const char* ssid) {
  strcpy(this->ssid, ssid);
}

void AiCamera::setPassword(const char* password) {
  strcpy(this->password, password);
}

void AiCamera::setWiFiMode(int wifiMode) {
  this->wifiMode = wifiMode;
}

void AiCamera::setWSPort(int port) {
  this->wsPort = port;
}

void AiCamera::setCameraMode(int cameraMode) {
  this->cameraMode = cameraMode;
}

void AiCamera::begin() {
  Serial.print("[DEBUG] AiCamera::begin: ");
  Serial.println(this->ssid);
  char result[100];
  this->set("SSID", this->ssid, result);
  this->set("PSK", this->password, result);
  this->set("MODE", this->wifiMode, result);
  this->set("PORT", this->wsPort, result);
  this->set("CAMERA_MODE", this->cameraMode, result);
  this->set("START", result);
  Serial.print("WebServer started on ws://");
  Serial.print(result);
  Serial.print(":");
  Serial.println(this->wsPort);
}

void AiCamera::readInto(char* buffer) {
  // char temp[1];
  int incomingByte = 0;
  char incomingChar;
  unsigned long timeoutStart = millis();
  buffer[0] = '\0';

  while (Serial.available() > 0 || (millis() - timeoutStart) < SERIAL_TIMEOUT) {
    incomingByte = Serial.read();
    incomingChar = (char) incomingByte;
    if (incomingChar == '\n') {
      break;
    } else if (incomingChar == '\r') {
      continue;
    } else if (incomingByte > 31 && incomingByte < 127) {
      StrAppend(buffer, incomingChar);
      // Serial.print(incomingChar);
      // Serial.println(strlen(buffer));
      // buffer[strlen(buffer)] = incomingChar;
      // buffer[strlen(buffer)+1] = '\0';
    }
  }
  // while ((millis() - timeoutStart) < SERIAL_TIMEOUT) {
  //   Serial.readBytes(temp, 1);
  //   if (temp[0] == '\n') {
  //     Serial.println("Line finished");
  //     break;
  //   } else if (temp[0] == '\r') {
  //     continue;
  //   } else if ((int)temp[0] > 31 && (int)temp[0] < 127) {
  //     this->concat(buffer, temp[0]);
  //   }
  // }
  if (strlen(buffer) > 0) {
    Serial.print("AiCamera::readInto buffer: ");
    Serial.println(strlen(buffer));
    if (IsStartWith(buffer, "[DEBUG] ")) {
      String tempString = String(buffer);
      tempString.replace("[DEBUG]", "[AI_CAMERA]");
      Serial.println(tempString);
      buffer[0] = "\0";
    }
  }
}

void AiCamera::sendData() {
  uint8_t* payload;
  this->sendBuffer.clear();
  deserializeJson(this->sendBuffer, payload);
  char* data = "WS+";
  strcat(data, (char*)payload);
  Write(data);
}

void AiCamera::set(const char* command, char* result) {
  this->set(command, "", result);
}

void AiCamera::set(const char* command, int value, char* result) {
  this->set(command, (char*)value, result);
}

void AiCamera::set(const char* command, const char* value, char* result) {
  char* data = "SET+";
  strcat(data, command);
  strcat(data, value);
  Write(data);
  while (1) {
    this->readInto(result);
    // Serial.println("AiCamera::set result: ");
    // Serial.println("ABCDEFG");
    // String tempString = String(result);
    // if (tempString.startsWith("[ERROR]")) {
    //   Serial.println(result);
    //   while (1)
    //     ;
    //   ;
    // }
    // Serial.println(result);
    // Serial.println(strlen(result));
    // Serial.println(strncmp((const char*)result, "[OK]", 4) == 0);
    // Serial.println(IsStartWith(result, "[OK]"));
    // Serial.println(IsStartWith("[OK]\0", "[OK]"));
    // Serial.println(IsStartWith("[OK] ", "[OK]"));
    // Serial.println(IsStartWith("[OK]\r\n", "[OK]"));
    // Serial.println(IsStartWith("[OK]1235534", "[OK]"));
    // Serial.println(IsStartWith("[OK] sdfddd", "[OK]"));
    // Serial.println(IsStartWith("[OK", "[OK]"));
    // Serial.println(IsStartWith("[OKfcf", "[OK]"));
    for (int i = 0; i < strlen(result); i++) {
      Serial.print((int)result[i]);
      Serial.print(" ");
      Serial.println((int)OK_FLAG[i]);
    }
    if (IsStartWith(result, OK_FLAG)){
      Serial.println(result);
    }
    // if (tempString.startsWith("[OK]")) {
    //   Serial.println(tempString);
    //   tempString = tempString.substring(4);
    //   tempString.trim();
    //   strcpy(result, tempString.c_str());
    //   break;
    // }
    delay(1000);
  }
}

void AiCamera::setOnReceived(void (*func)()) { __on_receive__ = func; }

void AiCamera::loop() {
  char* receive;
  this->readInto(receive);
  if (strlen(receive) == 0) {
  } else if (IsStartWith(receive, "[CONNECTED]")) {
    this->subString(receive, 11);
    Serial.print("Connected from ");
    Serial.println(receive);
  } else if (IsStartWith(receive, "[DISCONNECTED]")) {
    this->subString(receive, 14);
    Serial.println("Disconnected from ");
    Serial.println(receive);
    return;
  } else {
    this->recvBuffer.clear();
    String tempString = String(receive);
    serializeJson(this->recvBuffer, tempString);
    if (__on_receive__ != NULL) {
      __on_receive__();
    }
  }
  sendData();
}

int AiCamera::getSlider(const char* region) {
  int value = this->recvBuffer[region];
  return value;
}

int AiCamera::getButton(const char* region) {
  int value = this->recvBuffer[region];
  return value;
}

bool AiCamera::getSwitch(const char* region) {
  bool value = this->recvBuffer[region];
  return value;
}

int AiCamera::getJoystick(const char* region, int axis) {
  int value = this->recvBuffer[region][axis];
  return value;
}

int AiCamera::getDPad(const char* region) {
  const char* value = this->recvBuffer[region];
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
  int value = this->recvBuffer[region];
  return value;
}

void AiCamera::setMeter(const char* region, double value) {
  this->sendBuffer[region] = value;
}

void AiCamera::setRadar(const char* region, int angle, double distance) {
  #ifdef MC_DEBUG
  Serial.printf("AiCamera::setRadar(%d, %f)\n", angle, distance);
  #endif
  if (this->sendBuffer.containsKey(region)) {
    this->sendBuffer[region][0] = angle;
    this->sendBuffer[region][1] = distance;
  } else {
    JsonArray data = this->sendBuffer.createNestedArray(region);
    data.add(angle);
    data.add(distance);
  }
}

void AiCamera::setGreyscale(const char* region, int value1, int value2, int value3) {
  JsonArray data = this->sendBuffer.createNestedArray(region);
  data.add(value1);
  data.add(value2);
  data.add(value3);
}

void AiCamera::setValue(const char* region, double value) {
  this->sendBuffer[region] = value;
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

void AiCamera::concat(char* str1, char str2) {
  int length1 = strlen(str1);
  str1[length1] = str2;
  str1[length1 + 1] = '\0';
}

void AiCamera::concat(char* str1, char* str2) {
  int length1 = strlen(str1);
  int length2 = strlen(str2);
  for (int i = 0; i < length2; i++) {
    str1[length1 + i] = str2[i];
  }
  str1[length1 + length2] = '\0';
}
