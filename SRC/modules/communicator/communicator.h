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

/// @brief ������ ������� ����� ��������.
struct EntityAddress
{
	std::string address;
	uint16_t port;
	Basis::uid entityId;
};

class Communicator : public Basis::Entity
{
	struct Private;

public:
	Communicator(Basis::System* s);
	/// @brief ��������� ����� ������ �� ���������� ������.
	void send(const std::string& data, const EntityAddress& address);
	virtual void onMessage(const std::string& message) override;

private:
	std::unique_ptr<Private> _p;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);