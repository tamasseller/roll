#include "InteropListener.h"

#include "RpcStlArray.h"
#include "RpcStlList.h"
#include "RpcFdStreamAdapter.h"

#include "Contract.gen.h"

#include "Tcp.h"
#include "Common.h"

struct Service: InteropTestContract::ServerProxy<Service, rpc::FdStreamAdapter>
{
	using Service::ServerProxy::ServerProxy;
	std::mutex m;
	std::condition_variable cv;
	volatile bool locked = true;
	volatile int makeCnt = 0, delCnt = 0;

	struct StreamSession: InteropTestStreamServerSession<StreamSession>
	{
		Service* srv;

		uint16_t x;
		const uint16_t mod;

		StreamSession(Service* srv, uint8_t initial, uint8_t modulus):
			srv(srv), x(initial), mod(modulus)
		{
			srv->makeCnt++;
		}

		~StreamSession() {
			srv->delCnt++;
		}

		void generate(uint32_t nData)
		{
			std::vector<uint16_t> ret;
			ret.reserve(nData);

			while(nData--)
			{
				ret.push_back(x);
				x = x * x % mod;
			}

			takeResult(srv, ret);
		}

		void onClosed()
		{
			close(srv);
		}
	};

	std::pair<std::string, std::shared_ptr<StreamSession>> open(uint8_t initial, uint8_t modulus) {
		return {"asd", std::make_shared<StreamSession>(this, initial, modulus)};
	}

	std::shared_ptr<StreamSession> openDefault() {
		return std::make_shared<StreamSession>(this, defaultInitial, defaultModulus);
	}

	struct InstaCloseSession: InteropTestInstantCloseServerSession<InstaCloseSession>
	{
		Service* srv;

		InstaCloseSession(Service* srv): srv(srv) {
			srv->makeCnt++;
		}

		~InstaCloseSession() {
			srv->delCnt++;
		}

		void onClosed() {}
	};

	std::shared_ptr<InstaCloseSession> closeMe()
	{
		return std::make_shared<InstaCloseSession>(this);
	}

	void unlock(bool doIt)
	{
		if(doIt)
		{
			std::unique_lock<std::mutex> l(m);
			locked = false;
			cv.notify_all();
		}
	}

	std::string echo(const std::string& str) {
		return std::move(str);
	}

	void wait()
	{
		for(std::unique_lock<std::mutex> l(m); locked;)
		{
			cv.wait(l);
		}
	}
};

void runInteropListener(int sock)
{
	auto uut = std::make_shared<Service>(sock, sock);
	auto t = startServiceThread(uut);

	uut->wait();

    assert(uut->makeCnt > 0);
    assert(uut->delCnt == uut->makeCnt);

    closeNow(sock);
    t.join();
}
