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
    return sf::Color(255.f * atCoords(m_data, coords).r/m_rSetts->colorPerTile,
		     255.f * atCoords(m_data, coords).g/m_rSetts->colorPerTile,
		     255.f * atCoords(m_data, coords).b/m_rSetts->colorPerTile);
}

void Region::generate()
{
    m_data = std::vector< std::vector<Tile> >(m_rSetts->dimensions.x,
					      std::vector<Tile>(m_rSetts->dimensions.y, {1, 0, 0, 0}));
    std::map<sf::Vector2i, int, Vector2iComparator> genCenters;

    for(int i = 0; i < m_rSetts->genCenTotal; ++i)
    {
	sf::Vector2i coords;

	do{
	    coords.x = randomI(0, m_rSetts->dimensions.x - 1);
	    coords.y = randomI(0, m_rSetts->dimensions.y - 1);
	}while(genCenters.find(coords) != genCenters.end());

	genCenters.insert(std::make_pair(coords, randomIWeights(m_rSetts->colorGenWghts)));
    }

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
}

void Region::update()
{
    for(int i = 0; i < m_toUpdate.size(); ++i)
    {
	int index = (m_toUpdate[i].y * m_rSetts->dimensions.x + m_toUpdate[i].x) * 4;
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

bool Region::tick(int ticksPassed)
{
    update();

    return false;
}

void Region::draw(sf::RenderTarget& target)
{
    target.draw(&m_representation[0], m_representation.size(), sf::Quads, m_states);
}
