#include "Session.h"
#include <chrono>
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include <cassert>

using namespace std;

Session::Session(TCPSocket *connection, const char *root) : m_connection(connection), m_fileManager(root)
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
    const unsigned timeout = 10000000;
    m_connection->SetRecvTimeout(timeout);
    auto startTimer = chrono::system_clock::now();
    // Receive data
    char data[2048];
    uint32_t length = 0;

    cout << "Running session." << endl;

    while (1)
    {
        //assert(m_connection);
        //assert(!m_connection->IsBad());

        do
        {
            length = m_connection->BlockingRecv(data, 2048);

            if (m_connection->DidTimeout())
            {
                auto timeCheck = chrono::system_clock::now();
                if (chrono::duration_cast<chrono::microseconds>(timeCheck - startTimer).count() > timeout)
                {
                    cout << "Keep alive expired" << endl;
                    cout << "Ending session" << endl;
                    m_connection->CloseSocket();
                    return;
                }
            }
            else if (length == 0)
            {
                cout << "Peer closed connection." << endl;
                m_connection->CloseSocket();
                return;
            }

        } while (length == 0);

        // Display tthe header information (for fun)
        try
        {
            cout << "Received request of length: " << std::dec << length << endl;
            string dataString = string(data, length);
            HTTPRequest request(dataString);
            cout << request.GetUrl() << endl;

            // Print header information.
            cout << "Header:" << endl;
            cout << request.GetMethod() << endl;
            cout << request.GetUrl() << endl;
            cout << request.GetProtocol() << endl;
            cout << request.GetHost() << endl;
            cout << "Keepalive:" << request.GetKeepAlive() << endl;
            if (request.GetMethod() == "POST")
            {
                cout << "Post CL: " << request.GetContentLength() << endl;
                cout << request.GetPostData() << endl;
            }

            // Create response.
            HTTPResponse response(request);

            // If the url is present, retrieve the corresponding file.
            if (request.GetMethod() == "GET")
            {
                HandleGET(request, response);
            }
            else if (request.GetMethod() == "POST")
            {
                HandlePOST(request, response);
            }
            else
            {
                SendError(request);
            }

            if (request.GetKeepAlive())
            {
                cout << "Starting keep alive timer" << endl;
                startTimer = chrono::system_clock::now();
            }
            else
            {
                break;
            }
        }
        catch (std::exception &e)
        {
            cout << e.what() << endl;
            SendError();
            continue;
        }
    }

    cout << "Ending session" << endl;

    m_connection->CloseSocket();
}

void Session::HandleGET(HTTPRequest &request, HTTPResponse &response)
{
    std::string url = request.GetUrl();
    std::string filename;
    uint64_t size = 0;
    if (url != "")
    {
        try
        {
            filename = m_fileManager.GetFilename(url, size);

            response.SetHeaderField("Content-Length", to_string(size));
            response.SetHeaderField("Content-Type", m_fileManager.GetContentType(filename));
        }
        catch (std::runtime_error &e)
        {
            cerr << "Exception in URL processing. " << e.what() << endl;
            SendError(request);
            return;
        }
    }

    // Display response.
    std::string respHeader = response.GetHeader();

    m_connection->BlockingSend(respHeader.c_str(), respHeader.length());

    // Send the file.
    char buffer[1024];
    if (size > 0)
    {
        ifstream infile(filename);
        while (!infile.eof())
        {
            infile.read(buffer, 1024);
            m_connection->BlockingSend(buffer, infile.gcount());
        }
        infile.close();
    }
}

void Session::HandlePOST(HTTPRequest &request, HTTPResponse &response)
{
    std::cout << "Returning post response " << endl;
    std::string url = request.GetUrl();
    std::string filename;
    std::string postData = "<h1>POST DATA</h1><pre>";
    uint64_t size = 0;
    uint64_t postDataLength = 0;
    if (url != "")
    {
        try
        {
            if (request.GetContentLength() > 0)
            {
                postData += request.GetPostData();
            }
            postData += "</pre>";
            postDataLength = postData.length();
            filename = m_fileManager.GetFilename(url, size);
            response.SetHeaderField("Content-Length", to_string(size + postDataLength));
            response.SetHeaderField("Content-Type", m_fileManager.GetContentType(filename));
        }
        catch (std::runtime_error &e)
        {
            cerr << "Exception in URL processing. " << e.what() << endl;
            SendError(request);
            return;
        }
    }

    // Display response.
    std::string respHeader = response.GetHeader();

    uint32_t result = m_connection->BlockingSend(respHeader.c_str(), respHeader.length());

    if (result <= 0)
    {
        return;
    }

    // Send the file.
    char buffer[1024000];
    if (size > 0)
    {
        ifstream infile(filename);
        while (!infile.eof())
        {
            infile.read(buffer, 1024000);
            // Look for the body tag.
            string data(buffer, infile.gcount());
            size_t pos = data.find("<body>");
            if (pos != std::string::npos)
            {
                data.insert(pos + 6, postData);
            }

            result = m_connection->BlockingSend(data.c_str(), data.length());

            if (result <= 0)
            {
                break;
            }
        }
        infile.close();
    }
}

void Session::SendError(HTTPRequest &request)
{
    HTTPResponse response(request);
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
