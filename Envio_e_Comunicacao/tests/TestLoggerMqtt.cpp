#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "LoggerMqtt.hpp"
#include "MqttClient.hpp"

// Redefinição do Mock para usar CppUMock
class MqttClientMock : public MqttClient {
public:
    MqttClientMock() : MqttClient("", 0, "", "", "", "", "", "") {}
    bool publishLog(const char* data, size_t length) override {
        mock()
            .actualCall("publishLog")
            .withStringParameter("data", data)
            .withParameter("length", (int)length);
        return mock().boolReturnValue();
    }
};

TEST_GROUP(LoggerMqttTest)
{
    MqttClientMock* mqttMock;
    LoggerMqtt* logger;

    void setup()
    {
        mqttMock = new MqttClientMock();
        logger = new LoggerMqtt(*mqttMock);
    }

    void teardown()
    {
        delete logger;
        delete mqttMock;
        mock().clear();
    }
};

TEST(LoggerMqttTest, SimpleWriteAndPublish)
{
    const char* msg = "hello";
    logger->write((const uint8_t*)msg, 5);

    mock().expectOneCall("publishLog")
        .withStringParameter("data", "hello")
        .withParameter("length", 5)
        .andReturnValue(true);

    logger->update();
    CHECK_EQUAL(logger->getUnsentBuffer().length(), 0);

    mock().checkExpectations();
}

TEST(LoggerMqttTest, BufferOverflowFIFO)
{
    char fill[2041];
    for(int i=0; i<2040; i++) fill[i] = 'A';
    fill[2040] = '\0';
    fill[0] = 'B';
    fill[1] = 'B';

    const String& b = logger->getUnsentBuffer();

    logger->write((const uint8_t*)fill, 2040);

    CHECK_EQUAL(b.length(), 2040);
    STRCMP_EQUAL(b.substring(0, 5).c_str(), "BBAAA");
    
    logger->write((const uint8_t*)"1234567890", 10);

    CHECK_EQUAL(b.length(), 2048);
    STRCMP_EQUAL(b.substring(0, 5).c_str(), "AAAAA");
    STRCMP_EQUAL(b.substring(b.length() - 10).c_str(), "1234567890");

    mock().expectOneCall("publishLog")
        .withParameter("length", 2048)
        .ignoreOtherParameters()
        .andReturnValue(true);

    logger->update();
    CHECK_EQUAL(b.length(), 0);

    mock().checkExpectations();
}

TEST(LoggerMqttTest, ChunkLargerThanBuffer)
{
    uint8_t large[3000];
    for(int i=0; i<3000; i++) large[i] = (i < 3000 - 5) ? 'X' : 'Y';

    logger->write(large, 3000);
    const String& b = logger->getUnsentBuffer();
    CHECK_EQUAL(b.length(), 2048);
    STRCMP_EQUAL(b.substring(b.length() - 5).c_str(), "YYYYY");

    mock().expectOneCall("publishLog")
        .withParameter("length", 2048)
        .ignoreOtherParameters()
        .andReturnValue(true);

    logger->update();
    CHECK_EQUAL(b.length(), 0);

    mock().checkExpectations();
}
