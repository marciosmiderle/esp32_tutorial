#include "HttpClientLocal.hpp"
#include "Logger.hpp"

HttpClientLocal::HttpClientLocal(const char* _apiUrl, int maxRetries, 
                       unsigned long retryTimeoutMs, 
                       unsigned long tryLaterTimeoutMs)
  : apiUrl(_apiUrl), retry(maxRetries, retryTimeoutMs, tryLaterTimeoutMs), connected(false) {}

bool HttpClientLocal::send(const Message &message) {
  if (!retry.canRetry()) {
    Log.println("[MQTT] retry não permite iniciar");
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

bool HttpClientLocal::performRequest(const Message& message) {
  String jsonPayload = message.toJson();
  
  HTTPClient http;
  http.begin(apiUrl);
  http.addHeader("Content-Type", "application/json");
  
  Log.print("[HTTP] Enviando POST para ");
  Log.print(apiUrl);
  Log.print(" Payload: ");
  Log.print(jsonPayload.length());
  Log.println(" bytes");
  
  int httpResponseCode = http.POST(jsonPayload);
  
  bool success = false;
  String response = "";
  
  if (httpResponseCode > 0) {
    response = http.getString();
    Log.print("[HTTP] Response code: ");
    Log.print(httpResponseCode);
    Log.print(" Response: ");
    Log.print(response.length());
    Log.println(" bytes");
    
    if (httpResponseCode == 200 || httpResponseCode == 201) {
      success = true;
    }
  } else {
    Log.print("[HTTP] Erro na requisição: ");
    Log.println(httpResponseCode);
    response = "Erro HTTP: " + String(httpResponseCode);
  }
  
  http.end();
  
  logResult(success, response);
  return success;
}

void HttpClientLocal::logResult(bool success, const String& response) {
  lastResponse = response;
  
  if (success) {
    Log.println("[HTTP] Requisição bem-sucedida");
  } else {
    Log.print("[HTTP] Requisição falhou");
    Log.print(" Resposta: ");
    Log.println(response.length());
  }
}

void HttpClientLocal::update() {
  // Pode ser usado para lógica assíncrona se necessário
  // Por enquanto, a lógica de retry está no método send()
}

bool HttpClientLocal::isConnected() const {
  return connected;
}

const char* HttpClientLocal::getLastResponse() const {
  return lastResponse.c_str();
}
