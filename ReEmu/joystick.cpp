#include "joystick.h"
namespace joystick
{
	u8 getJoystickState()
	{
		return sf::Keyboard::isKeyPressed(sf::Keyboard::L) * 0x01
			+ sf::Keyboard::isKeyPressed(sf::Keyboard::K) * 0x02
			+ sf::Keyboard::isKeyPressed(sf::Keyboard::G) * 0x04
			+ sf::Keyboard::isKeyPressed(sf::Keyboard::H) * 0x08
			+ sf::Keyboard::isKeyPressed(sf::Keyboard::W) * 0x10
			+ sf::Keyboard::isKeyPressed(sf::Keyboard::S) * 0x20
			+ sf::Keyboard::isKeyPressed(sf::Keyboard::A) * 0x40
			+ sf::Keyboard::isKeyPressed(sf::Keyboard::D) * 0x80;
	}
}