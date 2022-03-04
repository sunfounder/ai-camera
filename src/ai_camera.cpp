#include "ai_camera.h"

#define CHECK "SunFounder Controller"

AI_Camera::AI_Camera(String _name, String _type) {
  name = _name;
  type = _type;
  send_buffer["Name"] = name;
  send_buffer["Type"] = type;
  send_buffer["Check"] = CHECK;
}

AI_Camera::begin(String _ssid, String _password, int _mode, int _port) {
  set("SSID", _ssid);
  set("PSK", _password);
  set("MODE", _mode);
  set("PORT", _port);
  String ip = self.set("START");
  Serial.print("WebServer started on ws://");
  Serial.print(ip);
  Serial.print(":");
  Serial.println(_port);
}

String AI_Camera::read() {
  String buf = "";
  char inChar;
  int temp;
  unsigned long timeoutStart = millis();
  while (Serial.available() && millis() - timeoutStart < SERIAL_TIMEOUT) {
    temp = Serial.read();
    inChar = (char)temp;
    if (inChar == '\n') {
      break;
    } else if (inChar == '\r') {
      continue;
    } else if ((int)inChar != 255) {
      buf += inChar;
    }
  }
  if (buf.startswith("[DEBUG] ") {
    buf = buf.replace("[DEBUG]", "[ESP_AI_CAM]");
    Serial.println(buf);
    return "";
  } else {
    return buf;
  }
}

void AI_Camera::write(String value) { Serial.println(value); }

void AI_Camera::send_data() {
  uint8_t* payload;
  send_buffer.clear();
  deserializeJson(send_buffer, payload);
  _command("WS", payload);
}

void AI_Camera::_command(String mode, String command, String value) {
  String data = mode + "+" + command + (char)value;
  write(data);
}

void AI_Camera::_command(String mode, String command) {
  String data = mode + "+" + command;
  write(data);
}

String AI_Camera::set(String command, String value) {
  _command("SET", command, value);
  while (1) {
    String result = read();
    if (result.startsWith("[ERROR]")) {
      Serial.println(result);
      while (1)
        ;
      ;
    }
    if (result.startsWith("[OK]")) {
      result = result.substring(4);
      result.trim();
      break;
    }
  }
  return result;
}

void AI_Camera::set(String command) {
  _command("SET", command);
  while (1) {
    String result = read();
    if (result.startsWith("[ERROR]")) {
      Serial.println(result);
      while (1)
        ;
    }
    if (result.startsWith("[OK]")) {
      result = result.substring(4);
      result.trim();
      break;
    }
  }
}

void AI_Camera::set_on_received(void (*func)()) { __on_receive__ = func; }

void AI_Camera::loop() {
  String receive = read();
  if (receive.length() == 0) {
  } else if (receive.startsWith("[CONNECTED]")) {
    Serial.println("Connected from " + receive.substring(11));
  } else if (receive.startsWith("[DISCONNECTED]")) {
    Serial.println("Disconnected from " + receive.substring(14));
    return;
  } else {
    recv_buffer.clear();
    serializeJson(recv_buffer, receive);
    if (__on_receive__ != NULL) {
      __on_receive__();
    }
  }
  send_data();
}

int AI_Camera::getSlider(const char* region) {
  int value = recvBuf[region];
  return value;
}

int AI_Camera::getButton(const char* region) {
  int value = recvBuf[region];
  return value;
}

bool AI_Camera::getSwitch(const char* region) {
  bool value = recvBuf[region];
  return value;
}

int AI_Camera::getJoystick(const char* region, int axis) {
  int value = recvBuf[region][axis];
  return value;
}

int AI_Camera::getDPad(const char* region) {
  const char* value = recvBuf[region];
  int result;
  if ((String)value == (String)"forward") result = DPAD_FORWARD;
  else if ((String)value == (String)"backward") result = DPAD_BACKWARD;
  else if ((String)value == (String)"left") result = DPAD_LEFT;
  else if ((String)value == (String)"right") result = DPAD_RIGHT;
  else if ((String)value == (String)"stop") result = DPAD_STOP;
  else result = -1;
  return result;
}

int AI_Camera::getThrottle(const char* region) {
  int value = recvBuf[region];
  return value;
}

void AI_Camera::setMeter(const char* region, double value) {
  sendBuf[region] = value;
}

void AI_Camera::setRadar(const char* region, int angle, double distance) {
  #ifdef MC_DEBUG
  Serial.printf("AI_Camera::setRadar(%d, %f)\n", angle, distance);
  #endif
  if (sendBuf.containsKey(region)) {
    sendBuf[region][0] = angle;
    sendBuf[region][1] = distance;
  } else {
    JsonArray data = sendBuf.createNestedArray(region);
    data.add(angle);
    data.add(distance);
  }
}

void AI_Camera::setGreyscale(const char* region, int value1, int value2, int value3) {
  JsonArray data = sendBuf.createNestedArray(region);
  data.add(value1);
  data.add(value2);
  data.add(value3);
}

void AI_Camera::setValue(const char* region, double value) {
  sendBuf[region] = value;
}
