#include "ant.hpp"


int getMoveTotal() {return 4; }

sf::Vector2i getMove(int direction)
{
    direction = modulo(direction, 4);
    
    switch(direction)
    {
	case 0: return sf::Vector2i( 0, -1); break;
	case 1: return sf::Vector2i( 1,  0); break;
	case 2: return sf::Vector2i( 0,  1); break;
	case 3: return sf::Vector2i(-1,  0); break;
    }
}

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
    m_storage(3, 0),
    m_currAction(ActionType::wait),
    m_actionProgress(0)
{
    m_storageLeft = m_aSetts->capacity;
    m_mask.push_back(m_coords);
    
    switch(m_type)
    {
	case worker: m_representation.setTexture(textures.get("worker")); break;
	case soldier: m_representation.setTexture(textures.get("soldier")); break;
    }
    sf::Vector2f size = sf::Vector2f(m_representation.getTexture()->getSize());
    m_representation.setOrigin(size * 0.5f);
    m_representation.setColor(m_aSetts->allColors[m_allegiance]);
    m_representation.setScale(sf::Vector2f(1.f, 1.f) * (float)m_aSetts->tileSize / size.x);
    m_position = (sf::Vector2f(m_coords) + sf::Vector2f(0.5f, 0.5f)) * (float)m_aSetts->tileSize;
    m_representation.setPosition(m_position);
}

bool Ant::attack()
{
    return true;
}

bool Ant::move()
{
    ++m_actionProgress;

    m_representation.setPosition(m_position +
				 sf::Vector2f(getMove(m_direction) *
					      (int)std::round((float)m_aSetts->tileSize * 
							      m_actionProgress / m_aSetts->walkingSpeed)));
    
    if(m_actionProgress >= m_aSetts->walkingSpeed)
    {
	m_coords += getMove(m_direction);
	m_position = (sf::Vector2f(m_coords) + sf::Vector2f(0.5f, 0.5f)) * (float)m_aSetts->tileSize;
	m_mask.clear();
	m_mask.emplace_back(m_coords);
	
	m_actionProgress = 0;
	return true;
    }
    else if(m_mask.size() < 2) m_mask.emplace_back(m_coords + getMove(m_direction));
    
    return false;
}

bool Ant::dig()
{
    ++m_actionProgress;

    if(m_actionProgress % 5 == 0)
    {
	m_representation.setPosition(m_position + sf::Vector2f(getMove(m_direction)));
    }
    else m_representation.setPosition(m_position);
    
    if(m_actionProgress >= m_aSetts->diggingSpeed)
    {
	std::vector<int> load = m_world->digOut(m_coords + getMove(m_direction), m_storageLeft);
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

std::pair<std::vector<int>, int> Ant::pathTo(sf::Vector2i target, bool dig)
{
    std::vector<int> result;
    int score = -1;
    std::vector< std::tuple<std::vector<int>, sf::Vector2i, int> >
	potenPaths(1, std::make_tuple(std::vector<int>(), m_coords, 0));
    std::set<sf::Vector2i, Vector2iComparator> visited;
    bool stop = false;
    
    visited.insert(m_coords);

    while(potenPaths.size() > 0)
    {
	auto curr = potenPaths[0];
	potenPaths.erase(potenPaths.begin());

	for(int i = 0; i < getMoveTotal(); ++i)
	{
	    sf::Vector2i next = std::get<1>(curr) + getMove(i);
	    
	    if(next == target || (visited.find(next) == visited.end() &&
				  (m_world->isWalkable(next) || (dig && m_world->isDiggable(next)))))
	    {
		std::vector<int> temp = std::get<0>(curr);
		temp.emplace_back(i);
		
		if(next == target)
		{
		    result = temp;
		    if(dig && m_world->isDiggable(next)) score = std::get<2>(curr) + m_aSetts->diggingSpeed;
		    else score = std::get<2>(curr) + m_aSetts->walkingSpeed;
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

    printVector(m_coords, true);
    std::cout << "{ ";
    for(int i = 0; i < result.size(); ++i)
    {
	std::cout << result[i] << " ";
    }
    std::cout << "} \n";
    printVector(target, true);

    return std::make_pair(result, score);
}

bool Ant::moveTo(sf::Vector2i target, bool dig)
{
    m_destination = target;
    std::vector<int> temp = pathTo(target, dig).first;

    if(temp.size() > 0)
    {
	m_path = temp;
	return true;
    }
    return false;
}

bool Ant::tick()
{
    /*
    std::cout << "{ ";
    for(int i = 0; i < m_path.size(); ++i)
    {
	std::cout << m_path[i] << " ";
    }
    std::cout << "} \n";
    */
    
    std::vector<sf::Vector2i> nests = m_world->getNests(m_allegiance);
    for(int i = 0; i < nests.size(); ++i)
    {
	if(m_coords == nests[i])
	{
	    unload();
	    break;
	}
    }
    
    if(m_actionProgress == 0)
    {
	if(m_path.size() == 0)
	{
	    if(m_destination != m_coords)
	    {
		if(!moveTo(m_destination, true))
		{
		    m_destination = m_coords;
		}
	    }
	}
	
	if(m_path.size() > 0)
	{
	    if(m_world->isDiggable(m_coords + getMove(m_path.back())) && m_storageLeft == 0)
	    {
		std::cout << nests.size() << std::endl;
		std::pair<std::vector<int>, int> bestPath; 
		std::pair<std::vector<int>, int> currPath;
	    
		if(nests.size() != 0)
		{
		    bestPath = pathTo(nests[0], false);
	    
		    for(int i = 1; i < nests.size(); ++i)
		    {
			currPath = pathTo(nests[i], false);
		    
			if(currPath.second != -1) // if path exists
			{
			    if(bestPath.second == -1 || bestPath.second > currPath.second) bestPath = currPath;
			}
		    }
		}

		if(nests.size() == 0 || bestPath.second == -1)
		{
		    m_currAction = ActionType::wait;
		    m_destination = m_coords;
		    m_path.clear();
		}
		else
		{
		    m_path = bestPath.first;
		}
	    
	    }
	}

	if(m_path.size() > 0)
	{
	    m_direction = m_path.back();
	    m_path.pop_back();
	    m_representation.setRotation(m_direction * 90.f);

	    if     (m_world->isWalkable(m_coords + getMove(m_direction))) m_currAction = ActionType::move;
	    else if(m_world->isDiggable(m_coords + getMove(m_direction))) m_currAction = ActionType::dig;
	    else
	    {
		m_currAction = ActionType::wait;
		m_path.clear();
	    }
	}
	else
	{
	    m_currAction = ActionType::wait;
	    m_path.clear();
	}
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
