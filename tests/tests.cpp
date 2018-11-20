#include "QuickTestCPP.h"
#include "../src/HTTPRequest.h"
#include "../src/HTTPResponse.h"
#include "../src/TCPSocket.h"
#include "../src/Session.h"
#include "../src/FileManager.h"
#include <iostream>
#include <string>
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
        "Must be able to create header from HTTPRequest header.",
        []() {
            string hdr = "GET /Protocols/rfc1945/rfc1945 HTTP/1.0\r\n";

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

            cout <<"Checking: "<< filename << endl;

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

            cout << "Checking: "<< filename << endl;

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
                cout << e.what() <<" jquery "<<endl;
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
            string hdr = "GET /Protocols/rfc1945/rfc1945 HTTP/1.1\r\n";
            HTTPRequest request(hdr);
            if (request.GetProtocol() != "HTTP/1.1")
                return 0;
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Request Parsing",
        "Must parse url",
        []() {
            string hdr = "GET /Protocols/rfc1945/rfc1945 HTTP/1.1\r\n";
            HTTPRequest request(hdr);
            if (request.GetUrl() != "/Protocols/rfc1945/rfc1945")
                return 0;
            return 1;
        });

    TestRunner::GetRunner()->AddTest(
        "HTTP Request Parsing",
        "Must parse GET method",
        []() {
            string hdr = "GET /Protocols/rfc1945/rfc1945 HTTP/1.1\r\n";
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
            string hdr = "POST /Protocols/rfc1945/rfc1945 HTTP/1.1\r\nConnection:Keep-alive\r\n";
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
            string hdr = "POST /Protocols/rfc1945/rfc1945 HTTP/1.1\r\nHost:  localhost\nConnection:Keep-alive\r\n";
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
            Session session(NULL, "./www/");
            try
            {
                session.Run();
            }
            catch (std::runtime_error &e)
            {
                cout << e.what() << endl;
                return 1;
            }
            return 0;
        });

    TestRunner::GetRunner()->Run();

    TestRunner::GetRunner()->PrintSummary();

    return TestRunner::GetRunner()->GetRetCode();
}