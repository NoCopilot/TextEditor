#include "TextBox.hpp"
#include "Cmd.hpp"
#include <fstream>
#include <sstream>


size_t index = 0;
std::vector<TextBox*> textbox;
sf::Font* font;
std::string current_path, exe_path;
const char _2pow[7] = {1, 3, 7, 15, 31, 63, 127};
enum Mode
{
	Explorer,
	Editor
};
Mode mode;
bool show_tabs;
struct FileInfo
{
	std::string line_break, path;
	bool encode;
	FileInfo()
	{
		line_break = "\r\n";
		path = "";
		encode = 1;
	}
	FileInfo(std::string line_break, std::string path, bool encode)
	{
		this->line_break = line_break;
		this->path = path;
		this->encode = encode;
	}
} last_read_info;
std::vector<FileInfo> open_file_info;

void renderingThread(sf::RenderWindow*);
TextBox* initTextBox(sf::RenderWindow* window);
sf::String readFile(std::string);
bool writeFile(std::vector<sf::String>&, FileInfo&);
bool isValidUTF8(std::string&);
std::string utf8Encode(sf::Uint32);
std::string getLineBreak(sf::String&);


int main(int argc, char* argv[])
{
	sf::Vector2u desktop = {sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height};
	
	#define DESKTOP_W   desktop.x
	#define DESKTOP_Y   desktop.y
	#define DESKTOP_W75 DESKTOP_W * 0.75f
	#define DESKTOP_Y75 DESKTOP_Y * 0.75f
	#define DESKTOP_W5  DESKTOP_W * 0.5f
	#define DESKTOP_Y5  DESKTOP_Y * 0.5f
	
    sf::RenderWindow window(
		sf::VideoMode(DESKTOP_W75, DESKTOP_Y75),
		"TextEditor"
	);
    window.setPosition(sf::Vector2i(
        DESKTOP_W5 - window.getSize().x * 0.5f,
        DESKTOP_Y5 - window.getSize().y * 0.5f
    ));
    sf::View view;
    // deactivate window OpenGL context
    window.setFramerateLimit(45);
    window.setActive(false);

    //get current path and exe path
    Cmd cmd;
    exe_path = argv[0];
    #ifdef WIN32
        exe_path = exe_path.substr(0, exe_path.rfind('\\'));
        current_path = cmd.execute("cd");
    #else
        exe_path = exe_path.substr(0, exe_path.rfind('/'));
        current_path = cmd.execute("pwd");
    #endif
	cmd.close();
	
	font = new sf::Font();
	if(!font->loadFromFile(exe_path + "/config/font.otf"))
		font->loadFromFile("config/font.otf");
    
	for(int i = 1; i < argc; i++)
	{
		textbox.push_back(initTextBox(&window));
		sf::String temp = readFile(argv[i]);
		textbox[i-1]->write(temp, last_read_info.line_break);
		open_file_info.push_back(last_read_info);
	}
	if(!textbox.size())
	{
		textbox.push_back(initTextBox(&window));
		open_file_info.push_back(FileInfo());
	}
	mode = Mode::Editor;
	show_tabs = false;
	
    // launch the rendering thread
    sf::Thread thread(renderingThread, &window);
    thread.launch();

    bool move_window = false;
    sf::Vector2i mouse_window_diff;

    sf::Event e;
    while(window.isOpen())
    {
        while(window.waitEvent(e))
        {
			if(e.type == sf::Event::Resized)
            {
                view.reset(sf::FloatRect{sf::Vector2f(0.f, 0.f), (sf::Vector2f)window.getSize()});
                window.setView(view);
                for(TextBox* t : textbox)
                {
                    t->resize((sf::Vector2f)window.getSize());
                }
            }
            if(e.type == sf::Event::KeyPressed)
            {
                if(e.key.control)
                {
                    if(e.key.code == sf::Keyboard::Tab)
					{
						if(mode == Mode::Editor) show_tabs = true;
					}					
                    if(e.key.code == sf::Keyboard::Q)
                    {
                        for(size_t i = 0; i < textbox.size(); i++)
                        {
                            writeFile(textbox[i]->getLines(), open_file_info[i]);
                        }
						
                        window.close();
                    }
                }
				if(e.key.code == sf::Keyboard::Escape)
				{
					if(show_tabs) show_tabs = false;
				}
                //manage window size and position shortcut
                if(e.key.alt)
                {
                    if(e.key.code == sf::Keyboard::Up)
                    {
                        window.setSize({DESKTOP_W, DESKTOP_Y});
                        window.setPosition({0, 0});
						continue;
                    }
                    if(e.key.code == sf::Keyboard::Down)
                    {
                        window.setSize(sf::Vector2u(DESKTOP_W75, DESKTOP_Y75));
                        window.setPosition(sf::Vector2i(
                            DESKTOP_W5 - window.getSize().x * 0.5f,
                            DESKTOP_Y5 - window.getSize().y * 0.5f
                        ));
						continue;
                    }
                    if(e.key.code == sf::Keyboard::Left)
                    {
                        window.setSize(sf::Vector2u(
							DESKTOP_W5,
							DESKTOP_Y
						));
                        window.setPosition({0, 0});
						continue;
                    }
                    if(e.key.code == sf::Keyboard::Right)
                    {
                        window.setSize(sf::Vector2u(
							DESKTOP_W5,
							DESKTOP_Y
						));
                        window.setPosition({(int)window.getSize().x, 0});
						continue;
                    }
                }
            }
            if(e.type == sf::Event::MouseButtonPressed)
            {
                if(e.mouseButton.button == sf::Mouse::Right)
                {
                    mouse_window_diff = sf::Mouse::getPosition() - window.getPosition();
                    move_window = true;
                }
            }
            if(e.type == sf::Event::MouseButtonReleased)
            {
                move_window = false;
            }
            if(e.type == sf::Event::MouseMoved)
            {
                if(move_window)
                {
                    sf::Vector2i mouse_pos = sf::Mouse::getPosition();
                    window.setPosition(mouse_pos - mouse_window_diff);
                }
            }
			
			textbox[index]->listen(e);
        }
		
		/*
		//clearing
        window.clear(sf::Color::White);

        //drawing
        textbox[index]->draw();

        //displaying
        window.display();
		*/
    }
    return 0;
}

