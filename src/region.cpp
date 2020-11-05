#include "region.hpp"


bool Vector2iComparator::operator()(const sf::Vector2i& a, const sf::Vector2i& b)
{
    if(a.x < b.x) return true;
    if(a.x == b.x && a.y < b.y) return true;
    return false;
}

Region::Region(std::shared_ptr<RegionSettings>& rSetts,
	       ResourceHolder<sf::Texture, std::string>& textures):
    m_rSetts(rSetts)
{
    m_states.texture = &textures.get("tileset");
    
    generate();
    
    for(int x = 0; x < m_rSetts->dimensions.x; ++x)
    {
	for(int y = 0; y < m_rSetts->dimensions.y; ++y)
	{
	    std::vector<sf::Vector2f> position = {
		{(float) x    * m_rSetts->tileSize, (float) y    * m_rSetts->tileSize},
		{(float)(x+1) * m_rSetts->tileSize, (float) y    * m_rSetts->tileSize},
		{(float)(x+1) * m_rSetts->tileSize, (float)(y+1) * m_rSetts->tileSize},
		{(float) x    * m_rSetts->tileSize, (float)(y+1) * m_rSetts->tileSize}
	    };
	    int type = atCoords(m_data, sf::Vector2i(x, y)).type;
	    std::vector<sf::Vector2f> texCoords = {
		{(float) type    * m_rSetts->texTileSize, 0.f},
		{(float)(type+1) * m_rSetts->texTileSize, 0.f},
		{(float)(type+1) * m_rSetts->texTileSize, (float)m_rSetts->texTileSize},
		{(float) type    * m_rSetts->texTileSize, (float)m_rSetts->texTileSize}
	    };

	    for(int i = 0; i < 4; ++i)
	    {
		m_representation.emplace_back(position[i], getTileColor(x, y), texCoords[i]);
	    }
	}
    }
}

sf::Color Region::getTileColor(sf::Vector2i coords)
{
    sf::Color result(0, 0, 0);
    
    switch(atCoords(m_data, coords).type)
    {
	case 0:
	    result.r = 0;
	    result.g = 0;
	    result.b = 0;
	    break;
	    
	case 1:
	    result.r = std::round(255.f * atCoords(m_data, coords).r/m_rSetts->colorPerTile);
	    result.g = std::round(255.f * atCoords(m_data, coords).g/m_rSetts->colorPerTile);
	    result.b = std::round(255.f * atCoords(m_data, coords).b/m_rSetts->colorPerTile);
	    break;
	    
	case 2: 
	    result.r = 255;
	    result.g = 255;
	    result.b = 255;
	    break;
    }
    
    return result;
}

void Region::generate()
{
    m_data = std::vector< std::vector<Tile> >(m_rSetts->dimensions.x,
					      std::vector<Tile>(m_rSetts->dimensions.y,
								{TileType::wall, 0, 0, 0}));
    std::map<sf::Vector2i, int, Vector2iComparator> genCenters;

    // stone centers generation
    for(int i = 0; i < m_rSetts->genCenTotal; ++i)
    {
	sf::Vector2i coords;

	do{
	    coords.x = randomI(0, m_rSetts->dimensions.x - 1);
	    coords.y = randomI(0, m_rSetts->dimensions.y - 1);
	}while(genCenters.find(coords) != genCenters.end());

	genCenters.insert(std::make_pair(coords, randomIWeights(m_rSetts->colorGenWghts)));
    }

    // stone generation
    for(int x = 0; x < m_rSetts->dimensions.x; ++x)
    {
	for(int y = 0; y < m_rSetts->dimensions.y; ++y)
	{
	    sf::Vector2i coords(x, y);
	    int closestColor = 0;
	    int smallestDistance = distance(coords - genCenters.begin()->first);
	    
	    for(auto it = genCenters.begin(); it != genCenters.end(); ++it)
	    {
		int tempDist = distance(coords - it->first);
		if(tempDist < smallestDistance)
		{
		    closestColor = it->second;
		    smallestDistance = tempDist;
		}
	    }

	    switch(closestColor)
	    {
		case 0: atCoords(m_data, sf::Vector2i(x, y)).r = m_rSetts->colorPerTile; break;
		case 1: atCoords(m_data, sf::Vector2i(x, y)).g = m_rSetts->colorPerTile; break;
		case 2: atCoords(m_data, sf::Vector2i(x, y)).b = m_rSetts->colorPerTile; break;
	    }
	}
    }
    
    // nest generation
    for(int i = 0; i < m_rSetts->nestTotal; ++i)
    {
	sf::Vector2f temp = alongSquare((float)i / m_rSetts->nestTotal);

	printVector(temp, true);
	
	atCoords(m_data, sf::Vector2i(m_rSetts->dimensions.x/2.f + temp.x *
				      m_rSetts->dimensions.x/4.f,
				      m_rSetts->dimensions.x/2.f + temp.y *
				      m_rSetts->dimensions.y/4.f)).type = TileType::nest;
    }
}

