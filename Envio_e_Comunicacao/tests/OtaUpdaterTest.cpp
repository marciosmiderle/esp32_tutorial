#include "mocks/millis.h"

#include "OtaUpdater.hpp"
#include "HttpTestServer.hpp"
#include "util.hpp"

#include "CppUTest/TestHarness.h"

TEST_GROUP(OtaUpdater) {
  using State     = OtaUpdater::State;
  using StepState = OtaUpdater::StepState;

  static constexpr const char* HOSTNAME = "localhost";
  //static const           int   PORT     = HttpTestServer::PORT;
  static const           int   PORT     = 8080;
  static constexpr const char* URI      = "/";
  // static constexpr const char* HOSTNAME = "httpbin.org";
  // static const           int   PORT     = 80;
  // static constexpr const char* URI      = "/post";
  static constexpr const char* firmwareUrl = "http://localhost:8080/tests/fixtures/firmware.bin";

  // millis() só anda com millis_forward().
  // Começamos em 10000 ("uptime") porque o RetryLogic zera lastAttemptMs para 0:
  static const unsigned long START_MS = 10000;

  OtaUpdater* ota = nullptr;
  HttpTestServer server;

  void setup() {
    millis_set(START_MS);  // sem isso um teste herda o tempo do anterior
    recreate();
    server.start();
  }

  void teardown() {
    delete ota;
    ota = nullptr;
    server.stop();
    // millis_set(START_MS);
  }

  void recreate() {
    delete ota;
    ota = new OtaUpdater();
  }

  void updateN(int n) {
    for (int i = 0; i < n; i++) ota->update();
  }

  void wait_until_ota_state_is(State st, unsigned long timeout_ms = 1000) {
    while_timed(timeout_ms, [this, &st]() {
      ota->update();
      if (ota->getState() == st) {
        return false;
      }
      return true;
    });
    millis_forward(100);
  }

  void update_timed_until_ota_step_state_is(StepState st, unsigned long timeout_ms = 1000, unsigned long forward_millis_by_ms = 100) {
    while_timed(timeout_ms, [this, &st, &forward_millis_by_ms]() {
      ota->update();
      millis_forward(forward_millis_by_ms);
      if (ota->getStepState() == st) {
        return false;
      }
      return true;
    });
  }

  SimpleString StringFrom(State st) {
    switch(st) {
    default:
    case State::Idle:               return "State::Idle";
    case State::FetchingHash:       return "State::FetchingHash";
    case State::Downloading:        return "State::Downloading";
    case State::DownloadValidation: return "State::DownloadValidation";
    case State::Reboot:             return "State::Reboot";
    };
  }

  SimpleString StringFrom(StepState st) {
    switch(st) {
    default:
    case StepState::Idle:      return "StepState::Idle";
    case StepState::Starting:  return "StepState::Starting";
    case StepState::Running:   return "StepState::Running";
    case StepState::Running_2: return "StepState::Running_2";
    case StepState::Finished:  return "StepState::Finished";
    case StepState::Failed:    return "StepState::Failed";
    };
  }
};

// ----------------------------------------------------------------

TEST(OtaUpdater, no_start_no_update) {
  CHECK_EQUAL(State::Idle, ota->getState());
  CHECK_FALSE(ota->startUpdate(firmwareUrl));
  CHECK_EQUAL(State::Idle, ota->getState());

  ota->start();

  CHECK_TRUE(ota->startUpdate(firmwareUrl));
  CHECK_EQUAL(State::FetchingHash, ota->getState());
}

