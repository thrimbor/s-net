#include "snet.hpp"
#include <iostream>
#include <string.h>
#include <stdio.h>

snet::TCP_client::TCP_client (unsigned char protocol_version, const std::string& host, unsigned short int port)
{
    snet::TCP_socket::watcher();
    addrinfo hints;
    addrinfo* addr;
    char port_str[6];

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = (protocol_version == snet::IPv6) ? PF_INET6 : PF_INET;

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_TCP;
    snprintf(port_str, 6, "%d", port);
    port_str[5] = 0;

    if (getaddrinfo(host.c_str(), port_str, &hints, &addr) != 0)
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

    if (connect(this->sock, addr->ai_addr, addr->ai_addrlen) == -1)
    {
        freeaddrinfo(addr);
        std::string ext_error;
        snet::get_error_message(ext_error);

        throw snet::Exception("connect() failed. " + ext_error);
    }

    freeaddrinfo(addr);
    return;
}

snet::TCP_client::~TCP_client ()
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
