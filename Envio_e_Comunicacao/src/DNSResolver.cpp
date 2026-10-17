#include "DNSResolver.hpp"
#include "os_posix_compat.hpp"

#if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP32)
#include "lwip/tcpip.h" // Necessário para interagir com a task do lwIP de forma segura

// Estrutura para transferir dados de forma thread-safe
struct dns_async_request {
    DNSResolver* instance;
    std::string hostname;
};

// 1. Callback chamado pelo lwIP quando a resposta do servidor DNS chega da internet
void DNSResolver::lwip_dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    DNSResolver* self = static_cast<DNSResolver*>(callback_arg);
    if (!self) return;

    self->started = false; // Libera o resolver para próximas requisições
        
    if (ipaddr != nullptr) {
        char ip_str[16];
        ipaddr_ntoa_r(ipaddr, ip_str, sizeof(ip_str));
        self->callback(name, ip_str, self->callback_arg);
    } else {
        self->callback(name, nullptr, self->callback_arg); // Falha na resolução
    }
}

// 2. Função executada com segurança de concorrência dentro da Thread do lwIP
void DNSResolver::lwip_dns_safe_call(void *ctx) {
    dns_async_request* req = static_cast<dns_async_request*>(ctx);
    if (!req) return;

    // Executa a busca nativa. O callback 'lwip_dns_found_cb' será disparado no futuro.
    err_t err = dns_gethostbyname(req->hostname.c_str(), &(req->instance->_lwip_resolved_ip), 
                                  lwip_dns_found_cb, req->instance);
    
    // Se o IP já estava em cache, o lwIP resolve IMEDIATAMENTE (retorna ERR_OK)
    if (err == ERR_OK) {
        req->instance->started = false;
        char ip_str[16];
        ipaddr_ntoa_r(&(req->instance->_lwip_resolved_ip), ip_str, sizeof(ip_str));
        req->instance->callback(req->hostname.c_str(), ip_str, req->instance->callback_arg);
    } 
    // Se retornar ERR_INPROGRESS, significa que a requisição UDP foi enviada e o 
    // lwIP chamará 'lwip_dns_found_cb' de forma assíncrona assim que a resposta chegar.
    else if (err != ERR_INPROGRESS) {
        // Qualquer outro retorno significa falha imediata (ex: falta de memória ou erro interno)
        req->instance->started = false;
        req->instance->callback(req->hostname.c_str(), nullptr, req->instance->callback_arg);
    }

    delete req; // Libera a estrutura temporária
}

void DNSResolver::update() {
    // Permanece vazio no ESP32 físico. O processamento assíncrono ocorre nas tasks de background do sistema.
}
#else
void DNSResolver::update() {
  if (!started) return;

  if (future_worker.valid() &&
      future_worker.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
    std::string ip = future_worker.get();
    started = false;
    
    if (!ip.empty()) {
      callback(hostname.c_str(), ip.c_str(), callback_arg);
    } else {
      callback(hostname.c_str(), nullptr, callback_arg); 
    }
  }
}
#endif

DNSResolver::DNSResolver() :
  callback(nullptr), callback_arg(nullptr), started(false) {}

DNSResolver& DNSResolver::getInstance() {
  static DNSResolver instance;
  return instance;
}

bool DNSResolver::resolve(const std::string& _hostname, AsyncDNSCallback cb, void* arg) {
  if (started) return false;
        
  hostname = _hostname;
  callback = cb;
  callback_arg = arg;
  started = true;

#if !defined(ESP_PLATFORM) && !defined(ARDUINO_ARCH_ESP32)
  if (future_worker.valid()) {
    future_worker.wait(); 
  }
  future_worker = std::async(std::launch::async, [this]() -> std::string {
    struct addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
            
    int status = getaddrinfo(hostname.c_str(), nullptr, &hints, &res);
    if (status == 0) {
      char ip_buf[16];
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
                
      if (os_posix_compat::inet_ntop(AF_INET, &ipv4->sin_addr, ip_buf, sizeof(ip_buf)) != nullptr) {
        freeaddrinfo(res);
        return std::string(ip_buf);
      }
    }
    return "";
  });
  return true;
#else
  // Aloca dinamicamente os argumentos para garantir que persistam até a troca de contexto de thread
  dns_async_request* req = new dns_async_request{this, hostname};
  
  // Despacha a chamada sem bloquear a thread atual. Retorna imediatamente.
  err_t env_err = tcpip_callback(lwip_dns_safe_call, req);
  if (env_err != ERR_OK) {
    started = false;
    delete req;
    return false;
  }
  
  return true; // Retorna true imediatamente, liberando o loopTask e o Watchdog
#endif
}

bool DNSResolver::isInProgress() const { return started; }

void DNSResolver::reset() {
#if !(defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_ESP32))
  if (future_worker.valid()) {
    future_worker.wait();
    try {
      future_worker.get();
    } catch (...) {}
  }
#endif
  started = false;
}