TextBox* initTextBox(sf::RenderWindow* window)
{
    TextBox* t = new TextBox(window);
    t->setFont(*font);
    t->setBackgroundColor({24, 26, 31});
    t->setTextColor({255, 255, 255});
    t->setSelectionColor({0, 118, 214});
    t->setSize((sf::Vector2f)window->getSize());
    return t;
}

void renderingThread(sf::RenderWindow* window)
{
	while(window->isOpen())
    {
		//clearing
        window->clear(sf::Color::White);

        //drawing
		switch(mode)
		{
			case Mode::Editor:
				textbox[index]->draw();				
				break;
			case Mode::Explorer:
				
				break;
		}

		/*
		if(show_tabs)
		{
			
		}
		*/
		
        //displaying
        window->display();
    }
}

bool isValidUTF8(std::string& str)
{
	//not my code
	for(size_t i = 0; i < str.length();)
	{
		unsigned char current = static_cast<unsigned char>(str[i]);
		// Single-byte character (0xxxxxxx)
		if(current <= 0x7F)	i += 1;
		else if(current <= 0xDF && i + 1 < str.length())
		{
			// Two-byte character (110xxxxx 10xxxxxx)
			if((static_cast<unsigned char>(str[i + 1]) & 0xC0) != 0x80) {
				return false; // Invalid UTF-8 sequence
			}
			i += 2;
		}
		else if(current <= 0xEF && i + 2 < str.length())
		{
			// Three-byte character (1110xxxx 10xxxxxx 10xxxxxx)
			if((static_cast<unsigned char>(str[i + 1]) & 0xC0) != 0x80 ||
				(static_cast<unsigned char>(str[i + 2]) & 0xC0) != 0x80) {
				return false; // Invalid UTF-8 sequence
			}
			i += 3;
		}
		else return false; // Invalid UTF-8 character
	}
	return true;
}

sf::String readFile(std::string filepath)
{
	std::ifstream file(filepath, std::ios::binary);
	if(!file.is_open()) return "";
	last_read_info.path = filepath;

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	if(!isValidUTF8(content))
	{
		last_read_info.encode = 0;
		last_read_info.line_break = getLineBreak(*(new sf::String(content)));
		return content;
	}
	
	sf::String str;
	int n = 0, k;
	last_read_info.encode = 1;
	const size_t size = content.length();
	for(size_t i = 0; i < size; i++)
	{
		if(n == 0)
		{
			last_read_info.encode = 1;
			k = 7;
			while(k > 0 && (content[i] >> k-- & 1) == 1)
				n++;
			str += (sf::Uint32)(content[i] & _2pow[6-n]);
			continue;
		}
		n--;
		str[str.getSize()-1] <<= 6;
		str[str.getSize()-1] += content[i] & _2pow[5];
	}
	file.close();
	
	last_read_info.line_break = getLineBreak(str);
	return str;
}
bool writeFile(std::vector<sf::String>& lines, FileInfo& info)
{
	std::ofstream file(info.path, std::ios::binary);
	if(!file.is_open()) return false;
	if(info.encode)
	{
		for(size_t i = 0; i < lines.size(); i++)
		{
			for(sf::Uint32 ch : lines[i])
				file << utf8Encode(ch);
			if((i+1) != lines.size()) file << info.line_break;
		}
	}
	else
	{
		for(size_t i = 0; i < lines.size(); i++)
		{
			for(sf::Uint32 ch : lines[i])
				file << (char)ch;
			if((i+1) != lines.size()) file << info.line_break;
		}
	}
	
	file.close();
	return true;
}

std::string utf8Encode(sf::Uint32 ch)
{
	if(ch < 256) return {(char)ch};
	unsigned short int n = 0;
	short int k = 1;
	std::string	res;
	
	while((ch >> (31 - n) & 1) == 0 && n < 32) n++;
	n = 32 - n;
	while(n >= 6)
	{
		//get last 6 bits
		res.insert(res.begin(), (ch & _2pow[5]) + 128);
		//right shift to prepare next bits
		ch >>= 6;
		n -= 6;
		k++;
	}
	char c = 0;
	int j = 7 - k;
	while(k >= 0)
	{
		c += 1;
		c <<= 1;
		k--;
	}
	c <<= j;
	c += ch & _2pow[4];
	res.insert(res.begin(), c);
	
	return res;
}

std::string getLineBreak(sf::String& str)
{
	int n = str.find('\r');
	if(n == -1)
	{
		n = str.find('\n');
		if(n != -1) return "\n";
		return "\r\n"; //default
	}
	if((size_t)(n+1) < str.getSize() && str[n+1] == '\n') return "\r\n";
	return "\r";
}