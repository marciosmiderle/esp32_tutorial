#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "MqttClient.hpp"

class MqttClientMock : public MqttClient {
public:
    MqttClientMock() : MqttClient("", 0, "") {}
    virtual ~MqttClientMock() {};

    bool sendMessage(const MqttMessage& msg) override {
	mock()
	    .actualCall("sendMessage")
	    .withParameter("msg.topic", msg.topic)
	    .withParameter("msg.content", msg.content)
	    .withParameter("msg.length", msg.content.length());
	return mock().boolReturnValue();
    }
};
