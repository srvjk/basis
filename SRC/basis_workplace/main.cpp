#include "basis.h"
//#include <boost/program_options/options_description.hpp>
//#include <boost/program_options/variables_map.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

using namespace Basis;
using namespace Iterable;
using namespace std;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
	cout << "Testing... " << endl;
	if (!Basis::test()) {
		cout << "Testing FAILED! Press ENTER to exit." << endl;
		getchar();
		return 1;
	}
	cout << "Testing: OK" << endl;

	System* system = System::instance();

	// обработка параметров командной строки:
	po::options_description optDesc("Options");
	optDesc.add_options()
		("help", "display help message")
		("modules-dir,D", po::value<vector<string>>(), "directory containing modules (current is the default)")
		("modules,M", po::value<vector<string>>(), "paths to modules to be loaded")
		("executors,E", po::value<vector<string>>(), "names of executors to run")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, optDesc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		cout << optDesc << endl;
		return 1;
	}

	// если указаны конкретные имена исполнителей, запоминаем их:
	vector<string> executors;
	if (vm.count("executors")) {
		executors = vm["executors"].as<vector<string>>();
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

	vector<shared_ptr<Entity>> executables;
	{
		auto exePtr = system->container()->entities(Basis::check_executable);
		cout << "Executable entities:" << std::endl;
		int i = 0;
		while (!exePtr->finished()) {
			auto exe = exePtr->value();
			executables.push_back(exe);
			cout << i + 1 << ": " << exe->typeName() << " {" << exe->id() << "} " << exe->name() << endl;
			++i;
			exePtr->next();
		}
	}

	if (!executors.empty()) {
		// назначаем исполнителей, указанных в параметрах командной строки:
		auto exePtr = system->container()->entities(Basis::check_executable);
		while (!exePtr->finished()) {
			if (std::find(executors.begin(), executors.end(), exePtr->value()->name()) != executors.end()) {
				if (system->container()->addExecutor(exePtr->value()->id())) {
					cout << "executor added: " << exePtr->value()->id() << endl;
				}
			}

			exePtr->next();
		}
	}
	else {
		// назначаем исполнителей в интерактивном режиме:
		int exeNum = executables.size();
		std::vector<int> choice;
		bool ok = false;
		do {
			std::string str;
			choice.clear();
			if (exeNum > 0)
				str = "Select active executors by numbers (e.g. 1, 2, 3) or enter 0 to exit: ";
			else
				str = "No executables found, press ENTER to exit.";
			cout << str << std::endl;

			cout << "> ";
			std::getline(std::cin, str);

			boost::char_separator<char> sep(",");
			boost::tokenizer<boost::char_separator<char>> tok(str, sep);
			for (boost::tokenizer<boost::char_separator<char>>::iterator i = tok.begin(); i != tok.end(); ++i) {
				int num = std::stoi(*i);
				if (num == 0)
					return 0;
				if (num > 0 && num <= exeNum)
					choice.push_back(num);
			}

			if (choice.empty())
				continue;

			cout << "The following entities will be set as executors:" << endl;
			for (int i : choice) {
				cout << i << ": " << executables[i - 1]->id() << endl;
			}
			cout << "Is this OK? (Yes/no) ";
			std::string ans;
			std::getline(std::cin, ans);
			boost::algorithm::to_lower(ans);
			if (ans == "" || ans == "y" || ans == "yes")
				ok = true;
		} while (!ok);

		for (int i : choice) {
			auto exe = executables[i - 1];
			if (system->container()->addExecutor(exe->id())) {
				cout << "executor added: " << exe->id() << endl;
			}
		}
	}

	//cout << "------------------------" << endl;
	//system->container()->print();
	//cout << "------------------------" << endl;

	// Основной рабочий цикл.
	// Выполняется в основном потоке, поскольку некоторым сущностям может потребоваться
	// создавать графические окна и т.п. Чтобы вынести часть работы в отдельный поток,
	// необходимо использовать контейнер.
	while (!system->shouldStop()) {
		system->step(); // TODO а что, если это всё-таки вынести в рабочий поток, а здесь читать ввод с консоли? или наоборот, ввод сделать в отдельном потоке?
	}

	return 0;
}