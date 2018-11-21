#include "UrlCache.hpp"
#include <fstream>
#include "ISocket.h"

UrlCache::UrlCache(std::string blackListFilename) : m_blacklistFilename(blackListFilename)
{
    // Read in the blacklist.
    using namespace std;

    ifstream infile(blackListFilename, ifstream::in);
    if (!infile.is_open())
    {
        throw std::runtime_error("File should be open.");
    }

    for (std::string hostname; std::getline(infile, hostname);)
    {
        cout << "Black list parsing: " << hostname << endl;

        // Insert hostname into the black list
        m_blackMap[hostname] = 1;

        // Do a name lookup as well
        struct addrinfo *resList;
        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = 0;

        int rc = getaddrinfo(hostname.c_str(), "80", &hints, &resList);
        if (rc != 0)
            continue;
        if (resList == NULL)
            continue;

        struct addrinfo *aiIter = resList;
        while (aiIter != NULL)
        {
            char nameBuffer[INET_ADDRSTRLEN];
            char hostnameBuffer[1024];
            char service[20];

            if (inet_ntop(AF_INET, &(((struct sockaddr_in*)aiIter->ai_addr)->sin_addr), nameBuffer, INET_ADDRSTRLEN) != NULL)
            {
                cout << "Blacklisting " << hostname << ":" << nameBuffer << endl;
                m_blackMap[nameBuffer] = 1;
            }
            if (aiIter->ai_canonname)
            {
                m_blackMap[aiIter->ai_canonname] = 1;
            }
            aiIter = aiIter->ai_next;
        }
        freeaddrinfo(resList);
        cout << "Done with " << hostname << endl;
    }
}

void UrlCache::Insert(std::string hostname, std::string port)
{
    using namespace std;
    if (CheckBL(hostname))
    {
        throw UrlCache::BlackListException();
    }

    std::cout << "Inserting: " << hostname << ":" << port << std::endl;

    struct addrinfo *resList;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_CANONNAME | AI_NUMERICSERV;

    int rc = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &resList);
    if (rc != 0)
    {
        cerr << "UrlCache : getaddrinfo : " << rc << endl;
        throw UrlCache::NoAddressException();
    }

    // Check first for empty address.
    if (resList == NULL)
    {
        cerr << "TCPSocket : CreateSocket : no addresses at hostname/port" << endl;
        throw UrlCache::NoAddressException();
    }

    // Parse the list and try to connect.
    struct addrinfo *aiIter = resList;
    while (aiIter != NULL)
    {
        if (aiIter->ai_canonname)
        {
            std::cout << "Canon name: " << aiIter->ai_canonname << std::endl;
            if (CheckBL(aiIter->ai_canonname))
            {
                freeaddrinfo(resList);
                throw UrlCache::BlackListException();
            }
        }

        int fd = socket(aiIter->ai_family, aiIter->ai_socktype, aiIter->ai_protocol);
        if (fd == -1)
        {
            aiIter = aiIter->ai_next;
            continue;
        }        

        if (connect(fd, aiIter->ai_addr, aiIter->ai_addrlen) != 0)
        {
            cerr << "TCPSocket : CreateSocket : connect : " << strerror(errno) << endl;
            aiIter = aiIter->ai_next;
            close(fd);
            continue;
        }

        close(fd);
        char nameBuffer[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &(((struct sockaddr_in*)aiIter->ai_addr)->sin_addr), nameBuffer, INET_ADDRSTRLEN) == NULL)
        {
            freeaddrinfo(resList);
            throw runtime_error("Failed to convert address to text.");
        }
        else
        {
            if(CheckBL(nameBuffer))
            {
                freeaddrinfo(resList);
                cout<<"Blacklisted: "<<nameBuffer<<endl;
                throw UrlCache::BlackListException();
            }
            cout << "Saving " << hostname << ":" << nameBuffer << endl;
            m_urlMap[hostname + port] = string(nameBuffer, INET_ADDRSTRLEN);
            sockaddr_in addr = *((sockaddr_in *)aiIter->ai_addr);
            m_addrMap[hostname + port] = addr;
        }
        break;
    }

    // Free address info list
    freeaddrinfo(resList);
}

std::string UrlCache::Get(std::string url, std::string port)
{
    std::string key = url + port;
    if (m_urlMap.find(key) != m_urlMap.end())
    {
        return m_urlMap[key];
    }
    else
    {
        throw ICache::CacheMissException();
    }
}

sockaddr_in *UrlCache::GetAddr(std::string url, std::string port)
{
    std::string key = url + port;
    if (m_addrMap.find(key) != m_addrMap.end())
    {
        return &m_addrMap[key];
    }
    else
    {
        throw ICache::CacheMissException();
    }
}

bool UrlCache::CheckCache(std::string hostname, std::string port)
{
    std::string key = hostname + port;
    if (m_urlMap.find(key) != m_urlMap.end())
    {
        std::cout << "Hostname in cache" << std::endl;

        return true;
    }

    return false;
}
