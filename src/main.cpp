/*
main.cpp

/author Christopher Bate
*/
#include <iostream>
#include <thread>
#include "TCPSocket.h"
#include "HTTPRequest.h"
#include <csignal>
#include <string>
#include "Session.h"

using namespace std;

TCPSocket server;

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "Usage: " << argv[0] << " [port num] [cache timeout]" << endl;
        return 0;
    }

    if (!server.CreateSocket(argv[1]))
    {
        cerr << "Failed to create socket" << endl;
        return -1;
    }

    if (!server.Listen())
    {
        cerr << "Failed to listen" << endl;
        return -1;
    }    

    //Create cache
    FileCache *cache = new FileCache("./cache_root/",std::stoul(argv[2]));
    UrlCache *urlCache = new UrlCache("./blacklist.txt");

    // Register interrupts.
    std::signal(SIGINT, [](int s) {
        cout << "Received signal " << s << endl;
        server.CloseSocket();
        exit(0);
    });
    std::signal(SIGTERM, [](int s) {
        cout << "Received signal " << s << endl;
        server.CloseSocket();
        exit(0);
    });

    cout << "Listening on " << argv[1] << endl;
    while (1)
    {       
        try
        {
            TCPSocket *newSocket = server.Accept();
            if (newSocket == NULL)
            {
                continue;
            }

            // Spawn the thread.
            std::thread session([newSocket, cache,urlCache]() {
                Session newSession(newSocket, cache,urlCache);
                newSession.Run();
            });
            session.detach();
        }
        catch (std::exception &e)
        {
            cerr << "Exception: " << e.what() << endl;
        }
    }

    delete cache;
    cache = NULL;
    delete urlCache;
    urlCache = NULL;

    return 0;
}