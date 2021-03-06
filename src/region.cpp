#include "region.hpp"


bool Vector2iComparator::operator()(const sf::Vector2i& a, const sf::Vector2i& b)
{
    if(a.x == b.x)
    {
	return (a.y < b.y);
    }
    else return (a.x < b.x);
}

bool ReservationComparator::operator()(const Reservation& a, const Reservation& b)
{
    if(a.x == b.x)
    {
	if(a.y == b.y)
	{
	    return (a.from < b.from);
	}
	else return (a.y < b.y);
    }
    else return (a.x < b.x);
}

std::vector<int> spreadEvenly(int toGet, int r, int g, int b)
{
    std::vector<int> result(3, 0);

    if(r == g && r == b && r == 0)
    {
	r = 1;
	g = 1;
	b = 1;
    }
    
    int total = r + g + b;
    result[0] = toGet * r/total;
    result[1] = toGet * g/total;
    result[2] = toGet * b/total;

    r -= result[0];
    g -= result[1];
    b -= result[2];
    toGet -= result[0] + result[1] + result[2];

    while(toGet > 0)
    {
	if(r > g)
	{
	    if(r > b)
	    {
		++result[0];
		--r;
	    }
	    else
	    {
		++result[2];
		++b;
	    }
	}
	else
	{
	    if(g > b)
	    {
		++result[1];
		--g;
	    }
	    else
	    {
		++result[2];
		++b;
	    }
	}
	--toGet;
    }

    return result;
}

Region::Region(std::shared_ptr<RegionSettings>& rSetts,
	       ResourceHolder<sf::Texture, std::string>& textures):
    m_rSetts(rSetts),
    m_ticks(0)
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
								{TileType::wall, 1, 1, 1}));
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
    /*for(int x = 0; x < m_rSetts->dimensions.x; ++x)
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
    }*/

    // marking generation centers' reaches
    for(auto it = genCenters.begin(); it != genCenters.end(); ++it)
    {
	sf::Vector2i coords(it->first);
	for(int r = 0; r < m_rSetts->genCenReach; ++r)
	{
	    if(r == 0)
	    {
		if(inBounds(coords))
		{
		    switch(it->second)
		    {
			case 0: atCoords(m_data, coords).r += m_rSetts->genCenReach-r; break;
			case 1: atCoords(m_data, coords).g += m_rSetts->genCenReach-r; break;
			case 2: atCoords(m_data, coords).b += m_rSetts->genCenReach-r; break;
		    }
		}
	    }
	    else
	    {
		for(int i = 0; i < 4; ++i)
		{
		    for(int j = 0; j < r; ++j)
		    {
			if(inBounds(coords))
			{   
			    switch(it->second)
			    {
				case 0: atCoords(m_data, coords).r += m_rSetts->genCenReach-r; break;
				case 1: atCoords(m_data, coords).g += m_rSetts->genCenReach-r; break;
				case 2: atCoords(m_data, coords).b += m_rSetts->genCenReach-r; break;
			    }
			}
			coords += getMove(i-3) + getMove(i-2);
		    }
		}
	    }
	    coords += getMove(0);
	}
    }

    // processing generation centers weights
    for(int x = 0; x < m_rSetts->dimensions.x; ++x)
    {
	for(int y = 0; y < m_rSetts->dimensions.y; ++y)
	{
	    std::vector<int> temp = spreadEvenly(m_rSetts->colorPerTile,
						 m_data[x][y].r, m_data[x][y].g, m_data[x][y].b);
	    
	    m_data[x][y].r = temp[0];
	    m_data[x][y].g = temp[1];
	    m_data[x][y].b = temp[2];

	    std::cout << "(" << m_data[x][y].r << " " << m_data[x][y].g << " " << m_data[x][y].b << ")\n";
	}
    }
    
    // nest generation
    m_nests = std::vector< std::vector<sf::Vector2i> >(m_rSetts->nestTotal, std::vector<sf::Vector2i>());
    m_nestDomains.emplace_back(std::map<sf::Vector2i, int, Vector2iComparator>());
    for(int i = 0; i < m_rSetts->nestTotal; ++i)
    {
	sf::Vector2f temp = alongSquare((float)i / m_rSetts->nestTotal);
	sf::Vector2i nestCoords = sf::Vector2i(m_rSetts->dimensions.x/2.f + temp.x *
					       m_rSetts->dimensions.x/4.f,
					       m_rSetts->dimensions.x/2.f + temp.y *
					       m_rSetts->dimensions.y/4.f);

	printVector(temp, true);
	
	atCoords(m_data, nestCoords).type = TileType::nest;
	m_nests[0].emplace_back(nestCoords);
	m_nestDomains[0].insert({nestCoords, i});
    }

    // obstacles registration
    for(int x = 0; x < m_rSetts->dimensions.x; ++x)
    {
	for(int y = 0; y < m_rSetts->dimensions.y; ++y)
	{
	    if(m_data[x][y].type == TileType::wall)
	    {
		m_reservations.insert({{x, y, 0, -1}, m_rSetts->colorPerTile});
	    }
	}
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

// first is whether it is reserved
// second is how much digging is needed if it is diggable (-1 means it can't be dug out)
std::pair<bool, int> Region::isReserved(int x, int y, int from, int duration)
{
    int toDig = 0;
    int to = from + duration-1;
    // checking if there are any reservations
    if(m_reservations.begin() != m_reservations.end())
    {
	// first element that is not smaller than time
	auto it = m_reservations.lower_bound({x, y, from, to});

	// is it overlapping with previous reservation
	--it;
	if(it->first.x == x && it->first.y == y && it->first.to >= from)
	{
	    if(it->second == 0) return std::make_pair(true, -1);
	    else toDig = it->second;
	}

	// is it ovelapping with next reservation
	++it;
	if(it->first.x == x && it->first.y == y && it->first.from <= to) std::make_pair(true, -1);
    }
    
    return std::make_pair(false, toDig);
}

std::pair<bool, int> Region::isReserved(sf::Vector2i coords, int from, int duration)
{
    return isReserved(coords.x, coords.y, from, duration);
}

void Region::reserve(sf::Vector2i coords, int from, int duration)
{
    int to = from + duration-1;
    Reservation temp = {coords.x, coords.y, from, to};
    m_reservations[temp] = 0;

    m_toCleanAt.insert({to, temp});
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

	    
	    for(int i = 0; i < getMoveTotal()-1; ++i)
	    {
		sf::Vector2i temp = coords + getMove(i);
		if(inBounds(temp) && isWalkable(temp))
		{
		    for(int j = 0; j < m_nestDomains.size(); ++j)
		    {
			m_nestDomains[j][coords] = m_nestDomains[j][temp];
		    }
		}
	    }
	}
    }

    return result;
}

