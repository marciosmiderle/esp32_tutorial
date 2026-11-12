#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "MqttStream.hpp"
#include "MqttClientMock.hpp"

TEST_GROUP(MqttStreamTest)
{
    MqttClientMock* mqttMock;
    MqttStream* stream;

    void setup()
    {
        mqttMock = new MqttClientMock();
        stream = new MqttStream(*mqttMock);
    }

    void teardown()
    {
        delete stream;
        delete mqttMock;
        mock().clear();
    }
};

TEST(MqttStreamTest, simple_write_and_send)
{
    const char* content = "hello";
    stream->write((const uint8_t*)content, 5);

    CHECK_EQUAL(5, stream->getMessageOut().length());

    MqttMessage msg(MQTT_STREAM_TOPIC_OUT, content);
    mock().expectOneCall("sendMessage")
        .withParameter("msg.topic", msg.topic)
        .withParameter("msg.content", msg.content)
        .withParameter("msg.length", msg.content.length() + 1)
        .andReturnValue(true);

    stream->write('\n');

    CHECK_EQUAL(0, stream->getMessageOut().length());

    mock().checkExpectations();
}

// TEST(MqttStreamTest, BufferOverflowFIFO)
// {
//     char fill[2041];
//     for(int i=0; i<2040; i++) fill[i] = 'A';
//     fill[2040] = '\0';
//     fill[0] = 'B';
//     fill[1] = 'B';

//     const String& b = stream->getUnsentBuffer();

//     stream->write((const uint8_t*)fill, 2040);

//     CHECK_EQUAL(b.length(), 2040);
//     STRCMP_EQUAL(b.substring(0, 5).c_str(), "BBAAA");
    
//     stream->write((const uint8_t*)"1234567890", 10);

//     CHECK_EQUAL(b.length(), 2048);
//     STRCMP_EQUAL(b.substring(0, 5).c_str(), "AAAAA");
//     STRCMP_EQUAL(b.substring(b.length() - 10).c_str(), "1234567890");

//     mock().expectOneCall("sendMessage")
//         .withParameter("msg.length", 2048)
//         .ignoreOtherParameters()
//         .andReturnValue(true);

//     stream->update();
//     CHECK_EQUAL(b.length(), 0);

//     mock().checkExpectations();
// }

// TEST(MqttStreamTest, ChunkLargerThanBuffer)
// {
//     uint8_t large[3000];
//     for(int i=0; i<3000; i++) large[i] = (i < 3000 - 5) ? 'X' : 'Y';

//     stream->write(large, 3000);
//     const String& b = stream->getUnsentBuffer();
//     CHECK_EQUAL(b.length(), 2048);
//     STRCMP_EQUAL(b.substring(b.length() - 5).c_str(), "YYYYY");

//     mock().expectOneCall("sendMessage")
//         .withParameter("msg.length", 2048)
//         .ignoreOtherParameters()
//         .andReturnValue(true);

//     stream->update();
//     CHECK_EQUAL(b.length(), 0);

//     mock().checkExpectations();
// }
