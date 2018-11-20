#ifndef CBI_SOCKET_H
#define CBI_SOCKET_H

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

/**
This is an abstract interface that all the socket implementations must use as a base class.
*/
class ISocket
{
  public:
    ISocket()
    {
        m_fd = -1;
        m_type = PASSIVE;
        m_remoteHost = "";
        m_bad = false;
        m_remotePort = "";
        m_timeout = false;
    }
    virtual ~ISocket()
    {
        CloseSocket();
    }

    virtual void CloseSocket() /** Closes socket, if it is open */
    {
        if (m_fd != -1)
            close(m_fd);
        m_fd = -1;
    }

    int GetFd() /** Returns the current file descriptor */
    {
        return m_fd;
    }

    /** Sets the current timeout for the next blocking call to BlockingRecv */
    bool SetRecvTimeout(uint64_t usec)
    {
        if (m_fd == -1)
            return false;
        struct timeval to;
        to.tv_usec = usec % 1000000;
        to.tv_sec = (usec >= 1000000 ? (usec - to.tv_usec) / 1000000 : 0);
        int rc = setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(struct timeval));
        if (rc != 0)
        {
            // cerr << "Socket : SetRecvTimeout : setsockopt, errno " << errno << endl;
            return false;
        }
        return true;
    }

    /** Returns true if the last call to BlockingRecv returned due to timeout.*/
    bool DidTimeout()
    {
        return m_timeout;
    }

    /** BlockingSend must be implemented by inheriting class */
    virtual uint32_t BlockingSend(const char *data, uint32_t length) = 0;

    /** BlockingRecv must be implemented by inheriting class */
    virtual uint32_t BlockingRecv(char *buffer, uint32_t size) = 0;

    /** Returns whether the socket is already open/active */
    bool IsOpen()
    {
        if (m_fd != -1)
            return true;
        return false;
    }

    bool IsBad()
    {
        return m_bad;
    }

  protected:
    enum SDType
    {
        CONN,
        PASSIVE
    };
    SDType m_type;            /** The type of socket passive/connecting */
    int32_t m_fd;             /** File descriptor for the socket */
    bool m_timeout;           /** Whether a timeout occured on the last recv. */
    std::string m_remoteHost; /** A string with the address of the remote host connected to. */
    std::string m_remotePort; /** A string with the remote port connected to */
    struct sockaddr_storage m_remoteAddr;
    bool m_bad;    
};

#endif