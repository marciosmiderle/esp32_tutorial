#include "mocks/millis.h"

#include "HttpClientLocal.hpp"
#include "Message.hpp"
#include "HttpTestServer.hpp"
#include "util.hpp"

#include "CppUTest/TestHarness.h"

static HttpRequest makeRequest(int n) {
  Message m;
  m.timestamp = (unsigned long)n;
  m.temperature = 20.0f + (float)n;
  return HttpRequest("GET", m.toJson());
}

//static String bodyOf(int n) { return makeRequest(n).toJson(); }

TEST_GROUP(HttpClientLocal) {
  static constexpr const char* HOSTNAME = "localhost";
  static const           int   PORT     = HttpTestServer::PORT;
  static constexpr const char* URI      = "/api/data";
  // static constexpr const char* HOSTNAME = "httpbin.org";
  // static const           int   PORT     = 80;
  // static constexpr const char* URI      = "/post";

  // millis() só anda com millis_forward().
  // Começamos em 10000 ("uptime") porque o RetryLogic zera lastAttemptMs para 0:
  static const unsigned long START_MS = 10000;
  static const size_t maxQueue = 3;
  static const int maxRetries = 3;


  NetworkClient* nc = nullptr;
  HttpClientLocal* client = nullptr;
  HttpTestServer server;

  void setup() {
    millis_set(START_MS);  // sem isso um teste herda o tempo do anterior
    recreate(maxQueue, maxRetries);

    server.response = HttpTestServer::okResponse();
    server.closeWithoutResponse = false;
    server.delayResponse = false;
    server.request.clear();
    server.start();
  }

  void teardown() {
    delete client;
    client = nullptr;
    server.shutdownNow();
    millis_set(START_MS);
  }

  // Padrão: retryTimeout = 0 (backoff desligado) e tryLater longo.
  // Os testes de backoff passam timeouts reais e avançam o tempo com millis_forward().
  void recreate(size_t _maxQueue, int _maxRetries = 3,
                unsigned long retryTimeoutMs = 2000, unsigned long tryLaterMs = 30000) {
    delete client;
    client = new HttpClientLocal(HOSTNAME, PORT, URI, _maxQueue, _maxRetries, retryTimeoutMs, tryLaterMs);
    nc = &client->getNetworkClient();
  }

  void updateN(int n) {
    for (int i = 0; i < n; i++) client->update();
  }

  void runUntilQueueDrained() {
    while_timed(std::chrono::milliseconds(5000), [this]() {
      client->update();
      if (client->pending() == 0)
        return false;
      return true;
    });
  }

  // Como runUntilQueueDrained, mas tolera mensagens que ficam na fila (5xx/backoff/tryLater).
  void runUntilFakeIdle(int maxIter = 200) {
    for (int i = 0; i < maxIter; i++) {
      client->update();
      if (client->getPhase() == Phase::Idle) return;
    }
    FAIL("transporte não voltou a Idle dentro do limite de iterações");
  }

  void wait_until_client_phase_is(Phase ph, unsigned long timeout_ms = 1000) {
    while_timed(std::chrono::milliseconds(timeout_ms), [this, &ph]() {
      client->update();
      if (client->getPhase() == ph) {
        return false;
      }
      return true;
    });
    millis_forward(100);
  }

  SimpleString StringFrom(Phase ph) {
    switch(ph) {
    case Phase::Idle:       return "Phase::Idle";
    case Phase::Connecting: return "Phase::Connecting";
    case Phase::Ready:      return "Phase::Ready";
    case Phase::Sending:    return "Phase::Sending";
    case Phase::Receiving:  return "Phase::Receiving";
    case Phase::Done:       return "Phase::Done";
    default:
    case Phase::Failed:     return "Phase::Failed";
    };
  }
};

// ----------------------------------------------------------------

TEST(HttpClientLocal, send_apenas_enfileira) {
  client->start();
  CHECK_TRUE(client->send(makeRequest(1)));
  CHECK_TRUE(client->send(makeRequest(2)));

  CHECK_TRUE(2 == client->pending());
}

TEST(HttpClientLocal, sem_start_update_nao_faz_nada) {
  client->send(makeRequest(1));
  updateN(5);

  CHECK_TRUE(1 == client->pending());
}

TEST(HttpClientLocal, fila_cheia_descarta_a_mais_antiga) {
  for (int i = 0; i < maxQueue + 1; i++)
    client->send(makeRequest(i));

  CHECK_TRUE(maxQueue == client->pending());
  CHECK_TRUE(1 == client->dropped());

  client->start();
  runUntilQueueDrained();

  CHECK_TRUE(0 == client->pending());
}

