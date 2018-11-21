#include "HTTPResponse.h"
#include <sstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include <iostream>

/** Default constructor -- you must then set all fields of the response manually */
HTTPResponse::HTTPResponse()
{
    m_status = 200;
    m_contentLength = 0;
    m_protocol = "HTTP/1.1";
}
/** Constructs an HTTPReponse object based on a request object */
HTTPResponse::HTTPResponse(HTTPRequest &request)
{
    if (request.GetKeepAlive())
    {
        headerValues["Connection"] = "keep-alive";
    }
    else
    {
        headerValues["Connection"] = "close";
    }
    m_status = 200;
    m_contentLength = 0;
    m_protocol = request.GetProtocol();
}

HTTPResponse::HTTPResponse(std::string &data)
{
    using namespace std;

    // Find the end of the header
    size_t endPos = data.find("\r\n\r\n");
    string header;
    if (endPos == string::npos)
    {
        throw runtime_error("Could not find end of response header.");
    }
    else
    {
        header = data.substr(0, endPos + 4);
        m_hdrLength = header.length();
    }

    // Tokenize the string by new lines first.
    std::stringstream stringStream(header);
    string line;
    string token;
    uint32_t lineCnt = 0;

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
                    string error = "Malformed response line: " + line;
                    throw runtime_error(error.c_str());
                }
                m_protocol = reqTok[0];
                m_status = std::stoul(reqTok[1]);
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
                    headerValues[line.substr(0, cPos)] = line.substr(cPos + 1, line.length());
                }
            }
            lineCnt++;
        }
    }

    // Pase the encoding.
    if (headerValues.find("Content-Length") != headerValues.end())
    {
        m_encoding = NORMAL;
    }
    else if (headerValues.find("Transfer-Encoding") != headerValues.end())
    {
        if (headerValues["Transfer-Encoding"] == "chunked")
        {
            m_encoding = CHUNKED;
        }
        else
        {
            throw std::runtime_error("Transfer encoding error");            
        }
    }
    else
    {
        m_encoding = NORMAL;
    }
}

HTTPResponse::~HTTPResponse()
{
}

void HTTPResponse::SetHeaderField(string key, string value)
{
    headerValues[key] = value;
}

std::string HTTPResponse::GetHeader()
{
    std::string header(m_protocol);
    header += " " + to_string(m_status);
    if (m_status >= 200 && m_status <= 300)
        header += " Document Follows";

    header += "\r\n";

    for (auto it = headerValues.begin(); it != headerValues.end(); it++)
    {
        header += it->first + ": " + it->second + "\r\n";
    }

    header += "\r\n";

    return header;
}

HTTPChunk::HTTPChunk()
{
}

HTTPChunk::HTTPChunk(std::string &data, bool includesHeader)
{
    if (includesHeader)
    {
        size_t endPos = data.find("\r\n\r\n");
        if (endPos == string::npos)
        {
            throw runtime_error("Could not find end of response header.");
        }
        else
        {
            //std::string body = data.substr(endPos + 4, data.length());
            //ParseSize(body);
            ProcessData(data,endPos+4);
        }
    }
    else
    {
        ProcessData(data,0);
    }
}

uint32_t HTTPChunk::ProcessData(std::string &data, uint32_t skipBytes)
{
    std::istringstream ss(data);

    for (auto i = 0; i < skipBytes; i++)
    {
        ss.get();
        if (ss.gcount() == 0)
        {
            throw std::runtime_error("Short read on chunk parsing skip bytes.");
        }
    }

    return ParseSize(ss);
}

uint32_t HTTPChunk::ParseSize(std::istringstream &stringStream)
{
    // Tokenize the string by new lines first.    
    string line;
    uint32_t lineCnt = 0;
    uint32_t chunkSize = 0;
    m_containsEnd = false;
    m_size = 0;
    // Parse the header.
    while (std::getline(stringStream, line))
    {
        // Remove carriage feeds.
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.length() > 0)
        {
            //cout << "Got line: " << line << endl;
            chunkSize = std::stoul(line, nullptr, 16);
            m_size += chunkSize;
            //std::cout << "Found chunk size: " << std::hex << chunkSize << endl;
            if (chunkSize == 0)
            {
                m_containsEnd = true;
            }
            for (auto i = 0; i < chunkSize; i++)
            {
                stringStream.get();
                if (stringStream.gcount() == 0)
                {
                    return chunkSize-i;
                }
            }
        }
    }
    return 0;
}