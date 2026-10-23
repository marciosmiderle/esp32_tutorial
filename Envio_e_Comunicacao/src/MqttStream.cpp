#include "MqttStream.hpp"
#include "MqttMessage.hpp"

MqttStream::MqttStream(MqttClient& _client) :
  client(_client) {

  client.setCallback([this] (const char* topic, const byte* payload, unsigned int length) {
    if (topicIn == topic) {
      messageIn.write(payload, length);
    }
  });
}

int MqttStream::available() {
  return messageIn.available();
}

int MqttStream::read() {
  return messageIn.read();
}

int MqttStream::peek() {
  return messageIn.peek();
}

size_t MqttStream::write(uint8_t val) {
  if (messageOut.length() == 0 && val == '\n') {
    return 0;
  }

  messageOut += val;

  if (val == '\n') {
    MqttMessage msg(topicOut, messageOut);
    client.sendMessage(msg);
    size_t len = messageOut.length();
    messageOut = "";
    return len;
  }

  return 1;
}
