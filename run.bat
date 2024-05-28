g++ *.cpp -DSFML_STATIC -IC:\SFML_MINGW\include -I./include -LC:\SFML_MINGW\lib -lsfml-graphics-s -lsfml-window-s -lsfml-system-s -lopengl32 -lwinmm -lgdi32 -lfreetype -o debug/main -std=c++20 -Wall -O3 res/my.res 
rem -mwindows
debug\main 