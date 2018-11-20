#ifndef HTTP_RESPONSE
#define HTTP_RESPONSE

#include "HTTPRequest.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "TCPSocket.h"

class HTTPResponse
{
public:
/** Default constructor -- you must then set all fields of the response manually */
HTTPResponse();
/** Constructs an HTTPReponse object based on a request object */
HTTPResponse(HTTPRequest &request);

~HTTPResponse();

void SetHeaderField( string key, string value);
void Status(int status){
    m_status = status;
}

string &GetProtocol(){
    return m_protocol;
}

std::string GetHeader();

void SetProtocol(string protocol){
    m_protocol = protocol;    
}

void SetKeepAlive(bool ka){    
    headerValues["Connection"] = !ka ? "close" : "keep-alive";
}

private:
    std::string m_filename;
    uint64_t m_contentLength;
    uint32_t m_status;
    std::unordered_map<string,string> headerValues;
    std::string m_protocol;
};

#endif

