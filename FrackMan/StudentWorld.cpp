#include "StudentWorld.h"
#include <string>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// destructor to free memory if cleanup function interrupted
StudentWorld::~StudentWorld()
{
    for (int i = 0 ; i < DIRT_HEIGHT; i++)
    {
        for (int j = 0; j < DIRT_WIDTH; j++)
        {
            if (dirt_map[i][j]!=nullptr)
                delete dirt_map[i][j];
        }
    }
    if (player != nullptr)
        delete player;
    vector<Actor*>::iterator it;
    for (it = live_object.begin(); it != live_object.end(); it++)
    {
        if ((*it)!=nullptr)
            delete (*it);
    }
}

//set the corresonding dirt to inactive and return true if the remove operation is done.
bool StudentWorld::remove_dirt(int x, int y)
{
    if (x>=0 && x < DIRT_WIDTH && y >= 0 && y < DIRT_HEIGHT)
    {
        if (dirt_map[y][x]->is_active())
        {
            dirt_map[y][x]->set_me_inactive();
            return 1;
        }
        else return 0;
    }
    
    else
        return 0;
}

// check if the current location is occupied by a dirt
bool StudentWorld::is_occupied_by_dirt(int x, int y)
{
    if (x >= 0 && x < DIRT_WIDTH && y >= 0 && y < DIRT_HEIGHT)
        return (dirt_map[y][x] -> is_active());
    else return false;
}

// check if one of the upper and right 4*4 square is occupied by a dirt
bool StudentWorld::is_current_16square_occupied_by_dirt(int x, int y)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (is_occupied_by_dirt(x+i, y+j))
                return true;
        }
    }
    return false;
}

// used for boulder to check if there is any Dirt in the 4 squares immediately below it; also for squirt
// also for other objects, to check the 4 squares in front of the object.
bool StudentWorld::is_dirt(GraphObject* goodie, GraphObject::Direction dir)
{
    //return true if there is dirt or hit the wall
    //return false if the current direction is available.
    int x = goodie -> getX();
    int y = goodie -> getY();
    // is true only when the *goodie passed in is squirt type
    bool is_squirt = (dynamic_cast<Squirt *>(goodie)!= nullptr);
    switch (dir)
    {
        case GraphObject::down:
        {
            if (!is_squirt)
            {
                if (y == 0)
                    return true; // there is wall(dirt), lower bound
                else
                {
                    for (int i = 0; i < 4; i++)
                    {
                        if (dirt_map[y-1][x+i] -> is_active())
                        return true;
                    }
                return false;
                }
            }
            else //is_squirt
            {
                if (y == 0)
                    return true; // there is wall(dirt), hit the lower bound or hit the upper bound
                else if  (y > DIRT_HEIGHT)
                    return false; // there is no dirt
                else if (dirt_map[y-1][x] -> is_active())
                        return true;
                else return false;
            }
            break;
        }
            
        case GraphObject::up:
        {
            if (!is_squirt)
            {
                if (y == DIRT_WIDTH -4)
                    return true; // there is wall(dirt)
                else if (y >= DIRT_HEIGHT - 4)
                    return false; // there is no dirt
                else
                {
                    for (int i = 0; i < 4; i++)
                    {
                        if (dirt_map[y+4][x+i] -> is_active())
                        return true;
                    }
                }
                return false;
            }
            else //is_squirt
            {
                if (y == DIRT_WIDTH -1)
                    return true; // there is wall(dirt), upper bound.
                else if (y >= DIRT_HEIGHT - 1)
                    return false; // thre is no dirt
                else if (dirt_map[y+1][x] -> is_active())
                        return true;
                else return false;
            }
            break;
        }
            
            
        case GraphObject::left:
        {
            if (!is_squirt)
            {
                if (x == 0)
                    return true; // there is dirt(wall), upper bound
                else
                {
                    for (int i = 0; i < 4; i++)
                    {
                        if (y+i < DIRT_HEIGHT)
                        {
                            if (dirt_map[y+i][x-1] -> is_active())
                                return true;
                        }
                    }
                }
                return false;
            }
            else //is_squirt
            {
                if (x == 0)
                    return true; // there is dirt(wall)
                else if (y >= DIRT_HEIGHT)
                    return false; // there is no dirt
                else
                {
                    if (dirt_map[y][x-1] -> is_active())
                        return true;
                    else return false;
                }
            }
            break;
        }
        
        case GraphObject::right:
        {
            if (!is_squirt)
            {
                if (x == DIRT_WIDTH - 4)
                    return true; // there is wall(dirt), upper bound
                else
                {
                    for (int i = 0; i < 4; i++)
                    {
                        if (y+i < DIRT_HEIGHT)
                        {
                            if (dirt_map[y+i][x+4] -> is_active())
                                return true;
                        }
                    }
                }
                return false;
            }
            else //is_squirt
            {
                if (x >= DIRT_WIDTH - 1)
                    return true; // there is wall(dirt)
                else if (y >= DIRT_HEIGHT)
                    return false;
                else if (dirt_map[y][x+1] -> is_active())
                        return true;
                    else return false;
            }
        }
        default:
        {
            return false;
            break;
        }
    }
}

