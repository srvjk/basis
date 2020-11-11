#include "aquacontroller.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm/remove_if.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "button.h"

using namespace std;
namespace asio = boost::asio;

static Basis::System* sys = nullptr;

static const string LEVL1 = "LEVL1";
static const string TMPR1 = "TMPR1";
static const string FLTR1 = "FLTR1";
static const string PHVL1 = "PHVL1";
static const string TDS01 = "TDS01";

enum class ModuleState 
{
	NotInitialized,
	Initialized
};

struct SerialReader::Private
{
	std::unique_ptr<asio::io_service> io;
	std::unique_ptr<asio::serial_port> port;
	boost::posix_time::time_duration timeout;
	std::unique_ptr<boost::asio::deadline_timer> timer;
	boost::asio::streambuf readBuf;
	ReadResult result;
	size_t bytesTransferred = 0;
	std::string delim = "";
};

SerialReader::SerialReader() : _p(make_unique<Private>())
{
	_p->io = std::make_unique<asio::io_service>();
	_p->port = std::make_unique<asio::serial_port>(*_p->io);
	_p->timer = std::make_unique<asio::deadline_timer>(*_p->io);
}

SerialReader::~SerialReader()
{
}

void SerialReader::open(const std::string& devName)
{
	if (isOpen())
		close();

	_p->port->open(devName);
}

void SerialReader::close()
{
	if (!isOpen())
		return;

	_p->port->close();
}

bool SerialReader::isOpen() const
{
	return _p->port->is_open();
}

void SerialReader::setTimeout(const boost::posix_time::time_duration& t)
{
	_p->timeout = t;
}

void SerialReader::writeString(const std::string& s)
{
	asio::write(*_p->port, asio::buffer(s.c_str(), s.size()));
}

bool SerialReader::readStringUntil(std::string& result, const std::string& delim)
{
	_p->delim = delim;
	setupRead();

	if (_p->timeout != boost::posix_time::seconds(0))
		_p->timer->expires_from_now(_p->timeout);
	else
		_p->timer->expires_from_now(boost::posix_time::hours(10000));

	_p->timer->async_wait(boost::bind(&SerialReader::timeoutExpired, this,
		asio::placeholders::error));

	_p->result = ReadResult::InProgress;
	_p->bytesTransferred = 0;
	while (true) {
		_p->io->run_one();
		switch (_p->result) {
		case ReadResult::Success:
		{
			_p->timer->cancel();
			_p->bytesTransferred -= delim.size();
			istream istr(&_p->readBuf);
			string res(_p->bytesTransferred, '\0');
			istr.read(&res[0], _p->bytesTransferred);
			istr.ignore(delim.size());
			result = res;
			return true;
		}
		break;
		case ReadResult::TimeoutExpired:
			_p->port->cancel();
			return false;
			break;
		case ReadResult::Error:
			_p->timer->cancel();
			_p->port->cancel();
			return false;
			break;
		case ReadResult::InProgress:
			break;
		}
	}

	return false;
}

