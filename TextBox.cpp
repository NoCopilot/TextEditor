#include "TextBox.hpp"

std::vector<sf::String> split(sf::String str, sf::String del)
{
	if(del.getSize() == 0)
		return {str};
	
	std::vector<sf::String> res;
	sf::String s = "", temp;
	
	bool found;
	for(std::size_t i = 0; i < str.getSize(); i++)
	{
		found = true;
		temp = "";
		for(sf::Uint32 ch : del)
		{
			temp += str[i];
			if(str[i] != ch)
			{
				found = false;
				break;
			}
			i++;
		}
		if(found)
		{
			res.push_back(s);
			s = "";
			i--;
			continue;
		}
		
		s += temp;
	}
	res.push_back(s);
	return res;
}

TextBox::TextBox(sf::RenderWindow* window): MyObject(window)
{
	//win = window;
	text_view = win->getView();
	text_view_pos = {0.f, 0.f};
	text_view_size = {0.f, 0.f};
	background_rect.setSize(text_view.getSize());
	selection_rect.setSize({0.f, line_height});

	lines.push_back("int main()");
	ty = 0;
	tx = 0;
	text_select = false;
	intervals = {};

	line_height = text.getCharacterSize() + 15;
	line_width = text_view.getSize().x*0.5f;
	max_width_line = 0;

	cursor[0] = sf::Vertex({1, 0});
	cursor[1] = sf::Vertex({1, line_height});
}
void TextBox::draw()
{
	win_view = win->getView();
	win->setView(text_view);

	win->draw(background_rect);
	text.setFillColor(text_color);
	
	size_t n = (text_view.getCenter().x + text_view.getSize().x*0.5f) / line_height + 1;
	size_t i = (text_view.getCenter().y - text_view.getSize().y*0.5f) / line_height;
	size_t ii = 0, ic;
	while(ii < intervals.size())
	{
		if(intervals[ii].y >= i)
			break;
		ii++;
	}
	
	for(; i < lines.size() && i < n; i++)
	{
		text.setPosition({0.f, line_height*i});
		
		if(intervals.size() < 0 || ii >= intervals.size())
		{
			drawNormal:{}
			text.setString(lines[i]);
			win->draw(text);
			continue;
		}
		
		if(intervals[ii].y != i)
			goto drawNormal;
		
		ic = 0;
		while(intervals[ii].y == i)
		{
			text.setString(lines[i].substring(ic, intervals[ii].x - ic));
			win->draw(text);
			text.move({text.getLocalBounds().width + text.getLocalBounds().left, 0.f});
			
			text.setFillColor(intervals[ii].color);
			text.setString(lines[i].substring(intervals[ii].x, intervals[ii].n));
			win->draw(text);
			text.move({text.getLocalBounds().width + text.getLocalBounds().left, 0.f});
			
			ic += intervals[ii].n + (intervals[ii].x - ic);
			text.setFillColor(text_color);
			ii++;
			/*
			text.setFillColor(text_color);
			text.setString(lines[i].substring(intervals[0].x + intervals[0].n));
			win->draw(text);
			*/
		}
		
		text.setString(lines[i].substring(ic));
		win->draw(text);
		
	}
	win->draw(cursor, 2, sf::Lines);

	win->setView(win_view);
}
void TextBox::listen(sf::Event& e)
{
	if(e.type == sf::Event::MouseWheelScrolled)
	{
		//change text font size
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
		{
			short int delta = e.mouseWheelScroll.delta;
			if(delta > 0 && text.getCharacterSize() > 80) return;
			if(delta < 0 && text.getCharacterSize() < 10) return;
			
			float temp = (text_view.getCenter().y - text_view.getSize().y * 0.5f) / line_height;
			float temp1 = line_width;

			text.setCharacterSize(text.getCharacterSize() + delta);
			test_text.setCharacterSize(text.getCharacterSize());
			line_height += delta;
			updateWidth();
			temp1 = line_width - temp1;
			
			//why 0.33 times the difference between the max width before and after the text size update? because it seems fine!
			text_view.setCenter({text_view.getCenter().x + temp1 * 0.33f, temp * line_height + text_view.getSize().y * 0.5f});
			background_rect.setPosition(text_view.getCenter() - text_view.getSize() * 0.5f);
			
			goto ajustCursor;
		}
		//check if view can be moved or the size is too small
		if((lines.size() * line_height) < text_view.getSize().y) return;

		//scroll view up or down
		text_view.move({0.f, - line_height * e.mouseWheelScroll.delta});

		//check if too up
		if((text_view.getCenter().y - text_view.getSize().y*0.5f) < 0.f)
			text_view.setCenter({text_view.getCenter().x, text_view.getSize().y*0.5f});
		//check if too down
		if((lines.size()*line_height) < (text_view.getCenter().y + text_view.getSize().y*0.5f))
			text_view.setCenter({text_view.getCenter().x, lines.size()*line_height - text_view.getSize().y*0.5f});			
		//adjust background
		background_rect.setPosition(text_view.getCenter() - text_view.getSize() * 0.5f);

		return;
	}
	
	if(e.type == sf::Event::MouseButtonPressed)
	{
		
	}
	
	if(e.type == sf::Event::KeyPressed)
	{
		executeCommand(e);
		goto listenEnd;
	}
	if(e.type == sf::Event::TextEntered)
	{
		if(e.text.unicode < 32 && e.text.unicode != TAB) return;
		lines[ty].insert(tx++, e.text.unicode);
		goto listenEnd;
	}

	return;

	listenEnd:
	{
		checkViewOutOfBounds({0, ty * line_height});	
	}
	
	ajustCursor:
	{
		cursor[0].position.x = getTextWidth(0, ty, tx) + 1.f;
		cursor[0].position.y = ty*line_height;
		cursor[1].position.x = cursor[0].position.x;
		cursor[1].position.y = (ty+1) * line_height;
	}
}