// move the goodie for one step in the current direction.
void StudentWorld::move_current_dir(GraphObject * goodie, GraphObject::Direction dir)
{
    switch (dir)
    {
        case GraphObject::right:
        {
            goodie -> moveTo(goodie->getX()+1, goodie -> getY());
            break;
        }
        case GraphObject::left:
        {
            goodie -> moveTo(goodie->getX()-1, goodie -> getY());
            break;
        }
        case GraphObject::up:
        {
            goodie -> moveTo(goodie->getX(), goodie -> getY()+1);
            break;
        }
        case GraphObject::down:
        {
            goodie -> moveTo(goodie->getX(), goodie -> getY()-1);
            break;
        }
        default:
            break;
    }
}

// update x and y if moving one step in the current dir.
void StudentWorld::my_next_coordinate(int &x, int &y, GraphObject::Direction dir)
{
    if (dir == GraphObject::up)
        y = y + 1;
    else if (dir == GraphObject::down)
        y = y - 1;
    else if (dir == GraphObject::left)
        x = x - 1;
    else if (dir == GraphObject::right)
        x = x + 1;
}

//used by "is_boulder" to check if the current location is occupied by a boulder
bool StudentWorld::is_occupied_by_boulder(int x, int y)
{
    vector<Actor*>::iterator it;
    for (it = live_object.begin(); it != live_object.end(); it++)
    {
        Boulder* is_another_boulder = dynamic_cast<Boulder*>(*it);
        if (is_another_boulder && (*it) -> is_active())
        {
            int another_boulder_x = (*it) -> getX();
            int another_boulder_y = (*it) -> getY();
            // check if the squirt will overlap with a boulder in next tick.
            if (y >= another_boulder_y && y <= (another_boulder_y+3) && x >= another_boulder_x && x <= another_boulder_x+3)
                return true;
        }
    }
    return false;
}

// function used by objects to check if there is boulders around within the distance
bool StudentWorld::is_within_radius_of_boulder(int x, int y, double radius)
{
    vector<Actor*>::iterator it;
    for (it = live_object.begin(); it != live_object.end(); it++)
    {
        Boulder* is_another_boulder = dynamic_cast<Boulder*>(*it);
        if (is_another_boulder && (*it)->is_active())
        {
            int another_boulder_x = (*it) -> getX();
            int another_boulder_y = (*it) -> getY();
            // check if the squirt will overlap with a boulder in next tick.
            if ((x-another_boulder_x)*(x-another_boulder_x)+(y-another_boulder_y)*(y-another_boulder_y) <= radius*radius)
                return true;
        }
    }
    return false;
}

