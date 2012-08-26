#include "snet.hpp"

void snet::Selector::add_receive (Socket sock)
{
    FD_SET(sock.sock, &this->set_receive);
    if ((sock.sock+1) > this->max_sock) this->max_sock = sock.sock+1;
}

void snet::Selector::add_send (Socket sock)
{
    FD_SET(sock.sock, &this->set_send);
    if ((sock.sock+1) > this->max_sock) this->max_sock = sock.sock+1;
}

void snet::Selector::add_error (Socket sock)
{
    FD_SET(sock.sock, &this->set_error);
    if ((sock.sock+1) > this->max_sock) this->max_sock = sock.sock+1;
}

void snet::Selector::wait ()
{
    select(this->max_sock, &this->set_receive, &this->set_send, &this->set_error, this->timeval_ptr);
}

void snet::Selector::set_timeout (int seconds, int microseconds)
{
    if (this->timeval_ptr == NULL) this->timeval_ptr = new timeval;
    this->timeval_ptr->tv_sec = seconds;
    this->timeval_ptr->tv_usec = microseconds;
}

bool snet::Selector::contains_receive (Socket sock)
{
    return FD_ISSET(sock.sock, &this->set_receive);
}

bool snet::Selector::contains_send (Socket sock)
{
    return FD_ISSET(sock.sock, &this->set_send);
}

bool snet::Selector::contains_error (Socket sock)
{
    return FD_ISSET(sock.sock, &this->set_error);
}
