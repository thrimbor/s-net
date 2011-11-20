#include "snet.hpp"
#include <iostream>
#include <string.h>
#include <stdio.h>

snet::TCP_server::TCP_server (unsigned char protocol_version, unsigned short int port, int max_sim_con_requests)
{
    snet::watcher();
    addrinfo hints;
    addrinfo* addr;
    char port_str[6];

    memset(&hints, 0, sizeof(hints));

    switch (protocol_version)
    {
        case snet::IPv6:
            hints.ai_family = PF_INET6;
            break;
        case snet::IPv4:
            hints.ai_family = PF_INET;
            break;
        default:
            throw snet::Exception("invalid protocol.");
            break;
    }

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
    if (this->sock == -1)
    {
        freeaddrinfo(addr);
        std::string ext_error;
        snet::get_error_message(ext_error);

        throw snet::Exception("socket() failed. " + ext_error);
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
    if (this->sock != -1)
    {
        this->destroy();
    }
}

snet::TCP_client* snet::TCP_server::accept ()
{
    int s;
    s = ::accept(this->sock, NULL, 0);
    if (s == -1)
    {
        std::string ext_error;
        snet::get_error_message(ext_error);

        throw snet::Exception("accept() failed. " + ext_error);
    }

    snet::TCP_client* t_client = new snet::TCP_client(s);
    return t_client;
}
