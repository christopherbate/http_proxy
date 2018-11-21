#include "TCPSocket.h"

/*
Constructor
*/
TCPSocket::TCPSocket() : ISocket()
{
    m_urlCache = NULL;
}

TCPSocket::TCPSocket(UrlCache *urlCache) : ISocket()
{
    m_urlCache = urlCache;
}

/*
Desctructor
*/
TCPSocket::~TCPSocket()
{
    CloseSocket();
}

/*
CreateSocket (connecting)
Creates a connecting (client) socket and attempts to connect to specified host and port
returns true if connection was successful, false otherwise.
*/
bool TCPSocket::CreateSocket(string host, string port)
{
    if (IsOpen())
    {
        cerr << "Socket already open at fd: " << m_fd << ". Close before reusing this interface." << endl;
        return false;
    }
    m_type = CONN;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_NUMERICSERV | AI_CANONNAME;
    m_remoteHost = host;

    // Get the address info structure
    struct addrinfo *resList;
    int rc = getaddrinfo(host.c_str(), port.c_str(), &hints, &resList);
    if (rc != 0)
    {
        cerr << "TCPSocket : CreateSocket : getaddrinfo : " << rc << endl;

        throw TCPSocket::BadHostnameException();

        return false;
    }

    // Check first for empty address.
    if (resList == NULL)
    {
        cerr << "TCPSocket : CreateSocket : no addresses at hostname/port" << endl;
        throw TCPSocket::BadHostnameException();
        return false;
    }

    // Parse the list and try to connect.
    struct addrinfo *aiIter = resList;
    while (aiIter != NULL)
    {
        char nameBuffer[INET_ADDRSTRLEN];
        inet_ntop(aiIter->ai_family, aiIter->ai_addr, nameBuffer, INET_ADDRSTRLEN);
        //cout << "Attempting to create TCP socket with cached target: " << (nameBuffer != NULL ? nameBuffer : "") << " " << resList->ai_canonname << endl;
        m_fd = socket(aiIter->ai_family, aiIter->ai_socktype, aiIter->ai_protocol);

        if (m_fd == -1)
        {
            cerr << "TCPSocket : CreateSocket : socket; trying next address " << endl;
            aiIter = aiIter->ai_next;
            continue;
        }

        if (connect(m_fd, aiIter->ai_addr, aiIter->ai_addrlen) == -1)
        {
            cerr << "TCPSocket : CreateSocket : connect : " << strerror(errno) << endl;
            aiIter = aiIter->ai_next;
            close(m_fd);
            continue;
        }

        break;
    }

    // Free address info list
    freeaddrinfo(resList);

    if (aiIter == NULL)
    {
        cerr << "TCPSocket : failed to connect to any addresses." << endl;
        close(m_fd);
        m_fd = -1;
        return false;
    }

    return true;
}

/*
CreateSocket (connecting, with address info)
Creates a connecting (client) socket and attempts to connect to specified host and port
returns true if connection was successful, false otherwise.

Bypasses DNS lookup
*/
bool TCPSocket::CreateSocket(sockaddr_in *addr)
{
    if (IsOpen())
    {
        cerr << "Socket already open at fd: " << m_fd << ". Close before reusing this interface." << endl;
        return false;
    }
    m_type = CONN;

    m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_fd ==-1){
        throw std::runtime_error("Could not create socket.");
    }

    if (connect(m_fd,(sockaddr*) addr, sizeof(sockaddr_in)) == -1)
    {
        cerr << "TCPSocket : CreateSocket : connect : " << strerror(errno) << endl;        
        close(m_fd);
        throw std::runtime_error("Could not connect using address bypass DNS.");
        return false;
    }

    return true;
}

