#include "communicator.h"
#include <iostream>
#include <boost/bind.hpp>
#include <thread>
#include <atomic>
#include <nng/nng.h>

using namespace std;

static Basis::System* sys = nullptr;

struct Communicator::Private
{
	std::string receiverPartUrl;
	std::unique_ptr<std::thread> recvThread;
	std::atomic_bool shouldStop = false;
};

Communicator::Communicator(Basis::System* sys) :
	Basis::Entity(sys), _p(make_unique<Private>())
{
}

void Communicator::send(const std::string& data, const EntityAddress& address)
{
	//int sock = nn_socket(AF_SP, NN_REQ);
	//if (sock < 0)
	//	return;

	//int rv = nn_connect(sock, address.url.c_str());
	//if (rv < 0)
	//	return;

	//int res = nn_send(sock, data.data(), data.size(), 0);
	//if (res < 0)
	//	return;

	//char *recvBuf = nullptr;
	//res = nn_recv(sock, &recvBuf, NN_MSG, 0);
	//if (res < 0)
	//	return;

	//nn_freemsg(recvBuf);
	//nn_shutdown(sock, 0);
}

void Communicator::onMessage(const std::string& message)
{
	cout << "That's what I have received: " << message << endl;
}

void Communicator::startRecv(const std::string& url)
{
	_p->receiverPartUrl = url;
	_p->recvThread = std::make_unique<std::thread>(&Communicator::recvFunc, this);
}

void Communicator::stopRecv()
{
	if (!_p->recvThread)
		return;

	_p->shouldStop = true;
	_p->recvThread->join();
}

void Communicator::recvFunc()
{
	//int sock = nn_socket(AF_SP, NN_REP);
	//if (sock < 0)
	//	return;

	//int rv = nn_bind(sock, _p->receiverPartUrl.c_str());
	//if (rv < 0)
	//	return;

	//_p->shouldStop = false;
	//while (_p->shouldStop) {
	//	char* recvBuf = nullptr;
	//	int res = nn_recv(sock, &recvBuf, NN_MSG, 0);
	//	if (res < 0)
	//		continue;

	//	nn_freemsg(recvBuf);
	//}
}

void setup(Basis::System* s)
{
	std::cout << "Communicator::setup()" << endl;

	sys = s;
	sys->registerEntity<Communicator>();
}