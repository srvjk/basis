#include "basis.h"
#include "basis_private.h"
#include <iostream>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/dll.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/format.hpp>

using namespace Basis;
using namespace std;

namespace fs = boost::filesystem;

point3d Basis::operator+(const point3d& p1, const point3d& p2)
{
	point3d ret;
	ret.set<0>(p1.get<0>() + p2.get<0>());
	ret.set<1>(p1.get<1>() + p2.get<1>());
	ret.set<2>(p1.get<2>() + p2.get<2>());
	return ret;
}

point3d Basis::operator-(const point3d& p1, const point3d& p2)
{
	point3d ret;
	ret.set<0>(p1.get<0>() - p2.get<0>());
	ret.set<1>(p1.get<1>() - p2.get<1>());
	ret.set<2>(p1.get<2>() - p2.get<2>());
	return ret;
}

point3d Basis::operator*(const point3d& p, double v)
{
	point3d ret;
	ret.set<0>(p.get<0>() * v);
	ret.set<1>(p.get<1>() * v);
	ret.set<2>(p.get<2>() * v);
	return ret;
}

point3d Basis::operator*(double v, const point3d& p)
{
	point3d ret;
	ret.set<0>(p.get<0>() * v);
	ret.set<1>(p.get<1>() * v);
	ret.set<2>(p.get<2>() * v);
	return ret;
}

double Basis::length(const point3d& v)
{
	double x = v.get<0>();
	double y = v.get<1>();
	double z = v.get<2>();

	return sqrt(x*x + y*y + z*z);
}

void Basis::cutoff(std::string& str, const std::string& what)
{
	size_t i = str.rfind(what);
	if (i != std::string::npos) { // подстрока найдена
		if ((i + what.length()) < str.size()) { // где-то в начале или в середине
			str = str.substr(i + what.length());
		}
		else { // в самом конце
			str.clear();
		}
	}
}

std::shared_ptr<Entity> Iterator::_value()
{
	return nullptr;
}

void Iterator::_next()
{
}

bool Iterator::_finished() const
{
	return true;
}

void Iterator::_reset()
{
}

IteratorPtr::IteratorPtr(Iterator* iter) : _iter(std::shared_ptr<Iterator>(iter))
{
}

IteratorPtr::IteratorPtr(const IteratorPtr& other)
{
	_iter = other._iter;
}

IteratorPtr IteratorPtr::operator=(const IteratorPtr& other)
{
	_iter = other._iter;
	return *this;
}

Iterator* IteratorPtr::operator->()
{
	return _iter.get();
}

Iterator& IteratorPtr::operator*()
{
	return *(_iter.get());
}

Iterator::Iterator()
{}

Iterator::Iterator(Iterator&& src) noexcept
{
	swap(src);
}

Iterator& Iterator::operator=(Iterator&& src) noexcept
{
	Iterator tmp(std::move(src));
	swap(tmp);
	return *this;
}

void Iterator::swap(Iterator& other) noexcept
{
	std::swap<>(_selector, other._selector);
}

std::shared_ptr<Entity> Iterator::value()
{
	// если условие выбора не задано, просто возвращаем текущий элемент
	if (_selector == nullptr)
		return _value();

	// если условие выбора задано:
	while (!_finished()) {
		std::shared_ptr<Entity> val = _value();
		if (_selector(val))
			return val; // ok, условие удовлетворено
		// условие не удовлетворено, переходим к следующему элементу
		_next();
	}

	return nullptr;
}

void Iterator::next()
{
	if (_finished())
		return;

	// если условие не задано, просто перемещаемся к следующему элементу
	if (_selector == nullptr) {
		_next();
		return;
	}

	// если условие задано, прокручиваем до следующего подходящего элемента
	// или до конца списка
	while (!_finished()) {
		_next();
		if (_finished())
			break;
		if (_selector(_value()))
			break;
	}
}

bool Iterator::finished() const
{
	return _finished();
}

bool Iterator::hasMore() const
{
	return (_finished() == false);
}

void Iterator::setSelector(Selector<Entity> selector)
{
	_reset();
	_selector = selector;
	value(); // прокрутка до первого элемента, удовлетворяющего условию
}

Selector<Entity> Iterator::selector() const
{
	return _selector;
}

