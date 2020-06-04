#include "basis.h"
#include "basis_private.h"
#include <boost/filesystem.hpp>
#include <boost/dll.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/format.hpp>
#include <iostream>

using namespace Basis;
using namespace Iterable;
using namespace std;

namespace fs = boost::filesystem;

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

Entity::Entity()
{
	_p = nullptr;
}

Entity::Entity(System* sys) : _p(make_unique<Private>())
{
	_p->system_ptr = sys;
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
	return _p->typeName;
}

const std::string Entity::name() const
{
	return _p->name;
}

void Entity::setName(const std::string& name)
{
	_p->name = name;
}

shared_ptr<Entity> Entity::addFacet(tid typeId)
{
	auto iter = _p->facets.find(typeId);
	if (iter != _p->facets.end())
		return iter->second; // такая грань уже есть

	auto newFacet = system()->newEntity(typeId);
	if (!newFacet)
		return nullptr;

	_p->facets.insert(std::make_pair<>(typeId, newFacet));

	return newFacet;
}

shared_ptr<Entity> Entity::as(tid typeId)
{
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

shared_ptr<Entity> Entity::newEntity(tid typeId)
{
	shared_ptr<Entity> ent = system()->createEntity(typeId);
	if (!ent)
		return nullptr;

	auto iter = _p->entities.insert(_p->entities.end(), ent);
	// добавляем элемент в uuid-индекс
	_p->uuid_index.insert(std::make_pair(ent->id(), iter));

	ent->init();

	return ent;
}

IteratorPtr<std::shared_ptr<Entity>> Entity::entityIterator(Selector<std::shared_ptr<Entity>> match)
{
	ListIterator<std::shared_ptr<Entity>> *iter = new ListIterator<std::shared_ptr<Entity>>(_p->entities);
	iter->setSelector(match);

	return IteratorPtr<std::shared_ptr<Entity>>(iter);
}

Iterable::IteratorPtr<std::shared_ptr<Entity>> Entity::entityIterator(tid typeId)
{
	return entityIterator([typeId](std::shared_ptr<Entity> ent)->bool {
		return (ent->typeId() == typeId);
	});
}

std::shared_ptr<EntityCollection> Entity::entityCollection()
{
	std::shared_ptr<EntityCollection> result;
	
	for (auto iter = _p->entities.begin(); iter != _p->entities.end(); ++iter) {
		result->append(*iter);
	}

	return result;
}

std::shared_ptr<EntityCollection> Entity::entityCollection(tid typeId)
{
	std::shared_ptr<EntityCollection> result;

	for (auto iter = _p->entities.begin(); iter != _p->entities.end(); ++iter) {
		auto ent = *iter;
		if (ent->typeId() == typeId)
			result->append(*iter);
	}

	return result;
}

void Entity::step()
{
	//ListIterator<std::shared_ptr<Entity>> *entIter = new ListIterator<std::shared_ptr<Entity>>(_p->executors);
	//while (!entIter->finished()) {
	//	IteratorPtr<std::shared_ptr<Entity>> exeIter = entIter->value()->facets(TYPEID(Executable));
	//	while (!exeIter->finished()) { // по граням
	//		std::shared_ptr<Executable> exe = static_pointer_cast<Executable>(exeIter->value());
	//		if (exe)
	//			exe->step();
	//		exeIter->next();
	//	}
	//	entIter->next();
	//}
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

EntityCollection::EntityCollection() :
	_p(make_unique<Private>())
{
}

void EntityCollection::append(std::shared_ptr<Entity>& item)
{
	_p->items.push_back(item);
}

std::shared_ptr<Entity> EntityCollection::at(int64_t index)
{
	if (index < 0 || index >= _p->items.size())
		return std::make_shared<Entity>(); // just return empty item

	return _p->items[index];
}

std::shared_ptr<Entity> EntityCollection::operator[](int64_t index)
{
	return at(index);
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
		boost::to_lower(*it);
	}

	// exit application
	if (lst.at(0) == "quit" && lst.size() == 1) {
		_p->shouldStop = true;
		return;
	}

	// display help message
	if (lst.at(0) == "help" || lst.at(0) == "sos" || lst.at(0) == "wtf") {
		usage();
	}

	// load modules
	if (lst.at(0) == "load") {
		string path = "."; // by default, load from current directory
		if (lst.size() > 1)
			path = lst.at(1);
		loadModules(path);
		return;
	}

	// list registered entities
	if (lst.at(0) == "list") {
		int i = 0;
		for (auto it = _p->factories.begin(); it != _p->factories.end(); ++it) {
			auto fact = it->second;
			cout << i + 1 << ": " << fact->typeName() << " {" << fact->typeId() << "} " << endl;
			++i;
		}
		return;
	}

	// list executable entities
	if (lst.at(0) == "listexec") {
		vector<shared_ptr<Entity>> executables;
		{
			auto exePtr = entityIterator(Basis::check_executable);
			cout << "Executable entities:" << std::endl;
			int i = 0;
			while (!exePtr->finished()) {
				auto exe = exePtr->value();
				cout << i + 1 << ": " << exe->typeName() << " {" << exe->id() << "} " << exe->name() << endl;
				++i;
				exePtr->next();
			}
		}
		return;
	}

	// create an instance of specified type
	if (lst.at(0) == "create") {
		if (lst.size() < 2)
			return;

		for (int i = 1; i < lst.size(); ++i) {
			string token = lst.at(i);

			for (auto it = _p->factories.begin(); it != _p->factories.end(); ++it) {
				auto fact = it->second;
				bool shouldBeAdded = false;

				string typeName = boost::to_lower_copy(fact->typeName());
				if (boost::starts_with(typeName, token)) { // by type name
					shouldBeAdded = true;
				}
				else {
					// by type id
					string str = std::to_string(fact->typeId());
					if (boost::starts_with(str, token)) {
						shouldBeAdded = true;
					}
				}

				if (shouldBeAdded) {
					auto newEnt = newEntity(fact->typeId());
					if (newEnt)
						cout << "New entity created: " << newEnt->typeName() << " {" << newEnt->id() << "} " << endl;
				}
			}
		}

		return;
	}

	// add executor
	if (lst.at(0) == "addexec") {
		if (lst.size() < 2)
			return;

		for (int i = 1; i < lst.size(); ++i) {
			string token = lst.at(i);

			auto entPtr = entityIterator(Basis::check_executable);
			while (!entPtr->finished()) {
				auto ent = entPtr->value();

				bool shouldBeAdded = false;

				if (ent->name() == token) { // by name 
					shouldBeAdded = true;
				}
				else {
					// by UUID
					std::string str = boost::uuids::to_string(ent->id());
					if (boost::starts_with(str, token)) {
						shouldBeAdded = true;
					}
				}

				if (shouldBeAdded) {
					auto exe = ent->as<Basis::Executable>();
					if (!exe->isNull())
						exe->setActive();
				}

				entPtr->next();
			}
		}

		return;
	}
}

void System::usage() const
{
	cout << "  quit      -   exit program"             << endl;
	cout << "  load      -   load module(s)"           << endl;
	cout << "  listexec  -   list executable entities" << endl;
	cout << "  addexec   -   add 'executor' entity"    << endl;
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
	auto entIter = entityIterator();
	while (!entIter->finished()) {
		auto ent = entIter->value();
		auto exe = ent->as<Executable>();
		if (exe) {
			if (exe->isActive()) {
				exe->step();
			}
		}

		entIter->next();
	}
}

bool System::shouldStop() const
{
	return _p->shouldStop;
}
