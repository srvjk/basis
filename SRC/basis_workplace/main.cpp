#include "basis.h"
//#include <boost/program_options/options_description.hpp>
//#include <boost/program_options/variables_map.hpp>
#include <boost/program_options.hpp>
#include <iostream>

using namespace Basis;
using namespace std;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
	System* system = System::instance();

	// обработка параметров командной строки:
	po::options_description optDesc("Options");
	optDesc.add_options()
		("help", "display help message")
		("modules-dir,D", po::value<vector<string>>(), "directory containing modules (current is the default)")
		("modules,M", po::value<vector<string>>(), "paths to modules to be loaded")
		("run-sketch,R", po::value<string>(), "name of sketch to run")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, optDesc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		cout << optDesc << endl;
		return 1;
	}

	// сначала загружаем модули из указанных директорий:
	if (vm.count("modules-dir")) {
		const vector<string> dirs = vm["modules-dir"].as<vector<string>>();
		for (auto dir : dirs) {
			cout << "loading modules from " << dir << endl;
			system->loadModules(dir);
		}
	}

	// затем загружаем модули, указанные отдельно:
	if (vm.count("modules")) {
		const vector<string> modules = vm["modules"].as<vector<string>>();
		for (auto module : modules) {
			system->loadModules(module);
		}
	}

	//Basis::Sketch sketch;

	// TODO выводим список доступных скетчей с их номерами и предлагаем выбрать один из них в качестве рабочего.
	// должна быть также возможность сразу выбрать рабочий скетч по его имени из командной строки!


	//sketch.exec();

	//sf::RenderWindow window(sf::VideoMode(800, 800), "Basis");

	//while (window.isOpen()) {
	//	sf::Event ev;
	//	while (window.pollEvent(ev)) {
	//		if (ev.type == sf::Event::Closed)
	//			window.close();
	//	}

	//	window.clear(sf::Color::Black);
	//	window.display();
	//}

	return 0;
}