#pragma once

#include "basis.h"

#ifdef PLATFORM_WINDOWS
#  ifdef COMMUNICATOR_LIB
#    define MODULE_EXPORT __declspec(dllexport)
#  else
#    define MODULE_EXPORT __declspec(dllimport)
#  endif
#else
#  define MODULE_EXPORT
#endif

/// @brief Полный сетевой адрес сущности.
struct EntityAddress
{
	std::string address;
	uint16_t port;
	Basis::uid entityId;
};

class Client
{
	struct Private;

public:
	Client();
	void start(ip::tcp::endpoint ep);
	void stop();

private:
	void startConnect(ip::tcp::endpoint ep);
	void handleConnect();
	void startWrite();
	void handleWrite();

private:
	std::unique_ptr<Private> _p;
};

class Communicator : public Basis::Entity
{
	struct Private;

public:
	Communicator(Basis::System* s);
	void step();
	/// @brief Отправить пакет данных по указанному адресу.
	void send(const std::vector<unsigned char> data, const EntityAddress& address);

private:
	std::unique_ptr<Private> _p;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);