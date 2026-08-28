#include "OtaUpdater.hpp"

#include <Update.h>
#include <WiFi.h>
#include "esp_ota_ops.h"

OtaUpdater::OtaUpdater(int maxRetries, unsigned long retryTimeoutMs,
                       unsigned long tryLaterTimeoutMs)
  : retry(maxRetries, retryTimeoutMs, tryLaterTimeoutMs) {}

void OtaUpdater::begin() {
}

void OtaUpdater::setStatusCallback(StatusCallback cb) {
  statusCb = std::move(cb);
}

bool OtaUpdater::isBusy() const {
  return state == State::FetchingHash || state == State::Downloading;
}

float OtaUpdater::getProgress() const {
  if (totalSize == 0) return 0.0f;
  return static_cast<float>(downloaded) / static_cast<float>(totalSize);
}

void OtaUpdater::setState(State s, const char* detail) {
  state = s;
  if (statusCb) {
    statusCb(s, detail ? detail : "");
  }
  Serial.print("[OTA] ");
  switch (s) {
    case State::Idle:         Serial.print("Idle"); break;
    case State::FetchingHash: Serial.print("FetchingHash"); break;
    case State::Downloading:  Serial.print("Downloading"); break;
    case State::Success:      Serial.print("Success"); break;
    case State::Failed:       Serial.print("Failed"); break;
  }
  if (detail && detail[0]) {
    Serial.print(" | ");
    Serial.print(detail);
  }
  Serial.println();
}

void OtaUpdater::setError(const char* msg) {
  strncpy(lastError, msg, sizeof(lastError) - 1);
  lastError[sizeof(lastError) - 1] = '\0';
  setState(State::Failed, lastError);
}

void OtaUpdater::clearError() {
  lastError[0] = '\0';
}

bool OtaUpdater::startUpdate(const char* url) {
  if (isBusy()) {
    setError("OTA ja em andamento");
    return false;
  }
  if (!url || strlen(url) < 8) {
    setError("URL invalida");
    return false;
  }
  if (WiFi.status() != WL_CONNECTED) {
    setError("WiFi desconectado");
    return false;
  }

  clearError();
  firmwareUrl = url;
  totalSize = 0;
  downloaded = 0;
  retry.reset();

  setState(State::FetchingHash, firmwareUrl.c_str());
  return true;
}

bool OtaUpdater::markOk() {
  if(esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
    Serial.println("[OTA] firmware markOk — imagem atual confirmada");
    setState(State::Success, "markOk");
  } else {
    Serial.println("[OTA] erro em markOk");
  }
  return true;
}

bool OtaUpdater::markInvalidReboot() {
  if (esp_ota_mark_app_invalid_rollback_and_reboot() == ESP_OK) {
    Serial.println("[OTA] firmware invalid, rollback e reboot");
    setState(State::Success, "markInvalidReboot");
  } else {
    Serial.println("[OTA] erro em markInvalidReboot");
  }
  return true;
}

void OtaUpdater::abort() {
  if (Update.isRunning()) {
    Update.abort();
  }
  downloaded = 0;
  totalSize = 0;
  setState(State::Idle, "aborted");
}

bool OtaUpdater::stepFetchHash() {
  if (!retry.canRetry()) return false;

  String urlMd5 = CheckSum::deriveHashUrl(firmwareUrl, false);
  String urlSha = CheckSum::deriveHashUrl(firmwareUrl, true);

  Serial.print("[OTA] GET ");
  Serial.println(urlMd5);
  http.begin(urlMd5);
  http.setTimeout(10000);
  int code = http.GET();
  String body;

  if (code == 200) {
    body = http.getString();
  }
  http.end();

  if (code != 200) {
    Serial.print("[OTA] GET ");
    Serial.println(urlSha);
    http.begin(urlSha);
    http.setTimeout(10000);
    code = http.GET();
    if (code == 200) {
      body = http.getString();
    }
    http.end();
  }

  if (code != 200) {
    Serial.printf("[OTA] hash HTTP %d\n", code);
    return false;
  }

  if(!sum.parseHash(body)) {
    setError("Arquivo de hash invalido");
    return false;
  }
  sum.begin();

  retry.reset();
  setState(State::Downloading, "hash ok");
  return true;
}

bool OtaUpdater::stepDownload() {
  if (!retry.canRetry()) return false;
  if (WiFi.status() != WL_CONNECTED) return false;

  downloaded = 0;

  http.begin(firmwareUrl);
  http.setTimeout(20000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  Serial.print("[OTA] GET ");
  Serial.println(firmwareUrl);
  int code = http.GET();
  if (code != 200 && code != 206) {
    Serial.printf("[OTA] firmware HTTP %d\n", code);
    http.end();
    return false;
  }

  int contentLen = http.getSize();
  if (contentLen <= 0) {
    http.end();
    setError("Content-Length ausente");
    return false;
  }
  totalSize = static_cast<size_t>(contentLen);

  if (!Update.begin(totalSize, U_FLASH)) {
    http.end();
    setError("Update.begin falhou");
    Update.printError(Serial);
    return false;
  }

  WiFiClient* stream = http.getStreamPtr();
  uint8_t buf[kChunkSize];

  while (http.connected() && downloaded < totalSize) {
    size_t avail = stream->available();
    if (avail == 0) {
      delay(1);
      continue;
    }

    size_t n = stream->readBytes(buf, min(avail, kChunkSize));
    if (n == 0) break;

    if (Update.write(buf, n) != n) {
      http.end();
      Update.abort();
      setError("Update.write falhou");
      Update.printError(Serial);
      return false;
    }

    sum.add(buf, n);

    downloaded += n;

    if (millis() - lastProgressLogMs >= 1000) {
      lastProgressLogMs = millis();
      Serial.printf("[OTA] %7.0u / %7.0u (%2.0f%%)\n",
                    (unsigned)downloaded, (unsigned)totalSize,
                    getProgress() * 100.0f);
    }
  }

  http.end();

  if (downloaded != totalSize) {
    Update.abort();
    Serial.println("[OTA] download incompleto — estado salvo");
    return false;
  }

  if (!Update.end(true)) {
    setError("Update.end falhou");
    Update.printError(Serial);
    return false;
  }

  String actual = sum.getDigest();

  Serial.print("[OTA] hash calculado: ");
  Serial.println(actual);

  if (actual != sum.getExpectedHash()) {
    setError("Integridade falhou (hash)");
    Serial.printf("[OTA] hash calculado = '%s'\n", actual.c_str());
    Serial.printf("[OTA] hash esperado  = '%s'\n", sum.getExpectedHash().c_str());
    return false;
  }

  setState(State::Success, "ok — reiniciando");
  Serial.println("[OTA] Sucesso. Reinicio em 1s. Envie 'firmware markOk' apos boot se tudo estiver ok.");
  delay(1000);
  ESP.restart();
  return true;
}

void OtaUpdater::update() {
  if (!isBusy()) return;
  if (WiFi.status() != WL_CONNECTED) return;

  if (state == State::FetchingHash) {
    if (stepFetchHash()) {
      // passou para Downloading
    } else if (state != State::Failed) {
      // tryLater já tratado pelo RetryLogic
    }
    return;
  }

  if (state == State::Downloading) {
    if (stepDownload()) {
      // reinicia no sucesso
    }
    return;
  }
}
