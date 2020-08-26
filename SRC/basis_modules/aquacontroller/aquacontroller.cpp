#include "aquacontroller.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm/remove_if.hpp>
#include <boost/format.hpp>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SFML/Graphics.hpp>

using namespace std;
namespace asio = boost::asio;

static Basis::System* sys = nullptr;

enum class ModuleState 
{
	NotInitialized,
	Initialized
};

class AquaController::Private
{
public:
	ModuleState state = ModuleState::NotInitialized;
	std::unique_ptr<asio::io_service> io;
	std::unique_ptr<asio::serial_port> serial;
	map<string, string> sensorData;
	std::mutex sensorDataMutex; /// мьютекс для защиты операций с данными сенсоров
};

AquaController::AquaController(Basis::System* sys) :
	Basis::Entity(sys), 
	_p(std::make_unique<Private>())
{
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&AquaController::step, this));
}

void AquaController::step()
{
	//std::cout << "AquaController::step()" << endl;

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
	//std::cout << "AquaController::reset()" << endl;

	_p->io = std::make_unique<asio::io_service>();
	_p->serial = std::make_unique<asio::serial_port>(*_p->io);
	_p->serial->open("COM3");

	_p->state = ModuleState::Initialized;
}

void AquaController::operate()
{
	//std::cout << "AquaController::operate()" << endl;

	static const string delim = "\r\n";
	asio::streambuf buf;
	size_t nRead = asio::read_until(*_p->serial, buf, delim);
	istream istr(&buf);
	string line;
	getline(istr, line);
	line.erase(boost::remove_if(line, boost::is_any_of("\r\n")), line.end());
	{
		_p->sensorDataMutex.lock();
		_p->sensorData["TMPR1"] = line;
		_p->sensorDataMutex.unlock();
	}
	//cout << line << endl;
}

double AquaController::getDoubleParam(const std::string& name, bool* ok)
{
	std::lock_guard<std::mutex> lock(_p->sensorDataMutex);

	if (ok)
		*ok = false;

	auto iter = _p->sensorData.find(name);
	if (iter == _p->sensorData.end())
		return 0.0;
	string valstr = iter->second;

	double res;
	try {
		res = boost::lexical_cast<double>(iter->second);
	}
	catch (boost::bad_lexical_cast) {
		return 0.0;
	}

	if (ok)
		*ok = true;
	return res;
}

AquaViewer::AquaViewer(Basis::System* s) :
	Basis::Entity(sys), 
	_p(std::make_unique<Private>())
{
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&AquaViewer::step, this));
}

struct WindowData
{
	int width = 1024; /// текущая ширина окна приложения
	int height = 786; /// текущая высота окна приложения
};

static WindowData winData;

void windowPosCallback(GLFWwindow* window, int x, int y)
{
	//Mosaic::System* system = Mosaic::System::instance();
	//std::shared_ptr<MosaicSettingsImpl> settings = std::dynamic_pointer_cast<MosaicSettingsImpl>(system->settings());
	//if (!settings)
	//	return;

	//settings->winPosX = x;
	//settings->winPosY = y;
}

void windowSizeCallback(GLFWwindow* window, int width, int height)
{
	winData.width = width;
	winData.height = height;
}

struct AquaViewer::Private
{
	ImVec4 clearColor = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
	ImVec4 buttonOnColor = ImVec4(78 / 256.0, 201 / 256.0, 176 / 256.0, 1.00f);
	ImVec4 buttonOffColor = ImVec4(10 / 256.0, 10 / 256.0, 10 / 256.0, 1.00f);
	GLFWwindow* mainWnd = nullptr;
	std::unique_ptr<sf::RenderWindow> window = nullptr;
	sf::Font generalFont;
};

void AquaViewer::step()
{
	////////////////////////////////////
	if (!_p->window) {
		_p->window = make_unique<sf::RenderWindow>(sf::VideoMode(1024, 768), "My Aquarium");

		if (!_p->generalFont.loadFromFile("EurostileBQ-BoldExtended.otf")) {
			printf("Warning: could not load font!");
		}
	}

	if (!_p->window)
		return;

	if (_p->window->isOpen())
	{
		sf::Event event;
		while (_p->window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				_p->window->close();
		}

		_p->window->clear();

		double t1 = 0.0;
		for (auto iter = sys->entityIterator(); iter.hasMore(); iter.next()) {
			auto ent = iter.value();
			auto contr = ent->as<AquaController>();
			if (contr) {
				t1 = contr->getDoubleParam("TMPR1");
				break;
			}
		}

		sf::Color bkColor = sf::Color(0, 0, 0);
		sf::Color textColor = sf::Color(255, 255, 255);

		sf::RectangleShape rectangle(sf::Vector2f(250.f, 60.f));
		if (t1 > 26.0) {
			bkColor = sf::Color(255, 127, 42);
			textColor = sf::Color(0, 0, 0);
		}

		rectangle.setFillColor(bkColor);
		_p->window->draw(rectangle);

		sf::Text text;
		text.setFont(_p->generalFont);

		string str = (boost::format("%.2f") % t1).str();
		text.setString(str);

		text.setCharacterSize(48); // in pixels, not points!
		text.setFillColor(textColor);
		text.setStyle(sf::Text::Bold);

		_p->window->draw(text);
		_p->window->display();
	}
	////////////////////////////////////

	//if (!_p->mainWnd) {
	//	if (glfwInit()) {
	//		_p->mainWnd = glfwCreateWindow(1024, 768, "AquaViewer", nullptr, nullptr);
	//		if (!_p->mainWnd) {
	//			glfwTerminate();
	//			return;
	//		}
	//	}
	//	else {
	//		std::cout << "Error: GLFW initialization failed!" << std::endl;
	//		return;
	//	}

	//	glfwMakeContextCurrent(_p->mainWnd);

	//	glfwSetWindowSizeCallback(_p->mainWnd, windowSizeCallback);
	//	glfwSetWindowPosCallback(_p->mainWnd, windowPosCallback);

	//	glfwSwapInterval(1);
	//	glewInit();
	//	ImGui::CreateContext();

	//	if (!ImGui_ImplGlfw_InitForOpenGL(_p->mainWnd, true)) {
	//		printf("Error: could not initialize Gui renderer");
	//		glfwTerminate();
	//		return;
	//	}
	//	if (!ImGui_ImplOpenGL3_Init()) {
	//		printf("Error: could not initialize Gui renderer (OpenGL 3)");
	//		ImGui_ImplGlfw_Shutdown();
	//		glfwTerminate();
	//		return;
	//	}

	//	ImGuiIO& io = ImGui::GetIO();

	//	if (!_p->mainWnd)
	//		return; // не удалось создать окно
	//}

	//ImGui_ImplOpenGL3_NewFrame();
	//ImGui_ImplGlfw_NewFrame();
	//ImGui::NewFrame();

	//// all painting here
	//showInfoPanel();
	////processKeyboardEvents();

	//ImGui::Render();
	//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//glfwSwapBuffers(_p->mainWnd);
	//glfwPollEvents();
}

void AquaViewer::showInfoPanel()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(winData.width / 2, winData.height));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

	if (!ImGui::Begin("Info", 0, window_flags)) {
		ImGui::End();
		return;
	}

	ImGui::End();
	ImGui::PopStyleColor();
}

void setup(Basis::System* s)
{
	//std::cout << "AquaController::setup()" << endl;

	sys = s;
	sys->registerEntity<AquaController>();
	sys->registerEntity<AquaViewer>();
	//dummy->setName("MrDummy");
}