ListIterator::ListIterator(std::shared_ptr<List<Entity>>& lst) :
	Iterator(),
	_list(lst)
{
	_position = _list->head();
}

ListIterator::ListIterator(ListIterator&& src) noexcept :
	Iterator(std::move(src))
{
	swap(src);
}

ListIterator& ListIterator::operator=(ListIterator&& src) noexcept
{
	ListIterator tmp(std::move(src));
	swap(tmp);
	return *this;
}

bool ListIterator::_finished() const
{
	if (_position != nullptr)
		return false;

	return true;
}

std::shared_ptr<Entity> ListIterator::_value()
{
	if (_finished())
		return nullptr;

	return _position->value();
}

void ListIterator::_next()
{
	if (_finished())
		return;

	_position = _position->next();
}

void ListIterator::_reset()
{
	_position = _position->prev();
}

void ListIterator::swap(ListIterator& other) noexcept
{
	std::swap<>(_list, other._list);
	std::swap<>(_position, other._position);
}

Entity::Entity()
{
	_p = nullptr;
}

Entity::Entity(System* sys) : _p(make_unique<Private>())
{
	_p->system_ptr = sys;
	_p->uuidIndex.clear();
	_p->nameIndex.clear();
	_p->entities = std::make_shared<List<Entity>>();
}

Entity::~Entity()
{
}

bool Entity::isNull() const
{
	return (_p.get() == nullptr);
}

void Entity::setTypeId(tid typeId)
{
	_p->typeId = typeId;
}

tid Entity::typeId() const
{
	return _p->typeId;
}

uid Entity::id() const
{
	return _p->id;
}

const std::string Entity::typeName() const
{
	return system()->typeIdToTypeName(_p->typeId);
}

const std::string Entity::name() const
{
	return _p->name;
}

void Entity::setName(const std::string& name)
{
	string oldName = _p->name;
	_p->name = name;
	Entity* parent = _p->parent;
	if (parent)
		parent->updateNameIndexRecord(shared_from_this(), name, oldName);
}

void Entity::updateNameIndexRecord(std::shared_ptr<Entity> ent, const std::string& name, const std::string& oldName)
{
	// если в индексе есть этот же элемент под старым именем, удаляем его:
	if (!oldName.empty() && oldName != name) {
		pair<multimap<string, std::shared_ptr<Entity>>::iterator,
			multimap<string, std::shared_ptr<Entity>>::iterator> itRange =
			_p->nameIndex.equal_range(oldName);

		auto it = itRange.first;
		while (it != itRange.second) {
			if (it->second == ent)
				it = _p->nameIndex.erase(it);
			else
				++it;
		}
	}

	_p->nameIndex.insert(std::make_pair(name, ent));
}

shared_ptr<Entity> Entity::addFacet(tid typeId)
{
	auto iter = _p->facets.find(typeId);
	if (iter != _p->facets.end())
		return iter->second; // такая грань уже есть

	auto newFacet = system()->createEntity(typeId);
	if (!newFacet)
		return nullptr;

	_p->facets.insert(std::make_pair<>(typeId, newFacet));

	return newFacet;
}

shared_ptr<Entity> Entity::as(tid typeId)
{
	if (typeId == _p->typeId)
		return shared_from_this();

	auto iter = _p->facets.find(typeId);
	if (iter != _p->facets.end())
		return iter->second;

	return nullptr;
}

bool Entity::hasFacet(tid typeId)
{
	auto iter = _p->facets.find(typeId);
	if (iter != _p->facets.end())
		return true;

	return false;
}

System* Entity::system() const
{
	return _p->system_ptr;
}

void Entity::print()
{
	std::cout << "-> entity" << endl;
	std::cout << "id: " << _p->id << endl;
	std::cout << "-> facets" << endl;
	for (auto fac : _p->facets) {
		std::cout << "-> facet" << endl;
		fac.second->print();
		std::cout << "<- facet" << endl;
	}
	std::cout << "<- facets" << endl;
	std::cout << "<- entity" << endl;
}

void Entity::onMessage(const std::string& message)
{
}

bool Entity::init()
{
	return true;
}

void Entity::cleanup()
{
}

Executable::Executable(System* sys) :
	Entity(sys),
	_p(make_unique<Private>())
{
}

Executable::~Executable()
{
}

void Executable::step()
{
	if (_p->stepFunction)
		_p->stepFunction();
}