TEST(HttpClientLocal, max_queue_zero_send_retorna_false) {
  recreate(0, 3);
  CHECK_FALSE(client->send(makeRequest(1)));
  LONGS_EQUAL(0, client->pending());
}

// ------------------------------------------------------------ falhas / retry

// TEST(HttpClientLocal, FalhasDeRedeEsgotamRetries_EntraEmTryLater) {
//   fake.pushFail("conexao recusada");
//   fake.pushFail("conexao recusada");
//   fake.pushFail("conexao recusada");
//   client->start();
//   client->send(makeRequest(1));

//   updateN(20);

//   LONGS_EQUAL(3, fake.beginCount);   // maxRetries = 3
//   LONGS_EQUAL(1, client->pending()); // mensagem preservada
//   CHECK_FALSE(client->isConnected());
//   STRCMP_EQUAL("conexao recusada", client->getLastResponse());

//   updateN(50);  // em tryLater: nenhuma tentativa nova
//   LONGS_EQUAL(3, fake.beginCount);
// }

// TEST(HttpClientLocal, BeginFalhandoSincrono_RegistraErroETentaDeNovo) {
//   fake.pushBeginFailure("DNS falhou");
//   client->start();
//   client->send(makeRequest(1));

//   client->update();
//   LONGS_EQUAL(1, fake.beginCount);
//   CHECK_FALSE(client->isConnected());
//   STRCMP_EQUAL("DNS falhou", client->getLastResponse());
//   LONGS_EQUAL(1, client->pending());

//   runUntilQueueDrained();
//   LONGS_EQUAL(2, fake.beginCount);
//   LONGS_EQUAL(0, client->pending());
//   CHECK_TRUE(client->isConnected());
// }

// // ------------------------------------------------- backoff / tryLater (tempo)

// TEST(HttpClientLocal, Backoff_5xxEsperaORetryTimeoutAntesDeReenviar) {
//   recreate(3, 3, 2000, 30000);
//   fake.pushDone(503);
//   client->start();
//   client->send(makeRequest(1));

//   updateN(5);  // 1a tentativa + resposta 503
//   LONGS_EQUAL(1, fake.beginCount);
//   LONGS_EQUAL(1, client->pending());

//   millis_forward(1999);
//   updateN(5);
//   LONGS_EQUAL(1, fake.beginCount);  // ainda dentro do backoff

//   millis_forward(1);  // exatamente 2000 ms desde a tentativa
//   runUntilQueueDrained();
//   LONGS_EQUAL(2, fake.beginCount);
//   LONGS_EQUAL(0, client->pending());
// }

// TEST(HttpClientLocal, TryLaterExpira_ERetomaOEnvio) {
//   recreate(3, 3, 1000, 30000);  // 3 tentativas, backoff 1 s, tryLater 30 s
//   fake.pushFail("sem rede");
//   fake.pushFail("sem rede");
//   fake.pushFail("sem rede");
//   // a 4a tentativa cai no padrão do fake: Done(200)
//   client->start();
//   client->send(makeRequest(1));

//   updateN(3);                         // t=10000: tentativa 1
//   LONGS_EQUAL(1, fake.beginCount);

//   millis_forward(999);
//   updateN(3);                         // 999 ms < retryTimeout: nada
//   LONGS_EQUAL(1, fake.beginCount);

//   millis_forward(1);
//   updateN(3);                         // t=11000: tentativa 2
//   LONGS_EQUAL(2, fake.beginCount);

//   millis_forward(1000);
//   updateN(3);                         // t=12000: tentativa 3 -> esgota -> tryLater até 42000
//   LONGS_EQUAL(3, fake.beginCount);
//   CHECK_FALSE(client->isConnected());

//   millis_forward(29999);
//   updateN(3);                         // t=41999: ainda em tryLater
//   LONGS_EQUAL(3, fake.beginCount);
//   LONGS_EQUAL(1, client->pending());

//   millis_forward(1);
//   updateN(3);                         // t=42000: tryLater expira, reset, tentativa 4
//   LONGS_EQUAL(4, fake.beginCount);
//   LONGS_EQUAL(0, client->pending());
//   CHECK_TRUE(client->isConnected());
// }

TEST(HttpClientLocal, stop_preserva_fila_start_retoma) {
  client->start();
  client->send(makeRequest(1));
  client->send(makeRequest(2));
  CHECK_EQUAL(Phase::Idle, client->getPhase());

  wait_until_client_phase_is(Phase::Ready);
  CHECK_EQUAL(Phase::Ready, client->getPhase());
  client->stop();
  CHECK_TRUE(2 == client->pending());
  CHECK_FALSE(client->isConnected());

  client->start();
  CHECK_TRUE(2 == client->pending());
  runUntilQueueDrained();
  CHECK_TRUE(0 == client->pending());
}

