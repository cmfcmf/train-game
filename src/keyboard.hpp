#pragma once

#include <map>

#include <GLFW/glfw3.h>

class Keyboard
{
public:
	void begin(GLFWwindow *window);
	void addTrackedKey(int key);
	bool isPressed(int key) const;
	double getScrollOffset() const;
	void resetScrollOffset();

private:
	std::map<int, bool> trackedKeys;
	double scrollOffset;

	static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
};