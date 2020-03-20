#include "basis.h"
#include "basis_private.h"
#include <boost/filesystem.hpp>
#include <boost/dll.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/format.hpp>
#include <iostream>

using namespace Basis;
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

Entity::Entity(System* sys) : _p(make_unique<Private>())
{
	_p->system_ptr = sys;
}

Entity::~Entity()
{
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
	if (_p->prototype)
		return _p->prototype->typeName();
	else
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

shared_ptr<Entity> Entity::addFacet(tid protoTypeId)
{
	// возвращаем первую корневую сущность типа protoTypeId (система гарантирует, что она единственна)
	shared_ptr<Entity> prototype = system()->container()->findEntity([protoTypeId](Entity* ent) -> bool {
		return ((ent->typeId() == protoTypeId) && !ent->hasPrototype());
	});

	// если прототип не существует, пытаемся создать его
	if (!prototype)
		prototype = system()->container()->newEntity(protoTypeId);
	if (!prototype)
		return nullptr; // не удалось создать прототип

	// здесь мы не проверяем, что такая грань еще не существует,
	// поскольку допустимо иметь несколько граней одного типа (например,
	// человек может быть сотрудником сразу двух компаний)
	auto newFct = system()->container()->newEntity(prototype.get());
	if (!newFct)
		return nullptr;

	_p->facets.push_back(newFct);
	return newFct;
}

shared_ptr<Entity> Entity::addFacet(Entity* prototype)
{
	// здесь мы не проверяем, что такая грань еще не существует,
	// поскольку допустимо иметь несколько граней одного типа (например,
	// человек может быть сотрудником сразу двух компаний)
	auto newFct = system()->container()->newEntity(prototype);
	if (!newFct)
		return nullptr;

	_p->facets.push_back(newFct);
	return newFct;
}

bool Entity::hasFacet(tid typeId)
{
	// здесь учитываются только грани 1-го уровня, т.е. относящиеся непосредственно 
	// к самой сущности, без прохода вглубь по вложенным граням!
	for (auto fct : _p->facets) {
		if (fct->typeId() == typeId)
			return true;
	}

	return false;
}

IteratorPtr<Entity> Entity::facets()
{
	ListIterator<Entity> *iter = new ListIterator<Entity>(_p->facets);

	return IteratorPtr<Entity>(iter);
}

IteratorPtr<Entity> Entity::facets(tid typeId)
{
	// здесь учитываются только грани 1-го уровня, т.е. относящиеся непосредственно 
	// к самой сущности, без прохода вглубь по вложенным граням!
	ListIterator<Entity> *iter = new ListIterator<Entity>(_p->facets);
	iter->setSelector([typeId](Entity* ent) -> bool {
		return (ent->typeId() == typeId);
	});

	return IteratorPtr<Entity>(iter);
}

bool Entity::hasPrototype() const
{
	return (_p->prototype != nullptr);
}

bool Entity::isKindOf(tid typeId) const
{
	if (_p->prototype)
		return _p->prototype->isKindOf(typeId); // если это образ, переходим к прототипу

	//Далее всё относится только к протипам.

	// сначала проверяем тип непосредственно самой сущности:
	if (_p->typeId == typeId)
		return true;

	// затем проверяем рекурсивно грани:
	for (auto fct : _p->facets) {
		if (fct->isKindOf(typeId))
			return true;
	}

	return false;
}

System* Entity::system() const
{
	return _p->system_ptr;
}

void Entity::print()
{
	cout << "-> entity" << endl;
	cout << "id: " << _p->id << endl;
	if (_p->prototype) {
		cout << "-> prototype:" << endl;
		_p->prototype->print();
		cout << "<- prototype:" << endl;
	}
	cout << "-> facets" << endl;
	for (auto fac : _p->facets) {
		cout << "-> facet" << endl;
		fac->print();
		cout << "<- facet" << endl;
	}
	cout << "<- facets" << endl;
	cout << "<- entity" << endl;
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

void Executable::print()
{
	Entity::print();

	cout << "[Executable]" << endl;
}

Container::Container(System* sys) :
	Entity(sys),
	_p(make_unique<Private>())
{
}

Container::~Container()
{
}

shared_ptr<Entity> Container::newEntity(tid typeId)
{
	shared_ptr<Entity> ent = findEntity([typeId](Entity* ent) -> bool {
		return ((ent->typeId() == typeId) && !ent->hasPrototype());
	});
	if (ent)
		return ent;

	ent = system()->createEntity(typeId);
	ent->_p->typeName = system()->typeIdToTypeName(typeId);
	if (ent) {
		auto iter = _p->entities.insert(_p->entities.end(), ent);
		// добавляем элемент в uuid-индекс
		_p->uuid_index.insert(std::make_pair(ent->id(), iter));
	}

	ent->init();

	return ent;
}

std::shared_ptr<Entity> Container::newEntity(Entity* prototype)
{
	if (!prototype)
		return nullptr;

	shared_ptr<Entity> ent = system()->createEntity(prototype->typeId());
	if (!ent)
		return nullptr;

	ent->_p->prototype = prototype;
	auto iter = _p->entities.insert(_p->entities.end(), ent);
	// добавляем элемент в uuid-индекс
	_p->uuid_index.insert(std::make_pair(ent->id(), iter));

	ent->init();

	return ent;
}

std::shared_ptr<Entity> Container::findEntity(std::function<bool(Entity*)> match)
{
	for (auto i = _p->entities.begin(); i != _p->entities.end(); ++i) {
		if (match(i->get()))
			return *i;
	}

	return nullptr;
}

std::vector<std::shared_ptr<Entity>> Container::findEntities(std::function<bool(Entity*)> match)
{
	vector<shared_ptr<Entity>> result;
	for (auto i = _p->entities.begin(); i != _p->entities.end(); ++i) {
		if (match(i->get()))
			result.push_back(*i);
	}

	return result;
}

bool Container::addExecutor(const uid& id)
{
	auto ent = findEntity([id](Entity* ent)->bool {
		return (ent->id() == id);
	});
	if (!ent)
		return false;

	for (auto e : _p->executors) {
		if (e->id() == id) {
			cout << "Entity " << id << " is already in executors list" << endl;
			return false; // такая сущность уже есть в списке
		}
	}

	_p->executors.push_back(ent);
	return true;
}

void Container::step()
{
	for (auto e : _p->executors) {
		IteratorPtr<Entity> ent = e->facets(TYPEID(Executable));
		while (!ent->isDone()) {
			Executable* exe = static_cast<Executable*>(ent->value());
			exe->step();
			ent->next();
		}
	}
}

void Container::print()
{
	Entity::print();

	cout << "[Container]" << endl;
	cout << "-> contents" << endl;

	for (auto ent : _p->entities) {
		ent->print();
	}

	cout << "<- contents" << endl;
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

System::System() : _p(make_unique<Private>())
{
	// регистрация системных сущностей
	registerEntity<Entity>();
	registerEntity<Executable>();
	registerEntity<Container>();
	registerEntity<Spatial>();

	_p->container = createEntity<Container>();
}

System::~System()
{
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

std::shared_ptr<Container> System::container()
{
	return _p->container;
}

bool System::addFactory(FactoryInterface* f)
{
	_p->factories[f->typeId()] = std::shared_ptr<FactoryInterface>(f);

	return true;
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
	//cout << "System::step()" << endl;

	// если для корневого контейнера определена исполняемая сущность,
	// выполняем её:
	//auto exe = _p->container->executor();
	//if (exe)
	//	exe->step();
	_p->container->step();
}

bool System::shouldStop() const
{
	return _p->shouldStop;
}
