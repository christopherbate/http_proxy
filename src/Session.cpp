#include "Session.h"
#include <chrono>
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include <cassert>
//#include <rand>
#include <cstdlib>
#include <thread>

using namespace std;

Session::Session(TCPSocket *connection, FileCache *cache, UrlCache *urlCache)
    : m_connection(connection), m_cache(cache), m_fileManager(cache->GetRoot().c_str()), m_urlCache(urlCache)
{
}

Session::~Session()
{
    // Close connection
    if (m_connection)
        delete m_connection;
}

void Session::Run()
{
    if (!m_connection)
    {
        throw std::runtime_error("Connection socket is not set.");
    }

    // Receive data
    char data[20048];
    uint32_t length = 0;
    auto tid = rand();

    m_connection->SetRecvTimeout(0);
    std::cout << tid << ":"
              << "Running session." << endl;
    while (1)
    {
        // Attempt to read in request from user.
        length = m_connection->BlockingRecv(data, 20048);
        if (length == 0)
        {
            std::cout << tid << ":"
                      << "Peer closed connection." << endl;
            m_connection->CloseSocket();
            break;
        }

        // Process proxy request.
        try
        {
            string requestString = string(data, length);
            HTTPRequest request(requestString);
            string totalUrl = request.GetProxyTargetHost() + request.GetUrl();
            // Print header information.
            std::cout << tid << ":Received: " << request.GetMethod() << " " << totalUrl << endl;

            // REQUIREMENT: REJECT ALL BESIDES GET
            if (request.GetMethod() != "GET")
            {
                std::cout << tid << ":"<< "Not GET method, rejecting with error code 400." << endl;
                SendError(request, 400, "Proxy Error: Incorrect method.");
                continue;
            }

            // REQUIREMENT: Use DNS cache and return error if name invalid
            TCPSocket proxyOut;
            std::string proxyTarget = request.GetProxyTargetHost();
            std::string targetPort = request.GetPort();
            try
            {
                if (!m_urlCache->CheckCache(proxyTarget, targetPort))
                {
                    m_urlCache->Insert(proxyTarget,targetPort);
                } else {
                    cout<<"DNS CACHE HIT: "<<proxyTarget<<":"<<targetPort<<endl;
                }
                proxyOut.CreateSocket(m_urlCache->GetAddr(proxyTarget, targetPort));
                proxyOut.SetRecvTimeout(0);
            }
            catch (UrlCache::NoAddressException &e)
            {
                std::cout << tid << ":"
                          << "Invalid hostname request, sending code 400." << endl;
                SendError(request, 400, "Proxy Error: That URL could not be located.");
                proxyOut.CloseSocket();
                continue;
            }
            catch(UrlCache::BlackListException &e){
                std::cout << tid <<":"<<"Black list hit "<<proxyTarget<<endl;
                SendError(request,403, "That host has been blacklisted.");
                continue;
            }
            catch (std::exception &e)
            {
                throw e;
            }


            // REQUIREMENT: Check for cached data.
            if (m_cache->CheckCache(totalUrl))
            {
                cout << "CACHE HIT" << endl;

                std::string filename = m_cache->GetCachedFile(totalUrl);

                ifstream infile(filename.c_str(), ifstream::binary | ifstream::in);

                do
                {
                    infile.read(data, 20048);
                    m_connection->BlockingSend(data, infile.gcount());
                } while (infile.gcount() == 20048);

                proxyOut.CloseSocket();
                continue;
            }

            // No data cached - forward request
            std::string &reqData = request.GetProxyRequest();
            proxyOut.BlockingSend(reqData.c_str(), reqData.length());

            // Accept the response
            proxyOut.SetRecvTimeout(0);
            length = proxyOut.BlockingRecv(data, 20048);
            uint32_t total = 0;

            // Process the response.
            if (length > 0)
            {
                std::string respData = std::string(data, length);
                HTTPResponse response(respData);

                m_cache->Insert(totalUrl, respData);

                // Chunked Response
                if (response.GetEncodingType() == HTTPResponse::CHUNKED)
                {
                    HTTPChunk chunk;
                    uint32_t skip = response.GetHeaderLength();
                    do
                    {
                        m_connection->BlockingSend(data, length);

                        if (length < skip)
                        {
                            skip = skip - length;
                        }
                        else
                        {
                            skip = chunk.ProcessData(respData, skip);
                        }

                        if (chunk.GetContainsEnd())
                        {
                            break;
                        }

                        length = proxyOut.BlockingRecv(data, 20048);
                        respData = std::string(data, length);
                        m_cache->InsertAppend(totalUrl, respData);
                    } while (1);
                }
                // Normal Response
                else
                {
                    m_connection->BlockingSend(data, length);
                    total = (length - response.GetHeaderLength());

                    if (total == response.GetContentLength())
                    {
                        continue;
                    }
                    else
                    {
                        while (total < response.GetContentLength())
                        {
                            length = proxyOut.BlockingRecv(data, 20048);
                            total += length;
                            m_connection->BlockingSend(data, length);
                            string newData(data, length);
                            m_cache->InsertAppend(totalUrl, newData);
                        }
                    }
                }
            }
            else if (proxyOut.DidTimeout())
            {
                cout << "Timed out." << endl;
            }
        }
        catch (std::exception &e)
        {
            std::cout << tid << ": Exception :" << e.what() << endl;
            SendError();
            continue;
        }
    }

    std::cout << tid << ":"
              << "Ending session" << endl;

    m_connection->CloseSocket();
}

void Session::SendError(HTTPRequest &request, int code, std::string error)
{
    HTTPResponse response(request);
    response.Status(code);
    response.SetKeepAlive(false);
    response.SetHeaderField("Content-Length", std::to_string(error.size()));
    response.SetHeaderField("Content-Type", "text/plain");
    // Display response.
    std::string respHeader = response.GetHeader();

    uint32_t result = m_connection->BlockingSend(respHeader.c_str(), respHeader.length());
    if (result <= 0)
    {
        return;
    }
    result = m_connection->BlockingSend(error.c_str(), error.length());
    if (result <= 0)
    {
        return;
    }
}

void Session::SendError()
{
    HTTPResponse response;
    response.Status(500);
    response.SetKeepAlive(false);
    response.SetHeaderField("Content-Length", "0");
    response.SetHeaderField("Content-Type", "text/plain");
    // Display response.
    std::string respHeader = response.GetHeader();

    uint32_t result = m_connection->BlockingSend(respHeader.c_str(), respHeader.length());
    if (result <= 0)
    {
        return;
    }
}