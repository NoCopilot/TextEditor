#ifndef TEXTBOX_HPP
#define TEXTBOX_HPP

#include "MyObject.hpp"
#include "SFML/Window.hpp"
#include <vector>
#include <iostream>

#define TAB 9

std::vector<sf::String> split(sf::String, sf::String);
struct Interval
{
	size_t x, y, n;
	sf::Color color;
	Interval(size_t X, size_t Y, size_t N, sf::Color c)
	{
		x = X; y = Y; n = N;
		color = c;
	}
};

class TextBox: MyObject
{
	public:
		//class functions
		TextBox(sf::RenderWindow*);
		void draw();
		void listen(sf::Event&);
		void resize(sf::Vector2f);
		
		void write(sf::String, std::string);
		void addInterval(Interval);

		//setters
		void setPosition(sf::Vector2f);
		void setSize(sf::Vector2f);
		void setFont(sf::Font&);
		void setTextColor(sf::Color);
		void setBackgroundColor(sf::Color);
		void setSelectionColor(sf::Color);

		//getters
		std::vector<sf::String>& getLines();
	private:
		//variables
		sf::View text_view, numbers_view, win_view;
		sf::Vector2f text_view_pos, text_view_size, number_view_size;
		sf::Color text_color;
		sf::RectangleShape background_rect, selection_rect;

		std::vector<sf::String> lines;
		size_t tx, ty;
		sf::Vector2<size_t> select_begin, select_end;
		bool text_select;
		sf::Vertex cursor[2];
		std::vector<Interval> intervals; //colored text

		sf::Text text, test_text;
		float line_height, line_width;
		size_t max_width_line;

		//functions
		sf::Vector2i getMousePosInView();
		sf::Vector2<size_t> getPosInText(sf::Vector2i);
		bool executeCommand(sf::Event&);
		void checkViewOutOfBounds(sf::Vector2f);
		void updateWidth();
		std::vector<float> getCharactersPos(sf::String);
		inline float getTextWidth(size_t x, size_t y, size_t n);
};

#endif