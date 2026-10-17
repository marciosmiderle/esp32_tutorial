#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

typedef int socklen_t;
#define close closesocket

#define MSG_NOSIGNAL 0

#define usleep(x) Sleep((x) < 1000 && (x) > 0 ? 1 : (x) / 1000)

#define poll WSAPoll

typedef unsigned long nfds_t;
typedef _sigset_t sigset_t;

inline int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *tmo_p, const sigset_t *sigmask) {
  (void)sigmask; // Ignora máscara de sinais no Windows
  int timeout_ms = -1;
  if (tmo_p != nullptr) {
    timeout_ms = (tmo_p->tv_sec * 1000) + (tmo_p->tv_nsec / 1000000);
  }
  return WSAPoll(fds, nfds, timeout_ms);
}

inline struct tm* gmtime_r(const time_t* timer, struct tm* result) {
  if (gmtime_s(result, timer) == 0) {
    return result;
  }
  return nullptr;
}

inline char* strptime(const char* buf, const char* fmt, struct tm* tm) {
  int day, year, hour, min, sec;
  char month_str[4], weekday_str[4];
  if (sscanf(buf, "%3s, %d %3s %d %d:%d:%d", weekday_str, &day, month_str, &year, &hour, &min, &sec) == 7) {
    tm->tm_mday = day; tm->tm_year = year - 1900; tm->tm_hour = hour; tm->tm_min = min; tm->tm_sec = sec;
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int i = 0; i < 12; i++) { if (strcmp(month_str, months[i]) == 0) { tm->tm_mon = i; break; } }
    return (char*)(buf + strlen(buf));
  }
  return nullptr;
}

#else
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace os_posix_compat {

  inline int set_nonblocking(int fd, bool nonblocking) {
#ifdef _WIN32
    unsigned long mode = nonblocking ? 1 : 0;
    return ioctlsocket(fd, FIONBIO, &mode);
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return flags;
    if (nonblocking) {
      return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    } else {
      return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }
#endif
  }

  inline int get_last_error() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
  }

  inline const char* inet_ntop(int af, const void* src, char* dst, size_t size) {
    if (af != AF_INET) return nullptr;
#ifdef _WIN32
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(src);
    int r = snprintf(dst, size, "%d.%d.%d.%d",
                     static_cast<int>(bytes[0]),
                     static_cast<int>(bytes[1]),
                     static_cast<int>(bytes[2]),
                     static_cast<int>(bytes[3]));
    return (r > 0 && static_cast<size_t>(r) < size) ? dst : nullptr;
#else
    return ::inet_ntop(af, src, dst, size);
#endif
  }
}
