#include <iostream>
#include <string>
#include "snet.hpp"

/*
int main(int argc, char* argv[])
{
    if (argv[1][0] == 's')
    {
        snet::TCP_server* server;
        try
        {
            server = new snet::TCP_server(snet::IPv6, 1337, 10);
        }
        catch (snet::Exception e)
        {
            std::cout << "Exception: " << e.what() << std::endl;
            return 0;
        }
        for (;;)
        {
            snet::TCP_client* client;
            int bytes = 256;
            char buffer[256];

            client = server->accept();
            client->set_receive_timeout(5000);
            std::cout << "new client: " << client->get_ip() << std::endl;

            bytes = client->receive(buffer, bytes);
            buffer[bytes] = 0;

            std::cout << "client said: " << buffer << "(" << bytes << ")" << std::endl;
            delete client;
        }
    } else {
        try
        {
            snet::TCP_client* client;
            std::cout << "connecting to: " << std::string(argv[2]) << std::endl;
            client = new snet::TCP_client(snet::IPv6, std::string(argv[2]), 1337);

            std::string buffer("hello!");
            client->send(buffer.data(), buffer.size());
            delete client;
        } catch (snet::Exception e) {
            std::cout << "Exception: " << e.what() << std::endl;
            return 0;
        }
    }
}
*/

int main (int argc, char* argv[])
{
    if (argc < 2) return 1;

    if (argv[1][0] == 's')
    {
        snet::UDP_socket testsocket(snet::IPv4, 1234);
        snet::UDP_peer peer;
        char buffer[256];

        std::list<snet::Socket_item> socklist;
        socklist.push_back(snet::Socket_item(testsocket.get_sock(), snet::FLAG_RECEIVE));
        snet::poll(socklist);

        if (testsocket.is_in_list(socklist)) std::cout << "select worked" << std::endl;

        int bytes = testsocket.receive(peer, buffer, 256);

        std::cout << "client said: " << buffer << "(" << bytes << ")" << std::endl;
        std::cout << "ip was: [" << peer.get_ip() << "]:" << peer.get_port() << std::endl;
        return 0;
    }
    else
    {
        snet::UDP_socket testsocket(snet::IPv4);
        snet::UDP_peer peer(snet::IPv4, std::string(argv[2]), 1234);
        char buffer[] = "hello world from my snet!";
        testsocket.send(peer, buffer, sizeof(buffer));
    }
}
