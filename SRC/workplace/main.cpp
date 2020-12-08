#include "basis.h"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include "consolerw.h"

using namespace Basis;
using namespace std;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
	System* system = System::instance();

	po::options_description optDesc("Options");
	optDesc.add_options()
		("help", "display help message")
		("test", "run tests")
		("starter", po::value<string>(), "path to the module to be loaded and launched first")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, optDesc), vm);
	po::notify(vm);

	if (vm.count("test") > 0) {
		cout << "Testing... " << endl;
		if (!Basis::Test::test()) {
			cout << "Testing FAILED! Press ENTER to exit." << endl;
			getchar();
			return 1;
		}
		cout << "Testing: OK" << endl;
	}

	if (vm.count("starter") > 0) {
		string starterPath = vm["starter"].as<string>();
		cout << "Starter module: " << starterPath << endl;

		system->loadModule(starterPath);
	}

	system->printWelcome();

	CommandReader cr;
	cr.addReceiver(std::bind(&System::onCommand, system, std::placeholders::_1));
	cr.start();

	// Основной рабочий цикл.
	// Выполняется в основном потоке, поскольку некоторым сущностям может потребоваться
	// создавать графические окна и т.п. Чтобы вынести часть работы в отдельный поток,
	// необходимо использовать контейнер.
	while (!system->shouldStop()) {
		system->step(); 
	}

	cout << "Main event loop finished" << endl;

	cr.stop();
	cout << "Command reader finished" << endl;

	return 0;
}