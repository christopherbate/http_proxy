#ifndef URL_CACHE_H
#define URL_CACHE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <exception>
#include "ISocket.h"
#include "ICache.hpp"

class UrlCache : public ICache
{
public:
  UrlCache(std::string blacklistFilename);

  ~UrlCache()
  {
  }

  void Insert(std::string hostname, std::string port);

  bool CheckCache(std::string, std::string port);

  std::string Get(std::string url, std::string port);

  sockaddr_in *GetAddr(std::string url, std::string port);

  std::string GetBlackListFilename()
  {
    return m_blacklistFilename;
  }

  bool CheckBL(std::string host)
  {
    if (m_blackMap.find(host) != m_blackMap.end())
    {
      return true;
    }
    return false;
  }

  struct NoAddressException : std::exception
  {
  public:
    const char *what() const throw()
    {
      return "No address found for hostname";
    }
  };

  struct BlackListException : std::exception
  {
  public:
    const char *what() const throw()
    {
      return "Blacklisted";
    }
  };

private:
  std::string m_blacklistFilename;
  std::unordered_map<std::string, std::string> m_urlMap;
  std::unordered_map<std::string, struct sockaddr_in> m_addrMap;
  std::unordered_map<std::string, int> m_blackMap;
};

#endif