void Executable::setStepFunction(std::function<void()> func)
{
	_p->stepFunction = func;
}

void Executable::setActive(bool active)
{
	_p->active = active;
}

bool Executable::isActive() const
{
	return _p->active;
}

void Executable::print()
{
	Entity::print();

	std::cout << "[Executable]" << endl;
}

std::shared_ptr<List<Entity>> Entity::entities()
{
	return _p->entities;
}

shared_ptr<Entity> Entity::newEntity(tid typeId)
{
	shared_ptr<Entity> ent = system()->createEntity(typeId);
	if (!ent)
		return nullptr;

	ent->setParent(this);

	auto iter = _p->entities->pushBack(ent);
	// добавляем элемент в uuid-индекс
	_p->uuidIndex.insert(std::make_pair(ent->id(), iter));

	ent->init();

	return ent;
}

void Entity::setParent(Entity* parent)
{
	_p->parent = parent;
}

Entity* Entity::parent() const
{
	return _p->parent;
}

void Entity::removeEntities(Selector<Entity> match)
{
	auto item = _p->entities->head();
	while (item) {
		if (!match) {
			item = _p->entities->remove(item);
		}
		else {
			if (match(item->value()))
				item = _p->entities->remove(item);
			else
				item = item->next();
		}
	}
}

void Entity::removeEntity(const uid& id)
{
	auto iter = _p->uuidIndex.find(id);
	if (iter != _p->uuidIndex.end()) {
		_p->entities->remove(iter->second);
	}
}

int64_t Entity::entityCount(Selector<Entity> match)
{
	if (!match)
		return _p->entities->size();

	int64_t count = 0;
	auto item = _p->entities->head();
	while (item) {
		if (match(item->value()))
			++count;
		item = item->next();
	}

	return count;
}

ListIterator Entity::entityIterator(Selector<Entity> match)
{
	ListIterator iter(_p->entities);
	if (match)
		iter.setSelector(match);

	return std::move(iter);
}

std::shared_ptr<Entity> Entity::findEntityById(const uid& id)
{
	auto iter = _p->uuidIndex.find(id);
	if (iter != _p->uuidIndex.end())
		return iter->second->value();

	return nullptr;
}

std::vector<std::shared_ptr<Entity>> Entity::findEntitiesByName(const std::string& name)
{
	vector<shared_ptr<Entity>> res;
	pair<multimap<string, std::shared_ptr<Entity>>::iterator,
		multimap<string, std::shared_ptr<Entity>>::iterator> itRange =
		_p->nameIndex.equal_range(name);

	for (auto it = itRange.first; it != itRange.second; ++it)
		res.push_back(it->second);

	return res;
}

Entity::operator bool() const
{
	return (isNull() == false);
}

Spatial::Spatial(System* sys) :
	Entity(sys),
	_p(make_unique<Private>())
{
}

Spatial::~Spatial()
{
}

point3d Spatial::position() const
{
	return _p->position;
}

void Spatial::setPosition(const point3d& pos)
{
	_p->position = pos;
}

point3d Spatial::orientation() const
{
	return _p->orientation;
}

void Spatial::setOrientation(const point3d& orient)
{
	_p->orientation = orient;
}

double Spatial::size() const
{
	return _p->size;
}

void Spatial::setSize(double sz)
{
	_p->size = sz;
}

System* System::instance()
{
	static System sys;
	return &sys;
}

System::System() : Entity(this), _p(new Private())
{
	// регистрация системных сущностей
	registerEntity<Entity>();
	registerEntity<Executable>();
	registerEntity<Spatial>();
}

System::~System()
{
	delete _p;
	_p = nullptr;
}

int System::loadModules(const string& path, bool recursive)
{
	fs::path p(path);
	if (!fs::exists(p)) {
		cout << path << "path does not exist: " << path << endl;
		return 0;
	}

	if (fs::is_regular_file(p)) {
		if (_p->loadModule(p.generic_string())) {
			return 1;
		}
	}
	else if (fs::is_directory(p)) {
		int res = 0;
		for (fs::directory_entry& entry : fs::directory_iterator(p)) {
			if (recursive) {
				res += loadModules(entry.path().generic_string(), recursive);
			}
			else {
				if (_p->loadModule(entry.path().generic_string()))
					res++;
			}
		}

		return res;
	}

	return 0;
}

