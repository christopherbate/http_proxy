/**
 * Session.cpp
 * Christopher Bate
 */
#ifndef CTB_SESSION_H
#define CTB_SESSION_H

#include <chrono>

#include "TCPSocket.h"
#include "HTTPResponse.h"
#include "HTTPRequest.h"
#include "FileManager.h"

class Session
{
public:
  Session(TCPSocket *connection, const char *root);

  ~Session();

  void Run();

  void HandleGET(HTTPRequest &request, HTTPResponse &response);
  void HandlePOST(HTTPRequest &request, HTTPResponse &response);

  void SendError(HTTPRequest &request);
  void SendError();

private:
  TCPSocket *m_connection;
  FileManager m_fileManager;
};

#endif