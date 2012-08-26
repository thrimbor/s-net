#pragma once

#include <iostream>
#include <sstream>
#include <exception>
#include <list>

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
    #include <netinet/tcp.h>
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
        IPv4 = 2
    };

	enum flags
	{
		REUSE_PORT = 1,
		DISABLE_NAGLE = 2
	};

	enum event_flags
	{
	    FLAG_RECEIVE = 1,
	    FLAG_SEND = 2,
	    FLAG_ERROR = 4
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

    class Exception : public std::exception
    {
        public:
            Exception (const std::string& details) : m_details(details) {};
            ~Exception() throw () {};
            const char* what() const throw()
            {
                return this->m_details.c_str();
            };
        protected:
            std::string m_details;
    };

    class Socket
    {
        protected:
            Socket() : sock(0) {};
            public:
            snet_socktype sock;
            protected:

            inline bool destroy()
            {
                #if defined (_WIN32)
                    closesocket(this->sock);
                #elif defined (__unix__)
                    close(this->sock);
                #endif
                return true;
            }

            inline static void watcher ()
            {
                #if defined (_WIN32)
                    static snet::Socket::Watchman watchman;
                #endif
            }

            #if defined (_WIN32)
                class Watchman
                {
                    public:
                        Watchman ()
                        {
                            WSAData wsa;
                            if (WSAStartup(MAKEWORD(2,1), &wsa) != 0)
                            {
                                // BIG PROBLEM!
                            }
                            // hier vielleicht noch WSAGetLastError einsetzen und das ganze mit exceptions machen
                            std::cout << "WSA initialized" << std::endl;
                        };

                        ~Watchman () { WSACleanup(); std::cout << "WSA closed" << std::endl; };
                };
            #endif

            friend class Selector;
    };

    class TCP_socket : public Socket
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
                if (rv == -1)
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
    };

    class TCP_client : public TCP_socket
    {
        public:
            TCP_client (snet_socktype sock_nr) { this->sock = sock_nr; };
            TCP_client (unsigned char protocol_version, const std::string& host, unsigned short int port);
            ~TCP_client ();
			inline std::string get_ip () {return this->ip;};
			inline unsigned short int get_port () {return this->port;};
        protected:
            std::string ip;
            unsigned short int port;
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

    class UDP_peer
    {
        public:
            UDP_peer () : paddr(NULL), p_version(0) {};
            UDP_peer (unsigned char protocol_version, const std::string& host, unsigned short int port);
            ~UDP_peer ()
            {
                if (this->paddr != 0)
                    delete[] this->paddr;
            };
            std::string get_ip();
            unsigned short int get_port();
        protected:
            sockaddr* paddr;
            int paddr_len;
            unsigned char p_version;
        friend class UDP_socket;
    };

    class UDP_socket : public Socket
    {
        public:
            UDP_socket (unsigned char protocol_version);
            UDP_socket (unsigned char protocol_version, unsigned short int port);
            ~UDP_socket ();

            inline bool send (UDP_peer& peer, const char* buffer, int len)
            {
                if (this->p_version != peer.p_version) return false;
                if (len != sendto(this->sock, buffer, len, 0, peer.paddr, peer.paddr_len)) return false;
                return true;
            };

            inline int receive (UDP_peer& peer, char* buffer, int len)
            {
                if (this->p_version != peer.p_version)
                {
                    if (peer.p_version == 0) peer.p_version = this->p_version;
                }
                else return false;

                std::cout << "peer ip version: " << (int)peer.p_version << std::endl;
                std::cout << "socket ip version: " << (int)this->p_version << std::endl;

                if (peer.paddr == 0)
                {
                    if (peer.p_version == snet::IPv6)
                    {
                        peer.paddr = (sockaddr*) new sockaddr_in6;
                        peer.paddr_len = sizeof(sockaddr_in6);
                        std::cout << "new ipv6 structure allocated" << std::endl;
                    }
                    else
                    {
                        peer.paddr = (sockaddr*) new sockaddr_in;
                        peer.paddr_len = sizeof(sockaddr_in);
                        std::cout << "new ipv4 structure allocated" << std::endl;
                    }
                }
                int rv = recvfrom(this->sock, buffer, len, 0, peer.paddr, &peer.paddr_len);
                if (rv == -1)
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
            };
        protected:
            unsigned char p_version;
    };

    class Socket_item
    {
        public:
            Socket_item (Socket socket, unsigned char flags) : sock(socket), flags(flags) {};
            Socket sock;
            unsigned char flags;
    };

    std::list<Socket_item> poll (std::list<Socket_item> socket_list);
}
