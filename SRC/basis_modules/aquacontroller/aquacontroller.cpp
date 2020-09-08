#include "aquacontroller.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm/remove_if.hpp>
#include <boost/format.hpp>
#include <SFML/Window.hpp>
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
	string portName = "COM3";

	_p->io = std::make_unique<asio::io_service>();
	_p->serial = std::make_unique<asio::serial_port>(*_p->io);
	try {
		_p->serial->open(portName);
	}
	catch (boost::system::system_error) {
		cout << "could not open serial port: " << portName << endl;
		return;
	}

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

struct AquaViewer::Private
{
	//ImVec4 clearColor = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
	//ImVec4 buttonOnColor = ImVec4(78 / 256.0, 201 / 256.0, 176 / 256.0, 1.00f);
	//ImVec4 buttonOffColor = ImVec4(10 / 256.0, 10 / 256.0, 10 / 256.0, 1.00f);
	std::unique_ptr<sf::RenderWindow> window = nullptr;
	sf::Font generalFont;
};

// Если размеры окна заданы в явном виде, создаём именно такое окно (это может быть полезно в процессе разработки).
// Если размеры окна не заданы, переходим в полноэкранный режим.
// В любом случае вычисляем размеры максимально возможной в данном режиме области, допускающей нужное форматное отношение,
// и всё рисуем внутри этой области.
void AquaViewer::step()
{
	if (!_p->window) {
		_p->window = make_unique<sf::RenderWindow>(sf::VideoMode(1024, 768), "My Aquarium");

		if (!_p->generalFont.loadFromFile("EurostileBQ-BoldExtended.otf")) {
			printf("Warning: could not load font!");
			// TODO здесь надо просто подгрузить другой шрифт, а не флудить в лог об ошибке
		}
	}

	if (!_p->window)
		return;

	// вычисляем размер и положение области рисования, исходя из размеров окна и нужного форматного отношения:
	double aspectRatio = 0.5625;
	sf::Vector2f viewPos;
	sf::Vector2f viewSize;
	{
		sf::Vector2u actualSize = _p->window->getSize();
		double desiredWidth = actualSize.y / aspectRatio;
		if (desiredWidth <= actualSize.x) {
			viewSize.x = desiredWidth;
			viewSize.y = actualSize.y;
		}
		else {
			viewSize.x = actualSize.x;
			viewSize.y = viewSize.x * aspectRatio;
		}

		double dx = actualSize.x - viewSize.x;
		double dy = actualSize.y - viewSize.y;
		viewPos.x = dx / 2.0;
		viewPos.y = dy / 2.0;
	}

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

		sf::FloatRect tempRect;      // область отрисовки температуры
		sf::FloatRect filterBtnRect; // область кнопки фильтра

		// рисуем границы области отображения
		{
			sf::Color bkColor = sf::Color(0, 0, 0);
			sf::Color foreColor = sf::Color(100, 100, 100);

			sf::RectangleShape outRect;
			outRect.setPosition(viewPos);
			outRect.setSize(viewSize);

			outRect.setFillColor(bkColor);
			outRect.setOutlineColor(foreColor);
			outRect.setOutlineThickness(1.0);
			_p->window->draw(outRect);
		}

		// температура
		{
			sf::Color bkColor = sf::Color(0, 0, 0);
			sf::Color textColor = sf::Color(255, 255, 255);

			tempRect.left = viewPos.x;
			tempRect.top = viewPos.y;
			tempRect.width = 250.f;
			tempRect.height = 60.f;

			sf::RectangleShape rectangle;
			rectangle.setPosition(tempRect.getPosition());
			rectangle.setSize(tempRect.getSize());
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
			text.setPosition(tempRect.getPosition());

			_p->window->draw(text);
		}

		// кнопка включения/выключения фильтра
		{
			sf::Color bkColor = sf::Color(100, 200, 200);
			sf::Color textColor = sf::Color(255, 255, 255);

			filterBtnRect.left = tempRect.left + tempRect.width;
			filterBtnRect.top = tempRect.top;
			filterBtnRect.width = 250.f;
			filterBtnRect.height = 60.f;

			sf::RectangleShape rectangle;
			rectangle.setPosition(filterBtnRect.getPosition());
			rectangle.setSize(filterBtnRect.getSize());

			rectangle.setFillColor(bkColor);
			_p->window->draw(rectangle);

			sf::Text text;
			text.setFont(_p->generalFont);
			text.setString("Filter");

			text.setCharacterSize(48);
			text.setFillColor(textColor);
			text.setStyle(sf::Text::Bold);
			text.setPosition(filterBtnRect.getPosition());

			_p->window->draw(text);
		}

		_p->window->display();
	}
}

void setup(Basis::System* s)
{
	//std::cout << "AquaController::setup()" << endl;

	sys = s;
	sys->registerEntity<AquaController>();
	sys->registerEntity<AquaViewer>();
	//dummy->setName("MrDummy");
}
