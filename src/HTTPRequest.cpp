#include "HTTPRequest.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <string>
#include <iostream>

HTTPRequest::HTTPRequest(std::string &data)
{
    m_keepAlive = false;
    m_method = "";
    m_protocol = "";
    m_postContentLegnth = 0;
    m_proxyTargetHost = "";
    m_port = "80";

    ParseHeader(data);
}
HTTPRequest::HTTPRequest()
{
    m_keepAlive = false;
    m_method = "";
    m_protocol = "";
    m_postContentLegnth = 0;
    m_proxyTargetHost = "";
    m_port = "80";
}

void HTTPRequest::ParseHeader(std::string &data)
{
    using namespace std;

    // Loop over all the lines.
    std::string line;
    std::string token;
    uint32_t lineCnt = 0;

    // Find the end of the header
    size_t endPos = data.find("\r\n\r\n");
    string header;
    if (endPos == string::npos)
    {
        throw runtime_error("Could not find end of header.");
    }
    else
    {
        header = data.substr(0, endPos + 4);
    }

    // Tokenize the string by new lines first.
    std::stringstream stringStream(header);

    // Parse the header.
    while (std::getline(stringStream, line))
    {
        // Remove carriage feeds.
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.length() > 0)
        {
            if (lineCnt == 0)
            {
                // Request line
                stringstream iss(line);
                vector<string> reqTok;
                copy(istream_iterator<string>(iss),
                     istream_iterator<string>(),
                     back_inserter(reqTok));

                if (reqTok.size() < 3)
                {
                    string error = "Malformed req line: " + line;
                    throw runtime_error(error.c_str());
                }
                m_method = reqTok[0];
                ParseUrl(reqTok[1]);
                m_protocol = reqTok[2];
            }
            else
            {
                // Other lines
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
                line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
                size_t cPos = line.find(":");
                if (cPos == string::npos)
                {
                    string error = "Malformed line: " + line;
                    throw runtime_error(error.c_str());
                }
                else
                {
                    m_headerMap[line.substr(0, cPos)] = line.substr(cPos + 1, line.length());
                }
            }
            lineCnt++;
        }
    }

    // Parse the keep alive.
    if (m_headerMap.find("Connection") != m_headerMap.end())
    {
        if (m_headerMap["Connection"] == "keep-alive")
        {
            m_keepAlive = true;
        }
        else if (m_headerMap["Connection"] == "close")
        {
            m_keepAlive = false;
        }
        else
        {
            throw std::runtime_error("Incorrect connection header.");
        }
    }

    // Check for post data
    ParsePostData(data);
}

HTTPRequest::~HTTPRequest()
{
}

void HTTPRequest::ParsePostData(std::string &data)
{
    // Check for content length header.
    if (m_headerMap.find("Content-Length") == m_headerMap.end())
    {
        m_postContentLegnth = 0;
        return;
    }

    m_postContentLegnth = std::stoull(m_headerMap["Content-Length"].c_str());

    // Find the "\r\n\r\n"
    //data.erase(std::remove(data.begin(), data.end(), '\r'), data.end());
    size_t pos = data.find("\r\n\r\n");
    if (pos == std::string::npos)
    {
        //std::cerr << "No nn delimmiter found." << std::endl;
        throw std::runtime_error("Malformed header given to post data parser.");
        m_postContentLegnth = 0;
        return;
    }
    // Copy the data.
    std::cout << "Retrieving post data." << std::endl;
    m_postData = data.substr(pos + 4, data.length());

    // Verify length.
    if (m_postContentLegnth != m_postData.length())
    {
        std::cerr << "Incorrect post data length " << m_postContentLegnth << " " << m_postData.length() << std::endl
                  << m_postData << std::endl;
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
    if (m_headerMap.find("Host") != m_headerMap.end())
    {
        return m_headerMap["Host"];
    }
    else
    {
        return "";
    }
}

std::string HTTPRequest::GetProtocol()
{
    return m_protocol;
}

void HTTPRequest::ParseUrl(std::string data)
{
    if (CheckAbsolute(data))
    {
        //Remove the protocol prefix
        size_t prefixPos = data.find("://");
        size_t portPos = 0;
        if (prefixPos != std::string::npos)
        {
            data = data.substr(prefixPos + 3, data.length());
            //std::cout<<"Removed protocol prefix: "<<data<<std::endl;
        }

        // Find the port.
        portPos = data.find(":");
        if (portPos != std::string::npos)
        {
            m_port = data.substr(portPos + 1, data.length());
            data = data.substr(0, portPos);
            //std::cout<<"Found port: "<<m_port<<std::endl;
        }
        else
        {
            m_port = "80";
        }

        // Extract the hostname and the uri
        size_t slashPos = data.find("/");
        if (slashPos != std::string::npos)
        {
            m_proxyTargetHost = data.substr(0, slashPos);
            m_url = data.substr(slashPos, data.length());
            //std::cout<<"Found url: "<<m_url<<std::endl;
            //std::cout<<"Found target-host: "<<m_proxyTargetHost<<std::endl;
        }
        else
        {
            m_proxyTargetHost = data;
            m_url = "/";
        }
    }
    else
    {
        m_url = data;
        m_port = "80";
    }
}

/**
 * Check absolute
 * This checks whether url form is absolute (used in proxy requrests)
 */
bool HTTPRequest::CheckAbsolute(std::string url)
{
    if (url.find("http://") != std::string::npos || url.find("https://") != std::string::npos)
    {
        return true;
    }
    return false;
}

/**
 * This creates the correctly formed forwarding request
 */
void HTTPRequest::CreateProxyRequest()
{
    using namespace std;

    m_proxyRequest.clear();

    vector<string> headerBlacklist = {
        "Connection",
        "Upgrade"};

    // Create the request line.
    m_proxyRequest = m_method + " " + m_url + " " + m_protocol + "\r\n";

    // Copy over the headers.
    for (std::pair<string, string> element : m_headerMap)
    {
        bool skip = false;
        for (auto bl = headerBlacklist.begin(); bl != headerBlacklist.end(); bl++)
        {
            if((*bl)==element.first){
                skip = true;
                break;
            }                
        }
        if(!skip){
            m_proxyRequest+=element.first+":"+element.second+"\r\n";
        }
    }
    //m_proxyRequest+="Connection:close\r\n";

    m_proxyRequest += "\r\n";
}