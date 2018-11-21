#include "QuickTestCPP.h"
#include "../src/HTTPRequest.h"
#include "../src/HTTPResponse.h"
#include "../src/TCPSocket.h"
#include "../src/Session.h"
#include "../src/FileManager.h"
#include "../src/FileCache.hpp"
#include "../src/UrlCache.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
using namespace std;
int main(int argc, char **argv)
{
    std::cout << "Starting tests." << std::endl;

    TestRunner::GetRunner()->AddTest(
        "HTTP Response Creation",
        "Must create default protocol to HTTP/1.1",
        []() {
            HTTPResponse response;

            if (response.GetProtocol() != "HTTP/1.1")
            {
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Response Creation",
        "Must create default header with status 200",
        []() {
            HTTPResponse response;

            cout << response.GetHeader();

            if (response.GetHeader() != "HTTP/1.1 200 Document Follows\r\n\r\n")
            {
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Response Creation",
        "Must be able to set status code.",
        []() {
            HTTPResponse response;

            response.Status(500);

            cout << response.GetHeader();

            if (response.GetHeader() != "HTTP/1.1 500\r\n\r\n")
            {
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Response Creation",
        "Must be able to set headers.",
        []() {
            HTTPResponse response;

            response.SetHeaderField("Content-Type", "text/html");

            cout << response.GetHeader();

            if (response.GetHeader() != "HTTP/1.1 200 Document Follows\r\nContent-Type: text/html\r\n\r\n")
            {
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Response Creation",
        "Must be able to create resposne header from HTTPRequest header.",
        []() {
            string hdr = "GET /Protocols/rfc1945/rfc1945 HTTP/1.0\r\n\r\n";

            HTTPRequest request(hdr);
            HTTPResponse response(request);

            cout << response.GetHeader();

            if (response.GetHeader() != "HTTP/1.0 200 Document Follows\r\nConnection: close\r\n\r\n")
            {
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Response Creation",
        "Must be able to create resposne from respone data.",
        []() {
            string hdr = "HTTP/1.1 200 OK\r\nDate: Tue, 20 Nov 2018\r\nConnection: close\r\nContent-Length: 1994\r\n\r\n";

            HTTPResponse resp(hdr);

            if (resp.GetProtocol() != "HTTP/1.1")
            {
                return 0;
            }

            if (resp.GetContentLength() != 1994)
            {
                cerr << "Incorrect content length: " << resp.GetContentLength() << endl;
                return 0;
            }

            if (resp.GetEncodingType() != HTTPResponse::NORMAL)
            {
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Response Creation",
        "Must be able to create resposne from respone data with chunked encoding.",
        []() {
            string hdr = "HTTP/1.1 200 OK\r\nDate: Tue, 20 Nov 2018\r\nConnection: close\r\nTransfer-Encoding: chunked\r\n\r\n";

            HTTPResponse resp(hdr);

            if (resp.GetProtocol() != "HTTP/1.1")
            {
                return 0;
            }

            if (resp.GetEncodingType() != HTTPResponse::CHUNKED)
            {
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Chunk Parsing ",
        "Must be able to parse chunks",
        []() {
            string hdr = "HTTP/1.1 200 OK\r\nDate: Tue, 20 Nov 2018\r\nConnection: close\r\nTransfer-Encoding: chunked\r\n\r\n8\r\n";

            HTTPChunk chunk(hdr, true);

            if (chunk.GetChunkSize() != 8)
            {
                cout << "Incorrect chunk size: " << chunk.GetChunkSize() << endl;
                return 0;
            }

            if (chunk.GetContainsEnd() == true)
            {
                cout << "Shouldnt contain end." << endl;
                return 0;
            }

            string c2 = "9\r\nsdomedata\r\n9\r\nsdomedata\r\n0\r\n\r\n";

            HTTPChunk chunk2(c2, false);
            if (chunk2.GetChunkSize() != 18)
            {
                cout << "Incorrect chunksize: " << chunk2.GetChunkSize() << endl;
                return 0;
            }
            if (chunk2.GetContainsEnd() != true)
            {
                cout << "Should contain end." << endl;
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "FileManager",
        "Sets default root via constructor.",
        []() {
            FileManager files("./www/");

            if (files.GetRoot() != "./www/")
            {
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "FileManager",
        "Throws exception when file does not exist.",
        []() {
            FileManager files("./www/");
            uint64_t size = 0;
            try
            {
                files.GetFilename("/non_existant_file.txt", size);
            }
            catch (std::runtime_error &e)
            {
                cout << e.what() << endl;
                if (size != 0)
                {
                    return 0;
                }
                return 1;
            }

            return 0;
        });

    TestRunner::GetRunner()->AddTest(
        "FileManager",
        "Sets correct filename and size when given directory",
        []() {
            FileManager files("./test_data/");
            uint64_t size = 0;
            string filename = "";

            try
            {
                filename = files.GetFilename("/", size);
            }
            catch (std::runtime_error &e)
            {
                cout << e.what() << endl;
                return 0;
            }

            cout << "Checking: " << filename << endl;

            if (filename != "./test_data/./index.html")
            {
                cout << "Filename incorrect: " << filename << endl;
                return 0;
            }
            if (size != 3346)
            {
                return 0;
            }

            try
            {
                filename = files.GetFilename("/another_test/", size);
            }
            catch (std::runtime_error &e)
            {
                cout << e.what() << endl;
                return 0;
            }

            cout << "Checking: " << filename << endl;

            if (filename != "./test_data/./another_test/index.htm")
            {
                return 0;
            }
            if (size != 0)
            {
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "FileManager",
        "Returns the correct filename for url requested",
        []() {
            FileManager files("./test_data/");
            uint64_t size;
            string filename = "";
            try
            {
                filename = files.GetFilename("/jquery-1.4.3.min.js", size);
            }
            catch (std::runtime_error &e)
            {
                cout << e.what() << " jquery " << endl;
                return 0;
            }
            cout << filename << endl;
            if (filename != "./test_data/./jquery-1.4.3.min.js")
            {
                return 0;
            }

            try
            {
                filename = files.GetFilename("/images/apple_ex.png", size);
            }
            catch (std::runtime_error &e)
            {

                cout << e.what() << " apple_ex.png" << endl;
                return 0;
            }
            cout << filename << endl;
            if (filename != "./test_data/./images/apple_ex.png")
            {
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Request Parsing",
        "Must parse protocol version",
        []() {
            string hdr = "GET /Protocols/rfc1945/rfc1945 HTTP/1.1\r\n\r\n";
            HTTPRequest request(hdr);
            if (request.GetProtocol() != "HTTP/1.1")
                return 0;
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Request Parsing",
        "Must parse url",
        []() {
            string hdr = "GET /Protocols/rfc1945/rfc1945 HTTP/1.1\r\n\r\n";
            HTTPRequest request(hdr);
            if (request.GetUrl() != "/Protocols/rfc1945/rfc1945")
                return 0;
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Request Parsing",
        "Must parse port",
        []() {
            string hdr = "GET /Protocols/rfc1945/rfc1945 HTTP/1.1\r\n\r\n";
            HTTPRequest request(hdr);
            if (request.GetPort() != "80")
            {
                std::cerr << "Got incorrect port: " << request.GetPort() << std::endl;
                return 0;
            }
            if (request.GetUrl() != "/Protocols/rfc1945/rfc1945")
                return 0;
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Request Parsing",
        "Must produce default port",
        []() {
            string hdr = "GET /Protocols/rfc1945/rfc1945 HTTP/1.1\r\n\r\n";
            HTTPRequest request(hdr);
            if (request.GetPort() != "80")
            {
                std::cerr << "Got incorrect port: " << request.GetPort() << std::endl;
                return 0;
            }
            if (request.GetUrl() != "/Protocols/rfc1945/rfc1945")
                return 0;
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Proxy Request Parsing",
        "Must strip protocol from url.",
        []() {
            string hdr = "GET http://www.netstech.org/index.html:8080 HTTP/1.1\r\n\r\n";
            HTTPRequest request(hdr);

            if (!request.CheckAbsolute(hdr))
            {
                std::cerr << "Absolute check failed." << std::endl;
                return 0;
            }
            std::cout << "Passed absolute" << std::endl;
            if (request.GetPort() != "8080")
            {
                std::cerr << "Got incorrect port: " << request.GetPort() << std::endl;
                return 0;
            }
            std::cout << "Passed port." << std::endl;
            if (request.GetProxyTargetHost() != "www.netstech.org")
            {
                std::cerr << "Proxy Target Host: " << request.GetProxyTargetHost() << std::endl;
                return 0;
            }
            std::cout << "Passed host." << std::endl;
            if (request.GetUrl() != "/index.html")
            {
                std::cerr << "Incorrect url: " << request.GetUrl() << std::endl;
                return 0;
            }
            std::cout << "Passed url." << std::endl;
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Proxy Request Parsing",
        "Must correctly form a forwarding request from original request.",
        []() {
            string hdr = "GET http://www.netstech.org/index.html:8080 HTTP/1.1\r\nConnection:keep-alive\r\nHost:www.cplusplus.com\r\nUpgrade:HTTPS/1.3\r\nAccept:text/html\r\n\r\n";
            HTTPRequest request(hdr);

            if (!request.CheckAbsolute(hdr))
            {
                std::cerr << "Absolute check failed." << std::endl;
                return 0;
            }

            if (request.GetPort() != "8080")
            {
                std::cerr << "Got incorrect port: " << request.GetPort() << std::endl;
                return 0;
            }

            if (request.GetProxyTargetHost() != "www.netstech.org")
            {
                std::cerr << "Proxy Target Host: " << request.GetProxyTargetHost() << std::endl;
                return 0;
            }

            if (request.GetUrl() != "/index.html")
            {
                std::cerr << "Incorrect url: " << request.GetUrl() << std::endl;
                return 0;
            }

            string &forward = request.GetProxyRequest();
            string correct = "GET /index.html HTTP/1.1\r\nAccept:text/html\r\nHost:www.cplusplus.com\r\n\r\n";
            if (forward != correct)
            {
                cout << "Forward size: " << forward.length() << " correct size " << correct.length() << endl;
                cout << "Proxy request: " << endl
                     << forward << endl;
                for (auto i = 0; i < forward.length(); i++)
                {
                    if (forward.c_str()[i] != correct.c_str()[i])
                    {
                        cout << "Difference: " << i << forward.c_str()[i] << " " << correct.c_str()[i] << endl;
                        break;
                    }
                }
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Request Parsing",
        "Must parse GET method",
        []() {
            string hdr = "GET /Protocols/rfc1945/rfc1945 HTTP/1.1\r\n\r\n";
            HTTPRequest request(hdr);
            if (request.GetMethod() != "GET")
                return 0;
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Request Parsing",
        "Must parse POST method",
        []() {
            string hdr = "POST /index.html HTTP/1.1\r\nHost: 192.168.1.1\r\nConnection: keep-alive\r\nContent-Length: 9\r\n\r\nPOST DATA";
            HTTPRequest request(hdr);
            if (request.GetMethod() != "POST")
                return 0;

            if (request.GetContentLength() != 9)
            {
                cout << "Incorrect content length " << std::dec << request.GetContentLength() << endl;
                return 0;
            }

            if (request.GetHost() != "192.168.1.1")
            {
                cout << "Incorrect host: " << request.GetHost() << endl;
                return 0;
            }

            if (request.GetUrl() != "/index.html")
            {
                cout << "Incorrect url: " << request.GetUrl() << endl;
                return 0;
            }
            if (request.GetPostData() != "POST DATA")
            {
                cout << "Incorrect post data " << request.GetPostData() << endl;
                return 0;
            }
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Request Parsing",
        "Must parse keepalive",
        []() {
            string hdr = "POST /Protocols/rfc1945/rfc1945 HTTP/1.1\r\nConnection:keep-alive\r\n\r\n";
            HTTPRequest request(hdr);
            if (request.GetKeepAlive() != true)
            {
                std::cerr << request.GetKeepAlive() << std::endl;
                return 0;
            }
            return 1;
        });
    TestRunner::GetRunner()->AddTest(
        "HTTP Request Parsing",
        "Must set connection close in absense of keep alive",
        []() {
            string hdr = "POST /Protocols/rfc1945/rfc1945 HTTP/1.1\r\n\r\n";
            HTTPRequest request(hdr);
            if (request.GetKeepAlive() != false)
            {
                std::cerr << request.GetKeepAlive() << std::endl;
                return 0;
            }
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Request Parsing",
        "Must parse host",
        []() {
            string hdr = "POST /Protocols/rfc1945/rfc1945 HTTP/1.1\r\nHost:  localhost\nConnection:keep-alive\r\n\r\n";
            HTTPRequest request(hdr);
            if (request.GetHost() != "localhost")
            {
                std::cerr << request.GetHost() << std::endl;
                return 0;
            }
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "TCP Socket Creation",
        "Must be able to create listen socket.",
        []() {
            TCPSocket socket;

            bool res = socket.CreateSocket("8080");
            if (!res)
            {
                std::cerr << "Failed to create socket." << endl;
                socket.ISocket::CloseSocket();
                return 0;
            }
            socket.ISocket::CloseSocket();
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "TCP Socket Creation",
        "Must be able to listen on listen socket.",
        []() {
            TCPSocket socket;

            bool res = socket.CreateSocket("8080");
            if (!res)
            {
                std::cerr << "Failed to create socket." << endl;
                socket.ISocket::CloseSocket();
                return 0;
            }

            if (!socket.Listen())
            {
                std::cerr << "Failed to listen." << endl;
                return 0;
            }
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "TCP Socket Creation",
        "Must be able to create listen socket.",
        []() {
            TCPSocket socket;

            bool res = socket.CreateSocket("8080");
            if (!res)
            {
                std::cerr << "Failed to create list socket." << endl;
                return 0;
            }

            socket.Listen();

            TCPSocket connSocket;
            res = connSocket.CreateSocket("localhost", "8080");
            if (!res)
            {
                std::cerr << "Failed to create active socket." << endl;
                return 0;
            }

            connSocket.ISocket::CloseSocket();
            socket.ISocket::CloseSocket();
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "TCP Socket Creation",
        "Must be able to create connecting socket and request data.",
        []() {
            TCPSocket connSocket;
            bool res = connSocket.CreateSocket("netstech.org", "80");
            if (!res)
            {
                std::cerr << "Failed to create active socket." << endl;
                return 0;
            }
            string data = string("GET / HTTP/1.1 \r\n\r\n");
            std::cout << "Sending request: " << data << std::endl;
            uint32_t resLen = connSocket.BlockingSend(data.c_str(), data.size());
            if (resLen != data.size())
            {
                return 0;
            }

            char recvBuffer[10000];
            resLen = connSocket.BlockingRecv(recvBuffer, 10000);
            std::cout << "Received response of length: " << resLen << std::endl;
            data = string(recvBuffer, resLen);
            std::cout << recvBuffer << std::endl;
            connSocket.ISocket::CloseSocket();
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "TCP Socket Creation",
        "TCP Socket should throw exception if connecting to invalid host.",
        []() {
            TCPSocket connSocket;
            try
            {
                bool res = connSocket.CreateSocket("www.noserv.erzxcvzksd", "80");
                if (res)
                {
                    std::cerr << "Created invalid connection." << endl;
                    return 0;
                }
            }
            catch (TCPSocket::BadHostnameException &e)
            {
                std::cerr << "Successfully caught exception: " << e.what() << std::endl;
                return 1;
            }
            catch (std::exception &e)
            {
                std::cerr << "Wrong type of exception caught: " << e.what() << std::endl;
                return 0;
            }

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "TCP Socket Creation",
        "Must be able to accept connections",
        []() {
            TCPSocket socket;

            bool res = socket.CreateSocket("8080");
            if (!res)
            {
                std::cerr << "Failed to create list socket." << endl;
                return 0;
            }

            socket.Listen();

            TCPSocket connSocket;
            res = connSocket.CreateSocket("localhost", "8080");
            if (!res)
            {
                std::cerr << "Failed to create active socket." << endl;
                return 0;
            }

            TCPSocket *connection = socket.Accept();

            if (connection == NULL)
            {
                std::cerr << "Did not accept connection" << endl;
                return 0;
            }

            delete connection;

            connSocket.ISocket::CloseSocket();
            socket.ISocket::CloseSocket();
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "Session",
        "Throws an exception when trying to run with NULL connection",
        []() {
            FileCache *cache = new FileCache("./cache_root/", 10);
            UrlCache *urlCache = new UrlCache("./blacklist.txt");
            Session session(NULL, cache, urlCache);
            try
            {
                session.Run();
            }
            catch (std::runtime_error &e)
            {
                cout << e.what() << endl;
                delete cache;
                return 1;
            }
            delete cache;
            delete urlCache;
            return 0;
        });

    TestRunner::GetRunner()->AddTest(
        "FileCache",
        "Must be able to create cache.",
        []() {
            FileCache cache("./cache_root/", 10);

            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "FileCache",
        "Must be able to insert and retrieve items.",
        []() {
            FileCache cache("./cache_root/", 2);
            string data = "some test data.";
            cache.Insert("www.test.nowhere", data);

            if (cache.CheckCache("www.test.nowhere") != true)
            {
                cout << "Should hit." << endl;
                return 0;
            }

            std::this_thread::sleep_for(chrono::seconds(2));

            if (cache.CheckCache("www.test.nowhere") != false)
            {
                cout << "Should miss." << endl;
                return 0;
            }

            try
            {
                std::string file = cache.GetCachedFile("asdasdf");
            }
            catch (FileCache::CacheMissException &e)
            {
                return 1;
            }

            return 0;
        });

    TestRunner::GetRunner()->AddTest(
        "UrlCache",
        "Must be able to insert and retrieve items.",
        []() {
            UrlCache uc("./blacklist.txt");
            try{
                uc.Insert("www.google.com", "80");
            }catch(UrlCache::BlackListException &e){
                cout<<"Should not be blacklisted."<<endl;
                return 0;
            }

            if (uc.CheckCache("www.google.com", "80") != true)
            {
                cout << "Should hit." << endl;
                return 0;
            }
            if (uc.CheckCache("www.google.com", "81") != false)
            {
                cout << "Should miss." << endl;
                return 0;
            }

            try
            {
                std::string file = uc.Get("www.yaoo.com", "80");
            }
            catch (ICache::CacheMissException &e)
            {
                return 1;
            }
            catch(UrlCache::BlackListException &e){
                return 0;
            }

            return 0;
        });

    TestRunner::GetRunner()->AddTest(
        "UrlCache",
        "Must be able to retrieve address structs",
        []() {
            UrlCache uc("./blacklist.txt");
            uc.Insert("www.google.com", "80");

            TCPSocket sock;

            if (!sock.CreateSocket(uc.Get("www.google.com", "80")))
            {
                return 1;
            }

            return 0;
        });

    TestRunner::GetRunner()->AddTest(
        "UrlCache",
        "Blacklist",
        []() {
            UrlCache uc("./blacklist.txt");

            if (!uc.CheckBL("www.yahoo.com"))
            {
                return 0;
            }

            try
            {
                uc.Insert("www.yahoo.com", "80");
            }
            catch (UrlCache::BlackListException &e)
            {
                return 1;
            }
            return 0;
        });

    TestRunner::GetRunner()->Run();

    TestRunner::GetRunner()->PrintSummary();

    return TestRunner::GetRunner()->GetRetCode();
}