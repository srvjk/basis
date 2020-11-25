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
	std::string url;
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
	/// @brief ��������� ���� �������� ���������.
	void startRecv(const std::string& url);
	/// @brief ���������� ���� �������� ���������.
	void stopRecv();

private:
	void recvFunc();

private:
	std::unique_ptr<Private> _p;
};

extern "C" MODULE_EXPORT void setup(Basis::System* s);