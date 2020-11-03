// Tsilkow

#include <iostream>
#include <vector>
#include <time.h>
#include <memory>
#include <math.h>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

#include "commons.hpp"
#include "resources.hpp"
#include "region.hpp"


using namespace std;

int main()
{
    srand(time(NULL));

    RegionSettings rSetts =
    {
	sf::Vector2f(50, 50), // dimensions
	16,                   // tileSize
	16,                   // texTileSize
	128,                  // colorPerTile
	100,                  // genCenTotal
	{1, 3, 7}             // colorGenWghts
    };

    shared_ptr<RegionSettings> shr_rSetts = make_shared<RegionSettings>(rSetts);

    ResourceHolder<sf::Texture, std::string> textures;
    textures.load("tileset", "data/tileset.png");

    sf::RenderWindow window(sf::VideoMode(850, 800), "Color Quarry");
    window.setFramerateLimit(60);
    
    sf::View actionView(sf::Vector2f(425.f, 400.f), sf::Vector2f(850, 800));
    window.setView(actionView);

    Region region(shr_rSetts, textures);

    enum GameState{Menu, Play, Scores};
    GameState currState = GameState::Play;
    bool hasFocus = true;
    int ticksPassed = 0;

    while(window.isOpen())
    {
	sf::Event event;
	std::pair<std::string, std::string> input;
	
	window.clear();
	
	while (window.pollEvent(event))
	{
	    switch(event.type)
	    {
		case sf::Event::Closed:
		    window.close();
		    break;
		case sf::Event::LostFocus:
		    hasFocus = false;
		    //std::cout << "LOST FOCUS" << std::endl;
		    break;
		case sf::Event::GainedFocus:
		    hasFocus = true;
		    //std::cout << "GAINED FOCUS" << std::endl;
		    break;
		case sf::Event::KeyPressed:
		    if(hasFocus)
		    {
			switch(event.key.code)
			{
			    case sf::Keyboard::Escape:
				window.close();
				break;
				
			    default: break;
			}
		    }
		    break;
		    
		default: break;
	    }

	    if(hasFocus)
	    {
		switch(currState)
		{
		    case GameState::Play:
			break;
			
		    default: break;
		}
	    }
	}
	
	switch(currState)
	{	
	    case GameState::Play:
	        region.tick(ticksPassed);
		region.draw(window);
		break;
	}

	++ticksPassed;
	
	window.display();
    }
    
    return 0;
}
