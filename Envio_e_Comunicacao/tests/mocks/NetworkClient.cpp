#include "NetworkClient.h"

NetworkClient::NetworkClient() : EthernetClient(), disconnSimul(false) {}

NetworkClient::NetworkClient(int fd) : EthernetClient(fd), disconnSimul(false) {}

int NetworkClient::fd() const {
  auto t = const_cast<NetworkClient*>(this);
  return dynamic_cast<EthernetClient*>(t)->fd();
}

bool NetworkClient::connected() const {
  if (disconnSimul) return false;

  auto t = const_cast<NetworkClient*>(this);
  return dynamic_cast<EthernetClient*>(t)->connected();
}

int NetworkClient::read() {
  int data;

  if (extraData.empty()) {
    data = arduino::EthernetClient::read();
  } else {
    String& f = extraData.front().extraResponseAtBeginning;
    data = f[0];
    f.remove(0, 1);

    if (f.length() == extraData.front().disconnectWhenRemainingNBytes) {
      disconnSimul = true;
    }
  }

  return data;
}

int NetworkClient::peek() {
  return extraData.empty() ?
      arduino::EthernetClient::peek() :
      (extraData.front().extraResponseAtBeginning.length() > 0 ? extraData.front().extraResponseAtBeginning[0] : -1);
}

int NetworkClient::available() {
  if (extraData.empty()) {
    return arduino::EthernetClient::available();
  }

  if (disconnSimul) return 0;

  String& f = extraData.front().extraResponseAtBeginning;
  if (f.length() == 0) {
    extraData.pop_front();
    return 0;
  }

  return f.length();
}

void NetworkClient::injectResponse(String data, int disconnectWhenRemaining) {
  extraData.push_back({data, disconnectWhenRemaining});
}

void NetworkClient::clear() {}