TEST(HttpClientLocal, release_from_queued_after_receiving) {
  client->start();
  client->send(makeRequest(4321));
  client->send(makeRequest(9876));
  wait_until_client_phase_is(Phase::Ready);
  CHECK_EQUAL(Phase::Ready, client->getPhase());
  CHECK_TRUE(2 == client->pending());

  server.response = HttpTestServer::okResponse();
  client->update(); // *ready -> sending
  client->update(); // *sending -> receiving
  CHECK_EQUAL(Phase::Receiving, client->getPhase());
  CHECK_TRUE(2 == client->getContentLengh());
  CHECK_TRUE(HTTP_CODE_OK == client->getHttpCode());

  CHECK_TRUE(2 == client->pending());
  client->update(); // -> ready
  CHECK_TRUE(1 == client->pending());
}

TEST(HttpClientLocal, no_release_from_queued_on_server_busy) {
  client->start();
  client->send(makeRequest(5678));
  wait_until_client_phase_is(Phase::Ready);
  CHECK_EQUAL(Phase::Ready, client->getPhase());
  CHECK_TRUE(1 == client->pending());

  server.response = HttpTestServer::nokResponse503();
  client->update(); // *ready -> sending
  client->update(); // *sending -> receiving

  CHECK_TRUE(HTTPC_ERROR_CONNECTION_LOST == client->getRequestReturn());
  CHECK_TRUE(503 == client->getHttpCode());
  CHECK_TRUE(0 == client->getContentLengh());

  CHECK_EQUAL(Phase::Receiving, client->getPhase());
  client->update(); // *receiving -> idle
  CHECK_EQUAL(Phase::Idle, client->getPhase());

  CHECK_TRUE(1 == client->pending());
}

TEST(HttpClientLocal, rejeicao4xx_descarta_e_continua_a_fila) {
  client->start();
  client->send(makeRequest(5678));
  client->send(makeRequest(8765));
  server.response = HttpTestServer::nokResponse400();
  wait_until_client_phase_is(Phase::Ready, 5000);
  CHECK_EQUAL(Phase::Ready, client->getPhase()); // before consume 400 code
  CHECK_TRUE(2 == client->pending());
  wait_until_client_phase_is(Phase::Ready, 5000);
  CHECK_EQUAL(Phase::Ready, client->getPhase()); // after consume 400 code
  CHECK_TRUE(1 == client->pending());

  server.response = HttpTestServer::okResponse();
  wait_until_client_phase_is(Phase::Ready);
  CHECK_EQUAL(Phase::Ready, client->getPhase());
  CHECK_TRUE(0 == client->pending());
}

TEST(HttpClientLocal, connection_phases) {
  CHECK_EQUAL(Phase::Idle, client->getPhase());
  client->update();
  CHECK_EQUAL(Phase::Idle, client->getPhase());
  client->start();
  CHECK_EQUAL(Phase::Idle, client->getPhase());
  client->send(makeRequest(1234));
  client->update();
  CHECK_EQUAL(Phase::Resolving, client->getPhase());

  wait_until_client_phase_is(Phase::Connecting);
  CHECK_EQUAL(Phase::Connecting, client->getPhase());

  wait_until_client_phase_is(Phase::Ready);
  CHECK_EQUAL(Phase::Ready, client->getPhase());

  CHECK_EQUAL(Phase::Ready, client->getPhase());
  server.response = HttpTestServer::okResponse();
  client->update();
  CHECK_EQUAL(Phase::Sending, client->getPhase());
  client->update();
  CHECK_EQUAL(Phase::Receiving, client->getPhase());
  CHECK_TRUE(2 == client->getContentLengh());
  client->update();
  CHECK_EQUAL(Phase::Ready, client->getPhase());

  client->stop();
  CHECK_EQUAL(Phase::Ready, client->getPhase());
  client->update();
  CHECK_EQUAL(Phase::Idle, client->getPhase());
}

TEST(HttpClientLocal, PostRecebidoEStatus200Lido) {
  client->start();
  client->send(makeRequest(4321));
  wait_until_client_phase_is(Phase::Ready);
  CHECK_EQUAL(Phase::Ready, client->getPhase());
  server.response = HttpTestServer::okResponse();
  client->update(); // *ready -> sending
  client->update(); // *sending -> receiving
  CHECK_EQUAL(Phase::Receiving, client->getPhase());
  CHECK_TRUE(2 == client->getContentLengh());
  client->update(); // ready
  CHECK_EQUAL(Phase::Ready, client->getPhase());
  CHECK_TRUE(HTTP_CODE_OK == client->getHttpCode());
}
