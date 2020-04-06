#include "observer.h"
#include <iostream>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <boost/uuid/uuid_io.hpp>

using namespace std;
using namespace Basis;
using namespace Iterable;

static Basis::System* sys = nullptr;

struct Observer::Private
{
	ImVec4 clearColor = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
	ImVec4 buttonOnColor = ImVec4(78 / 256.0, 201 / 256.0, 176 / 256.0, 1.00f);
	ImVec4 buttonOffColor = ImVec4(10 / 256.0, 10 / 256.0, 10 / 256.0, 1.00f);

	GLFWwindow* mainWnd = nullptr;
	ViewingMode viewingMode = ViewingMode::List;
};

Observer::Observer(Basis::System* sys) :
	Basis::Entity(sys),
	_p(std::make_unique<Private>())
{
	auto exe = addFacet<Basis::Executable>();
	if (exe)
		exe->setStepFunction(std::bind(&Observer::step, this));
}

Observer::~Observer()
{
}

bool Observer::init()
{
	return true;
}

void Observer::cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	if (_p->mainWnd)
		glfwDestroyWindow(_p->mainWnd);
	glfwTerminate();
}

void Observer::step()
{
	if (!_p->mainWnd) {
		if (glfwInit()) {
			_p->mainWnd = glfwCreateWindow(1024, 768, "Observer", nullptr, nullptr);
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

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// здесь вся отрисовка...
	showMainToolbar();
	if (_p->viewingMode == ViewingMode::List)
		showListView();

	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(_p->mainWnd, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(_p->clearColor.x, _p->clearColor.y, _p->clearColor.z, _p->clearColor.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(_p->mainWnd);
	glfwPollEvents();
}

void Observer::showMainToolbar()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
	//if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
	//if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
	//if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
	//if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
	//if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
	//if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
	//if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
	//if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
	//if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	//if (no_close)           p_open = NULL; // Don't pass our bool* to Begin

	//ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
	//ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver); 
	ImGui::SetNextWindowPos(ImVec2());
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

	if (!ImGui::Begin("Magic Eye", 0, window_flags)) {
		ImGui::End();
		return;
	}

	ImVec4 color;

	{ // List button
		color = _p->buttonOffColor;
		if (_p->viewingMode == ViewingMode::List)
			color = _p->buttonOnColor;
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		if (ImGui::Button("List")) {
			_p->viewingMode = ViewingMode::List;
		}
		ImGui::PopStyleColor();
	}
	ImGui::SameLine();

	{ // Graph button
		color = _p->buttonOffColor;
		if (_p->viewingMode == ViewingMode::Graph)
			color = _p->buttonOnColor;
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		if (ImGui::Button("Graph")) {
			_p->viewingMode = ViewingMode::Graph;
		}
		ImGui::PopStyleColor();
	}
	ImGui::SameLine();

	ImGui::End();
	ImGui::PopStyleColor();
}

void showEntity(Entity* ent)
{
	stringstream ss;

	// make id for node:
	ss << ent->id();
	string nodeId = ss.str();

	// make name for node:
	string nodeName = ent->name();
	if (nodeName.empty())
		nodeName = "noname";

	if (ImGui::TreeNode(nodeId.c_str(), nodeName.c_str())) {
		IteratorPtr<std::shared_ptr<Entity>> entPtr = ent->facets(TYPEID(Container));
		while (!entPtr->finished()) {
			shared_ptr<Entity> ent = entPtr->value();
			showEntity(ent.get());
			entPtr->next();
		}

		ImGui::TreePop();
	}
}

void Observer::showListView()
{
	ImGuiWindowFlags windowFlags = 0; // ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;

	//ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
	//ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver); 

	if (!ImGui::Begin("Entities List", 0, windowFlags)) {
		ImGui::End();
		return;
	}

	//list<shared_ptr<Entity>> entities = sys->container()->entities();
	IteratorPtr<std::shared_ptr<Entity>> entPtr = sys->container()->entities();
	while (!entPtr->finished()) {
		shared_ptr<Entity> ent = entPtr->value();
		showEntity(ent.get());
		entPtr->next();
	}

	ImGui::End();
}

void Observer::showGraphView()
{

}

void setup(Basis::System* s)
{
	std::cout << "Observer::setup()" << endl;

	sys = s;
	sys->registerEntity<Observer>();
	auto ent = sys->container()->newEntity<Observer>();
	ent->setName("MagicEye");
}

