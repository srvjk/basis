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
	std::string url;
	Basis::uid entityId;
};

class Communicator : public Basis::Entity
{
	struct Private;

public:
	Communicator(Basis::System* s);
	/// @brief Отправить пакет данных по указанному адресу.
	void send(const std::string& data, const EntityAddress& address);
	virtual void onMessage(const std::string& message) override;
	/// @brief Запустить приём входящих сообщений.
	void startRecv(const std::string& url);
	/// @brief Остановить приём входящих сообщений.
	void stopRecv();

private:
	void recvFunc();

private:
	std::unique_ptr<Private> _p;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);