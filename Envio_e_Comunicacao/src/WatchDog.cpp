#include "WatchDog.hpp"
#include "Logger.hpp"
#include <Arduino.h>
#include <esp_task_wdt.h>

WatchDog::WatchDog(uint32_t _timeout_ms)
  : timeout_ms(_timeout_ms) {}

void WatchDog::begin() {
  esp_task_wdt_config_t twdt_config = {
    .timeout_ms = timeout_ms,
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, // Monitora em ambos os cores
    .trigger_panic = true // Se estourar o tempo, gera um Panic e Reinicia
  };

  // Tenta reconfigurar o WDT que o próprio Arduino já inicializou
  esp_err_t err = esp_task_wdt_reconfigure(&twdt_config);

  // Se o WDT não estiver inicializado, nós inicializamos
  if (err == ESP_ERR_INVALID_STATE) {
    err = esp_task_wdt_init(&twdt_config);
    esp_err_t err = esp_task_wdt_init(&twdt_config);
    if (err != ESP_OK) {
      Log.println("[WatchDog] ERRO ao inicializar o WDT nativo");
      return;
    }
  }

  // 2. Registra a Task atual (a task do loop principal) no Watchdog
  err = esp_task_wdt_add(nullptr);
  if (err != ESP_OK) {
    Log.println("[WatchDog] ERRO ao registrar a task no WDT");
    return;
  }

  Log.printf("[WatchDog] Iniciado nativamente — Timeout: %lu ms\n", this->timeout_ms);
}

void WatchDog::feed() {
  esp_task_wdt_reset();
}
