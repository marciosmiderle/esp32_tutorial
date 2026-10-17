#include "CppUTest/CommandLineTestRunner.h"

int main(int ac, char** av)
{
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    return 1; // Cancela se a rede do Windows falhar
  }
#endif

  return CommandLineTestRunner::RunAllTests(ac, av);

#ifdef _WIN32
  WSACleanup();
#endif
}
