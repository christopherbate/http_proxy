#include "HTTPRequest.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include <iostream>

HTTPRequest::HTTPRequest(std::string &data)
{
    m_keepAlive = false;
    m_method = "";
    m_protocol = "";
    m_host = "";
    m_postContentLegnth = 0;
    ParseHeader(data);
}

void HTTPRequest::ParseHeader(std::string &data)
{
    // Tokenize the string by new lines first.
    std::stringstream stringStream(data);
    // Loop over all the lines.
    std::string line;
    std::string token;
    uint32_t index = 0;

    // Parse the header.
    while (std::getline(stringStream, line))
    {
        // Remove carriage feeds.
        line.erase(std::remove(line.begin(),line.end(),'\r'),line.end());

        // Loop over all the owrds.
        if (index == 0)
        {
            uint32_t wordIndex = 0;
            std::string token;
            std::stringstream lineStream(line);
            while (std::getline(lineStream, token, ' '))
            {
                switch (wordIndex)
                {
                case 0:
                    m_method = token;
                    break;
                case 1:
                    m_url = token;
                    break;
                case 2:
                    m_protocol = token;
                    break;
                }
                wordIndex++;
            }
            if (wordIndex != 3)
            {
                throw std::runtime_error("Not enough information in header.");
            }
        }
        else
        {
            size_t pos = line.find("Host:");
            if ( pos != std::string::npos)
            {
                m_host = line.substr(pos+5,line.length());
                m_host.erase(std::remove(m_host.begin(),m_host.end(),' '),m_host.end());
            }
            else
            {
                pos = line.find("Connection:");
                if (pos != std::string::npos )
                {
                    pos = line.find("Keep-alive");
                    if( pos == std::string::npos ){
                        pos = line.find("keep-alive");
                    }
                    m_keepAlive = pos != std::string::npos ? true : false;
                }
                else 
                {
                    pos = line.find("Content-Length:");
                    if(pos != std::string::npos)
                    {
                        m_postContentLegnth = strtoul(line.substr(pos+15,line.length()).c_str(),NULL,10);
                    }
                }                
            }
        }
        index++;
    }
    
    ParsePostData(data);
}

HTTPRequest::~HTTPRequest()
{
}

void HTTPRequest::ParsePostData(std::string &data)
{
    if(m_postContentLegnth == 0)
        return;

    std::cout << data << std::endl;

    // Find the "\r\n\r\n"
    data.erase(std::remove(data.begin(),data.end(),'\r'),data.end());
    size_t pos = data.find("\n\n");
    if(pos==std::string::npos){
        std::cerr<<"No nn delimmiter found."<<std::endl;
        m_postContentLegnth = 0;
        return;
    }
    // Copy the data.
    m_postData = data.substr(pos+2,m_postContentLegnth);

    // Verify length.
    if(m_postContentLegnth != m_postData.length())
    {
        std::cerr<<"Incorrect post data length "<<m_postContentLegnth<< " "<<m_postData.length() << std::endl<<m_postData<<std::endl;
    }
}

bool HTTPRequest::GetKeepAlive()
{
    return m_keepAlive;
}

std::string HTTPRequest::GetMethod()
{
    return m_method;
}

std::string HTTPRequest::GetUrl()
{
    return m_url;
}

std::string HTTPRequest::GetHost()
{
    return m_host;
}

std::string HTTPRequest::GetProtocol()
{
    return m_protocol;
}