/**
 * FileCache.cpp
 */
#include "FileCache.hpp"
#include <fstream>
#include <iostream>
#include <functional>

using namespace std;

/**
 * Constructor
 */
FileCache::FileCache(std::string cacheRoot, uint32_t cacheLife ): m_cacheLife(cacheLife),m_cacheRoot(cacheRoot)
{

}

/**
 * Destructor
 */
FileCache::~FileCache()
{
    
}

/**
 * CheckCache
 * Checks if item is in cache or not and valid.
 */
bool FileCache::CheckCache( std::string url )
{
    uint64_t timeNow = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
    if( m_timeMap.find(url) != m_timeMap.end() )
    {
        cout<<"Item in cache"<<endl;
        uint64_t ct = m_timeMap[url];
        //cout<<"TimeNow: "<<timeNow+" cache time: "<<ct<<" diff: "<<(timeNow-ct)<<endl;
        if(timeNow - ct < m_cacheLife){
            return true;
        } else {
            return false;
        }
    }

    return false;
}

/**
 * GetCachedFile
 */
std::string FileCache::GetCachedFile( std::string url)
{
    if( m_fileMap.find(url) != m_fileMap.end() )
    {
        return m_fileMap[url];
    } else {
        throw CacheMissException();
    }
}

/**
 * Insert
 */
void FileCache::Insert(std::string url, std::string &data)
{
    size_t urlHash = hash<string>{}(url);
    string filename = m_cacheRoot+to_string(urlHash);    
    ofstream outfile;
    outfile.open( filename.c_str() , ofstream::out);

    if(!outfile.is_open()){
        string error = "Could not open: "+filename;
        throw runtime_error(error.c_str());
    }

    outfile.write(data.c_str(), data.length());

    m_fileMap[url] = filename;
    uint64_t timeNow = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
    m_timeMap[url] = timeNow;

    outfile.close();
}

/**
 * Insert
 */
void FileCache::InsertAppend(std::string url, std::string &data)
{
    size_t urlHash = hash<string>{}(url);
    string filename = m_cacheRoot+to_string(urlHash);    
    ofstream outfile;
    outfile.open( filename.c_str() , ofstream::out|ofstream::app);
    if(!outfile.is_open()){
        string error = "Could not open: "+filename;
        throw runtime_error(error.c_str());
    }

    outfile.write(data.c_str(), data.length());

    m_fileMap[url] = filename;
    uint64_t timeNow = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
    m_timeMap[url] = timeNow;

    outfile.close();
}