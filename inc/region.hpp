#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <math.h>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

#include "commons.hpp"
#include "resources.hpp"


struct RegionSettings
{
    sf::Vector2f dimensions;
    int tileSize;
    int texTileSize;
    int colorPerTile;
    int genCenTotal; // generation centers total
    std::vector<int> colorGenWghts; // color generation weights: r = 0, g = 1, b = 2
};

struct Tile
{
    int type; // 0 - empty, 1 - stone, 2 - nest
    int r;
    int g;
    int b;
};

struct Vector2iComparator
{
    bool operator()(const sf::Vector2i& a, const sf::Vector2i& b);
};

class Region
{
    private:
    std::shared_ptr<RegionSettings> m_rSetts;
    std::vector< std::vector<Tile> > m_data;
    std::vector<sf::Vertex> m_representation;
    sf::RenderStates m_states;

    std::vector<sf::Vector2i> m_toUpdate;
    
    sf::Color getTileColor(sf::Vector2i coords);
        
    sf::Color getTileColor(int x, int y) {return getTileColor(sf::Vector2i(x, y)); };
    
    void generate();

    void update();

    public:
    Region(std::shared_ptr<RegionSettings>& rSetts, ResourceHolder<sf::Texture, std::string>& textures);
    
    bool tick(int ticksPassed);

    void draw(sf::RenderTarget& target);
};
