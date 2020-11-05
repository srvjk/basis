#include "communicator.h"
#include <iostream>
#include <boost/asio.hpp>

using namespace std;
namespace asio = boost::asio;
namespace ip = boost::asio::ip;

static Basis::System* sys = nullptr;

struct Client::Private
{
	ip::tcp::endpoint ep;
};

Client::Client() :
	_p(make_unique<Private>())
{
}

void Client::start(ip::tcp::endpoint ep)
{
	_p->ep = ep;
	startConnect(ep);


void Client::startConnect(ip::tcp::endpoint ep)
{
	_p->ep = ep;
	startConnect(ep);
}

struct Communicator::Private
{
	//asio::ip::address remoteAddress;
	//uint16_t remotePort;
};

Communicator::Communicator(Basis::System* sys) :
	Basis::Entity(sys), _p(make_unique<Private>())
{
}

void Communicator::step()
{
	std::cout << "Communicator::step()" << endl;
}

void Communicator::send(const std::vector<unsigned char> data, const EntityAddress& address)
{
	asio::io_service service;
	ip::tcp::endpoint ep(ip::address::from_string(address.address), address.port);
	ip::tcp::socket sock(service);
	sock.async_connect
}

void setup(Basis::System* s)
{
	std::cout << "Communicator::setup()" << endl;

	sys = s;
	sys->registerEntity<Communicator>();
}