// function used by objects to check if there is boulders around
bool StudentWorld::is_boulder(GraphObject * goodie, GraphObject::Direction dir, double distance, int x, int y)
{
    //two case:
    // if goodie!= nullptr, disgard x and y
    //1.
    //when distance is not specified(== -1.0), direction must have been specified
    //see if the nearby grid is occupied by the boulder, used by boulder & squirt & protester
    // for boulder, we only need to is next 4*1 lower dirt is occupied bu boulder
    // for squirt, it means the next square is occupied by a boulder
    // for protester, it means that the next 4*1 in that direction is occupied by a boulder. walls has already been checked.
    
    //2.
    //when distance is specified to be larger than 1, then direction is not specified(==none)
    //see if where is a boulder's coordinate inside the radius of distance( <=)
    
    //3.
    // if the goodie == nullptr, then current location is (x, y), neglect goodie, dir & distance, check is the current location is within the radius
    if ( goodie != nullptr && distance < 0 && dir != GraphObject::none)
    {
        x = goodie -> getX();
        y = goodie -> getY();
        Boulder* is_boulder = dynamic_cast<Boulder*>(goodie);
        Squirt* is_squirt = dynamic_cast<Squirt*>(goodie);
        Protester* is_protester = dynamic_cast<Protester*>(goodie);
        if (is_boulder)
        {
            vector<Actor*>::iterator it;
            for (it = live_object.begin(); it != live_object.end(); it++)
            {
                // check if the current boulder is the goodie itself.
                if ((*it) != goodie)
                {
                    Boulder* is_another_boulder = dynamic_cast<Boulder*>(*it);
                    // there is another boulder in the vector storing the objects;
                    if (is_another_boulder)
                    {
                        int another_boulder_x = (*it) -> getX();
                        int another_boulder_y = (*it) -> getY();
                        // check if this boulder will fall on another boulder.
                        if (another_boulder_y == (y-4) && another_boulder_x >= (x-3) && another_boulder_x <= (x+3))
                            return true;
                    }
                }
            }
            return false;
        }
        else if (is_squirt)
        {
            my_next_coordinate(x, y, dir);
            return is_occupied_by_boulder(x, y);
        }
        else if (is_protester)
        {
            if (dir == GraphObject::up)
            {
                y = y + 1;
                for (int i = 0; i < 4; i++)
                {
                    if (is_occupied_by_boulder(x+i, y))
                        return true;
                }
                return false;
            }
            else if (dir == GraphObject::down)
            {
                y = y - 1;
                for (int i = 0; i < 4; i++)
                {
                    if (is_occupied_by_boulder(x+i, y))
                        return true;
                }
                return false;
            }
            else if (dir == GraphObject::left)
            {
                x = x - 1;
                for (int i = 0; i < 4; i++)
                {
                    if (is_occupied_by_boulder(x, y + i))
                        return true;
                }
                return false;
            }
            else if (dir == GraphObject::right)
            {
                x = x + 1;
                for (int i = 0; i < 4; i++)
                {
                    if (is_occupied_by_boulder(x, y + i))
                        return true;
                }
                return false;
            }
            else return false;
        }
    }
    else if (distance > 0)
    {
        if (goodie != nullptr)
        {
            x = goodie -> getX();
            y = goodie -> getY();
        }
        // now we need to determine if (x,y) is within the radius of 3 of a boulder.
        return is_within_radius_of_boulder(x, y, distance);
    }
    return false;
}

// bribe the protester.
// only need to check is there s protester nearby and call protester's get_bribed function,
// return 0 if not bribe any protester, return 1 if bribe a regular protester, return 2 is bribe a hardcore protester
int StudentWorld::bribe_protester(GraphObject * goodie, double distance)
{
    int gold_x = goodie -> getX();
    int gold_y = goodie -> getY();
    vector<Actor*>::iterator it;
    Protester *pro = nullptr;
    for (it = live_object.begin(); it != live_object.end(); it++)
    {
        if ((gold_x - (*it)->getX())*(gold_x - (*it)->getX()) + (gold_y - (*it)->getY())*(gold_y - (*it)->getY()) <= distance*distance)
        {
            pro = dynamic_cast<Protester*>(*it);
            if (pro!=nullptr)
                break;
        }
    }
    Hardcore_Protester *hardcore = dynamic_cast<Hardcore_Protester*>(pro);
    // bribed a hardcore protester;
    if (hardcore)
    {
        pro -> get_bribed();
        return 2;
    }
    //bribed a regular protester;
    else if (pro)
    {
        pro -> get_bribed();
        return 1;
    }
    // not bribed a protester.
    else return 0;
}

