#include <iostream>
#include <vector>
#include <memory>
#include <math.h>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

#include "commons.hpp"
#include "region.hpp"


struct AntSettings
{
    int capacity;
    std::vector<sf::Color> allColors;
    int tileSize;
    int walkingSpeed;
    int diggingSpeed;
};

std::vector<sf::Vector2i> g_moves = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

enum ActionType {wait, move, dig, attack};

struct Action
{
    ActionType type;
    int direction;
};

enum AntType {worker, soldier};
    
class Ant
{
    private:
    std::shared_ptr<AntSettings> m_aSetts;
    std::string m_name;
    int m_allegiance;
    std::shared_ptr<Region> m_world;
    AntType m_type;
    sf::Vector2i m_coords;
    std::vector<sf::Vector2i> m_mask;
    int m_direction;
    sf::Vector2f m_position;
    sf::Sprite m_representation;

    int m_storageLeft;
    std::vector<int> m_storage;
    ActionType m_currAction;
    int m_actionProgress;

    std::vector<Action> m_plan;

    bool attack();
    
    bool move();

    bool dig();

    std::vector<int> unload();
    
    public:
    Ant(std::shared_ptr<AntSettings>& aSetts, std::shared_ptr<Region>& world,
	ResourceHolder<sf::Texture, std::string>& textures, std::string name, int allegiance, AntType type,
	sf::Vector2i coords);

    bool moveTo(sf::Vector2i target, bool dig=false);

    bool tick(int ticksPassed);

    void draw(sf::RenderTarget& target);


    const AntType& getType() {return m_type; }
};
