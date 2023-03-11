#include <iostream>
#include <SFML/Graphics.hpp>
#include "chip8.h"

int main()
{
    CHIP8 emulator("C:\\Users\\thoma\\Documents\\CPP\\chip-8-emulator\\roms\\IBMLogo.ch8");

    sf::RenderWindow window(sf::VideoMode(1024, 512), "Game", sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(2);

    sf::RectangleShape pixel(sf::Vector2f(0, 0));
    pixel.setSize(sf::Vector2f(16, 16));

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        
        emulator.Update();

        window.display();
    }

    return 0;
}