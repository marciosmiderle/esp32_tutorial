#include "OtaUpdater.hpp"

#include <HTTPClient.h>
#include <Update.h>
#include <Preferences.h>
#include <WiFi.h>
#include <MD5Builder.h>
#include <mbedtls/sha256.h>

namespace {
Preferences prefs;
}

OtaUpdater::OtaUpdater(int maxRetries, unsigned long retryTimeoutMs,
                       unsigned long tryLaterTimeoutMs)
  : retry(maxRetries, retryTimeoutMs, tryLaterTimeoutMs) {}

void OtaUpdater::begin() {
  prefs.begin(kNvsNamespace, false);

  if (loadPending()) {
    Serial.println("[OTA] Estado pendente encontrado (falha anterior)");
    Serial.print("  url=");
    Serial.println(firmwareUrl);
    // Auto-retoma se havia download incompleto
    setState(State::FetchingHash, "auto-resume");
  }
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

String OtaUpdater::deriveHashUrl(const String& binUrl, bool sha256) {
  String base = binUrl;
  int q = base.indexOf('?');
  if (q >= 0) base = base.substring(0, q);
  if (base.endsWith(".bin")) {
    base.remove(base.length() - 4);
  }
  base += sha256 ? ".sha256" : ".md5";
  return base;
}

bool OtaUpdater::parseHashBody(const String& body, String& outHex, bool& isSha256) {
  String t = body;
  t.trim();
  t.toLowerCase();

  String hex;
  for (size_t i = 0; i < t.length(); ++i) {
    char c = t.charAt(i);
    const bool isHex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
    if (isHex) {
      hex += c;
    } else if (hex.length() == 32 || hex.length() == 64) {
      break;
    } else {
      hex = "";
    }
  }

  if (hex.length() == 32) {
    outHex = hex;
    isSha256 = false;
    return true;
  }
  if (hex.length() == 64) {
    outHex = hex;
    isSha256 = true;
    return true;
  }
  return false;
}

String OtaUpdater::toHex(const uint8_t* data, size_t len) {
  static const char* kDigits = "0123456789abcdef";
  String s;
  s.reserve(len * 2);
  for (size_t i = 0; i < len; ++i) {
    s += kDigits[(data[i] >> 4) & 0x0F];
    s += kDigits[data[i] & 0x0F];
  }
  return s;
}

void OtaUpdater::savePending() {
  prefs.putBool("pending", true);
  prefs.putString("url", firmwareUrl);
  prefs.putString("hash", expectedHash);
  prefs.putBool("sha256", useSha256);
  prefs.putULong("total", totalSize);
  prefs.putULong("offset", downloaded);
}

void OtaUpdater::clearPending() {
  prefs.putBool("pending", false);
  prefs.remove("url");
  prefs.remove("hash");
  prefs.remove("sha256");
  prefs.remove("total");
  prefs.remove("offset");
}

bool OtaUpdater::loadPending() {
  if (!prefs.getBool("pending", false)) return false;
  firmwareUrl = prefs.getString("url", "");
  expectedHash = prefs.getString("hash", "");
  useSha256 = prefs.getBool("sha256", false);
  totalSize = prefs.getULong("total", 0);
  downloaded = prefs.getULong("offset", 0);
  return firmwareUrl.length() > 0;
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
  clearPending();
  firmwareUrl = url;
  expectedHash = "";
  useSha256 = false;
  totalSize = 0;
  downloaded = 0;
  retry.reset();

  setState(State::FetchingHash, firmwareUrl.c_str());
  return true;
}

bool OtaUpdater::markOk() {
  // Confirma a imagem atual. Em Arduino-ESP32 o boot bem-sucedido
  // já indica sucesso; markOk serve como confirmação explícita (canário)
  // e limpa flags locais.
  prefs.putBool("await_ok", false);
  Serial.println("[OTA] firmware markOk — imagem atual confirmada");
  setState(State::Success, "markOk");
  return true;
}

void OtaUpdater::abort() {
  if (Update.isRunning()) {
    Update.abort();
  }
  clearPending();
  downloaded = 0;
  totalSize = 0;
  setState(State::Idle, "aborted");
}

bool OtaUpdater::stepFetchHash() {
  if (!retry.canRetry()) return false;

  HTTPClient http;
  String urlMd5 = deriveHashUrl(firmwareUrl, false);
  String urlSha = deriveHashUrl(firmwareUrl, true);

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

  String hex;
  bool sha = false;
  if (!parseHashBody(body, hex, sha)) {
    setError("Arquivo de hash invalido");
    clearPending();
    return false;
  }

  expectedHash = hex;
  useSha256 = sha;
  Serial.printf("[OTA] hash (%s): %s\n", useSha256 ? "sha256" : "md5",
                expectedHash.c_str());

  savePending();
  retry.reset();
  setState(State::Downloading, "hash ok");
  return true;
}

bool OtaUpdater::stepDownload() {
  if (!retry.canRetry()) return false;
  if (WiFi.status() != WL_CONNECTED) return false;

  // Sempre inicia gravação do zero na partição (API Update).
  // O "resume" evita re-baixar o hash e re-dispara o GET do bin.
  downloaded = 0;

  HTTPClient http;
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

  MD5Builder md5;
  mbedtls_sha256_context shaCtx;
  if (useSha256) {
    mbedtls_sha256_init(&shaCtx);
    mbedtls_sha256_starts(&shaCtx, 0);
  } else {
    md5.begin();
  }

  WiFiClient* stream = http.getStreamPtr();
  uint8_t buf[kChunkSize];

  while (http.connected() && downloaded < totalSize) {
    size_t avail = stream->available();
    if (avail == 0) {
      delay(1);
      yield();
      continue;
    }

    size_t n = stream->readBytes(buf, min(avail, kChunkSize));
    if (n == 0) break;

    if (Update.write(buf, n) != n) {
      http.end();
      Update.abort();
      setError("Update.write falhou");
      Update.printError(Serial);
      savePending();
      return false;
    }

    if (useSha256) {
      mbedtls_sha256_update(&shaCtx, buf, n);
    } else {
      md5.add(buf, n);
    }

    downloaded += n;

    if (millis() - lastProgressLogMs >= 1000) {
      lastProgressLogMs = millis();
      Serial.printf("[OTA] %u / %u (%.0f%%)\n",
                    (unsigned)downloaded, (unsigned)totalSize,
                    getProgress() * 100.0f);
      savePending();
    }
    yield();
  }

  http.end();

  if (downloaded != totalSize) {
    Update.abort();
    Serial.println("[OTA] download incompleto — estado salvo");
    savePending();
    return false;
  }

  if (!Update.end(true)) {
    setError("Update.end falhou");
    Update.printError(Serial);
    clearPending();
    return false;
  }

  String actual;
  if (useSha256) {
    uint8_t digest[32];
    mbedtls_sha256_finish(&shaCtx, digest);
    mbedtls_sha256_free(&shaCtx);
    actual = toHex(digest, 32);
  } else {
    md5.calculate();
    actual = md5.toString();
    actual.toLowerCase();
  }

  Serial.print("[OTA] hash calculado: ");
  Serial.println(actual);

  if (actual != expectedHash) {
    setError("Integridade falhou (hash)");
    clearPending();
    return false;
  }

  clearPending();
  prefs.putBool("await_ok", true);
  setState(State::Success, "ok — reiniciando");
  Serial.println("[OTA] Sucesso. Reinicio em 1s. Envie 'firmware markOk' apos boot.");
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
    } else if (state != State::Failed && !retry.hasRetriesLeft()) {
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