bool System::isEntityRegistered(tid typeId) const
{
	if (_p->factories.count(typeId) > 0)
		return true;

	return false;
}

int64_t System::entityTypesCount() const
{
	return _p->factories.size();
}

bool System::addFactory(FactoryInterface* f)
{
	_p->factories[f->typeId()] = std::shared_ptr<FactoryInterface>(f);

	return true;
}

bool System::removeFactory(tid typeId)
{
	_p->factories.erase(typeId);

	if (_p->factories.find(typeId) != _p->factories.end())
		return false;
	return true;
}

void System::executeBatchFile(const std::string& path)
{
	fs::path p(path);
	if (!fs::exists(p)) {
		cout << path << "path does not exist: " << path << endl;
		return;
	}

	if (!fs::is_regular_file(p)) {
		cout << path << "not a valid batch file: " << path << endl;
		return;
	}

	ifstream fs(p.generic_string());
	if (!fs.is_open()) {
		cout << "unable to open file: " << path << endl;
		return;
	}

	string line;
	while (getline(fs, line)) {
		onCommand(line);
	}

	fs.close();
}

void System::onCommand_to(const std::string& command)
{
	size_t pos = command.find(':');
	if (pos == string::npos)
		return;

	string firstPart = command.substr(0, pos);   // от начала строки до первого двоеточия
	string secondPart = command.substr(pos + 1); // от первого двоеточия до конца строки
	if (firstPart.empty())
		return;
	if (secondPart.empty())
		return;

	vector<string> lst;
	boost::split(lst, firstPart, [](char c) {return c == ' '; });

	if (lst.empty())
		return;

	if (lst.size() < 2)
		return;

	for (int i = 1; i < lst.size(); ++i) {
		string token = lst.at(i);

		for (auto entPtr = entityIterator(); entPtr.hasMore(); entPtr.next()) {
			auto ent = entPtr.value();

			bool selected = false;

			if (ent->name() == token) { // по имени
				selected = true;
			}
			else {
				// по UUID
				std::string str = boost::uuids::to_string(ent->id());
				if (boost::starts_with(str, token)) {
					selected = true;
				}
			}

			if (selected) {
				ent->onMessage(secondPart);
			}
		}
	}
}

