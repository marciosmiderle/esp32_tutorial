#include "OtaUpdater.hpp"
#include "Logger.hpp"

#include <Update.h>
#include "esp_ota_ops.h"

using State     = OtaUpdater::State;
using StepState = OtaUpdater::StepState;

OtaUpdater::OtaUpdater(const int maxRetries,
                       const unsigned long retryTimeoutMs,
                       const unsigned long tryLaterTimeoutMs)
  : retry(maxRetries, retryTimeoutMs, tryLaterTimeoutMs, OTA), started(false),
    http("localhost", 80, "/") {}

void OtaUpdater::start() {
  if (started) return;

  started = true;
  http.start();
}

void OtaUpdater::stop() {
  started = false;
  http.stop();
}

void OtaUpdater::restart() {
  stop();
  start();
}

State OtaUpdater::getState() const {
  return state;
}

StepState OtaUpdater::getStepState() const {
  return stepState;
}

bool OtaUpdater::getDownloadComplete() const {
  return downloadComplete;
}

bool OtaUpdater::isUpdating() const {
  return state != State::Idle;
}

float OtaUpdater::getProgress() const {
  return downloaded == 0 ? 0.0f :
    static_cast<float>(downloaded) / static_cast<float>(http.getContentLength());
}

bool OtaUpdater::startUpdate(const char* url) {
  if (!started) return false;

  if (isUpdating()) {
    Log.println(OTA, "ja em andamento");
    return false;
  }

  firmwareUrl = url;
  downloaded = 0;
  retry.reset();

  state = State::FetchingHash;
  stepState = StepState::Starting;
  http.start();
  return true;
}

bool OtaUpdater::stopUpdate() {
  if (isUpdating()) {
    retry.reset();
    Update.abort();
    state = State::Idle;
    stepState = StepState::Idle;
    downloaded = 0;
    lastProgressLogMs = 0;
    http.stop();

    return true;
  }
  Log.println(OTA, "não está em andamento");
  return false;
}

bool OtaUpdater::markOk() {
  if(esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
    Log.println(OTA, "firmware markOk - imagem atual confirmada");
  } else {
    Log.println(OTA, "erro em markOk");
  }
  return true;
}

bool OtaUpdater::markInvalidReboot() {
  if (esp_ota_mark_app_invalid_rollback_and_reboot() == ESP_OK) {
    Log.println(OTA, "firmware invalid, rollback e reboot");
  } else {
    Log.println(OTA, "erro em markInvalidReboot");
  }
  return true;
}

void OtaUpdater::stepFetchHash() {
  if (!retry.canRetry()) return;

  switch (stepState) {

  case StepState::Starting: {
    String uriMd5 = CheckSum::deriveHashUrl(firmwareUrl, false);
    http.setHostPortEndpointFromUri(uriMd5);
    HttpRequest request("GET");
    http.send(request);
    downloadComplete = false;
    http.onData = [this](bool& isComplete) { isComplete = downloadComplete; };
    stepState = StepState::Running;
    break;
  }

  case StepState::Running: {
    if (http.getPhase() != HttpClientLocal::Phase::Receiving) {
      break;
    }
    int code = http.getHttpCode();
    String body;

    if (code != 200) {
      Log.println(OTA, "hash HTTP ", code);
      stepState = StepState::Failed;
    } else if (http.getContentLength() > maxCheckSumFileSize) {
      Log.println(OTA, "Arquivo de hash com tamanho grande demais");
      stepState = StepState::Failed;
    } else {

      body = http.getContent();
      if(sum.parseHash(body)) {
        stepState = StepState::Finished;
      } else {
        Log.println(OTA, "Arquivo de hash invalido");
        stepState = StepState::Failed;
      }
      downloadComplete = true;

    }
    break;
  }

  default:
    break;
  }

  retry.reset();
  return;
}

