#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "NetManager.hpp"
#include "mocks/millis.h"
#include <NetworkManager.h>

using Event = NetworkManagerClass::Event;

static bool cbConnectingCalled;
static bool cbConnectedCalled;
static bool cbDisconnectedCalled;
static NetworkManagerClass& nm = NetworkManagerClass::instance();

static int startCount;
static int stopCount;
static int connectedCount;
static int disconnectedCount;
static int fallbackCount;
static int restoredCount;

TEST_GROUP(NetManagerTest)
{
    const int maxRetries = 5;
    const unsigned long retryTimeoutMs = 3000;
    const unsigned long tryLaterTimeoutMs = 30000;
    NetManager& netManager = NetManager::getInstance(maxRetries, retryTimeoutMs, tryLaterTimeoutMs);
    NetworkAdapter adapter;

    void resetCounters() {
        cbConnectingCalled = false;
        cbConnectedCalled = false;
        cbDisconnectedCalled = false;
        startCount = stopCount = 0;
        connectedCount = disconnectedCount = fallbackCount = restoredCount = 0;
    }

    void setupCountersCBs()
    {
        netManager.startEvent = []() { startCount++; };
        netManager.stopEvent = []() { stopCount++; };
        netManager.connectedEvent = []() { connectedCount++; };
        netManager.disconnectedEvent = []() { disconnectedCount++; };
        netManager.connectionFallbackEvent = []() { fallbackCount++; };
        netManager.connectionRestoreEvent = []() { restoredCount++; };
    }

    void setup()
    {
        millis_set(0);
        resetCounters();
        setupCountersCBs();
    }

    void teardown()
    {
        mock().clear();
    }

    void simulate(Event e) { nm.simulateEvent(e, adapter); }

    void startManager()
    {
        resetManager();

        mock().expectOneCall("NetworkManagerClass::begin");
        millis_forward(retryTimeoutMs);
        netManager.start();
    }

    void resetManager()
    {
        mock().expectOneCall("NetworkManagerClass::end");
        netManager.stop();
        millis_set(0);
        resetCounters();
        setupCountersCBs();
    }

    // Um ciclo de update() com a rede fora do ar.
    void updateDisconnected(bool expectAttempt)
    {
        mock().expectOneCall("NetworkManagerClass::isConnected").andReturnValue(false);
        if (expectAttempt) {
            mock().expectOneCall("NetworkManagerClass::begin");
        }
        mock().expectOneCall("NetworkManagerClass::update");
        netManager.update();
    }

    // Depois de startManager() (tentativa 1): gasta as tentativas 2..maxRetries.
    void consumeRemainingAttempts()
    {
        for (int i = 2; i <= maxRetries; i++) {
            millis_forward(retryTimeoutMs);
            updateDisconnected(true);
        }
        CHECK_EQUAL(maxRetries, startCount);
    }

    // Esgota as tentativas, dispara o evento e confirma que voltou a poder tentar.
    void checkEventRestoresRetryBudget(Event ev)
    {
        startManager();
        consumeRemainingAttempts();

        simulate(ev);

        millis_forward(retryTimeoutMs);
        updateDisconnected(true);
        mock().checkExpectations();
    }

    void checkEvents(Event ev, int connected, int disconnected, int fallback, int restored)
    {
        startManager();
        simulate(ev);

        CHECK_EQUAL(connected, connectedCount);
        CHECK_EQUAL(disconnected, disconnectedCount);
        CHECK_EQUAL(fallback, fallbackCount);
        CHECK_EQUAL(restored, restoredCount);
    }
};

TEST(NetManagerTest, SuccessfulAsyncConnectionFlow)
{
    resetManager();

    netManager.startEvent = []() { cbConnectingCalled = true; };
    netManager.connectedEvent  = []() { cbConnectedCalled = true; };

    mock().expectOneCall("NetworkManagerClass::begin");
    netManager.start();
    CHECK_FALSE(cbConnectingCalled);

    millis_forward(retryTimeoutMs - 1);
    netManager.start();
    CHECK_FALSE(cbConnectingCalled);

    mock().expectOneCall("NetworkManagerClass::isConnected").andReturnValue(false);
    mock().expectOneCall("NetworkManagerClass::update");
    millis_forward(1);
    netManager.update();
    CHECK_TRUE(cbConnectingCalled);

    NetworkAdapter adapter;
    nm.simulateEvent(NetworkManagerClass::Event::CONNECTED, adapter);
    CHECK_TRUE(cbConnectedCalled);

    mock().checkExpectations();
}