bool PathCoordComparator::operator() (const PathCoord& a, const PathCoord& b)
{
    if(a.x == b.x)
    {
	if(a.y == b.y)
	{
	    return (a.t < b.t);
		}
	return (a.y < b.y);
    }
    return (a.x < b.x);
}

bool PathCoordHeuresticComparator::operator() (const PathCoord& a, const PathCoord& b)
{
    if(a.h == b.h)
    {
	if(a.t == b.t)
	{
	    if(a.x == b.x)
	    {
		return (a.y < b.y);
	    }
	    return (a.x < b.x);
	}
	return (a.t < b.t);
    }
    return (a.h < b.h);
};

int Region::evalByHeurestic(PathCoord path, sf::Vector2i target)
{
    return (distance(path.coords() - target) * 256 + path.t);
}

std::vector<int> Region::findPath
(sf::Vector2i start, int time, sf::Vector2i target, int walkingSpeed, int diggingSpeed,
 int ableToDig, bool isReserving, bool infDigging)
{
    return {4};
    /*
    if(time == -1) time = m_ticks;
    int timeOfArrival;
    std::vector<int> finalPath;
    std::vector<sf::Vector3i> digStops;
    std::set<PathCoord, PathCoordHeuresticComparator> potenPaths;
    // map storing best direction from these coords to start
    std::map<PathCoord, std::pair<int, int>, PathCoordComparator> directions; 
    PathCoord winner(start, time, ableToDig);
    bool stop = false;
    
    winner.h = evalByHeurestic(winner, target);
    directions[winner] = std::make_pair(4, 1);
    potenPaths.insert(winner);
    
    while(potenPaths.size() > 0)
    {
	std::cout << potenPaths.size();
	std::cout << "}\n";

	//getchar();
	
	PathCoord curr = *potenPaths.begin();
	potenPaths.erase(potenPaths.begin());

	// check if these coords are available for walking off of them
	if(!isReserved(curr.coords(), curr.t, walkingSpeed).first)
	{
	    for(int dir = 0; dir < getMoveTotal()-1; ++dir) // consider moves without the waiting one
	    {
		// coords which it checks
		PathCoord next(curr.coords() + getMove(dir), curr.t, curr.d);

		if(inBounds(next.coords()))
		{
		    // is it possible to walk there
		    bool nextWalk = !isReserved(next.coords(), next.t, walkingSpeed).first;
		
		    // is it possible to dig there and how much digging there is
		    std::pair<bool, int> nextDig =
			isReserved(next.coords(), next.t, walkingSpeed + diggingSpeed);
		    nextDig.first = !nextDig.first;
		    if(isReserved(curr.coords(), curr.t, walkingSpeed + diggingSpeed).first)
			nextDig.first = false;

		    if(nextDig.first)
		    {
			next.t += walkingSpeed + diggingSpeed;
			if(!infDigging) next.d -= nextDig.second;
		    }
		    else if(nextWalk)
		    {
			next.t += walkingSpeed;
		    }
		    // evaluate next using a heurestic
		    next.h = evalByHeurestic(next, target);

		    if(directions.find(next) == directions.end() &&
		       // checks if coords haven't been visited
		       (nextWalk || (nextDig.first && next.d >= 0)))
		    {
			// marks which direction it came to these coords
			std::cout << directions[next].second << std::endl;

			// if it has to (and can) dig
			if(nextDig.first && next.d >= 0)
			{
			    directions[next] = std::make_pair(dir, walkingSpeed + diggingSpeed);
			    potenPaths.insert(next);
			}
			else // if it can walks here
			{
			    directions[next] = std::make_pair(dir, walkingSpeed);
			    potenPaths.insert(next);
			}

			// if it has arrived, stop search
			if(next.coords() == target)
			{
			    stop = true;
			    timeOfArrival = next.t;
			    winner = next;
			    std::cout << dir << std::endl;
			    break;
			}
		    }
		}
	    }
	    if(!stop)
	    {
		// consider waiting as a move
		PathCoord next(curr.coords(), curr.t+1, curr.d);
		next.h = evalByHeurestic(next, target);
		
		if(!isReserved(curr.coords(), curr.t, 1).first &&
		   directions.find(next) == directions.end())
		{
		    directions[next] = std::make_pair(4, 1);
		    potenPaths.insert(next);
		
		}
	    }
	}
	else std::cout << "duud, my place is reserved\n";
	if(stop) break;
    }

//getchar();

// if seach found the destination
    if(stop)
    {
	PathCoord curr(target, timeOfArrival, ableToDig);
	// recreate best path found
	while(curr.coords() != target)
	{
	    finalPath.push_back(directions[curr].first);
	    curr.t -= directions[curr].second;
	    curr.move(reverseDirection(finalPath.back()));
	    //getchar();
	}

	std::reverse(finalPath.begin(), finalPath.end());

	// calculate time and reserve the coords it was traversing
	for(int i = 0; i < finalPath.size(); ++i)
	{
	    // if it's waiting
	    if(finalPath[i] == 4)
	    {
		if(isReserving) reserve(curr.coords(), time, 1);
		time += 1;
	    } 
	    else
	    {
		// else if walking is possible
		if(!isReserved(curr.coords(), time, walkingSpeed).first)
		{
		    if(isReserving)
		    {
			reserve(curr.coords()                        , time, walkingSpeed);
			reserve(curr.coords() + getMove(finalPath[i]), time, walkingSpeed);
		    }
		    time += walkingSpeed;
		}
		else // otherwise dig (it has to be a legal move, cause it was checked before)
		{
		    if(isReserving)
		    {
			reserve(curr.coords()                        , time, walkingSpeed + diggingSpeed);
			reserve(curr.coords() + getMove(finalPath[i]), time, walkingSpeed + diggingSpeed);
		    }
		    digStops.emplace_back(curr.toords());
		    time += walkingSpeed + diggingSpeed;
		}
		curr.move(finalPath[i]);
	    }
	}
    }

    return std::make_tuple(finalPath, time, digStops);*/
}