void System::onCommand(const std::string& command)
{
	//cout << "-> New command received: " << command << endl;

	vector<string> lst;
	boost::split(lst, command, [](char c) {return c == ' '; });

	if (lst.empty())
		return;

	for (auto it = lst.begin(); it != lst.end(); ++it) {
		boost::trim(*it);
		//boost::to_lower(*it);
	}

	string cmd = boost::to_lower_copy(lst.at(0));

	// exit application
	if (cmd == "quit" && lst.size() == 1) {
		_p->shouldStop = true;
		return;
	}

	// display help message
	if (cmd == "help" || cmd == "sos" || cmd == "wtf") {
		printUsage();
		return;
	}

	// execute commands from batch file
	if (cmd == "exec") {
		string path;
		if (lst.size() > 1)
			path = lst.at(1);

		if (!path.empty()) {
			cout << "executing batch file: " << path << endl;
			executeBatchFile(path);
		}

		return;
	}

	// load modules
	if (cmd == "load") {
		string path = "."; // by default, load from current directory
		if (lst.size() > 1)
			path = lst.at(1);
		loadModules(path);
		return;
	}

	// list all registered entities (i.e. those that can be created)
	if (cmd == "listavailable") {
		int i = 0;
		cout << "Registered entities:" << std::endl;
		for (auto it = _p->factories.begin(); it != _p->factories.end(); ++it) {
			auto fact = it->second;
			cout << i + 1 << ": " << fact->typeName() << " {" << fact->typeId() << "} " << endl;
			++i;
		}

		return;
	}

	// list all entities that have been created
	if (cmd == "listexistent") {
		vector<shared_ptr<Entity>> executables;
		{
			cout << "Existent entities:" << std::endl;
			int i = 0;
			for (auto entPtr = entityIterator(); entPtr.hasMore(); entPtr.next()) {
				auto ent = entPtr.value();
				cout << i + 1 << ": " << ent->typeName() << " {" << ent->id() << "} " << ent->name() << endl;
				++i;
			}
		}

		return;
	}

	// list executable entities
	if (cmd == "listexec") {
		vector<shared_ptr<Entity>> executables;
		{
			cout << "Executable entities:" << std::endl;
			int i = 0;
			for (auto entPtr = entityIterator(); entPtr.hasMore(); entPtr.next()) {
				auto ent = entPtr.value();
				auto exe = ent->as<Basis::Executable>();
				if (exe) {
					cout << i + 1 << ": " << exe->typeName() << " {" << exe->id() << "} " << exe->name() << endl;
					++i;
				}
			}
		}

		return;
	}

	// create an instance of specified type
	if (cmd == "create") {
		if (lst.size() < 2)
			return;

		for (int i = 1; i < lst.size(); ++i) {
			string token = lst.at(i);

			vector<string> sublst;
			boost::split(sublst, token, [](char c) {return c == ':'; });
			if (sublst.size() < 1)
				continue;

			string token_for_id = sublst.at(0);
			string token_for_name;
			if (sublst.size() > 1)
				token_for_name = sublst[1];

			for (auto it = _p->factories.begin(); it != _p->factories.end(); ++it) {
				auto fact = it->second;
				bool selected = false;

				string typeName = boost::to_lower_copy(fact->typeName());
				if (boost::starts_with(typeName, token_for_id)) { // by type name
					selected = true;
				}
				else {
					// by type id
					string str = std::to_string(fact->typeId());
					if (boost::starts_with(str, token_for_id)) {
						selected = true;
					}
				}

				if (selected) {
					auto newEnt = newEntity(fact->typeId());
					if (newEnt) {
						if (!token_for_name.empty())
							newEnt->setName(token_for_name);
						cout << "New entity created: " << newEnt->typeName() << " {" << newEnt->id() << "} ";
						if (!newEnt->name().empty())
							cout << ": " << newEnt->name();
						cout << endl;
					}
				}
			}
		}

		return;
	}

	// добавить сущности к списку исполнителей
	if (cmd == "addexec") {
		if (lst.size() < 2)
			return;

		for (int i = 1; i < lst.size(); ++i) {
			string token = lst.at(i);

			for (auto entPtr = entityIterator(); entPtr.hasMore(); entPtr.next()) {
				auto ent = entPtr.value();

				bool selected = false;

				if (ent->name() == token) { // по имени
					selected = true;
				}
				else {
					// по UUID
					std::string str = boost::uuids::to_string(ent->id());
					if (boost::starts_with(str, token)) {
						selected = true;
					}
				}

				if (selected) {
					auto exe = ent->as<Basis::Executable>();
					if (!exe->isNull())
						exe->setActive();
				}
			}
		}

		return;
	}

	// отправить сообщение (команду) определённым сущностям
	if (cmd == "to") {
		onCommand_to(command);
		return;
	}

	if (cmd == "pause") {
		pause();
		return;
	}

	if (cmd == "paused?") {
		if (isPaused())
			cout << "yes" << std::endl;
		else
			cout << "no" << std::endl;

		return;
	}

	if (cmd == "resume") {
		resume();
		return;
	}

	if (cmd == "step") {
		int64_t n = 1;
		if (lst.size() > 1) {
			try {
				n = boost::lexical_cast<int64_t>(lst.at(1));
			}
			catch (boost::bad_lexical_cast) {
				cout << "bad value: " << n << endl;
				n = -1;
			}
		}

		if (n > 0)
			doSteps(n);

		return;
	}

	if (cmd == "setdelay") {
		if (lst.size() > 1) {
			int64_t d = 1;
			try {
				d = boost::lexical_cast<int64_t>(lst.at(1));
			}
			catch (boost::bad_lexical_cast) {
				cout << "bad value: " << d << endl;
				d = -1;
			}

			if (d >= 0) {
				setDelay(d);
			}
			else {
				cout << "bad delay value:" << d << ", must be positive integer or 0" << endl;
			}
		}
		else {
			cout << "not enough params, please specify delay value in milliseconds" << endl;
		}
	}

	cout << "unknown command: " << cmd << endl;
	cout << "print 'help' to see supported commands" << endl;
}

void System::pause()
{
	_p->stepsToDo = -1;
	_p->paused = true;
}

void System::resume()
{
	_p->paused = false;
}

bool System::isPaused() const
{
	return _p->paused;
}

