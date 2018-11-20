#ifndef HTTP_REQ_CTB
#define HTTP_REQ_CTB

#include <string>

class HTTPRequest
{
  public:
    HTTPRequest(std::string &data);
    ~HTTPRequest();
    void ParseHeader( std::string &data );
    std::string GetHeader();
    bool GetKeepAlive();
    std::string GetProtocol();
    std::string GetHost();
    std::string GetMethod();
    uint64_t GetContentLength()
    {
      return m_postContentLegnth;
    }
    std::string &GetPostData()
    {
      return m_postData;
    }
    std::string GetUrl();

    void ParsePostData(std::string &data);
  private:
    bool m_keepAlive;
    uint64_t m_postContentLegnth;
    std::string m_postData;
    std::string m_method;
    std::string m_protocol;
    std::string m_url;
    std::string m_host;
};

#endif