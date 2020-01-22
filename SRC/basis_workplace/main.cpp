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

	// ��������� ���������� ��������� ������:
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

	// ��������� ������ ����������, �� ������� ����� ��������� ������
	vector<string> dirs;
	if (vm.count("modules-dir")) {
		dirs = vm["modules-dir"].as<vector<string>>();
	}
	else {
		dirs.push_back("."); // ���� ������� ���������� �� �������, ��������� �������
	}

	// ������� ��������� ������ �� ��������� ����������:
	for (auto dir : dirs) {
		cout << "loading modules from " << dir << endl;
		system->loadModules(dir);
	}

	// ����� ��������� ������, ��������� ��������:
	if (vm.count("modules")) {
		const vector<string> modules = vm["modules"].as<vector<string>>();
		for (auto module : modules) {
			system->loadModules(module);
		}
	}

	cout << "Entities registered: " << system->entityTypesCount() << endl;

	vector<shared_ptr<Entity>> executables = system->findEntities([](Entity* ent) -> bool {
		return ((ent->typeId() == TYPEID(Executable)) && !ent->hasPrototype());
	});
	cout << "Executable entities: " << executables.size() << std::endl;

	//Basis::Sketch sketch;

	// TODO ������� ������ ��������� ������� � �� �������� � ���������� ������� ���� �� ��� � �������� ��������.
	// ������ ���� ����� ����������� ����� ������� ������� ����� �� ��� ����� �� ��������� ������!


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