// annoy the protester by the goodie.
bool StudentWorld::annoy_protesters(GraphObject* goodie, double distance)
{
    // if there are protesters within the distance, reduce the points amount of hit_points and
    // if the protester die, add some points points for the frackman.
    // return true if at least one protester is annoyed.
    // 1. boulder annoye protesters, hitpoints = 100, bonus_point = 500
    // 2. squirt annoy protesters, hitpoints = 2, bonuspoint = 100 (regular), bonus_point = 250(hardcore)
    int gold_x = goodie -> getX();
    int gold_y = goodie -> getY();
    vector<Actor*>::iterator it;
    Protester *pro = nullptr;
    Hardcore_Protester *hardcore = nullptr;
    Boulder *is_boulder = dynamic_cast<Boulder*>(goodie);
    Squirt *is_squirt = dynamic_cast<Squirt*>(goodie);
    bool is_annoyed = 0;
    for (it = live_object.begin(); it != live_object.end(); it++)
    {
        if ((gold_x - (*it)->getX())*(gold_x - (*it)->getX()) + (gold_y - (*it)->getY())*(gold_y - (*it)->getY()) <= distance*distance && (*it) -> is_active())
        {
            pro = dynamic_cast<Protester*>(*it);
            hardcore = dynamic_cast<Hardcore_Protester*>(*it);
            //annoy a protester
            if (pro!=nullptr && !pro -> is_in_leaving_state())
            {
                if (is_boulder) // the goodie is a boulder.
                {
                    pro -> get_annoyed(100);
                    increaseScore(500);
                }
                else if (is_squirt) // the goodie is a squirt
                {
                    pro -> get_annoyed(2);
                    if (pro ->is_in_leaving_state()) // after annoyed, the protester die.
                    {
                        if (hardcore)
                            increaseScore(250);
                        else increaseScore(100);
                    }
                }
                is_annoyed = 1;
            }
                
        }
    }
    if (is_annoyed)
        return true;
    else return false;
}

// determine if the player is within the distance
bool StudentWorld::is_player_near(GraphObject* goodie, double distance)
{
    int player_x = player -> getX();
    int player_y = player -> getY();
    int goodie_x = goodie -> getX();
    int goodie_y = goodie -> getY();
    double real_distance = (goodie_x - player_x)*(goodie_x - player_x) + (goodie_y - player_y)*(goodie_y - player_y);
    if (real_distance <= (distance*distance))
        return true;
    else return false;
}

// determine if the goodie is facing the player
bool StudentWorld::is_facing_player(GraphObject* goodie)
{
    int player_x = player -> getX();
    int player_y = player -> getY();
    int goodie_x = goodie -> getX();
    int goodie_y = goodie -> getY();
    GraphObject::Direction goodie_dir = goodie -> getDirection();
    if (goodie_dir == GraphObject::right && goodie_y>=(player_y-4) && goodie_y<=(player_y+4) && player_x >= goodie_x)
        return true;
    else if (goodie_dir==GraphObject::left && goodie_y>=(player_y-4) && goodie_y<=(player_y+4) && player_x <= goodie_x)
        return true;
    else if (goodie_dir == GraphObject::up && goodie_x>=(player_x-4) && goodie_x<=(player_x+4) && player_y >= goodie_y)
        return true;
    else if (goodie_dir == GraphObject::down && goodie_x>=(player_x-4) && goodie_x<=(player_x+4) && player_y <= goodie_y)
        return true;
    else return false;
}

// create a squirt object for frackman, new a Squirt and insert it to the vector
void StudentWorld::create_squirt(int x, int y, GraphObject::Direction dir)
{
    object = new Squirt(this, x, y, dir);
    live_object.push_back(object);
}

//  create a temporary gold for frackman with lifetime 100.
void StudentWorld::create_temporary_gold(int x, int y)
{
    object = new Gold(this, x, y, Goodie_with_lifetime_and_state::temporary);
    live_object.push_back(object);
}

//  create a sonar inside the game field.
void StudentWorld::create_sonar()
{
    object = new Sonar(this, 0, 60);
    live_object.push_back(object);
}

// create a water pool inside the game field at dirtless field.
void StudentWorld::create_pool()
{
    int object_x = rand()%61;
    int object_y = rand()%37 + 20;
    while (is_current_16square_occupied_by_dirt(object_x, object_y))
    {
        object_x = rand()%61;
        object_y = rand()%37 + 20;
    }
    object = new Pool(this, object_x, object_y);
    live_object.push_back(object);
}

// Determine should the created protester be hardcore or regular at this moment by calculating probability
bool StudentWorld::Hardcore_or_Regular()
{
    // generate a random number between 0 - 99
    int rand_num = rand()%100;
    if (rand_num >= 0 && rand_num < probabilityofHardcore)
        return true; // hardcore
    else return false; //regular
}

