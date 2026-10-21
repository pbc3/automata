//#include <vector>
//#include <cstdint>
//#include <iostream>
//#include <unordered_map>
//#include <map>
#include "CrossWindow/CrossWindow.h"
#include "automata_math.h"


/*
* code style. members will be m_Var
* functions will be snake case
* globals will be CAPS
* local vars will be camel case
* types will be capatalized
*/

using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;
//constexpr int INF = std::numeric_limits<int>::max();

void xmain(int argc, const char** argv) {
	xwin::WindowDesc windowDesc;
	windowDesc.title = "automata engine";
	windowDesc.name = "MainWindow";
	windowDesc.visible = true;
	windowDesc.height = 640; windowDesc.width = 640;
	windowDesc.fullscreen = false;

	xwin::Window window;
	xwin::EventQueue eventQueue;

	if (!window.create(windowDesc, eventQueue)) {
		return;
	}

	//Renderer renderer(window);
	
	bool isRunning = true;
	while (isRunning) {
		bool shouldRender = true;
	
		while (!eventQueue.empty()) {
				const xwin::Event& event = eventQueue.front();
				if (event.type == xwin::EventType::Resize) {
					xwin::ResizeData data = event.data.resize;
					//renderer.resize()
					}
				if (event.type == xwin::EventType::Close) {
					window.close(); shouldRender = false; isRunning = false;
				}
				eventQueue.pop();
				}
		if (shouldRender) {
			//renderer.Render();
		}
		}
}