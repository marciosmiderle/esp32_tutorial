#include "MqttStream.hpp"
#include "MqttMessage.hpp"

MqttStream::MqttStream(MqttClient& _client) :
  client(_client) {

  client.addSubscribeTopic(MQTT_STREAM_TOPIC_IN);
  client.setCallback([this] (const char* topic, const byte* payload, unsigned int length) {
    const String fqtn = client.getFullQualifiedTopicName(MQTT_STREAM_TOPIC_IN);
    if (fqtn == topic) {
      messageIn.write(payload, length);
    }
  });
}

MqttStream::~MqttStream() {}

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

  messageOut += static_cast<char>(val);

  if (val == '\n') {
    MqttMessage msg(MQTT_STREAM_TOPIC_OUT, messageOut);
    client.sendMessage(msg);
    size_t len = messageOut.length();
    messageOut = "";
    return len;
  }

  return 1;
}

String MqttStream::getMessageOut() const {
  return messageOut;
}