// Determine should a sonar or pool be created at this moment by calculating probability
int StudentWorld::Sonar_or_Pool()
{
    int rand_num = rand()%probabilityofPoolSonar;
    // we should add a sonar or pool at this tick.
    if (rand_num == 0)
    {
        rand_num = rand()%5;
        if (rand_num == 0)
            return 1; // we should add a sonar;
        else return 2; // we should add a water pool
    }
    // we should not add a sonar or pool
    else return 0;
}

// create a protester inside the oil field.
void StudentWorld::create_protester()
{
    if (Hardcore_or_Regular())
        object = new Hardcore_Protester(this, 60, 60);
    else object = new Protester(this, 60, 60);
    live_object.push_back(object);
    num_of_protesters++;
    tick_for_adding_protester = num_of_ticks_between_protesters;
}

// set all hidden game object(gold&barrel) within the radius of 12(<=12)to visible
void StudentWorld::use_sonar(FrackMan * frackman)
{
    int player_x = frackman -> getX();
    int player_y = frackman -> getY();
    vector<Actor*>::iterator it;
    for (it = live_object.begin(); it != live_object.end(); it++)
    {
        if ((player_x - (*it)->getX())*(player_x - (*it)->getX()) + (player_y - (*it)->getY())*(player_y - (*it)->getY()) <= 12*12 && (*it) -> is_active())
        {
            Gold *is_gold = dynamic_cast<Gold*>(*it);
            Barrel *is_barrel = dynamic_cast<Barrel*>(*it);
            if (is_gold || is_barrel)
                (*it) -> set_me_visible();
        }
    }
}

// check to see of the protester can move straight to the frackman without any dirt or boulder blocking
bool StudentWorld::can_move_straight_to_frackman(GraphObject * object, GraphObject::Direction &dir)
{
    int player_x = player -> getX();
    int player_y = player -> getY();
    int object_x = object -> getX();
    int object_y = object -> getY();
    GraphObject * temp = new GraphObject(IID_DIRT, object_x, object_y);
    if (object_y == player_y && object_x <= player_x)
    {
        dir = GraphObject::right;
        // if protester can move entire way to the frackman, return true.
        while (temp -> getX() != player_x)
        {
            if (is_dirt(temp, dir) || is_boulder(temp, GraphObject::none, 3.0))
            {
                delete temp;
                return false;
            }
            else temp -> moveTo(temp -> getX()+1, temp -> getY());
        }
        delete temp;
        return true;
    }
    else if (object_y == player_y && object_x >= player_x) // left
    {
        dir = GraphObject::left;
        while (temp -> getX() != player_x)
        {
            if (is_dirt(temp, dir) || is_boulder(temp, GraphObject::none, 3.0))
            {
                delete temp;
                return false;
            }
            else temp -> moveTo(temp -> getX()-1, temp -> getY());
        }
        delete temp;
        return true;
    }
    else if (object_y <= player_y && object_x == player_x) // up
    {
        dir = GraphObject::up;
        while (temp -> getY() != player_y)
        {
            if (is_dirt(temp, dir) || is_boulder(temp, GraphObject::none, 3.0))
            {
                delete temp;
                return false;
            }
            else temp -> moveTo(temp-> getX(), temp -> getY()+1);
        }
        delete temp;
        return true;
    }
    else if (object_y >= player_y && object_x == player_x)
    {
        dir = GraphObject::down;
        while (temp -> getY() != player_y)
        {
            if (is_dirt(temp, dir) || is_boulder(temp, GraphObject::none, 3.0))
            {
                delete temp;
                return false;
            }
            else temp -> moveTo(temp -> getX(), temp -> getY()-1);
        }
        delete temp;
        return true;
    }
    delete temp;
    return false;
}

// if facing the dir direction, get my left direction
GraphObject::Direction StudentWorld::my_left(GraphObject::Direction dir)
{
    switch (dir)
    {
        case GraphObject::up:
            return GraphObject::left;
        case GraphObject::left:
            return GraphObject::down;
        case GraphObject::down:
            return GraphObject::right;
        case GraphObject::right:
            return GraphObject::up;
        default:
            return GraphObject::none;
    }
}