TEST(OtaUpdater, happy_path_states) {
  ota->start();

  CHECK_TRUE(ota->startUpdate(firmwareUrl));
  CHECK_EQUAL(State::FetchingHash, ota->getState());
  CHECK_EQUAL(StepState::Starting, ota->getStepState());

  ota->update();
  CHECK_EQUAL(State::FetchingHash, ota->getState());
  CHECK_EQUAL(StepState::Running, ota->getStepState());

  CHECK_FALSE(ota->getDownloadComplete());
  update_timed_until_ota_step_state_is(StepState::Finished);
  CHECK_EQUAL(State::FetchingHash, ota->getState());
  CHECK_EQUAL(StepState::Finished, ota->getStepState());
  CHECK_TRUE(ota->getDownloadComplete());

  ota->update();
  CHECK_EQUAL(State::Downloading, ota->getState());
  CHECK_EQUAL(StepState::Starting, ota->getStepState());

  ota->update();
  CHECK_EQUAL(State::Downloading, ota->getState());
  CHECK_EQUAL(StepState::Running, ota->getStepState());

  CHECK_FALSE(ota->getDownloadComplete());
  update_timed_until_ota_step_state_is(StepState::Running_2);
  CHECK_EQUAL(State::Downloading, ota->getState());
  CHECK_EQUAL(StepState::Running_2, ota->getStepState());

  update_timed_until_ota_step_state_is(StepState::Finished);
  CHECK_EQUAL(State::Downloading, ota->getState());
  CHECK_EQUAL(StepState::Finished, ota->getStepState());
  CHECK_TRUE(ota->getDownloadComplete());

  ota->update();
  CHECK_EQUAL(State::DownloadValidation, ota->getState());
  CHECK_EQUAL(StepState::Starting, ota->getStepState());

  ota->update();
  CHECK_EQUAL(State::DownloadValidation, ota->getState());
  CHECK_EQUAL(StepState::Finished, ota->getStepState());
}

// ------------------------------------------------------------ falhas / retry

// TEST(OtaUpdater, FalhasDeRedeEsgotamRetries_EntraEmTryLater) {
//   fake.pushFail("conexao recusada");
//   fake.pushFail("conexao recusada");
//   fake.pushFail("conexao recusada");
//   ota->start();
//   ota->send(makeRequest(1));

//   updateN(20);

//   LONGS_EQUAL(3, fake.beginCount);   // maxRetries = 3
//   LONGS_EQUAL(1, ota->pending()); // mensagem preservada
//   CHECK_FALSE(ota->isConnected());
//   STRCMP_EQUAL("conexao recusada", ota->getLastResponse());

//   updateN(50);  // em tryLater: nenhuma tentativa nova
//   LONGS_EQUAL(3, fake.beginCount);
// }

// TEST(OtaUpdater, BeginFalhandoSincrono_RegistraErroETentaDeNovo) {
//   fake.pushBeginFailure("DNS falhou");
//   ota->start();
//   ota->send(makeRequest(1));

//   ota->update();
//   LONGS_EQUAL(1, fake.beginCount);
//   CHECK_FALSE(ota->isConnected());
//   STRCMP_EQUAL("DNS falhou", ota->getLastResponse());
//   LONGS_EQUAL(1, ota->pending());

//   runUntilQueueDrained();
//   LONGS_EQUAL(2, fake.beginCount);
//   LONGS_EQUAL(0, ota->pending());
//   CHECK_TRUE(ota->isConnected());
// }

// // ------------------------------------------------- backoff / tryLater (tempo)

// TEST(OtaUpdater, Backoff_5xxEsperaORetryTimeoutAntesDeReenviar) {
//   recreate(3, 3, 2000, 30000);
//   fake.pushDone(503);
//   ota->start();
//   ota->send(makeRequest(1));

//   updateN(5);  // 1a tentativa + resposta 503
//   LONGS_EQUAL(1, fake.beginCount);
//   LONGS_EQUAL(1, ota->pending());

//   millis_forward(1999);
//   updateN(5);
//   LONGS_EQUAL(1, fake.beginCount);  // ainda dentro do backoff

//   millis_forward(1);  // exatamente 2000 ms desde a tentativa
//   runUntilQueueDrained();
//   LONGS_EQUAL(2, fake.beginCount);
//   LONGS_EQUAL(0, ota->pending());
// }

// TEST(OtaUpdater, TryLaterExpira_ERetomaOEnvio) {
//   recreate(3, 3, 1000, 30000);  // 3 tentativas, backoff 1 s, tryLater 30 s
//   fake.pushFail("sem rede");
//   fake.pushFail("sem rede");
//   fake.pushFail("sem rede");
//   // a 4a tentativa cai no padrão do fake: Done(200)
//   ota->start();
//   ota->send(makeRequest(1));

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
//   CHECK_FALSE(ota->isConnected());

