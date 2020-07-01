#include "keyboard.hpp"
#include "game.hpp"
#include "util.hpp"

void Keyboard::begin(GLFWwindow *window)
{
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
}

void Keyboard::addTrackedKey(int key)
{
	trackedKeys.emplace(std::make_pair(key, false));
}

bool Keyboard::isPressed(int key) const
{
	return trackedKeys.at(key);
}

int Keyboard::isOnePressed(int key1, int key2) const {
	const auto pressed1 = isPressed(key1);
	const auto pressed2 = isPressed(key2);

	if (pressed1 && !pressed2) {
		return 1;
	}
	if (pressed2 && !pressed1) {
		return -1;
	}
	return 0;
}


double Keyboard::getScrollOffset() const
{
	return scrollOffset;
}

void Keyboard::resetScrollOffset()
{
	scrollOffset = 0;
}

void Keyboard::key_callback(GLFWwindow *window, int key, __attribute__((unused)) int scancode, int action, __attribute__((unused)) int mods)
{
	auto &keyboard = TrainGameApplication::fromWindow(window)->keyboard;
	auto entry = keyboard.trackedKeys.find(key);
	if (entry != keyboard.trackedKeys.end())
	{
		if (action == GLFW_PRESS)
		{
			entry->second = true;
		}
		else if (action == GLFW_RELEASE)
		{
			entry->second = false;
		}
	}
}

void Keyboard::scroll_callback(GLFWwindow *window, __attribute__((unused)) double xoffset, double yoffset)
{
	auto &keyboard = TrainGameApplication::fromWindow(window)->keyboard;
	keyboard.scrollOffset += yoffset;
}