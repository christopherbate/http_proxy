/*
TCPSocket.h
Christopher Bate
September 2018
*/
#ifndef SOCKET_DEVICE_H
#define SOCKET_DEVICE_H

#include "ISocket.h"

using namespace std;

/**
This is a class I wrote for PA1, modified here to use TCP sockets instead of UDP sockets

Implements TCPSockets using ISocket interface as a base class.

*/
class TCPSocket : public ISocket
{
public:
  TCPSocket(); 
  ~TCPSocket();  
  
  /** Creates a connecting socket */
  bool CreateSocket(string host, string port);

  /** Creates a listening socket */
  bool CreateSocket(string port);

  /** Listen for connection */
  bool Listen();

  TCPSocket *Accept();

  /** Blocking send to connect peer
   * Will return 0 if the socket is a passive listening socket.
  */
  uint32_t BlockingSend(const char *data, uint32_t length ) override;

  /** Blocking recv on socket.*/
  uint32_t BlockingRecv(char *buffer, uint32_t size ) override;

  void SetSocketFD(int fd){
    m_fd = fd;
  }

  void SetPeername( string name ){
    m_remoteHost = name;
  }

  string GetPeer(){
    return m_remoteHost;
  }
};

#endif