void SerialReader::setupRead()
{
	asio::async_read_until(*_p->port, _p->readBuf, _p->delim, boost::bind(&SerialReader::readCompleted, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void SerialReader::timeoutExpired(const boost::system::error_code& error)
{
	if (!error && _p->result == ReadResult::InProgress)
		_p->result = ReadResult::TimeoutExpired;
}

void SerialReader::readCompleted(const boost::system::error_code& error, const size_t bytesTransferred)
{
	if (!error) {
		_p->result = ReadResult::Success;
		_p->bytesTransferred = bytesTransferred;
		return;
	}

#ifdef _WIN32
	if (error.value() == 995)
		return;
#elif defined(__APPLE__)
	if (error.value() == 45) {
		setupRead();
		return;
	}
#else // Linux
	if (error.value() == 125)
		return;
#endif

	_p->result = ReadResult::Error;
}

struct TimePoint
{
	float value;
	boost::posix_time::ptime time;
};

class AquaController::Private
{
public:
	ModuleState state = ModuleState::NotInitialized;
	std::unique_ptr<asio::io_service> io;
	std::unique_ptr<asio::serial_port> serial;
	map<string, string> sensorData;
	std::mutex sensorDataMutex; /// мьютекс для защиты операций с данными сенсоров
	asio::streambuf readBuf;    /// буфер для чтения из последовательного порта
	std::atomic_bool readerBusy = false;
	std::unique_ptr<std::thread> serialPortThread; /// поток для работы с последовательным портом
	std::atomic_bool serialPortThreadStop;
	SerialReader serialReader;
	string stringFromDevice;
};

AquaController::AquaController(Basis::System* sys) :
	Basis::Entity(sys), 
	_p(std::make_unique<Private>())
{
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&AquaController::step, this));
}

AquaController::~AquaController()
{
	_p->serialPortThreadStop = false;
	_p->serialPortThread->join();
}

void AquaController::step()
{
	switch (_p->state) {
	case ModuleState::NotInitialized:
		reset();
		break;
	case ModuleState::Initialized:
		readDataFromController();
		break;
	default:
		break;
	}
}

void AquaController::serialWorker()
{
	_p->serialReader.open("COM3");
	if (!_p->serialReader.isOpen()) {
		cout << "could not open serial port" << endl;
		return;
	}

	_p->serialReader.setTimeout(boost::posix_time::seconds(1));

	_p->serialPortThreadStop = false;
	string stringFromDevice;
	while (!_p->serialPortThreadStop) {
		if (_p->serialReader.readStringUntil(stringFromDevice)) {
			std::lock_guard<std::mutex> lock(_p->sensorDataMutex);
			_p->stringFromDevice = stringFromDevice;
		}
	}
}

void AquaController::reset()
{
	_p->serialPortThread = std::make_unique<std::thread>(&AquaController::serialWorker, this);
	_p->state = ModuleState::Initialized;
}

void AquaController::readDataFromController()
{
	string line;
	{
		std::lock_guard<std::mutex> lock(_p->sensorDataMutex);
		line = _p->stringFromDevice;
		_p->stringFromDevice.clear();
	}
	if (line.empty())
		return;

	line.erase(boost::remove_if(line, boost::is_any_of("\r\n")), line.end());

	string header = line.substr(0, 5);
	string body = line.substr(5);

	if (header == LEVL1) {
		_p->sensorDataMutex.lock();
		_p->sensorData[LEVL1] = body;
		_p->sensorDataMutex.unlock();
	}
	if (header == TMPR1) {
		_p->sensorDataMutex.lock();
		_p->sensorData[TMPR1] = body;
		_p->sensorDataMutex.unlock();
	}
	if (header == FLTR1) {
		_p->sensorDataMutex.lock();
		_p->sensorData[FLTR1] = body;
		_p->sensorDataMutex.unlock();
	}
	if (header == PHVL1) {
		_p->sensorDataMutex.lock();
		_p->sensorData[PHVL1] = body;
		_p->sensorDataMutex.unlock();
	}
	if (header == TDS01) {
		_p->sensorDataMutex.lock();
		_p->sensorData[TDS01] = body;
		_p->sensorDataMutex.unlock();
	}

	//cout << line << endl;
}

double AquaController::getDoubleParam(const std::string& name, bool* ok) const
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

int32_t AquaController::getInt32Param(const std::string& name, bool* ok) const
{
	std::lock_guard<std::mutex> lock(_p->sensorDataMutex);

	if (ok)
		*ok = false;

	auto iter = _p->sensorData.find(name);
	if (iter == _p->sensorData.end())
		return 0;
	string valstr = iter->second;

	int32_t res;
	try {
		res = boost::lexical_cast<int32_t>(iter->second);
	}
	catch (boost::bad_lexical_cast) {
		return 0;
	}

	if (ok)
		*ok = true;
	return res;
}

void AquaController::sendCommand(const std::string& cmd)
{
	static const string delim = "\r\n";
	stringstream sstr;
	sstr << cmd << delim;

	_p->serialReader.writeString(sstr.str());
}

void AquaController::switchFilter(bool on)
{
	stringstream sstr;
	sstr << FLTR1 << (on == true ? 1 : 0);

	sendCommand(sstr.str());
}

bool AquaController::isFilterOn() const
{
	int val = getInt32Param(FLTR1);
	if (val == 1)
		return true;

	return false;
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
			cout << "could not load font" << endl;
			// TODO здесь надо просто подгрузить другой шрифт, а не флудить в лог об ошибке
		}
	}

	if (!_p->window)
		return;

	// вычисляем размер и положение области рисования, исходя из размеров окна и нужного форматного отношения:
	double aspectRatio = 0.5625;
	sf::Vector2f viewPos;
	sf::Vector2f viewSize;
	float textMargin = 5; // поля вокруг текста
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
		double pH1 = 0.0;
		double tds1 = 0.0;
		int filterState = 0;
		int waterLevelOk = 0;
		for (auto iter = sys->entityIterator(); iter.hasMore(); iter.next()) {
			auto ent = iter.value();
			auto contr = ent->as<AquaController>();
			if (contr) {
				t1 = contr->getDoubleParam(TMPR1);
				pH1 = contr->getDoubleParam(PHVL1);
				tds1 = contr->getDoubleParam(TDS01);
				filterState = contr->getInt32Param(FLTR1);
				waterLevelOk = contr->getInt32Param(LEVL1);
				break;
			}
		}

		sf::FloatRect tempRect;       // область отрисовки температуры
		sf::FloatRect pHRect;         // область отрисовки кислотности
		sf::FloatRect tdsRect;        // область отрисовки солёности
		sf::FloatRect filterBtnRect;  // область кнопки фильтра
		sf::FloatRect waterLevelRect; // область кнопки фильтра

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
			rectangle.setPosition(sf::Vector2f(tempRect.left, tempRect.top));
			rectangle.setSize(sf::Vector2f(tempRect.width, tempRect.height));
			if (t1 > 26.0) {
				bkColor = sf::Color(255, 127, 42);
				textColor = sf::Color(0, 0, 0);
			}

			rectangle.setFillColor(bkColor);
			_p->window->draw(rectangle);

			sf::Text text;
			text.setFont(_p->generalFont);

			string str = (boost::format("t %.2f\xB0\x43") % t1).str();
			text.setString(str);

			text.setCharacterSize(48); // in pixels, not points!
			text.setFillColor(textColor);
			text.setStyle(sf::Text::Bold);
			text.setPosition(sf::Vector2f(tempRect.left, tempRect.top));

			_p->window->draw(text);
		}

		// кислотность
		{
			sf::Color bkColor = sf::Color(0, 0, 0);
			sf::Color textColor = sf::Color(255, 255, 255);

			pHRect.left = viewPos.x;
			pHRect.top = tempRect.top + tempRect.height;
			pHRect.width = 250.f;
			pHRect.height = 60.f;

			sf::RectangleShape rectangle;
			rectangle.setPosition(sf::Vector2f(pHRect.left, pHRect.top));
			rectangle.setSize(sf::Vector2f(pHRect.width, pHRect.height));

			rectangle.setFillColor(bkColor);
			_p->window->draw(rectangle);

			sf::Text text;
			text.setFont(_p->generalFont);

			string str = (boost::format("pH %.2f") % pH1).str();
			text.setString(str);

			text.setCharacterSize(48); // in pixels, not points!
			text.setFillColor(textColor);
			text.setStyle(sf::Text::Bold);
			text.setPosition(sf::Vector2f(pHRect.left, pHRect.top));

			_p->window->draw(text);
		}

		// солёность
		{
			sf::Color bkColor = sf::Color(0, 0, 0);
			sf::Color textColor = sf::Color(255, 255, 255);

			tdsRect.left = viewPos.x;
			tdsRect.top = pHRect.top + pHRect.height;
			tdsRect.width = 400.f;
			tdsRect.height = 60.f;

			sf::RectangleShape rectangle;
			rectangle.setPosition(sf::Vector2f(tdsRect.left, tdsRect.top));
			rectangle.setSize(sf::Vector2f(tdsRect.width, tdsRect.height));

			rectangle.setFillColor(bkColor);
			_p->window->draw(rectangle);

			sf::Text text;
			text.setFont(_p->generalFont);

			string str = (boost::format("Sal %.3f") % (tds1 / 1000.0)).str();
			text.setString(str);

			text.setCharacterSize(48); // in pixels, not points!
			text.setFillColor(textColor);
			text.setStyle(sf::Text::Bold);
			text.setPosition(sf::Vector2f(tdsRect.left, tdsRect.top));

			_p->window->draw(text);
		}

		// уровень воды
		{
			sf::Color bkColor = sf::Color(0, 0, 0);
			sf::Color textColor = sf::Color(255, 255, 255);
			string okMessage = "Water level OK";
			string problemMessage = "Water level LOW";
			string longestMessage = okMessage.length() > problemMessage.length() ? okMessage : problemMessage;

			sf::Text text;
			text.setString(longestMessage);
			text.setFont(_p->generalFont);
			text.setCharacterSize(48);
			text.setStyle(sf::Text::Bold);

			// определяем размеры текста:
			sf::FloatRect textBounds = text.getLocalBounds();
			waterLevelRect.left = viewPos.x;
			waterLevelRect.top = tdsRect.top + tdsRect.height;
			waterLevelRect.width = textBounds.width + 2 * textMargin;
			waterLevelRect.height = textBounds.top + textBounds.height + 2 * textMargin;

			string message = okMessage;
			sf::RectangleShape rectangle;
			rectangle.setPosition(sf::Vector2f(waterLevelRect.left, waterLevelRect.top));
			rectangle.setSize(sf::Vector2f(waterLevelRect.width, waterLevelRect.height));
			if (waterLevelOk != 1) {
				bkColor = sf::Color(255, 50, 50);
				textColor = sf::Color(0, 0, 0);
				message = problemMessage;
			}

			rectangle.setFillColor(bkColor);
			_p->window->draw(rectangle);

			text.setFillColor(textColor);
			text.setPosition(sf::Vector2f(waterLevelRect.left + textMargin, waterLevelRect.top + textMargin));

			text.setString(message);

			_p->window->draw(text);
		}

		// фильтр
		{
			filterBtnRect.left = viewPos.x;
			filterBtnRect.top = waterLevelRect.top + waterLevelRect.height;
			filterBtnRect.width = 250.f;
			filterBtnRect.height = 60.f;

			auto filterButton = Button::make("FilterButton", _p->window.get());
			filterButton->setRect(filterBtnRect);
			if (filterState == 1) {
				filterButton->setBkColor(sf::Color(100, 200, 200));
			}
			else {
				filterButton->setBkColor(sf::Color(50, 50, 50));
			}

			sf::Text text;
			text.setFont(_p->generalFont);
			text.setString("Filter");

			text.setCharacterSize(48);
			text.setFillColor(sf::Color(255, 255, 255));
			text.setStyle(sf::Text::Bold);

			filterButton->setText(text);

			if (filterButton->clicked()) {
				for (auto iter = sys->entityIterator(); iter.hasMore(); iter.next()) {
					auto ent = iter.value();
					auto contr = ent->as<AquaController>();
					if (contr) {
						bool filterOn = contr->isFilterOn();
						contr->switchFilter(!filterOn);
						break;
					}
				}
			}
			filterButton->draw(_p->window.get());
		}

		_p->window->display();
	}
}

void setup(Basis::System* s)
{
	sys = s;
	sys->registerEntity<AquaController>();
	sys->registerEntity<AquaViewer>();
}
