#include "snet.hpp"
#include <string.h>
#include <stdio.h>

snet::UDP_peer::UDP_peer (unsigned char protocol_version, const std::string& host, unsigned short int port) : paddr(0), p_version(protocol_version)
{
    addrinfo hints;
    addrinfo* addr;
    char port_str[6];

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = (protocol_version == snet::IPv6) ? PF_INET6 : PF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_UDP;
    snprintf(port_str, 6, "%d", port);

    if (getaddrinfo(host.c_str(), port_str, &hints, &addr) != 0)
    {
        std::string ext_error;
        snet::get_error_message(ext_error);
        throw snet::Exception("getaddrinfo() failed. " + ext_error);
    }

    this->paddr_len = addr->ai_addrlen;
    this->paddr = (sockaddr*) new char[addr->ai_addrlen];
    memcpy(this->paddr, addr->ai_addr, addr->ai_addrlen);
    freeaddrinfo(addr);
}

std::string snet::UDP_peer::get_ip ()
{
    if (this->paddr == NULL) return std::string("");
    char str[INET6_ADDRSTRLEN];
    memset(str, 0, INET6_ADDRSTRLEN);

    if (this->p_version == snet::IPv6)
    {
        sockaddr_in6* ptr = (sockaddr_in6*)this->paddr;
        inet_ntop(AF_INET6, (void*)&ptr->sin6_addr, str, (socklen_t)INET6_ADDRSTRLEN);
    }
    else
    {
        sockaddr_in* ptr = (sockaddr_in*)this->paddr;
        inet_ntop(AF_INET, (void*)&ptr->sin_addr, str, (socklen_t)INET_ADDRSTRLEN);
    }

    return std::string(str);
}

unsigned short int snet::UDP_peer::get_port ()
{
    if (this->paddr == NULL) return 0;

    if (this->p_version == IPv6)
    {
        sockaddr_in6* ptr = (sockaddr_in6*)this->paddr;
        return ptr->sin6_port;
    }
    else
    {
        sockaddr_in* ptr = (sockaddr_in*)this->paddr;
        return ptr->sin_port;
    }
}

snet::UDP_socket::UDP_socket (unsigned char protocol_version) : p_version(protocol_version)
{
    this->watcher();

    int type = (protocol_version == snet::IPv6) ? PF_INET6 : PF_INET;

    this->sock = socket(type, SOCK_DGRAM, IPPROTO_UDP);
    #if defined (_WIN32)
        if (this->sock == INVALID_SOCKET)
    #elif defined (__unix__)
        if (this->sock == -1)
    #endif
    {
        std::string ext_error;
        snet::get_error_message(ext_error);

        throw snet::Exception("socket() failed. " + ext_error);
    }

}

snet::UDP_socket::UDP_socket (unsigned char protocol_version, unsigned short int port) : p_version(protocol_version)
{
    this->watcher();
    addrinfo hints;
    addrinfo* addr;
    char port_str[6];

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = (protocol_version == snet::IPv6) ? PF_INET6 : PF_INET;

    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;
    snprintf(port_str, 6, "%d", port);
    //port_str[5] = 0;

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

    if (bind(this->sock, addr->ai_addr, addr->ai_addrlen) == -1)
    {
        freeaddrinfo(addr);
        std::string ext_error;
        snet::get_error_message(ext_error);

        throw snet::Exception("bind() failed. " + ext_error);
    }

    freeaddrinfo(addr);
}

snet::UDP_socket::~UDP_socket ()
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