void System::doSteps(uint64_t n)
{
	if (!isPaused())
		return;

	// запоминаем, сколько шагов надо сделать перед тем, как снова встать на паузу:
	_p->stepsToDo = n;
	// и начинаем работу:
	resume();
}

void System::setDelay(int d)
{
	_p->delayBetweenSteps = d;
}

int System::delay() const
{
	return _p->delayBetweenSteps;
}

void System::printWelcome() const
{
	cout << "Hello, this is Basis. If you don't know what to do next, type 'help'." << endl;
}

void System::printUsage() const
{
	cout << "  quit           - exit program"                        << endl;
	cout << "  load           - load module(s)"                      << endl;
	cout << "  listexec       - list executable entities"            << endl;
	cout << "  addexec        - add 'executor' entity"               << endl;
	cout << "  listavailable  - show entities that can be created"   << endl;
	cout << "  listexistent   - show entities that has been created" << endl;
	cout << "  pause          - pause main loop"                     << endl;
	cout << "  paused?        - check if we are in paused state"     << endl;
	cout << "  resume         - resume main loop"                    << endl;
	cout << "  step           - make one step forward while paused"  << endl;
}

int System::randomInt(int from, int to)
{
	boost::random::uniform_int_distribution<> dist(from, to);
	return dist(_p->randGen);
}

double System::randomDouble(double from, double to)
{
	boost::random::uniform_real_distribution<> dist(from, to);
	return dist(_p->randGen);
}

int64_t System::stepsFromStart() const
{
	return _p->stepsFromStart;
}

std::shared_ptr<Module> System::Private::loadModule(const std::string& path)
{
	shared_ptr<Module> retval = nullptr;

	//cout << "loading '" << path << "'" << endl;

	fs::path p(path);
	if (!fs::exists(p)) {
		//cout << "cannot load module: path '" << path << "' not exists" << endl;
		return retval;
	}
	if (!fs::is_regular_file(p)) {
		//cout << "cannot load module: path '" << path << "' is not a file" << endl;
		return retval;
	}

	// проверка на DLL:
	try {
		boost::dll::library_info info(path, true);
	}
	catch (boost::exception& e) {
		//cout << path << " is not a module" << endl;
		return retval;
	}

	string moduleName = p.stem().generic_string();

	if (modules.find(moduleName) != modules.end())
		cout << "already loaded: " << moduleName << endl;

	auto module = make_shared<Module>();
	module->name = moduleName;

	try {
		module->lib = boost::dll::shared_library(path);
		module->setup_func = module->lib.get<void(Basis::System*)>("setup");
	}
	catch (boost::system::system_error& err) {
		cout << err.what() << endl;
		return retval;
	}

	module->setup_func(System::instance());
	modules[moduleName] = module;

	cout << "module '" << module->name << "' successfully loaded." << endl;

	return module;
}

shared_ptr<Entity> System::createEntity(tid typeId)
{
	auto iter = _p->factories.find(typeId);
	if (iter == _p->factories.end())
		return nullptr;

	FactoryInterface* factory = iter->second.get();
	if (!factory)
		return nullptr;

	shared_ptr<Entity> ent = shared_ptr<Entity>(factory->newEntity(this));
	ent->_p->system_ptr = this;
	// генерируем новый uuid:
	ent->_p->id = boost::uuids::random_generator()();

	return ent;
}

std::string System::typeIdToTypeName(tid typeId) const
{
	auto iter = _p->factories.find(typeId);
	if (iter == _p->factories.end())
		return std::string();

	FactoryInterface* factory = iter->second.get();
	if (!factory)
		return std::string();

	return factory->typeName();
}

void System::step()
{
	if (_p->stepsToDo > 0) {
		_p->stepsToDo--;
	}
	else if (_p->stepsToDo == 0) {
		pause();
	}

	if (isPaused()) {
		std::this_thread::sleep_for(1s);
		return;
	}

	int d = delay();
	if (d > 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(d));

	for (auto iter = entityIterator(); iter.hasMore(); iter.next()) {
		auto ent = iter.value();
		auto exe = ent->as<Executable>();
		if (exe) {
			if (exe->isActive()) {
				exe->step();
			}
		}
	}

	_p->stepsFromStart++;
}

bool System::shouldStop() const
{
	return _p->shouldStop;
}
