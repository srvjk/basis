#include "aquacontroller.h"
#include <iostream>
#include <boost\asio.hpp>

using namespace std;
namespace asio = boost::asio;

static Basis::System* sys = nullptr;
static const int MAXLEN = 512;

enum class ModuleState {
	NotInitialized,
	Initialized
};

class AquaController::Private
{
public:
	ModuleState state = ModuleState::NotInitialized;
	std::unique_ptr<asio::io_service> io;
	std::unique_ptr<asio::serial_port> serial;
};

AquaController::AquaController(Basis::System* sys) :
	Basis::Entity(sys), _p(std::make_unique<Private>())
{
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&AquaController::step, this));
}

void AquaController::step()
{
	std::cout << "AquaController::step()" << endl;

	switch (_p->state) {
	case ModuleState::NotInitialized:
		reset();
		break;
	case ModuleState::Initialized:
		operate();
		break;
	default:
		break;
	}
}

void AquaController::reset()
{
	std::cout << "AquaController::reset()" << endl;

	_p->io = std::make_unique<asio::io_service>();
	_p->serial = std::make_unique<asio::serial_port>(*_p->io);
	_p->serial->open("COM3");

	_p->state = ModuleState::Initialized;
}

void AquaController::operate()
{
	std::cout << "AquaController::operate()" << endl;

	char data[MAXLEN];
	size_t nRead = asio::read(*_p->serial, asio::buffer(data, MAXLEN));
	string message(data, nRead);
	cout << message << endl;
}

void setup(Basis::System* s)
{
	std::cout << "AquaController::setup()" << endl;

	sys = s;
	sys->registerEntity<AquaController>();
	//dummy->setName("MrDummy");
}
