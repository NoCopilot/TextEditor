# TextEditor
My text editor in c++ using sfml library

# build
To build the executable, just start "compile.bat".
Make sure the path to sfml library and include is correct.
I build with static sfml, if you don't want to, just add the .dll files (found in sfml/bin folder) in the same directory of the executable built. Then remove: "-DSFML_STATIC", the path to /sfml/lib and "-lopengl32 -lwinmm -lgdi32 -lfreetype".

# customize
This editor can be customized: edit the config/.config file as a json file.
 - In future also keyboard shortcuts will be customizable.

# help
In the help.txt you can find the keyboard shortcuts to use the editor and what you can modify in .cofing file to be valid.
