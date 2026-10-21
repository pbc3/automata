#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>
#include <renderer.h>
#include <vector>

#include <thread>

int main() {


	if (!glfwInit()) {
		return -1;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* window = glfwCreateWindow(640, 640, "automata engine", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	HWND hwnd = glfwGetWin32Window(window);
	
	// init d3d12
	Renderer renderer(hwnd,640,640);
	int tick = 0;
	//while (!glfwWindowShouldClose(window)) {
	//	glfwPollEvents();

	//	// handle resize
	//	// 
	//	// 
	//	// update();
	//	// render();
	//	HRESULT hr = renderer.mSwapChain->Present(1, 0);
	//	ThrowIfFailed(hr);
	//}
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		renderer.render();

		std::this_thread::sleep_for(
			std::chrono::milliseconds(5)
		);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}