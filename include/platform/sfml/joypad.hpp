#pragma once

#include "../../joypad.hpp"
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>

class JoypadSFML : public Joypad {
public:
	void keyUpdate()
	{
		right = sf::Keyboard::isKeyPressed(sf::Keyboard::D) || (sf::Joystick::getAxisPosition(0, (sf::Joystick::Axis)6) > 60);
		left = sf::Keyboard::isKeyPressed(sf::Keyboard::A) || (sf::Joystick::getAxisPosition(0, (sf::Joystick::Axis)6) < -60);
		up = sf::Keyboard::isKeyPressed(sf::Keyboard::W) || (sf::Joystick::getAxisPosition(0, (sf::Joystick::Axis)7) < -60);
		down = sf::Keyboard::isKeyPressed(sf::Keyboard::S) || (sf::Joystick::getAxisPosition(0, (sf::Joystick::Axis)7) > 60);
		a = sf::Keyboard::isKeyPressed(sf::Keyboard::K) || sf::Joystick::isButtonPressed(0, 1);
		b = sf::Keyboard::isKeyPressed(sf::Keyboard::L) || sf::Joystick::isButtonPressed(0, 0);
		rightBump = sf::Keyboard::isKeyPressed(sf::Keyboard::I) || sf::Joystick::isButtonPressed(0, 5);
		leftBump = sf::Keyboard::isKeyPressed(sf::Keyboard::O) || sf::Joystick::isButtonPressed(0, 4);
		select = sf::Keyboard::isKeyPressed(sf::Keyboard::Num2) || sf::Joystick::isButtonPressed(0, 6);
		start = sf::Keyboard::isKeyPressed(sf::Keyboard::Num1) || sf::Joystick::isButtonPressed(0, 7);

		bp = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
		esc = sf::Keyboard::isKeyPressed(sf::Keyboard::Escape);
	}
};