//   millis_forward(29999);
//   updateN(3);                         // t=41999: ainda em tryLater
//   LONGS_EQUAL(3, fake.beginCount);
//   LONGS_EQUAL(1, ota->pending());

//   millis_forward(1);
//   updateN(3);                         // t=42000: tryLater expira, reset, tentativa 4
//   LONGS_EQUAL(4, fake.beginCount);
//   LONGS_EQUAL(0, ota->pending());
//   CHECK_TRUE(ota->isConnected());
// }

// TEST(OtaUpdater, PostRecebidoEStatus200Lido) {
  // ota->start();
  // ota->send(makeRequest(4321));
  // String body;
  // ota->onData = [this, &body](bool& isComplete) {
  //   body += ota->getContent();
  //   isComplete = true;
  // };

  // wait_until_ota_state_is(State::Ready);
  // CHECK_EQUAL(State::Ready, ota->getState());
  // server.response = HttpTestServer::okResponse();
  // // ota->update(); // *ready -> sending
  // // ota->update(); // *sending -> receiving
  // wait_until_ota_state_is(State::Receiving);
  // CHECK_EQUAL(State::Receiving, ota->getState());
  // CHECK_TRUE(2 == ota->getContentLength());
  // ota->update(); // ready
  // CHECK_EQUAL(State::Ready, ota->getState());
  // CHECK_TRUE(HTTP_CODE_OK == ota->getHttpCode());
// }

// TEST(OtaUpdater, test_download) {
//   ota->setHostPortEndpointFromUri("http://localhost:8080/tests/fixtures/firmware.bin.md5");

//   ota->start();
//   ota->send(HttpRequest("GET"));
//   String body;
//   int count = 0;
//   ota->onData = [this, &body, &count](bool& isComplete) {
//     body += ota->getContent();
//     count++;
//     isComplete = true;
//   };

//   STRCMP_EQUAL("localhost", ota->getHost().c_str());
//   LONGS_EQUAL(8080, ota->getPort());

//   wait_until_ota_state_is(State::Receiving);
//   CHECK_EQUAL(State::Receiving, ota->getState());
//   LONGS_EQUAL(HTTP_CODE_OK, ota->getHttpCode());

//   wait_until_ota_state_is(State::Ready);
//   CHECK_EQUAL(State::Ready, ota->getState());
//   LONGS_EQUAL(body.length(), ota->getContentLength());
//   LONGS_EQUAL(1, count);
// }

// TEST(OtaUpdater, test_chunked_download) {
//   ota->setHostPortEndpointFromUri("http://localhost:8080/tests/fixtures/firmware.merged.bin");

//   ota->start();
//   ota->send(HttpRequest("GET"));
//   int count = 0;
//   int downloaded = 0;
//   static constexpr size_t kChunkSize = 4096;
//   ota->onData = [this, &count, &downloaded](bool& isComplete) {

//     if (downloaded < ota->getContentLength()) {
//       NetworkClient& stream = ota->getNetworkClient();
//       size_t avail = stream.available();
//       if (avail == 0) {
//         return;
//       }
//       uint8_t buf[kChunkSize];

//       size_t n = stream.readBytes(buf, min(avail, kChunkSize));
//       if (n == 0) return;

//       downloaded += n;
//       count++;

//     } else {
//       isComplete = true;
//     }

//   };

//   STRCMP_EQUAL("localhost", ota->getHost().c_str());
//   LONGS_EQUAL(8080, ota->getPort());

//   wait_until_ota_state_is(State::Receiving);
//   CHECK_EQUAL(State::Receiving, ota->getState());
//   LONGS_EQUAL(HTTP_CODE_OK, ota->getHttpCode());

//   wait_until_ota_state_is(State::Ready);
//   CHECK_EQUAL(State::Ready, ota->getState());
//   LONGS_EQUAL(downloaded, ota->getContentLength());
// }
