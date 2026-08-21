#include "HttpClient.hpp"

HttpClient::HttpClient(const char* _apiUrl, int maxRetries, 
                       unsigned long retryTimeoutMs, 
                       unsigned long tryLaterTimeoutMs)
  : apiUrl(_apiUrl), retry(maxRetries, retryTimeoutMs, tryLaterTimeoutMs), connected(false) {}

bool HttpClient::send(const Message& message) {
  // Se está em tryLater, não tenta enviar
  // if (retry.isInTryLater()) {
  //   Serial.println("[HTTP] Aguardando tryLater expirar...");
  //   return false;
  // }
  
  // // Se tryLater expirou, reseta
  // if (retry.tryLaterExpired()) {
  //   Serial.println("[HTTP] TryLater expirado, resetando retries");
  //   retry.reset();
  // }
  
  // // Verifica se pode tentar
  // if (!retry.canStart()) {
  //   Serial.println("[HTTP] Não pode iniciar (tryLater ativo)");
  //   return false;
  // }

  if (!retry.canRetry()) {
    Serial.println("[MQTT] retry não permite iniciar");
    return false;
  }
  
  bool success = performRequest(message);
  
  if (success) {
    retry.reset();
    connected = true;
    return true;
  } else {
    // retry.registerAttempt();
    
    // if (!retry.hasRetriesLeft()) {
    //   Serial.println("[HTTP] Sem retries restantes, entrando em tryLater");
    //   retry.enterTryLater();
    // }
    
    connected = false;
    return false;
  }
}

bool HttpClient::performRequest(const Message& message) {
  String jsonPayload = message.toJson();
  
  HTTPClient http;
  http.begin(apiUrl);
  http.addHeader("Content-Type", "application/json");
  
  Serial.print("[HTTP] Enviando POST para ");
  Serial.println(apiUrl);
  Serial.print("[HTTP] Payload: ");
  Serial.println(jsonPayload);
  
  int httpResponseCode = http.POST(jsonPayload);
  
  bool success = false;
  String response = "";
  
  if (httpResponseCode > 0) {
    response = http.getString();
    Serial.print("[HTTP] Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("[HTTP] Response: ");
    Serial.println(response);
    
    if (httpResponseCode == 200 || httpResponseCode == 201) {
      success = true;
    }
  } else {
    Serial.print("[HTTP] Erro na requisição: ");
    Serial.println(httpResponseCode);
    response = "Erro HTTP: " + String(httpResponseCode);
  }
  
  http.end();
  
  logResult(success, response);
  return success;
}

void HttpClient::logResult(bool success, const String& response) {
  lastResponse = response;
  
  if (success) {
    Serial.println("[HTTP] Requisição bem-sucedida");
  } else {
    Serial.println("[HTTP] Requisição falhou");
    Serial.print("[HTTP] Resposta: ");
    Serial.println(response);
  }
}

void HttpClient::update() {
  // Pode ser usado para lógica assíncrona se necessário
  // Por enquanto, a lógica de retry está no método send()
}

bool HttpClient::isConnected() const {
  return connected;
}

const char* HttpClient::getLastResponse() const {
  return lastResponse.c_str();
}
