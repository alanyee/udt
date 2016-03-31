#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <iostream>
#include <udt.h>
#include "cc.h"
#include "test_util.h"

using namespace std;

void* monitor(void*);
void* timedmonitor(void*);
void* clientsend(void*);

struct time {
  void* u;
  int seconds;
};

int main(int argc, char* argv[]) {
  if ((argc != 3 || atoi(argv[2]) == 0) && (argc != 5 || strcmp(argv[3], "-t") || atoi(argv[4]) == 0)) {
    cout << "usage: appclient server_ip server_port [-t] [seconds]" << endl;
    return 0;
  }

  // Automatically start up and clean up UDT module.
  UDTUpDown _udt_;

  struct addrinfo hints, *local, *peer;

  memset(&hints, 0, sizeof(hints));

  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(NULL, "9000", &hints, &local) != 0) {
    cout << "incorrect network address." << endl;
    return 0;
  }

  UDTSOCKET client = UDT::socket(local->ai_family, local->ai_socktype, local->ai_protocol);

  freeaddrinfo(local);

  if (getaddrinfo(argv[1], argv[2], &hints, &peer) != 0) {
    cout << "incorrect server/peer address. " << argv[1] << ":" << argv[2] << endl;
    return 0;
  }

  // connect to the server, implict bind
  if (UDT::ERROR == UDT::connect(client, peer->ai_addr, peer->ai_addrlen)) {
    cout << "connect: " << UDT::getlasterror().getErrorMessage() << endl;
    return 0;
  }

  freeaddrinfo(peer);

  int size = 100000;
  char* data = new char[size];

  if (argc == 5) {
    char* endptr;
    long int x = strtol(argv[4], &endptr, 10);
    struct time input;
    input.u = &client;
    input.seconds = x;
    pthread_t monthread;
    pthread_create(&monthread, NULL, timedmonitor, (void*)& input);
    pthread_create(new pthread_t, NULL, clientsend, &client);
    pthread_join(monthread, NULL);
  } else {
    pthread_create(new pthread_t, NULL, monitor, &client);
    int ss;
    for (int i = 0; i < 1000000; ++i) {
      int ssize = 0;
      while (ssize < size) {
        if (UDT::ERROR == (ss = UDT::send(client, data + ssize, size - ssize, 0))) {
          cout << "send:" << UDT::getlasterror().getErrorMessage() << endl;
          break;
        }

        ssize += ss;
      }

      if (ssize < size) break;
    }
  }

  UDT::close(client);
  delete [] data;
  return 0;
}

void* monitor(void* s) {
  UDTSOCKET u = *(UDTSOCKET*)s;

  UDT::TRACEINFO perf;

  cout << "SendRate(Mb/s)\tRTT(ms)\tCWnd\tPktSndPeriod(us)\tRecvACK\tRecvNAK" << endl;

  while (true) {
    sleep(1);

    if (UDT::ERROR == UDT::perfmon(u, &perf)) {
      cout << "perfmon: " << UDT::getlasterror().getErrorMessage() << endl;
      break;
    }

    cout << perf.mbpsSendRate << "\t\t" 
        << perf.msRTT << "\t" 
        << perf.pktCongestionWindow << "\t" 
        << perf.usPktSndPeriod << "\t\t\t" 
        << perf.pktRecvACK << "\t" 
        << perf.pktRecvNAK << endl;
  }

  return NULL;
}

void* timedmonitor(void* s) {
  struct time* timer = (struct time*)s;
  UDTSOCKET u = *(UDTSOCKET*)timer->u;
  int t = timer->seconds;
  double total_bytes = 0;

  UDT::TRACEINFO perf;

  cout << "SendRate(Mb/s)\tRTT(ms)\tCWnd\tPktSndPeriod(us)\tRecvACK\tRecvNAK" << endl;

  for (int i = 0; i < t; ++i) {
    sleep(1);

    if (UDT::ERROR == UDT::perfmon(u, &perf)) {
      cout << "perfmon: " << UDT::getlasterror().getErrorMessage() << endl;
      break;
    }

    cout << perf.mbpsSendRate << "\t\t"
        << perf.msRTT << "\t"
        << perf.pktCongestionWindow << "\t"
        << perf.usPktSndPeriod << "\t\t\t"
        << perf.pktRecvACK << "\t"
        << perf.pktRecvNAK << endl;

    total_bytes += perf.mbpsSendRate;

  }

  total_bytes *= 1000;
  cout << "Summary: " << total_bytes << " bytes sent" << endl;
  return NULL;
}

void* clientsend(void* s) {
  UDTSOCKET u = *(UDTSOCKET*)s;
  int size = 100000;
  char* data = new char[size];
  int ss;
  for (int i = 0; i < 1000000; ++i) {
    int ssize = 0;
    while (ssize < size) {
      if (UDT::ERROR == (ss = UDT::send(u, data + ssize, size - ssize, 0))) {
        cout << "send:" << UDT::getlasterror().getErrorMessage() << endl;
        break;
      }

      ssize += ss;
    }
    
    if (ssize < size) break;
  }

  return NULL;
}
