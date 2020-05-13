#include "neuroviewer.h"
#include <iostream>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <boost/uuid/uuid_io.hpp>
#include "neuro/neuro.h"
#include <functional>

using namespace std;
using namespace Basis;
using namespace Iterable;

static Basis::System* sys = nullptr;
using Color = point3d;

void drawSphere(GLUquadricObj* quadric, const point3d& p, const Color& color, float size)
{
	float hsize = size / 2.0;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); // сохраняем мировую матрицу, чтобы не повлиять на другие объекты

	glColor3f(color.get<0>(), color.get<1>(), color.get<2>());
	glTranslatef(p.get<0>(), p.get<1>(), p.get<2>());

	gluSphere(quadric, hsize, 16, 16);

	glPopMatrix();
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

void setProjection(GLFWwindow* window)
{
	int w = winData.width;
	int h = winData.height;
	if (h == 0)
		h = 1;
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	GLfloat minZ = -1.0;
	GLfloat maxZ = 1.0;

	glfwMakeContextCurrent(window);
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w >= h)
		gluOrtho2D(minZ * aspect, maxZ * aspect, minZ, maxZ);
	else
		gluOrtho2D(minZ, maxZ, minZ / aspect, maxZ / aspect);
}

struct NeuroViewer::Private
{
	ImVec4 clearColor = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
	ImVec4 buttonOnColor = ImVec4(78 / 256.0, 201 / 256.0, 176 / 256.0, 1.00f);
	ImVec4 buttonOffColor = ImVec4(10 / 256.0, 10 / 256.0, 10 / 256.0, 1.00f);
	GLFWwindow* mainWnd = nullptr;
	ViewingMode viewingMode = ViewingMode::List;
	shared_ptr<NeuroNet> activeNet;

	double minDistance = 10;                /// минимальное расстояние от центра сцены в 3D
	double maxDistance = 1e3;               /// максимальное расстояние от центра сцены в 3D
	GLfloat ambientLight[4] = { 0.6f, 0.6f, 0.6f, 0.6f };
	GLfloat sunLight[4] = { 0.6f, 0.6f, 0.6f, 0.6f };
	GLfloat sunPosition[4] = { 0.0f, 0.0f, 100.0f, 1.0f };
	point3d lookFrom = { -100, -100, 100 };
	point3d lookAt = { 100, 100, 0 };
	point3d up = { 0, 0, 1 };
	GLUquadricObj* quadric = nullptr;

	Private() 
	{
		quadric = gluNewQuadric();
	}
};

NeuroViewer::NeuroViewer(Basis::System* sys) :
	Basis::Entity(sys),
	_p(std::make_unique<Private>())
{
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&NeuroViewer::step, this));
}

NeuroViewer::~NeuroViewer()
{
}

bool NeuroViewer::init()
{
	return true;
}

void NeuroViewer::cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	if (_p->mainWnd)
		glfwDestroyWindow(_p->mainWnd);
	glfwTerminate();
}

void NeuroViewer::showMainToolbar()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::SetNextWindowPos(ImVec2());
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

	if (!ImGui::Begin("Magic Eye", 0, window_flags)) {
		ImGui::End();
		return;
	}

	ImVec4 color;

	// look for entities we know how to deal with
	IteratorPtr<std::shared_ptr<Entity>> entPtr = sys->container()->entities();
	while (!entPtr->finished()) {
		shared_ptr<Entity> ent = entPtr->value();

		if (ent->typeId() == TYPEID(NeuroNet)) {
			auto net = static_pointer_cast<NeuroNet>(ent);

			color = _p->buttonOffColor;
			if (_p->activeNet) {
				if (_p->activeNet.get() == ent.get())
					color = _p->buttonOnColor;
			}

			ImGui::PushStyleColor(ImGuiCol_Button, color);
			if (ImGui::Button(net->name().c_str())) {
				if (_p->activeNet.get() != net.get())
					_p->activeNet = net;
				else
					_p->activeNet = nullptr;
			}
			ImGui::PopStyleColor();
			ImGui::SameLine();
		}

		if (ent->typeId() == TYPEID(Trainer)) {
			auto trainer = static_pointer_cast<Trainer>(ent);

			color = _p->buttonOffColor;
			if (trainer->isActive())
				color = _p->buttonOnColor;

			ImGui::PushStyleColor(ImGuiCol_Button, color);
			if (ImGui::Button(trainer->name().c_str()))
				trainer->setActive(!trainer->isActive());

			ImGui::PopStyleColor();
			ImGui::SameLine();
		}

		entPtr->next();
	}

	ImGui::End();
	ImGui::PopStyleColor();
}

