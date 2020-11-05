#include "communicator.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using namespace std;
namespace asio = boost::asio;
namespace ip = boost::asio::ip;

static Basis::System* sys = nullptr;

class Sender
{
	struct Private;

public:
	Sender(boost::asio::io_context& io_context);
	void start(boost::asio::ip::tcp::endpoint ep, const std::string& data);
	void stop();

private:
	void startConnect(boost::asio::ip::tcp::endpoint ep);
	void handleConnect(boost::asio::ip::tcp::endpoint ep, const boost::system::error_code& ec);
	void startRead();
	void handleRead(const boost::system::error_code& ec, std::size_t n);
	void startWrite();
	void handleWrite(const boost::system::error_code& ec);

private:
	std::unique_ptr<Private> _p;
};

struct Sender::Private
{
	ip::tcp::endpoint ep;
	asio::steady_timer deadline;
	ip::tcp::socket sock;
	bool stopped;
	string inputBuffer;
	string outputData;

	Private(asio::io_context& io_context) :
		deadline(io_context),
		sock(io_context),
		stopped(false)
	{
	}
};

Sender::Sender(boost::asio::io_context& io_context) :
	_p(make_unique<Private>(io_context))
{
}

void Sender::start(ip::tcp::endpoint ep, const std::string& dataToSend)
{
	_p->ep = ep;
	_p->outputData = dataToSend;
	startConnect(ep);
}

void Sender::stop()
{
	_p->stopped = true;
	boost::system::error_code ec;
	_p->sock.close(ec);
	_p->deadline.cancel();
}

void Sender::startConnect(ip::tcp::endpoint ep)
{
	_p->deadline.expires_after(asio::chrono::seconds(15));
	_p->sock.async_connect(ep, boost::bind(&Sender::handleConnect, this, ep, _1));
}

void Sender::handleConnect(ip::tcp::endpoint ep, const boost::system::error_code& ec)
{
	if (_p->stopped)
		return;

	if (!_p->sock.is_open()) {
		cout << "Connect timed out" << endl;
		return;
	}

	if (ec) {
		cout << "Connect error: " << ec.message() << endl;
		_p->sock.close();
		return;
	}

	cout << "Connected to " << ep << endl;

	startRead();
	startWrite();
}

void Sender::startRead()
{
	_p->deadline.expires_after(asio::chrono::seconds(15));
	asio::async_read_until(_p->sock, boost::asio::dynamic_buffer(_p->inputBuffer),
		'\n', boost::bind(&Sender::handleRead, this, _1, _2));
}

void Sender::handleRead(const boost::system::error_code& ec, size_t n)
{
}

void Sender::startWrite()
{
	if (_p->stopped)
		return;

	boost::asio::async_write(_p->sock, boost::asio::buffer(_p->outputData, _p->outputData.size()),
		boost::bind(&Sender::handleWrite, this, _1));
}

void Sender::handleWrite(const boost::system::error_code& ec)
{
	// здесь вроде бы нечего делать...
	if (_p->stopped)
		return;
}

struct Communicator::Private
{
	asio::io_context context;
};

Communicator::Communicator(Basis::System* sys) :
	Basis::Entity(sys), _p(make_unique<Private>())
{
}

void Communicator::send(const std::string& data, const EntityAddress& address)
{
	Sender sender(_p->context);
	ip::tcp::endpoint ep = ip::tcp::endpoint(ip::address::from_string(address.address), address.port);
	sender.start(ep, data);
	_p->context.run();
}

void setup(Basis::System* s)
{
	std::cout << "Communicator::setup()" << endl;

	sys = s;
	sys->registerEntity<Communicator>();
}
