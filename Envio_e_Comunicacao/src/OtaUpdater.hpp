#pragma once

#include <Arduino.h>
#include <functional>
#include "RetryLogic.hpp"

/**
 * OTA por HTTP com verificação de integridade (MD5/SHA-256), acionado via MQTT.
 *
 * Comandos (payload no tópico de entrada):
 *   firmware update <url_do_bin>
 *     Baixa o .bin e o hash de mesmo nome (.md5 ou .sha256).
 *     Grava na partição OTA livre (Update library) e reinicia.
 *
 *   firmware markOk
 *     Confirma o firmware atual como válido (após boot bem-sucedido).
 *
 * Resume:
 *   Em falha de rede o progresso (URL, hash, offset) é salvo em NVS.
 *   Na próxima tentativa o download recomeça do zero na partição OTA
 *   (limitação da API Update: não há seek seguro após abort).
 *   O HTTP Range é usado quando o servidor suporta, para economizar
 *   banda no caso de retomada em memória (mesma sessão, sem reboot).
 *
 * Chame begin() no setup e update() no loop (não-bloqueante entre fases).
 */
class OtaUpdater {
public:
  enum class State {
    Idle,
    FetchingHash,
    Downloading,
    Success,
    Failed
  };

  using StatusCallback = std::function<void(State state, const char* detail)>;

  OtaUpdater(int maxRetries = 5,
             unsigned long retryTimeoutMs = 3000,
             unsigned long tryLaterTimeoutMs = 30000);

  void begin();
  void setStatusCallback(StatusCallback cb);

  /** Inicia (ou retoma) update a partir da URL do .bin. */
  bool startUpdate(const char* firmwareUrl);

  /** Confirma o firmware em execução (canário / mark valid). */
  bool markOk();

  void abort();
  void update();

  State getState() const { return state; }
  bool isBusy() const;
  float getProgress() const;
  const char* getLastError() const { return lastError; }

private:
  static constexpr size_t kChunkSize = 4096;
  static constexpr const char* kNvsNamespace = "ota";

  State state = State::Idle;
  RetryLogic retry;
  StatusCallback statusCb;

  String firmwareUrl;
  String expectedHash;   // hex lowercase
  bool useSha256 = false;

  size_t totalSize = 0;
  size_t downloaded = 0;

  char lastError[96] = {0};
  unsigned long lastProgressLogMs = 0;

  void setState(State s, const char* detail = nullptr);
  void setError(const char* msg);
  void clearError();

  void savePending();
  void clearPending();
  bool loadPending();

  bool stepFetchHash();
  bool stepDownload();

  static String deriveHashUrl(const String& binUrl, bool sha256);
  static bool parseHashBody(const String& body, String& outHex, bool& isSha256);
  static String toHex(const uint8_t* data, size_t len);
};
