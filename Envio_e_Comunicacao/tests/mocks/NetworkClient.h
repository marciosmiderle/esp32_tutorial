#pragma once

#include <Arduino.h>
#include <Ethernet.h>

#include <deque>

class NetworkClient : public arduino::EthernetClient {
private:
  struct InjectedData {
    String extraResponseAtBeginning;
    int disconnectWhenRemainingNBytes = -1;
  };
  std::deque<InjectedData> extraData;
  bool disconnSimul = false;


public:
  NetworkClient();
  NetworkClient(int fd);

  int fd() const;
  bool connected() const;
  int read() override;
  int peek();
  int available() override;
  void injectResponse(String data, int disconnectWhenRemaining = -1);
  void clear();
};