void TextBox::write(sf::String str, std::string line_break)
{
	std::vector<sf::String> v = split(str, line_break);
	if(!v.size()) return;
	
	if(v.size() == 1)
	{
		lines[ty] = lines[ty].substring(0, tx) + v[0] + lines[ty].substring(tx);
		return;
	}
	sf::String temp = lines[ty].substring(tx);

	lines[ty] = lines[ty].substring(0, tx) + v[0];
	for(size_t i = 1; i < v.size(); i++)
		lines.insert(lines.begin() + ty + i, v[i]);
	lines[ty + v.size() - 1] += temp;
}

void TextBox::addInterval(Interval interval)
{
	intervals.push_back(interval);
}


std::vector<sf::String>& TextBox::getLines()
{
	return lines;
}

bool TextBox::executeCommand(sf::Event& e)
{
	//new line
	if(e.key.code == sf::Keyboard::Return)
	{
		//bring the line from cursor to end in a new line down
		lines.insert(lines.begin() + ty + 1, lines[ty].substring(tx));
		//then cut the line
		lines[ty] = lines[ty].substring(0, tx);
		//in the end, add the indendation used on upper line
		tx = 0;
		for(size_t i = 0; i < lines[ty].getSize(); i++)
		{
			if(lines[ty][i] != '\t' && lines[ty][i] != ' ') break;
			else
			{
				lines[ty + 1].insert(i, lines[ty][i]);
				tx++;
			}
		}
		ty++;
		return true;
	}
	//move left
	if(e.key.code == sf::Keyboard::Left)
	{
		//check if can go left
		if(tx > 0)
		{
			if(e.key.control)
			{
				
				return true;
			}
			tx--;
		}
		//else go up
		else if(ty > 0)
		{
			ty--;
			tx = lines[ty].getSize();
		}
		return true;
	}
	//move right
	if(e.key.code == sf::Keyboard::Right)
	{
		//check if can go right
		if(tx < lines[ty].getSize())
		{
			if(e.key.control)
			{
				
				return true;
			}
			tx++;
		}
		//else go down
		else if(ty < (lines.size() - 1))
		{
			ty++;
			tx = 0;
		}
		return true;
	}
	//move up
	if(e.key.code == sf::Keyboard::Up)
	{
		if(ty > 0) ty--;
		if(tx > lines[ty].getSize()) tx = lines[ty].getSize();
		return true;
	}
	//move down
	if(e.key.code == sf::Keyboard::Down)
	{
		if((ty+1) < lines.size()) ty++;
		if(tx > lines[ty].getSize()) tx = lines[ty].getSize();
		return true;
	}
	//canc
	if(e.key.code == sf::Keyboard::Delete)
	{
		//checking if there are character to canc
		if(tx < lines[ty].getSize())
		{
			if(e.key.control)
			{
				
				return true;
			}
			lines[ty].erase(tx);
		}
		//else bring the under line up
		else if((ty+1) < lines.size())
		{
			lines[ty] += lines[ty+1];
			lines.erase(lines.begin()+ty+1);
		}
		return true;
	}
	//delete
	if(e.key.code == sf::Keyboard::Backspace)
	{
		//check if there are character to delete
		if(tx > 0)
		{
			if(e.key.control)
			{
				
				return true;
			}
			lines[ty].erase(--tx);
		}
		//else bring this line upper
		else if(ty > 0)
		{
			tx = lines[ty-1].getSize();
			lines[ty-1] += lines[ty];
			lines.erase(lines.begin()+ty--);
		}
		return true;
	}
	
	if(e.key.control)
	{
		//copy
		if(e.key.code == sf::Keyboard::C)
		{
			
			return true;
		}
		
		//paste
		if(e.key.code == sf::Keyboard::V)
		{
			
			return true;
		}
		
		//cut
		if(e.key.code == sf::Keyboard::X)
		{
			
			return true;
		}
	}

	return false;
}