TEST(NetManagerTest, startStopEventCounts)
{
    startManager();
    netManager.start();
    mock().expectOneCall("NetworkManagerClass::end");
    netManager.stop();

    CHECK_EQUAL(1, startCount);
    CHECK_EQUAL(1, stopCount);
    mock().checkExpectations();
}

TEST(NetManagerTest, UpdateBeforeBeginDoesNothing)
{
    resetManager();
    netManager.update();

    CHECK_TRUE(0 == startCount);
    mock().checkExpectations();
}

TEST(NetManagerTest, UpdateWhileConnectedOnlyUpdateNetworkManager)
{
    startManager();

    mock().expectOneCall("NetworkManagerClass::isConnected").andReturnValue(true);
    mock().expectOneCall("NetworkManagerClass::update");
    netManager.update();

    CHECK_EQUAL(1, startCount);  // conectado: nenhuma nova tentativa
    mock().checkExpectations();
}

TEST(NetManagerTest, UpdateRetriesOnlyAfterRetryTimeout)
{
    startManager();

    millis_forward(retryTimeoutMs - 1);
    updateDisconnected(false);
    CHECK_EQUAL(1, startCount);

    millis_forward(1);
    updateDisconnected(true);
    CHECK_EQUAL(2, startCount);

    mock().checkExpectations();
}

TEST(NetManagerTest, StopsAttemptingAfterMaxRetriesUntilTryLaterExpires)
{
    startManager();
    consumeRemainingAttempts();

    millis_forward(retryTimeoutMs);
    updateDisconnected(false);                 // sem retries, entra no tryLater
    millis_forward(tryLaterTimeoutMs - 1);
    updateDisconnected(false);                 // ainda no tryLater
    CHECK_EQUAL(maxRetries, startCount);

    millis_forward(1);
    updateDisconnected(true);                  // tryLater expirou: volta a tentar
    CHECK_EQUAL(maxRetries + 1, startCount);

    mock().checkExpectations();
}

TEST(NetManagerTest, CheckEventRestoresRetryBudget)
{
    checkEventRestoresRetryBudget(Event::CONNECTED);
    checkEventRestoresRetryBudget(Event::DISCONNECTED);
    checkEventRestoresRetryBudget(Event::FALLBACK);
    checkEventRestoresRetryBudget(Event::RESTORED);
}

TEST(NetManagerTest, networkEventFiresNetManagerCB)
{
    checkEvents(Event::CONNECTED, /*connected*/1, /*disconnected*/0, /*fallback*/0, /*restored*/0);
    checkEvents(Event::DISCONNECTED, /*connected*/0, /*disconnected*/1, /*fallback*/0, /*restored*/0);
    checkEvents(Event::FALLBACK, /*connected*/0, /*disconnected*/0, /*fallback*/1, /*restored*/0);
    checkEvents(Event::RESTORED, /*connected*/0, /*disconnected*/0, /*fallback*/0, /*restored*/1);

    mock().checkExpectations();
}

TEST(NetManagerTest, StopNotifiesAndHaltsUpdates)
{
    startManager();

    mock().expectOneCall("NetworkManagerClass::end");
    netManager.stop();
    CHECK_EQUAL(1, stopCount);

    netManager.update();  // parado: nenhuma chamada ao NetworkManager
    mock().checkExpectations();
}

TEST(NetManagerTest, StopThenBeginRestartsWithFreshRetryBudget)
{
    startManager();
    consumeRemainingAttempts();

    mock().expectOneCall("NetworkManagerClass::end");
    netManager.stop();

    mock().expectOneCall("NetworkManagerClass::begin");
    netManager.start();

    CHECK_EQUAL(maxRetries + 1, startCount);
    mock().checkExpectations();
}

TEST(NetManagerTest, EventsWithoutHandlersAreIgnored)
{
    startManager();
    netManager.startEvent = [](){};
    netManager.stopEvent = [](){};
    netManager.connectedEvent = [](){};
    netManager.disconnectedEvent = [](){};
    netManager.connectionFallbackEvent = [](){};
    netManager.connectionRestoreEvent = [](){};

    CHECK_EQUAL(0, connectedCount);
    simulate(Event::CONNECTED);
    CHECK_EQUAL(0, connectedCount);

    CHECK_EQUAL(0, disconnectedCount);
    simulate(Event::DISCONNECTED);
    CHECK_EQUAL(0, disconnectedCount);

    CHECK_EQUAL(0, fallbackCount);
    simulate(Event::FALLBACK);
    CHECK_EQUAL(0, fallbackCount);

    CHECK_EQUAL(0, restoredCount);
    simulate(Event::RESTORED);
    CHECK_EQUAL(0, restoredCount);

    mock().expectOneCall("NetworkManagerClass::end");
    netManager.stop();

    mock().checkExpectations();
}
