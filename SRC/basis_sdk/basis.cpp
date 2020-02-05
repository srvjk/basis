#include "basis.h"
#include "basis_private.h"
#include <boost/filesystem.hpp>
#include <boost/dll.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <iostream>

using namespace Basis;
using namespace std;

namespace fs = boost::filesystem;

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

bool Entity::hasPrototype() const
{
	return (_p->prototype != nullptr);
}

System* Entity::system() const
{
	return _p->system_ptr;
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
	if (ent) {
		auto iter = _p->entities.insert(_p->entities.end(), ent);
		// добавляем элемент в uuid-индекс
		_p->uuid_index.insert(std::make_pair(ent->id(), iter));
	}

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

Executable* Container::executor() const
{
	return _p->executor.get();
}

System::System() : _p(make_unique<Private>())
{
	// регистрация системных сущностей
	registerEntity<Entity>();
	registerEntity<Executable>();
	registerEntity<Container>();

	_p->container = createEntity<Container>();
	_p->executor = createEntity<Executable>();
	if (_p->executor)
		_p->executor->setStepFunction(std::bind(&System::step, this));
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

	cout << "loading '" << path << "'" << endl;

	fs::path p(path);
	if (!fs::exists(p)) {
		cout << "cannot load module: path '" << path << "' not exists" << endl;
		return retval;
	}
	if (!fs::is_regular_file(p)) {
		cout << "cannot load module: path '" << path << "' is not a file" << endl;
		return retval;
	}

	// проверка на DLL:
	try {
		boost::dll::library_info info(path, true);
	}
	catch (boost::exception& e) {
		cout << path << " is not a module" << endl;
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

void System::step()
{
	cout << "System::step()" << endl;

	// если для корневого контейнера определена исполняемая сущность,
	// выполняем её:
	auto exe = _p->container->executor();
	if (exe)
		exe->step();
}

bool System::shouldStop() const
{
	return _p->shouldStop;
}
