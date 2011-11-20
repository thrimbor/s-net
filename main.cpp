#include <iostream>
#include "snet.hpp"

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
            std::cout << "new client: " << std::endl;

            bytes = client->receive(buffer, bytes);
            buffer[bytes] = 0;

            std::cout << "client said: " << buffer << "(" << bytes << ")" << std::endl;
            //client->destroy();
            delete client;
        }
    } else {
        try
        {
            snet::watcher();
            snet::TCP_client* client;
            std::cout << "connecting to: " << std::string(argv[2]) << std::endl;
            client = new snet::TCP_client(snet::IPv6, std::string(argv[2]), 1337);

            char buffer[7];
            buffer[0] = 'h';
            buffer[1] = 'e';
            buffer[2] = 'l';
            buffer[3] = 'l';
            buffer[4] = 'o';
            buffer[5] = '!';
            buffer[6] = 0;
            client->send(buffer, 7);
            delete client;
        } catch (snet::Exception e) {
            std::cout << "Exception: " << e.what() << std::endl;
            return 0;
        }
    }
}
