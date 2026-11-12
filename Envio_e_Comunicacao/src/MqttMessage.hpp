#pragma once

#include <Arduino.h>
#include <map>

class MqttMessage {
public:
  String topic;
  String content;

  MqttMessage(String _topic, String _content) : topic(_topic), content(_content) {};
};
