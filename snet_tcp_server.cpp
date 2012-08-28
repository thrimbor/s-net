#include "snet.hpp"
#include <iostream>
#include <string.h>
#include <stdio.h>

snet::TCP_server::TCP_server (unsigned char protocol_version, unsigned short int port, int max_sim_con_requests, int flags)
{
    snet::TCP_socket::watcher();
    addrinfo hints;
    addrinfo* addr;
    char port_str[6];

    memset(&hints, 0, sizeof(hints));

    this->ip_version = (protocol_version == snet::IPv6) ? PF_INET6 : PF_INET;
    hints.ai_family = this->ip_version;

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;
    snprintf(port_str, 6, "%d", port);
    port_str[5] = 0;

    if (getaddrinfo(NULL, port_str, &hints, &addr) != 0)
    {
        std::string ext_error;
        snet::get_error_message(ext_error);

        throw snet::Exception("getaddrinfo() failed. " + ext_error);
    }

    this->sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    #if defined (_WIN32)
        if (this->sock == INVALID_SOCKET)
    #elif defined (__unix__)
        if (this->sock == -1)
    #endif
    {
        freeaddrinfo(addr);
        std::string ext_error;
        snet::get_error_message(ext_error);

        throw snet::Exception("socket() failed. " + ext_error);
    }

    if (flags & snet::REUSE_PORT)
    {
        int opt = 1;
        int errorcode = setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), sizeof(opt));
        #if defined (_WIN32)
            if (errorcode == SOCKET_ERROR)
        #elif defined (__unix__)
            if (this->sock == -1)
        #endif
        {
            freeaddrinfo(addr);
            std::string ext_error;
            snet::get_error_message(ext_error);

            throw snet::Exception("setsockopt() failed. " + ext_error);
        }
    }

    if (flags & snet::DISABLE_NAGLE)
    {
        int opt = 1;
        int errorcode = setsockopt(this->sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&opt), sizeof(opt));
        #if defined (_WIN32)
            if (errorcode == SOCKET_ERROR)
        #elif defined (__unix__)
            if (this->sock == -1)
        #endif
        {
            freeaddrinfo(addr);
            std::string ext_error;
            snet::get_error_message(ext_error);

            throw snet::Exception("setsockopt() failed. " + ext_error);
        }
    }

    if (bind(this->sock, addr->ai_addr, addr->ai_addrlen) == -1)
    {
        freeaddrinfo(addr);
        std::string ext_error;
        snet::get_error_message(ext_error);

        throw snet::Exception("bind() failed. " + ext_error);
    }

    if (::listen(this->sock, max_sim_con_requests) == -1)
    {
        freeaddrinfo(addr);
        std::string ext_error;
        snet::get_error_message(ext_error);

        throw snet::Exception("listen() failed. " + ext_error);
    }

    freeaddrinfo(addr);
    return;
}

snet::TCP_server::~TCP_server ()
{
    #if defined (_WIN32)
        if (this->sock != INVALID_SOCKET)
    #elif defined (__unix__)
        if (this->sock != -1)
    #endif
    {
        this->destroy();
    }
}

snet::TCP_client* snet::TCP_server::accept ()
{
    snet_socktype s;
    char str[INET6_ADDRSTRLEN];
    memset(str, 0, INET6_ADDRSTRLEN);
    unsigned short int port;

    if (this->ip_version == PF_INET6)
    {
        sockaddr_in6 sadr;
        socklen_t addr_size = sizeof(sockaddr_in6);
        s = ::accept(this->sock, (sockaddr*)&sadr, &addr_size);
        snet::inet_ntop(AF_INET6, (void*)&sadr.sin6_addr, str, (socklen_t)INET6_ADDRSTRLEN);
        port = sadr.sin6_port;
    }
    else
    {
        sockaddr_in sadr;
        socklen_t addr_size = sizeof(sockaddr_in);
        s = ::accept(this->sock, (sockaddr*)&sadr, &addr_size);
        inet_ntop(AF_INET, (void*)&sadr.sin_addr, str, (socklen_t)INET_ADDRSTRLEN);
        port = sadr.sin_port;
    }

    #if defined (_WIN32)
        if (s == INVALID_SOCKET)
    #elif defined (__unix__)
        if (s == -1)
    #endif
    {
        std::string ext_error;
        snet::get_error_message(ext_error);

        throw snet::Exception("accept() failed. " + ext_error);
    }

    snet::TCP_client* socket = new snet::TCP_client(s);
    socket->ip = std::string(str);
    socket->port = port;
    return socket;
}
