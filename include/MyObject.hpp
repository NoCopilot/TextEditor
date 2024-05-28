#ifndef MYOBJECT_HPP
#define MYOBJECT_HPP

#include "SFML/Graphics.hpp"

class MyObject
{
	public:
		MyObject(sf::RenderWindow* window)
		{
			win = window;
		}
		void setWindow(sf::RenderWindow* window)
		{
			win = window;
		}
		void listen(sf::Event&){}
		void draw(){}
	protected:
		sf::RenderWindow* win;
};

#endif