void drawAxes(double spread, bool singleColor)
{
	glDisable(GL_LIGHTING);
	glLineWidth(1);
	if (singleColor)
		glColor3f(1.0, 1.0, 1.0);

	glBegin(GL_LINES);

	// Ось X
	if (!singleColor)
		glColor3f(0.5, 0.0, 0.0); // красный
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(spread, 0.0, 0.0);

	// Ось Y
	if (!singleColor)
		glColor3f(0.0, 0.5, 0.0); // зеленый
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, spread, 0.0);

	// Ось Z
	if (!singleColor)
		glColor3f(0.0, 0.0, 0.5); // синий
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, spread);

	glEnd();
}

void NeuroViewer::drawScene()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, (GLsizei)winData.width, (GLsizei)winData.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)winData.width / (GLfloat)winData.height,
		_p->minDistance, _p->maxDistance);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(
		_p->lookFrom.get<0>(), _p->lookFrom.get<1>(), _p->lookFrom.get<2>(),
		_p->lookAt.get<0>(), _p->lookAt.get<1>(), _p->lookAt.get<2>(),
		_p->up.get<0>(), _p->up.get<1>(), _p->up.get<2>()
	);

	//if (s->showAxes)
		drawAxes(1000, false);

	//if (s->enableLighting) {
		glEnable(GL_LIGHTING);
		glLightfv(GL_LIGHT1, GL_AMBIENT, _p->ambientLight);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, _p->sunLight);
		glLightfv(GL_LIGHT1, GL_POSITION, _p->sunPosition);
		glEnable(GL_LIGHT1);
	//}
	//else {
	//	glDisable(GL_LIGHTING);
	//}

	drawActiveNet();
}

void NeuroViewer::drawActiveNet()
{
	if (!_p->activeNet)
		return;

	shared_ptr<NeuroNet> net = _p->activeNet;

	// NeuroNet is a container filled with neurons:
	auto contPtr = net->facets(TYPEID(Basis::Container));
	if (!contPtr)
		return;

	auto cont = static_pointer_cast<Basis::Container>(contPtr->value());
	if (!cont)
		return;

	int i = 0;
	for (auto entPtr = cont->entities(); entPtr->finished() == false; entPtr->next()) {
		shared_ptr<Entity> ent = entPtr->value();
		if (ent->typeId() != TYPEID(Neuron))
			continue;

		auto neuron = static_pointer_cast<Neuron>(ent);
		auto spatPtr = neuron->facets(TYPEID(Basis::Spatial));
		if (!spatPtr)
			continue;

		shared_ptr<Basis::Spatial> spat = static_pointer_cast<Basis::Spatial>(spatPtr->value());
		if (!spat)
			continue;

		drawSphere(_p->quadric, spat->position(), { 1.0, 1.0, 1.0 }, 10);
		++i;
	}
}

void NeuroViewer::step()
{
	if (!_p->mainWnd) {
		if (glfwInit()) {
			_p->mainWnd = glfwCreateWindow(1024, 768, "NeuroViewer", nullptr, nullptr);
			if (!_p->mainWnd) {
				glfwTerminate();
				return;
			}
		}
		else {
			std::cout << "Error: GLFW initialization failed!" << std::endl;
			return;
		}

		glfwMakeContextCurrent(_p->mainWnd);

		glfwSetWindowSizeCallback(_p->mainWnd, windowSizeCallback);
		glfwSetWindowPosCallback(_p->mainWnd, windowPosCallback);

		glfwSwapInterval(1);
		glewInit();
		ImGui::CreateContext();

		if (!ImGui_ImplGlfw_InitForOpenGL(_p->mainWnd, true)) {
			printf("Error: could not initialize Gui renderer");
			glfwTerminate();
			return;
		}
		if (!ImGui_ImplOpenGL3_Init()) {
			printf("Error: could not initialize Gui renderer (OpenGL 3)");
			ImGui_ImplGlfw_Shutdown();
			glfwTerminate();
			return;
		}

		ImGuiIO& io = ImGui::GetIO();

		if (!_p->mainWnd)
			return; // не удалось создать окно
	}

	setProjection(_p->mainWnd);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// all painting here
	drawScene();
	showMainToolbar();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(_p->mainWnd);
	glfwPollEvents();
}

void setup(Basis::System* s)
{
	std::cout << "NeuroViewer::setup()" << endl;

	sys = s;
	sys->registerEntity<NeuroViewer>();
	auto ent = sys->container()->newPrototype<NeuroViewer>();
	ent->setName("MagicEye");
}

