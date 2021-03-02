#include "RpcStlFacade.h"
#include "RpcFdStreamAdapter.h"
#include "RpcStlArray.h"
#include "RpcStlList.h"
#include "RpcStlTuple.h"
#include "RpcStreamReader.h"

#include <string>
#include <thread>
#include <memory>
#include <iostream>
#include <random>

#include <arpa/inet.h>
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <string.h>
#include <unistd.h>

static constexpr inline uint16_t port = 0xafaf;

namespace syms
{
    static constexpr auto echo = rpc::symbol<std::string, rpc::Call<std::string>>(rpc::CTStr("echo"));
    static constexpr auto nope = rpc::symbol<>(rpc::CTStr("nope"));
    static constexpr auto unlock = rpc::symbol<bool>(rpc::CTStr("unlock"));
    using Close = rpc::Call<>;
    using Read = rpc::Call<uint32_t, rpc::Call<std::list<uint16_t>>>;
    using Methods = std::tuple<Close, Read>;
    static constexpr auto open = rpc::symbol<uint8_t, uint8_t, rpc::Call<Methods>>(rpc::CTStr("open"));
}

static inline auto generateUniqueKey()
{
    std::random_device r;
    std::default_random_engine e1(r());
    std::uniform_int_distribution<char> uniform_dist('a', 'z');
    std::stringstream ss;

    for(int i = 0; i < 10; i++)
        ss << uniform_dist(e1);

    return ss.str();
}

void runTest(int sock)
{
    using Rpc = rpc::StlFacade<rpc::FdStreamAdapter>;
    auto uut = std::make_shared<Rpc>(sock, sock);

    uut->provide(syms::echo, [](Rpc::Endpoint& h, const rpc::MethodHandle &self, const auto &str, const auto &cb) {
        return h.call(cb, str);
    });

    volatile bool locked = true;
    volatile int makeCnt = 0, delCnt = 0;

    uut->provide(syms::unlock, [&locked](Rpc::Endpoint& h, const rpc::MethodHandle &self, bool doIt)
    {
        if(doIt)
            locked = false;
    });

    uut->provide(syms::open, [&makeCnt, &delCnt](Rpc::Endpoint& h, const rpc::MethodHandle &self, uint8_t init, uint8_t modulus, rpc::Call<syms::Methods> cb)
    {
        struct State
        {
            decltype(delCnt) &dcnt;
            uint16_t x;
            const uint16_t mod;
            inline State(decltype(makeCnt) &makeCnt, decltype(delCnt) &delCnt, uint16_t x, uint16_t mod): 
                dcnt(delCnt), x(x), mod(mod) 
            {
                makeCnt++;
            }

            inline ~State() {
                dcnt++;
            }

            syms::Read read;
        };

        auto state = std::make_shared<State>(makeCnt, delCnt, init, modulus);

        state->read = h.install([state](Rpc::Endpoint& h, const rpc::MethodHandle &self, uint32_t n, rpc::Call<std::vector<uint16_t>> cb)
        {
            std::vector<uint16_t> ret;
            ret.reserve(n);

            while(n--)
            {
                ret.push_back(state->x);
                state->x = state->x * state->x % state->mod;
            }

            return h.call(cb, ret);
        });

        auto close = h.install([state](Rpc::Endpoint& h, const rpc::MethodHandle &self)
        {
            if(auto err = h.uninstall(state->read))
                return err;

            return h.uninstall(self);
        });

        return h.call(cb, syms::Methods(close, state->read));
    });

    auto t = std::thread([uut](){
        while(((rpc::FdStreamAdapter*)uut.get())->receive([&uut](auto message)
        {    
            if(auto a = message.access(); auto err = uut->process(a))
                std::cerr << "RPC SRV: " << err << std::endl;

            return true;
        }));
    });

    uut->call(uut->lookup(syms::unlock).get(), false);

    bool erred = false;
    try {
        auto nope = uut->lookup(syms::nope).get();
        (void)&nope;
    } 
    catch(const std::exception& e)
    {
        erred = true;
        assert(std::string(e.what()).find(rpc::Errors::noSuchSymbol) != std::string::npos);
    }
    assert(erred);

    rpc::Call<std::string, rpc::Call<std::string>> echo = uut->lookup(syms::echo).get();
    auto key = generateUniqueKey();
    auto result = uut->call(echo, key);
    assert(result.get() == key);

    auto open = uut->lookup(syms::open).get();

    struct Tester
    {
        uint16_t state;
        const uint16_t mod;
        syms::Methods methods;
        
        void runTest(decltype(uut)& uut) 
        {
            int idx = 0;
            for(auto v: uut->call(std::get<1>(methods), 5).get())
            {
                idx++;
                assert(v == state);
                state = state * state % mod;
            }   
            assert(idx == 5);
        }

        void close(decltype(uut)& uut) {
            uut->call(std::get<0>(methods));
        }
    };

    Tester tc1{3, 31, uut->call(open, 3, 31).get()};
    Tester tc2{5, 17, uut->call(open, 5, 17).get()};

    for(int i = 0; i<10; i++)
        tc1.runTest(uut);

    for(int i = 0; i<5; i++)
        tc2.runTest(uut);

    for(int i = 0; i<7; i++)
    {
        tc1.runTest(uut);
        tc2.runTest(uut);
    }

    tc1.close(uut);
    tc2.close(uut);
    uut->call(uut->lookup(syms::unlock).get(), true);

    while(locked) std::this_thread::yield();

    assert(makeCnt > 0);
    assert(delCnt == makeCnt);
    
    shutdown(sock, SHUT_WR);
    close(sock);
    t.join();
}

void listen()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;
        exit(0);
    } 

    struct sockaddr_in servaddr; 
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port); 
  
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
        std::cerr << "socket bind failed" << std::endl;
        exit(0);
    } 

    if ((listen(sockfd, 5)) != 0) {
        std::cerr << "listen failed" << std::endl;
        exit(0);
    } 

    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    int connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
    if (connfd < 0) {
        std::cerr << "server acccept failed" << std::endl;
        exit(0);
    }
    
    runTest(connfd);
    close(sockfd); 
}

void connect()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        std::cerr << "socket creation failed" << std::endl; 
        exit(0); 
    } 

    struct sockaddr_in servaddr; 
    memset(&servaddr, 0, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(port); 
  
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) { 
        std::cerr << "connection with the server failed" << std::endl;
        exit(0); 
    } 
  
    runTest(sockfd); 
}

int main(int argc, const char* argv[])
{
    if(argc > 1)
    {
        if(std::string(argv[1]) != "-l" || argc > 2)
        {
            std::cerr << "RPC interoperability test utility." << std::endl;
            std::cerr << std::endl << "\tusage: " << argv[1] << " [-l]" << std::endl << std::endl;
            return -1;
        }
        else
        {
            listen();
        }
    }
    else
    {
        connect();
    }

    std::cout << "ok" << std::endl;
    return 0;
}