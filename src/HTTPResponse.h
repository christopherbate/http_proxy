#ifndef HTTP_RESPONSE
#define HTTP_RESPONSE

#include "HTTPRequest.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "TCPSocket.h"

class HTTPChunk
{
  public:
    HTTPChunk(std::string &data, bool includesHeader);
    HTTPChunk();
    uint32_t GetChunkSize( ){
        return m_size;
    }
    uint32_t ProcessData(std::string &data,uint32_t skipBytes = 0);
    bool GetContainsEnd(){return m_containsEnd;}
    uint32_t ParseSize(std::istringstream &stringStream);
    uint32_t m_size;
    bool m_containsEnd;
};

class HTTPResponse
{
  public:
    /** Default constructor -- you must then set all fields of the response manually */
    HTTPResponse();
    /** Constructs an HTTPReponse object based on a request object */
    HTTPResponse(HTTPRequest &request);

    HTTPResponse(std::string &data);

    ~HTTPResponse();

    void SetHeaderField(string key, string value);
    void Status(int status)
    {
        m_status = status;
    }

    uint32_t GetContentLength()
    {
        if (headerValues.find("Content-Length") != headerValues.end())
        {
            return std::stoul(headerValues["Content-Length"]);
        }
        else
        {
            return 0;
        }
    }

    string &GetProtocol()
    {
        return m_protocol;
    }

    std::string GetHeader();

    void SetProtocol(string protocol)
    {
        m_protocol = protocol;
    }

    void SetKeepAlive(bool ka)
    {
        headerValues["Connection"] = !ka ? "close" : "keep-alive";
    }

    enum Encoding
    {
        NORMAL = 0,
        CHUNKED = 1
    };

    Encoding GetEncodingType()
    {
        return m_encoding;
    }

    uint32_t GetHeaderLength()
    {
        return m_hdrLength;
    }

  private:
    std::string m_filename;
    uint32_t m_hdrLength;
    Encoding m_encoding;
    uint64_t m_contentLength;
    uint32_t m_status;
    std::unordered_map<string, string> headerValues;
    std::string m_protocol;
};

#endif
