#include "observer.h"
#include <iostream>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

static Basis::System* sys = nullptr;

struct Observer::Private
{
	GLFWwindow* mainWnd = nullptr;
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
	std::cout << "Observer::step()" << endl;

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

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(_p->mainWnd);
	glfwPollEvents();
}

void setup(Basis::System* s)
{
	std::cout << "Observer::setup()" << endl;

	sys = s;
	sys->registerEntity<Observer>();
	sys->container()->newEntity<Observer>();
}

