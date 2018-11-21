/**
 * FileCache.cpp
 */
#include "FileCache.hpp"
#include <fstream>
#include <iostream>
#include <functional>
#include <thread>
#include <regex>
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "TCPSocket.h"
#include "Session.h"

using namespace std;

/**
 * Constructor
 */
FileCache::FileCache(std::string cacheRoot, uint32_t cacheLife) : m_cacheLife(cacheLife), m_cacheRoot(cacheRoot)
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
bool FileCache::CheckCache(std::string url)
{
    uint64_t timeNow = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
    if (m_timeMap.find(url) != m_timeMap.end())
    {
        cout << "Item in cache" << endl;
        uint64_t ct = m_timeMap[url];
        //cout<<"TimeNow: "<<timeNow+" cache time: "<<ct<<" diff: "<<(timeNow-ct)<<endl;
        if (timeNow - ct < m_cacheLife)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

/**
 * RunPrefetch
 * Prefetches links in the given file
 */
void FileCache::RunPrefetch(std::string &url)
{
    try
    {
        std::string filename = GetCachedFile(url);

        std::thread prefetchThread([this, filename]() {
            PrefetchThread(filename);
        });

        prefetchThread.detach();
    }
    catch (std::exception &e)
    {
        return;
    }
}

/**
 * Prefetch thread function
 * for executing prefetch.
 */
void FileCache::PrefetchThread(std::string filename)
{
    ifstream infile(filename, ifstream::in);
    static uint32_t count = 0;
    for (std::string line; std::getline(infile, line);)
    {
        regex urlMatch("\"http://[a-z0-9./?-_]*\"");
        smatch m;
        if (regex_search(line, m, urlMatch))
        {
            for (auto x : m)
            {
                try
                {
                    // Remove quotes
                    string match = x.str();
                    match.erase(std::remove(match.begin(), match.end(), '\"'), match.end());

                    // Request the file.
                    string request = "GET " + match + " HTTP/1.1\r\n\r\n";                    

                    HTTPRequest req(request);
                    req.SetHeader("Host",req.GetProxyTargetHost());

                    // Send the request.
                    TCPSocket out;
                    out.CreateSocket(req.GetProxyTargetHost(), req.GetPort());

                    std::string &reqData = req.GetProxyRequest();                    

                    out.BlockingSend(reqData.c_str(), reqData.length());

                    Session::ProcessResponse(NULL,&out,this,req.GetProxyTargetHost()+req.GetUrl());                    
                    count++;
                }
                catch (std::exception &e)
                {
                    cerr << "Error with prefetch: " << e.what();
                    return;
                }
            }
        }
    }

    cout <<"Prefetched "<<count<<" files."<<endl;
    infile.close();
}

/**
 * GetCachedFile
 */
std::string FileCache::GetCachedFile(std::string url)
{
    if (m_fileMap.find(url) != m_fileMap.end())
    {
        return m_fileMap[url];
    }
    else
    {
        throw CacheMissException();
    }
}

/**
 * Insert
 */
void FileCache::Insert(std::string url, std::string &data)
{
    size_t urlHash = hash<string>{}(url);
    string filename = m_cacheRoot + to_string(urlHash);
    ofstream outfile;
    outfile.open(filename.c_str(), ofstream::out);

    if (!outfile.is_open())
    {
        string error = "Could not open: " + filename;
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
    string filename = m_cacheRoot + to_string(urlHash);
    ofstream outfile;
    outfile.open(filename.c_str(), ofstream::out | ofstream::app);
    if (!outfile.is_open())
    {
        string error = "Could not open: " + filename;
        throw runtime_error(error.c_str());
    }

    outfile.write(data.c_str(), data.length());

    m_fileMap[url] = filename;
    uint64_t timeNow = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
    m_timeMap[url] = timeNow;

    outfile.close();
}