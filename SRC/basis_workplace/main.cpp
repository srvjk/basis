#include "basis.h"
//#include <boost/program_options/options_description.hpp>
//#include <boost/program_options/variables_map.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/format.hpp>
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

	// формируем список директорий, из которых будем загружать модули
	vector<string> dirs;
	if (vm.count("modules-dir")) {
		dirs = vm["modules-dir"].as<vector<string>>();
	}
	else {
		dirs.push_back("."); // если никакие директории не указаны, добавляем текущую
	}

	// сначала загружаем модули из указанных директорий:
	for (auto dir : dirs) {
		cout << "loading modules from " << dir << endl;
		system->loadModules(dir);
	}

	// затем загружаем модули, указанные отдельно:
	if (vm.count("modules")) {
		const vector<string> modules = vm["modules"].as<vector<string>>();
		for (auto module : modules) {
			system->loadModules(module);
		}
	}

	cout << "Entities registered: " << system->entityTypesCount() << endl;

	vector<shared_ptr<Entity>> executables = system->container()->findEntities([](Entity* ent) -> bool {
		return ((ent->typeId() == TYPEID(Executable)) && ent->hasPrototype());
	});
	cout << "Executable entities:" << std::endl;
	for (int i = 0; i < executables.size(); ++i) {
		auto exe = executables[i];
		cout << i + 1 << ": " << exe->id() << endl;
	}

	int exeNum = executables.size();
	int choice = 0;
	do {
		std::string str;
		choice = 0;
		if (exeNum > 1)
			str = (boost::format("Enter (1.. %d) to select or 0 to exit: ") % exeNum).str();
		else if (executables.size() == 1)
			str = "Enter 1 to set the executable or 0 to exit: ";
		else
			str = "No executables found, press ENTER to exit.";
		cout << str << std::endl;

		// TODO Должна быть также возможность сразу выбрать рабочую сущность по ее имени из командной строки!

		cout << "> ";
		cin >> choice;
	} while (choice < 0 || choice > exeNum);

	if (choice > 0 && choice <= exeNum) {
		auto exe = executables[choice - 1];
		auto cnt = system->container();
		cnt->setExecutor(exe->id());
		Executable* pExe = system->container()->executor();
		if (!pExe)
			return 0;
		if (pExe->id() == exe->id())
			cout << "Ok, root executor is " << pExe->id() << endl;
		else
			cout << "Something went wrong, root executor is " << pExe->id() << endl;
	}
	else {
		return 0;
	}

	cout << "------------------------" << endl;
	system->container()->print();
	cout << "------------------------" << endl;

	// Основной рабочий цикл.
	// Выполняется в основном потоке, поскольку некоторым сущностям может потребоваться
	// создавать графические окна и т.п. Чтобы вынести часть работы в отдельный поток,
	// необходимо использовать контейнер.
	while (!system->shouldStop()) {
		system->step();
	}

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