#include "ant.hpp"


Ant::Ant(std::shared_ptr<AntSettings>& aSetts, std::shared_ptr<Region>& world,
	ResourceHolder<sf::Texture, std::string>& textures, std::string name, int allegiance, AntType type,
	sf::Vector2i coords):
    m_aSetts(aSetts),
    m_world(world),
    m_name(name),
    m_allegiance(allegiance),
    m_type(type),
    m_coords(coords),
    m_direction(0),
    m_storage(3, 0)
{
    m_storageLeft = m_aSetts->capacity;
    
    switch(m_type)
    {
	case worker: m_representation.setTexture(textures.get("worker")); break;
	case soldier: m_representation.setTexture(textures.get("soldier")); break;
    }
    sf::Vector2f size = sf::Vector2f(m_representation.getTexture()->getSize());
    m_representation.setOrigin(size * 0.5f);
    m_representation.setColor(m_aSetts->allColors[m_allegiance]);
    m_representation.setScale(sf::Vector2f(1.f, 1.f) * (float)m_aSetts->tileSize / size.x);
    m_representation.setPosition(sf::Vector2f(coords * m_aSetts->tileSize));
}

bool Ant::attack()
{
    return true;
}

bool Ant::move()
{
    ++m_actionProgress;

    m_representation.setPosition(
	   (float)m_aSetts->tileSize * (sf::Vector2f(m_coords + g_moves[m_direction]) *
					std::round((float)m_actionProgress/m_aSetts->walkingSpeed)));
    
    if(m_actionProgress >= m_aSetts->walkingSpeed)
    {
	m_coords += g_moves[m_direction];
	m_mask.clear();
	m_mask.emplace_back(m_coords);
	
	m_actionProgress = 0;
	return true;
    }
    else if(m_mask.size() < 2) m_mask.emplace_back(m_coords + g_moves[m_direction]);
    
    return false;
}

bool Ant::dig()
{
    ++m_actionProgress;

    if(m_actionProgress % 5 == 0)
    {
	m_representation.setPosition(sf::Vector2f(m_coords * m_aSetts->tileSize + g_moves[m_direction]));
    }
    else m_representation.setPosition(sf::Vector2f(m_coords * m_aSetts->tileSize));
    
    if(m_actionProgress >= m_aSetts->diggingSpeed)
    {
	std::vector<int> load = m_world->digOut(m_coords + g_moves[m_direction], m_storageLeft);
	int amount = load[0] + load[1] + load[2];

	for(int i = 0; i < 3; ++i)
	{
	    m_storage[i] += load[i];
	}
	m_storageLeft -= amount;
	m_actionProgress = 0;
	return true;
    }
    
    return false;
}

std::vector<int> Ant::unload()
{
    std::vector<int> result = {0, 0, 0};
    
    if(m_world->getTile(m_coords).type == 2)
    {
	result = m_storage;
	m_storage = {0, 0, 0};
	m_storageLeft = m_aSetts->capacity;
    }
    return result;
}

bool Ant::moveTo(sf::Vector2i target, bool dig)
{
    std::vector<int> result;
    std::vector< std::tuple<std::vector<int>, sf::Vector2i, int> >
	potenPaths(1, std::make_tuple(std::vector<int>(), m_coords, 0));
    std::set<sf::Vector2i, Vector2iComparator> visited;
    bool stop = false;
    
    visited.insert(m_coords);

    while(potenPaths.size() > 0)
    {
	auto curr = potenPaths.back();
	potenPaths.pop_back();

	for(int i = 0; i < g_moves.size(); ++i)
	{
	    sf::Vector2i next = std::get<1>(curr) + g_moves[i];
	    
	    if(next == target || ((m_world->isWalkable(next) || (dig && m_world->isDiggable(next))) &&
				  visited.find(next) == visited.end()))
	    {
		std::vector<int> temp = std::get<0>(curr);
		temp.emplace_back(i);
		
		if(next == target)
		{
		    result = temp;
		    stop = true;
		}
		else
		{
		    visited.insert(next);
		    if(dig && m_world->isDiggable(next))
		    {
			potenPaths.emplace_back(temp, next, std::get<2>(curr) + m_aSetts->diggingSpeed);
		    }
		    else potenPaths.emplace_back(temp, next, std::get<2>(curr) + m_aSetts->walkingSpeed);
		}
	    }
	    if(stop) break;
	}
	if(stop) break;
    }

    if(stop)
    {
	sf::Vector2i curr = m_coords;
	m_plan.clear();
	
	for(int i = 0; i < result.size(); ++i)
	{
	    if(dig && m_world->isDiggable(curr + g_moves[result[i]]))
	    {
		m_plan.push_back({ActionType::dig, result[i]});
	    }
	    m_plan.push_back({ActionType::move, result[i]});
	}
    }
    return stop;
}

bool Ant::tick(int ticksPassed)
{
    if(m_actionProgress == 0)
    {
	if(m_plan.size() > 0)
	{
	    m_currAction = m_plan.back().type;
	    m_direction = m_plan.back().direction;
	    m_representation.setRotation(m_direction * 90.f);
	}
	else m_currAction = wait;
    }
    switch(m_currAction)
    {
	case ActionType::attack: attack(); break;
	case ActionType::move:   move();   break;
	case ActionType::dig:    dig();    break;
	case ActionType::wait:   ; break;
    }

    return true;
}

void Ant::draw(sf::RenderTarget& target)
{
    target.draw(m_representation);
}
