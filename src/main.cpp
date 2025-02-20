#include "core/Application.h"
#include "GameLayer.h"
using namespace ORNG;

void main() {
	Application app;
	ApplicationData init_data{};
	init_data.initial_window_dimensions = { 1200, 1200 };
	init_data.disabled_modules = (ApplicationModulesFlags)(
		ApplicationModulesFlags::AUDIO | ApplicationModulesFlags::PHYSICS);

	app.layer_stack.PushLayer(new ::GameLayer());
	app.Init(init_data);
}