/*
std::pair<std::vector<int>, int> findPath(sf::Vector2i start, int time, std::vector<sf::Vector2i> targets,
					  int walkingSpeed, int diggingSpeed, int ableToDig, bool 
    std::pair<std::vector<int>, int> result;
    
    for(int i = 0; i < targets.size(); ++i)
    {
	sf::Vector2i beginning;
	if(i == 0) beginning = start;
	else beginning = targets[i-1];
	
	auto temp = findPath(beginning, time, targets[i], walkingSpeed,
							 diggingSpeed, ableToDig, reserve);

	result.first.insert(result.end(), temp.begin(), temp.end());
	result.second += temp.second;
    }

    return result;
}

std::pair<std::vector<int>, int> findDiggingPath(sf::Vector2i start, int time, sf::Vector2i target,
						 int walkingSpeed, int diggingSpeed, int ableToDig)
{
    std::pair<std::vector<int>, int> result;
    auto diggingPath = findPath(start, time, target, walkingSpeed, diggingSpeed, ableToDig, false, true);
    sf::Vector2i waypoint; // coords to which ant can dig to on these settings without unloading
    int rubbleSoFar = 0;

    for(int i = 0; i < diggingPath.first.size(); ++i)
    {
	
    }
}
*/

bool Region::tick(int ticksPassed)
{
    m_ticks = ticksPassed;

    auto found = m_toCleanAt.find(m_ticks);
    while(found != m_toCleanAt.end())
    {
	m_reservations.erase(found->second);
	m_toCleanAt.erase(found);
	found = m_toCleanAt.find(m_ticks);
    }
    
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

sf::Vector2i Region::getNestAt(int allegiance, sf::Vector2i coords)
{
    if(m_nestDomains[allegiance].find(coords) != m_nestDomains[allegiance].end())
    {
	return m_nests[allegiance][m_nestDomains[allegiance][coords]];
    }
    return sf::Vector2i(-1, -1);
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
