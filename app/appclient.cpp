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

int main(int argc, char* argv[]) {
  if (((3 != argc) || (0 == atoi(argv[2]))) && ((5 != argc) || strcmp(argv[3], "-t") || (0 == atoi(argv[4])))) {
    cout << "usage: appclient server_ip server_port [-t] [seconds]" << endl;
    return 0;
  }

  // Automatically start up and clean up UDT module.
  UDTUpDown _udt_;

  struct addrinfo hints, *local, *peer;

  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if (0 != getaddrinfo(NULL, "9000", &hints, &local)) {
    cout << "incorrect network address." << endl;
    return 0;
  }

  UDTSOCKET client = UDT::socket(local->ai_family, local->ai_socktype, local->ai_protocol);

  // Windows UDP issue
  // For better performance, modify HKLM\System\CurrentControlSet\Services\Afd\Parameters\FastSendDatagramThreshold
  #ifdef WIN32
    UDT::setsockopt(client, 0, UDT_MSS, new int(1052), sizeof(int));
  #endif

  freeaddrinfo(local);

  if (0 != getaddrinfo(argv[1], argv[2], &hints, &peer)) {
    cout << "incorrect server/peer address. " << argv[1] << ":" << argv[2] << endl;
    return 0;
  }

  // connect to the server, implict bind
  if (UDT::ERROR == UDT::connect(client, peer->ai_addr, peer->ai_addrlen)) {
    cout << "connect: " << UDT::getlasterror().getErrorMessage() << endl;
    return 0;
  }

  freeaddrinfo(peer);

  // using CC method

  int size = 100000;
  char* data = new char[size];

  pthread_create(new pthread_t, NULL, monitor, &client);

  for (int i = 0; i < 1000000; ++i) {
    int ssize = 0;
    int ss;
    while (ssize < size) {
      if (UDT::ERROR == (ss = UDT::send(client, data + ssize, size - ssize, 0))) {
        cout << "send:" << UDT::getlasterror().getErrorMessage() << endl;
        break;
      }

      ssize += ss;
    }

    if (ssize < size) break;
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