// if facing the dir direction, get my right direction
GraphObject::Direction StudentWorld::my_right(GraphObject::Direction dir)
{
    switch (dir)
    {
        case GraphObject::up:
            return GraphObject::right;
        case GraphObject::left:
            return GraphObject::up;
        case GraphObject::down:
            return GraphObject::left;
        case GraphObject::right:
            return GraphObject::down;
        default:
            return GraphObject::none;
    }
}

// set the player to dead.
void StudentWorld::set_player_dead()
{
    player -> set_me_inactive();
}

// get the max value among x and y
int StudentWorld::max(int x, int y)
{
    if (x >= y)
        return x;
    else return y;
}

// get the min value among x and y
int StudentWorld::min(int x, int y)
{
    if (x >= y)
        return y;
    else return x;
}

// check if there is an object within the distance of 6 from (x, y), used for distributing object in init().
bool StudentWorld::is_object_within_distance_six(int x, int y)
{
    vector<Actor*>::iterator it;
    for (it = live_object.begin(); it!=live_object.end(); it++)
    {
        if (((*it) -> getX() - x)*((*it) -> getX() - x) + ((*it) -> getY() - y)*((*it) -> getY() - y) <= 36.0)
            return true;
    }
    return false;
}

// transfer int to string and filled the empty bits with zeros
string StudentWorld::int_to_string_with_leading_zeros(int x, int digits)
{
    string result_string;
    int remain = x;
    while (digits > 0)
    {
        int s = remain % 10;
        remain = remain/10;
        result_string += ('0' + s);
        digits--;
    }
    reverse(result_string.begin(), result_string.end());
    return result_string;
}

// transfer int to string and filled the empty bits with spaces
string StudentWorld::int_to_string_with_leading_space(int x, int digits)
{
    if (digits == 3)
    {
        if (x < 100)
            return (" " + to_string(x));
        else return (to_string(x));
    }
    if (digits == 2)
    {
        if (x < 10)
            return (" " + to_string(x));
        else return (to_string(x));
    }
    else return to_string(x);
}

// update the text appear at the top of the game window
void StudentWorld::updateDisplayText()
{
    int score = getScore();
    int level = getLevel();
    int lives = getLives();
    int health = getCurrentHealth() * 10;
    int squirts = getSquirtsLeftInSquirtGun();
    int gold = getPlayerGoldCount();
    int sonar = getPlayerSonarChargeCount();
    int barrelsleft = getNumOfBarrelRemained();
    
    string s = "Scr:" + int_to_string_with_leading_zeros(score, 8) + "  Lvl:" + int_to_string_with_leading_space(level, 2) + "  Lives:" + int_to_string_with_leading_space(lives, 1) + "  Hlth:" + int_to_string_with_leading_space(health, 3) + "%  Wtr:" + int_to_string_with_leading_space(squirts, 2) + "  Gld:" + int_to_string_with_leading_space(gold, 2) + "  Sonar:" + int_to_string_with_leading_space(sonar, 2) + "  Oil Left:" + int_to_string_with_leading_space(barrelsleft, 2);
    
    setGameStatText(s);
}

// delete the dead objects in the vector
void StudentWorld::removeDeadGameObject()
{
    vector<Actor*>::iterator it = live_object.begin();
    while (it!=live_object.end())
    {
        if (!(*it)->is_active())
        {
            Protester* is_protester = dynamic_cast<Protester*>(*it);
            if (is_protester)
                protester_die();
            delete (*it);
            it = live_object.erase(it);
        }
        else it++;
    }
}