void Region::update()
{
    for(int i = 0; i < m_toUpdate.size(); ++i)
    {
	int index = (m_toUpdate[i].x * m_rSetts->dimensions.y + m_toUpdate[i].y) * 4;
	int type = atCoords(m_data, m_toUpdate[i]).type;
	std::vector<sf::Vector2f> texCoords = {
	    {(float) type    * m_rSetts->tileSize, 0.f},
	    {(float)(type+1) * m_rSetts->tileSize, 0.f},
	    {(float)(type+1) * m_rSetts->tileSize, (float)m_rSetts->tileSize},
	    {(float) type    * m_rSetts->tileSize, (float)m_rSetts->tileSize}
	};
		
	for(int j = 0; j < 4; ++j)
	{
	    m_representation[index + j].color = getTileColor(m_toUpdate[i]);
	    m_representation[index + j].texCoords = texCoords[j];
	}
    }

    m_toUpdate.clear();
}

std::vector<int> Region::digOut(sf::Vector2i coords, int amount)
{
    std::vector<int> result = {0, 0, 0};
    
    if(isDiggable(coords))
    {
	int r = atCoords(m_data, coords).r;
	int g = atCoords(m_data, coords).g;
	int b = atCoords(m_data, coords).b;
	int total = r + g + b;
	amount = std::max(amount, total);

	result[0] = amount * r / total;
	result[1] = amount * g / total;
	result[2] = amount * b / total;
	amount -= result[0] + result[1] + result[2];

	while(amount > 0)
	{
	    if(((float)r - result[0])/total >= ((float)g - result[1])/total)
	    {
		if(((float)r - result[0])/total >= ((float)b - result[2])/total) ++result[0];
		else ++result[2];
	    }
	    else if(((float)g - result[1])/total >= ((float)b - result[2])/total) ++result[1];
	    else ++result[2];
	    
	    --amount;
	}

	atCoords(m_data, coords).r -= result[0];
	atCoords(m_data, coords).g -= result[1];
	atCoords(m_data, coords).b -= result[2];

	if(atCoords(m_data, coords).r + atCoords(m_data, coords).g + atCoords(m_data, coords).b == 0)
	{
	    //printVector(coords, true);
	    atCoords(m_data, coords).type = TileType::open;
	    m_toUpdate.push_back(coords);
	}
    }

    return result;
}

bool Region::tick(int ticksPassed)
{
    update();

    return false;
}

bool Region::inBounds(sf::Vector2i coords)
{
    if(coords.x < 0 || coords.x >= m_rSetts->dimensions.x ||
       coords.y < 0 || coords.y >= m_rSetts->dimensions.y)
	return false;
    return true;
}

bool Region::isWalkable(sf::Vector2i coords)
{
    if(inBounds(coords) && m_rSetts->walkable[atCoords(m_data, coords).type] == 1)
    {
	return true;
    }
    return false;
}

bool Region::isDiggable(sf::Vector2i coords)
{
    if(inBounds(coords) && m_rSetts->diggable[atCoords(m_data, coords).type] == 1)
    {
	return true;
    }
    return false;
}

void Region::draw(sf::RenderTarget& target)
{
    target.draw(&m_representation[0], m_representation.size(), sf::Quads, m_states);
}
