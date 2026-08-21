#include "HttpClient.hpp"

HttpClient::HttpClient(const char* _apiUrl, int maxRetries, 
                       unsigned long retryTimeoutMs, 
                       unsigned long tryLaterTimeoutMs)
  : apiUrl(_apiUrl), retry(maxRetries, retryTimeoutMs, tryLaterTimeoutMs), connected(false) {}

bool HttpClient::send(const Message &message) {
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
  Serial.print(apiUrl);
  Serial.print(" Payload: ");
  Serial.print(jsonPayload.length());
  Serial.println(" bytes");
  
  int httpResponseCode = http.POST(jsonPayload);
  
  bool success = false;
  String response = "";
  
  if (httpResponseCode > 0) {
    response = http.getString();
    Serial.print("[HTTP] Response code: ");
    Serial.print(httpResponseCode);
    Serial.print(" Response: ");
    Serial.print(response.length());
    Serial.println(" bytes");
    
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
    Serial.print("[HTTP] Requisição falhou");
    Serial.print(" Resposta: ");
    Serial.println(response.length());
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
