#include "basis.h"
//#include <boost/program_options/options_description.hpp>
//#include <boost/program_options/variables_map.hpp>
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

	cout << "Testing... " << endl;
	if (!Basis::test()) {
		cout << "Testing FAILED! Press ENTER to exit." << endl;
		getchar();
		return 1;
	}
	cout << "Testing: OK" << endl;

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

	cr.stop();

	return 0;
}