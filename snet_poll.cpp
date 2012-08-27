#include "snet.hpp"

std::list<snet::Socket_item> snet::poll (const std::list<snet::Socket_item>& socket_list)
{
    fd_set set_receive;
    fd_set set_send;
    fd_set set_error;

    FD_ZERO(&set_receive);
    FD_ZERO(&set_send);
    FD_ZERO(&set_error);

    unsigned int max_sock;

    for (std::list<Socket_item>::const_iterator it = socket_list.begin(); it != socket_list.end(); it++)
    {
        if ((*it).flags > 0) if ((*it).sock+1 > max_sock) max_sock = (*it).sock+1;
        if ((*it).flags & snet::FLAG_RECEIVE) FD_SET((*it).sock, &set_receive);
        if ((*it).flags & snet::FLAG_SEND) FD_SET((*it).sock, &set_send);
        if ((*it).flags & snet::FLAG_ERROR) FD_SET((*it).sock, &set_error);
    }

    select(max_sock, &set_receive, &set_send, &set_error, NULL);

    std::list<Socket_item> return_list;

    for (std::list<Socket_item>::const_iterator it = socket_list.begin(); it != socket_list.end(); it++)
    {
        if ((*it).flags > 0)
        {
            unsigned char flags = 0;
            if (FD_ISSET((*it).sock, &set_receive)) flags |= snet::FLAG_RECEIVE;
            if (FD_ISSET((*it).sock, &set_send)) flags |= snet::FLAG_SEND;
            if (FD_ISSET((*it).sock, &set_error)) flags |= snet::FLAG_ERROR;
            return_list.push_back(Socket_item((*it).sock, flags));
        }
    }
    return return_list;
}