sf::Vector2i TextBox::getMousePosInView()
{
	sf::Vector2i mouse_pos = sf::Mouse::getPosition(*win);
	sf::Vector2f view_origin(
		text_view.getCenter().x - text_view.getSize().x / 2, 
		text_view.getCenter().y - text_view.getSize().y / 2
	);

	mouse_pos.x = (int)((mouse_pos.x - text_view_pos.x) * (text_view.getSize().x / text_view_size.x) + view_origin.x);
	mouse_pos.y = (int)((mouse_pos.y - text_view_pos.y) * (text_view.getSize().y / text_view_size.y) + view_origin.y);

	return mouse_pos;
}

sf::Vector2<size_t> TextBox::getPosInText(sf::Vector2i mouse)
{
	//get y pos in v text
	mouse.y /= line_height;
	if((size_t)mouse.y >= lines.size()) mouse.y = lines.size() - 1;
	//get x pos in v text
	if(getTextWidth(0, mouse.y, lines[mouse.y].getSize()) <= mouse.x) return {lines[mouse.y].getSize(), (size_t)mouse.y};
	else for(size_t i = 1; i <= lines[mouse.y].getSize(); i++)
	{
		if(getTextWidth(0, mouse.y, i) <= mouse.x) continue;
		return {i - 1, (size_t)mouse.y};
	}
	return {0, 0};
}

void TextBox::checkViewOutOfBounds(sf::Vector2f p)
{
	//check text width
	if((lines.size() * line_height) < text_view.getSize().y)
	{
		text_view.setCenter({text_view.getCenter().x, text_view.getSize().y * 0.5f});
		goto checkHeight;
	}
	
	if(p.x < (text_view.getCenter().x - text_view.getSize().x * 0.5f))
	{
		//too left
	}
	if(p.x > (text_view.getCenter().x + text_view.getSize().x * 0.5f))
	{
		//too right
	}

	checkHeight:{}
	//check text height
	if(line_width < text_view.getSize().x)
		text_view.setCenter({text_view.getSize().x*0.5f, text_view.getCenter().y});

	if(p.y < (text_view.getCenter().y - text_view.getSize().y * 0.5f))
	{
		//too up

	}
	if((p.y + line_height) > (text_view.getCenter().y + text_view.getSize().y * 0.5f))
	{
		//too down
		text_view.move({0.f, (p.y + line_height) - (text_view.getCenter().y + text_view.getSize().y * 0.5f)});
	}
	background_rect.setPosition(text_view.getCenter() - text_view.getSize() * 0.5f);
}

void TextBox::setPosition(sf::Vector2f pos)
{
	text_view_pos = pos;
	
	text_view.setViewport(sf::FloatRect{
		pos.x / win->getSize().x,
		pos.y / win->getSize().y,
		text_view.getViewport().getSize().x,
		text_view.getViewport().getSize().y
	});
}
void TextBox::setSize(sf::Vector2f size)
{
	text_view_size = size;
	sf::Vector2f temp = {(text_view.getCenter().x - text_view.getSize().x*0.5f) / line_width,
		(text_view.getCenter().y - text_view.getSize().y*0.5f) / (lines.size() * line_height)
	};

	text_view.setViewport(sf::FloatRect{
		text_view.getViewport().getPosition().x,
		text_view.getViewport().getPosition().y,
		size.x / win->getSize().x,
		size.y / win->getSize().y
	});
	
	text_view.setSize(size);
	text_view.setCenter(temp.x*line_width + text_view.getSize().x*0.5f, 
		temp.y*(line_height*lines.size()) + text_view.getSize().y*0.5f);

	background_rect.setSize(size);
	background_rect.setPosition(text_view.getCenter() - text_view.getSize()*0.5f);
}
void TextBox::resize(sf::Vector2f size)
{
	setSize(size);
	//text_view.setSize(size);
}

void TextBox::updateWidth()
{

}

float TextBox::getTextWidth(size_t x, size_t y, size_t n)
{
	float tot = 0.f;
	if(y >= lines.size()) return tot;
	if(x >= lines.size()) return tot;
	test_text.setString(lines[y]);
	return test_text.findCharacterPos(n).x - test_text.findCharacterPos(x).x;
}

std::vector<float> TextBox::getCharactersPos(sf::String str)
{
	test_text.setString(str);
	
	std::vector<float> res;
	size_t size = str.getSize();
	
	res.reserve(size);
	for(size_t i = 0; i < size; i++)
		res.push_back(test_text.findCharacterPos(i).x);
	
	return res;
}

//setters
void TextBox::setFont(sf::Font& font)
{
	text.setFont(font);
	test_text.setFont(font);
}
void TextBox::setBackgroundColor(sf::Color color)
{
	background_rect.setFillColor(color);
}
void TextBox::setTextColor(sf::Color color)
{
	text_color = color;
}
void TextBox::setSelectionColor(sf::Color color)
{
	selection_rect.setFillColor(color);
}