//init function
int StudentWorld::init()
{
    // initialize the dirt objects
    srand(time(NULL));
    num_of_Boulders = min(getLevel()/2 + 2, 6);
    num_of_Golds = max((5-getLevel()/2), 2);
    num_of_Barrel = min((2+getLevel()), 20);
    num_of_ticks_between_protesters = max(25, (200 - getLevel()));
    max_num_of_protesters = min(15, (2 + getLevel()*1.5));
    probabilityofHardcore = min(90, (getLevel()*10 + 30));
    probabilityofPoolSonar = getLevel()*25 + 300;
    current_barrel_num = 0;
    num_of_protesters = 0;
    
    tick_for_adding_protester = 0;
    // object's x is [0, 60], y is [20, 56]
    for (int i = 0 ; i < DIRT_HEIGHT; i++)
    {
        for (int j = 0; j < DIRT_WIDTH; j++)
        {
            dirt_map[i][j] = new Dirt(j,i);
            if (j >= 30 && j <= 33 && i >= 4 && i <= 59)
                dirt_map[i][j]->set_me_inactive();
        }
    }
    // create the frackman
    player = new FrackMan(this);
    // create boulders.
    for (int i = 0; i < num_of_Boulders; i++)
    {
        int object_x = rand()%61;
        int object_y = rand()%37 + 20;
        while (is_object_within_distance_six(object_x, object_y) || (object_x >26 && object_x < 34))
        {
            object_x = rand()%61;
            object_y = rand()%37 + 20;
        }
        object = new Boulder(this, object_x, object_y);
        live_object.push_back(object);
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                remove_dirt(object_x+j,object_y+k);
            }
        }
    }
    // create gold
    for (int i = 0; i < num_of_Golds; i++)
    {
        int object_x = rand()%61;
        int object_y = rand()%37 + 20;
        while (is_object_within_distance_six(object_x, object_y) || (object_x >26 && object_x < 34))
        {
            object_x = rand()%61;
            object_y = rand()%37 + 20;
        }
        object = new Gold(this, object_x, object_y, Goodie_with_lifetime_and_state::permanent);
        live_object.push_back(object);
    }
    //create barrel of oil
    for (int i = 0; i < num_of_Barrel; i++)
    {
        int object_x = rand()%61;
        int object_y = rand()%37 + 20;
        while (is_object_within_distance_six(object_x, object_y) || (object_x >26 && object_x < 34))
        {
            object_x = rand()%61;
            object_y = rand()%37 + 20;
        }
        object = new Barrel(this, object_x, object_y);
        live_object.push_back(object);
    }
    
    //add a new Protester(this, 60, 60);
    create_protester();
    
    return GWSTATUS_CONTINUE_GAME;
}

//move function
int StudentWorld::move()
{
    updateDisplayText();
    vector<Actor*>::iterator it;
    // update the tick counting for adding protester.
    if (tick_for_adding_protester > 0)
        tick_for_adding_protester--;
    // add protesters to the field periodically
    if (tick_for_adding_protester ==0 && num_of_protesters < max_num_of_protesters)
        create_protester();
    // 1. we should add a sonar;
    // 2. we should add a water pool
    if (Sonar_or_Pool() == 1)
        create_sonar();
    else if (Sonar_or_Pool() == 2)
        create_pool();
    for(it = live_object.begin(); it != live_object.end(); it++)
    {
        if ((*it) -> is_active())
        {
            (*it) -> doSomething();
            // Player dies.
            if (!player -> is_active())
            {
                decLives();
                GameController::getInstance().playSound(SOUND_PLAYER_GIVE_UP);
                return GWSTATUS_PLAYER_DIED;
            }
            // player finish the current level
            else if (num_of_Barrel == current_barrel_num)
            {
                GameController::getInstance().playSound(SOUND_FINISHED_LEVEL);
                return GWSTATUS_FINISHED_LEVEL;
            }
        }
    }
    removeDeadGameObject();
    player -> doSomething();
    if (!player->is_active())
    {
        decLives();
        GameController::getInstance().playSound(SOUND_PLAYER_GIVE_UP);
        return GWSTATUS_PLAYER_DIED;
    }
    else if (num_of_Barrel == current_barrel_num)
    {
        GameController::getInstance().playSound(SOUND_FINISHED_LEVEL);
        return GWSTATUS_FINISHED_LEVEL;
    }
    else
        return GWSTATUS_CONTINUE_GAME;
}


//cleanup function
void StudentWorld::cleanUp()
{
    current_barrel_num = 0;
    // delete the dirt objects
    for (int i = 0 ; i < DIRT_HEIGHT; i++)
    {
        for (int j = 0; j < DIRT_WIDTH; j++)
        {
            delete dirt_map[i][j];
            dirt_map[i][j] = nullptr;
        }
    }
    delete player;
    player = nullptr;
    
    vector<Actor*>::iterator it = live_object.begin();
    
    while (it!=live_object.end())
    {
        delete (*it);
        it = live_object.erase(it);
    }
    //delete p;
}
