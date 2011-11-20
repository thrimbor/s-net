#pragma once

#include <iostream>
#include <exception>

#if defined (_WIN32)
    #include "winsock2.h"
    #include "ws2tcpip.h"
#elif defined (__unix__)
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <errno.h>
    #include <string.h>
#endif


namespace snet
{
    enum protocol_versions
    {
        IPv6 = 1,
        IPv4 = 2,
    };

    inline void get_error_message (std::string& errstring)
    {
        #if defined (_WIN32)
            LPSTR errString = NULL;

            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, WSAGetLastError(), 0, (LPSTR)&errString, 0, 0);
            errstring = "WSA: " + std::string(errString);
            LocalFree(errString);
        #elif defined(__unix__)
            char buffer[256];
            strerror_r(errno, buffer, 254);
            buffer[255] = 0;

            errstring = "BS: " + std::string(buffer);
        #endif
        return;
    }

    class Watchman
    {
        public:
            Watchman ()
            {
                #if defined (_WIN32)
                    WSAData wsa;
                    if (WSAStartup(MAKEWORD(2,1), &wsa) != 0)
                    {
                        // BIG PROBLEM!
                    }
                    // hier vielleicht noch WSAGetLastError einsetzen und das ganze mit exceptions machen
                #endif
            };

            ~Watchman ()
            {
                #if defined (_WIN32)
                    WSACleanup();
                #endif
            };
    };

    inline void watcher ()
    {
        static snet::Watchman watchman;
    }

    class Exception : public std::exception
    {
        public:
            Exception (const std::string& details)
            {
                this->m_details = details;
            };
            ~Exception() throw () {};
            const char* what() const throw()
            {
                return this->m_details.c_str();
            };
        protected:
            std::string m_details;
    };

    class TCP_socket
    {
        public:
            inline bool send (const char* buffer, int len)
            {
                int rv;
                rv = ::send(this->sock, buffer, len, 0);
                if (rv != len) return false;
                return true;
            }

            inline int receive (char* buffer, int len)
            {
                int rv;
                rv = recv(this->sock, buffer, len, 0);
                if (rv==-1)
                {
                    return 0;
                }
                return rv;
            }

        protected:
            int sock;

            inline bool destroy()
            {
                #if defined (_WIN32)
                    closesocket(this->sock);
                #elif defined (__unix__)
                    close(this->sock);
                #endif
                return true;
            }
    };

    class TCP_client : public TCP_socket
    {
        public:
            TCP_client (int sock_nr);
            TCP_client (unsigned char protocol_version, const std::string& host, unsigned short int port);
            ~TCP_client ();
    };

    class TCP_server : public TCP_socket
    {
        public:
            TCP_server (unsigned char protocol_version, unsigned short int port, int max_sim_con_requests);
            ~TCP_server ();
            snet::TCP_client* accept ();
    };

    class UDP_socket
    {
        public:

        protected:
            int dummy;
    };
}