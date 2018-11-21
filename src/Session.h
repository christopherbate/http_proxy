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
#include "FileCache.hpp"
#include "UrlCache.hpp"


class Session
{
public:
  Session(TCPSocket *connection, FileCache *cache,UrlCache *urlCache);

  ~Session();

  void Run();

  void SendError(HTTPRequest &request, int code = 500, std::string error = "Proxy Error");
  void SendError();  

  static void ProcessResponse(TCPSocket *forwardSocket,
           TCPSocket *recvSocket, FileCache *fCache, string totalUrl  );

private:
  TCPSocket *m_connection;
  TCPSocket m_outgoing;
  FileManager m_fileManager;
  FileCache *m_cache;
  UrlCache *m_urlCache;
};

#endif