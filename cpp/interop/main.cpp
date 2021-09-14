#include <unistd.h>

#include <iostream>

#include "Tcp.h"
#include "InteropTest.h"

static constexpr inline uint16_t port = 0xafaf;

int main(int argc, const char* argv[])
{
    bool srv = false;

    if(argc > 1)
    {
        if(std::string(argv[1]) != "-l" || argc > 2)
        {
            std::cerr << "RPC interoperability test utility." << std::endl;
            std::cerr << "\tusage: " << argv[1] << " [-l]" << std::endl;
            return -1;
        }

        srv = true;
    }

    try
    {
        const auto sock = srv ? listen(port) : connect("127.0.0.1", port);
        std::thread t;

        if(srv)
        {
        	t = runInteropListener(std::make_shared<Service>(sock, sock));
        }
        else
        {
        	t = runInteropTests(std::make_shared<Client>(sock, sock));
        }

        closeNow(sock);
        t.join();

        std::cout << "testing completed successfully" << std::endl;
        return 0;
    } 
    catch(const std::exception& e) 
    {
        std::cerr << "exception caught: " << e.what() << std::endl;
        std::cerr << "testing aborted" << std::endl;
        return -2;
    }
}
