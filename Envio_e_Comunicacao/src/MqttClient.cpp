#include "MqttClient.hpp"
#include <ArduinoJson.h>

static MqttClient *instance = nullptr;

MqttClient::MqttClient(const char* _broker, int _port, const char* _clientId,
                       const char* _telemetryTopic, const char* _eventTopic,
                       const char* _commandTopic, const char* _commandSubscribeTopic,
                       int maxRetries, unsigned long retryTimeoutMs,
                       unsigned long tryLaterTimeoutMs)
  : broker(_broker), port(_port), clientId(_clientId),
    telemetryTopic(_telemetryTopic), eventTopic(_eventTopic),
    commandTopic(_commandTopic), commandSubscribeTopic(_commandSubscribeTopic),
    retry(maxRetries, retryTimeoutMs, tryLaterTimeoutMs), connected(false) {
  client.setClient(wifiClient);
  client.setServer(broker, port);
  instance = this;
  client.setCallback(mqttCallbackWrapper);
}

bool MqttClient::connect() {
  if (connected) return true;
  
  // // Se está em tryLater, não tenta conectar
  // if (retry.isInTryLater()) {
  //   Serial.println("[MQTT] Aguardando tryLater expirar...");
  //   return false;
  // }
  
  // // Se tryLater expirou, reseta
  // if (retry.tryLaterExpired()) {
  //   Serial.println("[MQTT] TryLater expirado, resetando retries");
  //   retry.reset();
  // }
  
  // // Verifica se pode tentar
  // if (!retry.canStart()) {
  //   Serial.println("[MQTT] Não pode iniciar (tryLater ativo)");
  //   return false;
  // }


  if (!retry.canRetry()) {
    //Serial.println("[MQTT] retry não permite iniciar");
    return false;
  }

  Serial.print("[MQTT] Tentando conectar a ");
  Serial.print(broker);
  Serial.print(":");
  Serial.println(port);
  
  bool success = client.connect(clientId);
  
  if (success) {
    Serial.println("[MQTT] Conectado com sucesso");
    
    // Subscreve ao tópico de comandos
    if (client.subscribe(commandSubscribeTopic)) {
      Serial.print("[MQTT] Inscrito no tópico: ");
      Serial.println(commandSubscribeTopic);
    } else {
      Serial.print("[MQTT] Falha ao subscrever: ");
      Serial.println(commandSubscribeTopic);
    }
    
    retry.reset();
    connected = true;
    return true;
  } else {
    Serial.print("[MQTT] Falha na conexão. Estado: ");
    Serial.println(client.state());
    
    // retry.registerAttempt();
    
    // if (!retry.hasRetriesLeft()) {
    //   Serial.println("[MQTT] Sem retries restantes, entrando em tryLater");
    //   retry.enterTryLater();
    // }
    
    connected = false;
    return false;
  }
}

void MqttClient::disconnect() {
  client.disconnect();
  connected = false;
  retry.reset();
  Serial.println("[MQTT] Desconectado");
}

bool MqttClient::publishTelemetry(const Message& message) {
  if (!ensureConnection()) {
    return false;
  }
  
  String jsonPayload = message.toJson();
  
  Serial.print("[MQTT] Publicando telemetria em ");
  Serial.print(telemetryTopic);
  Serial.print(" Payload: ");
  Serial.print(jsonPayload.substring(0, 10));
  
  bool success = client.publish(telemetryTopic, jsonPayload.c_str());
  
  if (success) {
    Serial.println(" OK");
  } else {
    Serial.println(" Falha");
  }
  
  return success;
}

bool MqttClient::publishEvent(const char* eventType, const char* eventData) {
  if (!ensureConnection()) {
    return false;
  }
  
  JsonDocument doc;
  doc["type"] = eventType;
  doc["data"] = eventData;
  doc["timestamp"] = millis();
  
  String jsonPayload;
  serializeJson(doc, jsonPayload);
  
  Serial.print("[MQTT] Publicando evento em ");
  Serial.print(eventTopic);
  Serial.print(" Payload: ");
  Serial.print(jsonPayload.substring(0, 10));
  
  bool success = client.publish(eventTopic, jsonPayload.c_str());
  
  if (success) {
    Serial.println(" OK");
  } else {
    Serial.println(" Falha");
  }
  
  return success;
}

void MqttClient::setCallback(MqttCallback callback) {
  userCallback = callback;
}

void MqttClient::mqttCallbackWrapper(char* topic, byte* payload, unsigned int length) {
  if (instance != nullptr) {
    instance->handleCommand(topic, payload, length);
  }
}

void MqttClient::handleCommand(char* topic, byte* payload, unsigned int length) {
  Serial.print("[MQTT] Comando recebido em ");
  Serial.print(topic);
  Serial.print(": ");
  
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.println(message);
  
  // Chama o callback do usuário se estiver definido
  if (userCallback) {
    userCallback(topic, payload, length);
  }
}

bool MqttClient::ensureConnection() {
  if (connected && client.connected()) {
    return true;
  }
  
  connected = false;
  return connect();
}

void MqttClient::update() {
  if (!connected) {
    // Tenta reconectar automaticamente
    connect();
  } else {
    // Mantém a conexão viva
    client.loop();
  }
}

bool MqttClient::isConnected() {
  return connected && client.connected();
}
