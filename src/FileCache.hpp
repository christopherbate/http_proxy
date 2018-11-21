#ifndef FILE_CACHE_H
#define FILE_CACHE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <exception>
#include "ICache.hpp"

class FileCache : public ICache
{
  public:
    FileCache(std::string cacheRoot, uint32_t cacheLife);
    ~FileCache();

    bool CheckCache(std::string);

    std::string GetCachedFile(std::string url);

    void Insert(std::string url, std::string &data);
    void InsertAppend(std::string url, std::string &data);

    

    std::string GetRoot(){
        return m_cacheRoot;
    }

  private:
    uint32_t m_cacheLife; // Seconds
    std::string m_cacheRoot;
    std::unordered_map<std::string, std::string> m_fileMap;
    std::unordered_map<std::string, uint64_t> m_timeMap;
};

#endif