void OtaUpdater::stepDownload() {
  if (!retry.canRetry()) return;

  switch (stepState) {

  case StepState::Starting: {
    http.setHostPortEndpointFromUri(firmwareUrl);
    HttpRequest request("GET");
    http.send(request);
    downloadComplete = false;
    http.onData = [this](bool& isComplete) { isComplete = downloadComplete; };
    stepState = StepState::Running;
    downloaded = 0;
    break;
  }

  case StepState::Running: {
    if (http.getPhase() != HttpClientLocal::Phase::Receiving) {
      break;
    }
    if (http.getContentLength() <= 0) {
      Log.println(OTA, "Content-Length ausente");
      stepState = StepState::Failed;
      break;
    }
    if (!Update.begin(http.getContentLength(), U_FLASH)) {
      Log.println(OTA, "Update.begin falhou");
      Update.printError(Log);
      stepState = StepState::Failed;
      break;
    }
    sum.begin();

    stepState = StepState::Running_2;
    break;
  }

  case StepState::Running_2: {
    if (downloaded < http.getContentLength()) {
      NetworkClient& stream = http.getNetworkClient();
      size_t avail = stream.available();
      if (avail == 0) {
        break;
      }
      uint8_t buf[kChunkSize];

      size_t n = stream.readBytes(buf, min(avail, kChunkSize));
      if (n > 0) {

        if (Update.write(buf, n) != n) {
          Log.println(OTA, "Update.write falhou");
          Update.printError(Log);
          stepState = StepState::Failed;
          break;
        }

        sum.add(buf, n);
        downloaded += n;

        if (millis() - lastProgressLogMs >= 1000 ||
            downloaded == http.getContentLength()) {
          lastProgressLogMs = millis();
          Log.printf("%s%7.0u / %7.0u [%5.1f%%]\n", OTA,
                     (unsigned)downloaded, (unsigned)http.getContentLength(),
                     getProgress() * 100.0f);
        }
      }
      break;
    }
    downloadComplete = true;
    stepState = StepState::Finished;
    break;
  }

  default:
    break;
  }

  retry.reset();
  return;
}

void OtaUpdater::stepDownloadValidation() {
  if (downloaded != http.getContentLength()) {
    Log.println(OTA, "download incompleto — update cancelado");
    stepState = StepState::Failed;
    return;
  }

  if (!Update.end(true)) {
    Log.println(OTA, "Update.end falhou");
    Update.printError(Log);
    stepState = StepState::Failed;
    return;
  }

  String actual = sum.getDigest();

  Log.println(OTA, "hash calculado: ", actual.c_str());

  if (actual != sum.getExpectedHash()) {
    Log.println(OTA, "Integridade falhou (hash)");
    Log.println(OTA, "hash calculado = ", actual.c_str());
    Log.println(OTA, "hash esperado  = ", sum.getExpectedHash().c_str());
    stepState = StepState::Failed;
    return;
  }

  stepState = StepState::Finished;
}

void OtaUpdater::update() {
  if (!started) return;
  if (state == State::Idle) return;
  if (stepState == StepState::Failed) {
    stopUpdate();
    return;
  }

  http.update();

  if (state == State::FetchingHash) {
    if (stepState == StepState::Finished) {
      state = State::Downloading;
      stepState = StepState::Starting;
      return;
    }
    stepFetchHash();
    return;
  }

  if (state == State::Downloading) {
    if (stepState == StepState::Finished) {
      state = State::DownloadValidation;
      stepState = StepState::Starting;
      return;
    }
    stepDownload();
    return;
  }

  if (state == State::DownloadValidation) {
    if (stepState == StepState::Finished) {
      state = State::Reboot;
      stepState = StepState::Starting;
      return;
    }
    stepDownloadValidation();
    return;
  }

  if (state == State::Reboot) {
    stepReboot();
    return;
  }
}

void OtaUpdater::stepReboot() {
  static unsigned long start_ms;

  if (stepState == StepState::Starting) {
    Log.println(OTA, "Sucesso. Reinicio em 1s. Envie 'firmware markOk' apos boot se tudo estiver ok.");
    start_ms = millis();
    stepState = StepState::Running;
    return;
  }

  if (stepState == StepState::Running) {
    if (millis() - start_ms >= REBOOT_DELAY_MS) {
      ESP.restart();
    }
  }

  return;
}
