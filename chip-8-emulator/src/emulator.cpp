#include <iostream>
#include <SFML/Graphics.hpp>
#include "chip8.h"

int main()
{
    LoadFile("C:\\Users\\thoma\\Documents\\CPP\\chip-8-emulator\\roms\\IBMLogo.ch8");
    Init();

    for (int i = 0; i < 10; i++)
        Update();

    sf::RenderWindow window(sf::VideoMode(720, 480), "Game", sf::Style::Close | sf::Style::Titlebar);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        // Draw

        window.display();
    }

    return 0;
}