/*
CreateSocket (passive/listening)
Creates a listening socket on specified port.
Returns true if bind was successful, false otherwise
*/
bool TCPSocket::CreateSocket(string port)
{
    if (m_fd != -1)
    {
        cerr << "Socket already open at fd: " << m_fd << ". Close before reusing this interface." << endl;
        return false;
    }
    m_type = PASSIVE;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_NUMERICSERV | AI_PASSIVE;

    // get the address info structure
    struct addrinfo *resList;
    int rc = getaddrinfo(NULL, port.c_str(), &hints, &resList);
    if (rc != 0)
    {
        return false;
    }

    // Loop through and create socket
    struct addrinfo *aiIter = resList;
    while (aiIter != NULL)
    {

        char nameBuffer[INET_ADDRSTRLEN];
        inet_ntop(aiIter->ai_family, &((sockaddr_in*)aiIter->ai_addr)->sin_addr, nameBuffer, INET_ADDRSTRLEN);
        cout << "Attempting to create socket for: " << (nameBuffer != NULL ? nameBuffer : "") << endl;

        // Create the socket.
        m_fd = socket(aiIter->ai_family, aiIter->ai_socktype, aiIter->ai_protocol);
        if (m_fd == -1)
        {
            cerr << "TCPSocket : CreateSocket : socket" << endl;
            aiIter = aiIter->ai_next;
            continue;
        }

        // Enable  reuse
        int reuse = 1;
        int res = 0;
        res = setsockopt(m_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(int));
        if (res < 0)
        {
            cerr << "Failed to set socket reuse." << endl;
            return false;
        }

        if (bind(m_fd, aiIter->ai_addr, aiIter->ai_addrlen) == -1)
        {
            cerr << "TCPSocket : CreateSocket : bind; errno " << strerror(errno) << endl;
            aiIter = aiIter->ai_next;
            close(m_fd);
            continue;
        }
        break;
    }

    // Free address info list
    freeaddrinfo(resList);

    if (aiIter == NULL)
    {
        cerr << "TCPSocket : failed to bind to any addresses." << endl;
        close(m_fd);
        m_fd = -1;
        return false;
    }

    cout << "TCP Socket successfully bound to port " << port << endl;

    return true;
}

/*
BlockingSend

Sends data at at data of size length to remoteAddr (if specified, otherwise to already save remote
*/
uint32_t TCPSocket::BlockingSend(const char *data, uint32_t length)
{
    if (m_fd < 0)
    {
        //cerr << "Socket not open." << endl;
        return 0;
    }
    ssize_t sl = length;
    ssize_t res = 0;
    ssize_t total = 0;

    // If we are passive, we need to specify address. Otherwise, we saved it using "connect"
    // although both are using datagram udp.
    while (total != length)
    {
        res = send(m_fd, data, sl, MSG_NOSIGNAL);
        if (res == -1)
        {
            break;
        }
        total += res;
    }

    if (res == -1)
    {
        cerr << "Blocking send : error " << strerror(errno) << endl;
        return 0;
    }

    if (res != sl)
    {
        cerr << "Failed/short send: " << res << "/" << length << endl;
        return res;
    }
    return res;
}

/*
    BlockingRecv

    Receives data into buffer of at most size 

    Saves receiving address into remoteAddr if not null

    Receive from recvAddress 
*/
uint32_t TCPSocket::BlockingRecv(char *buffer, uint32_t size)
{
    if (m_fd < 0)
    {
        // ////cerr << "Socket not open." << endl;
        return 0;
    }
    m_timeout = false;

    ssize_t res = recv(m_fd, buffer, size, 0);

    if (res == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            //cerr << "Timeout" << endl;
            m_timeout = true;
        }
        else
        {
            m_bad = true;
        }
        return 0;
    }

    return res;
}

/** Listen for connectinos on passive socket*/
bool TCPSocket::Listen()
{
    if (m_fd < 0)
    {
        // ////cerr << "Socket not open." << endl;
        return false;
    }

    int res = listen(m_fd, 10);

    if (res == -1)
    {
        std::cerr << "Failed to listen on socket: " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

/** Accepts a connection and returns new handle to that connection.*/
TCPSocket *TCPSocket::Accept()
{
    if (m_fd < 0)
        return NULL;

    sockaddr_in peerAddr;
    socklen_t len = sizeof(sockaddr_in);
    int res = accept(m_fd, (sockaddr *)&peerAddr, &len);

    if (res < 0)
    {
        std::cerr << "Failed to accept: " << strerror(errno) << endl;
        return NULL;
    }
    char nameBuffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(peerAddr.sin_addr), nameBuffer, INET_ADDRSTRLEN);

    //std::cout << "Connection from "<<nameBuffer<<std::endl;

    TCPSocket *sock = new TCPSocket();
    sock->SetSocketFD(res);
    sock->SetPeername(nameBuffer);

    return sock;
}