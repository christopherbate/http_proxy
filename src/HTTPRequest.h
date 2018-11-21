#ifndef HTTP_REQ_CTB
#define HTTP_REQ_CTB

#include <string>
#include <unordered_map>

class HTTPRequest
{
  public:
    HTTPRequest(std::string &data);
    HTTPRequest();
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
    std::string GetPort(){
      return m_port;
    }
    std::string GetProxyTargetHost(){
      return m_proxyTargetHost;
    }
    std::string GetUrl();

    void ParseUrl(std::string);

    bool CheckAbsolute(std::string);

    void ParsePostData(std::string &data);

    void SetContentLength( uint64_t length){
      m_postContentLegnth = length;
    }
    void SetProtocol(std::string protocol){
      m_protocol = protocol;
    }
    void SetMethod(std::string method){
      m_method = method;
    }
    std::string &GetProxyRequest(){
      CreateProxyRequest();
      return m_proxyRequest;
    }
    void CreateProxyRequest();    
  private:    
    std::unordered_map<std::string,std::string> m_headerMap;  
    bool m_keepAlive;
    uint64_t m_postContentLegnth;
    std::string m_postData;    
    std::string m_protocol;
    std::string m_method;
    std::string m_url;
    std::string m_proxyTargetHost;
    std::string m_port;
    std::string m_proxyRequest;
};

#endif