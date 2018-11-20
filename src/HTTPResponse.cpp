#include "HTTPResponse.h"
#include <sstream>
#include <string>

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
    } else {
        headerValues["Connection"] = "close";
    }
    m_status = 200;
    m_contentLength = 0;
    m_protocol = request.GetProtocol();
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