#pragma once

#include <iostream>
#include <sstream>
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

#if defined (_WIN32)
    #define snet_socktype SOCKET
#elif defined (__unix__)
    #define snet_socktype int
#endif

#if !defined (inet_pton)
    #define SNET_INET_PTON_REPLACEMENT
    #include "snet_inet_pton.hpp"
#endif
#if !defined (inet_ntop)
    #define SNET_INET_NTOP_REPLACEMENT
    #include "snet_inet_ntop.hpp"
#endif


namespace snet
{
    enum protocol_versions
    {
        IPv6 = 1,
        IPv4 = 2,
    };

	enum flags
	{
		REUSE_PORT = 1,
		DISABLE_NAGLE = 2,
	};

    inline void get_error_message (std::string& errstring)
    {
        std::ostringstream str;
        #if defined (_WIN32)
            LPSTR errString = NULL;

            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, WSAGetLastError(), 0, (LPSTR)&errString, 0, 0);
            str << "WSA: " << errString;
            LocalFree(errString);
        #elif defined(__unix__)
            int errsv = errno;
            char buffer[256];
            strerror_r(errsv, buffer, 254);
            buffer[255] = 0;

            str << "BS: errno=" << errsv << ", " << buffer;
        #endif
        errstring = str.str();
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
                #if defined (_WIN32)
                    if (rv == SOCKET_ERROR)
                    {
                        std::string ext_error;
                        snet::get_error_message(ext_error);

                        throw snet::Exception("receive() failed. " + ext_error);
                    }
                #endif
                return rv;
            }

            inline void set_receive_timeout (int ms)
            {
                setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&ms), sizeof(ms));
            }

        protected:
            snet_socktype sock;

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
            TCP_client (snet_socktype sock_nr);
            TCP_client (unsigned char protocol_version, const std::string& host, unsigned short int port);
            ~TCP_client ();
        //protected:
            std::string ip;
        friend class TCP_server;
    };

    class TCP_server : public TCP_socket
    {
        public:
            TCP_server (unsigned char protocol_version, unsigned short int port, int max_sim_con_requests, int flags = 0);
            ~TCP_server ();
            snet::TCP_client* accept ();
        protected:
            int ip_version;
    };

    class UDP_socket
    {
        public:

        protected:
            int dummy;
    };
}
