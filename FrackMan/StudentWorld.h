#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include "GameController.h"
#include <vector>

const int DIRT_WIDTH = 64;
const int DIRT_HEIGHT = 60;
const int OBJECT_LOCATION_LIMIT = 60;


class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
	 : GameWorld(assetDir), current_barrel_num(0)
    {
	}
    
    // destructor to free memory if cleanup function interrupted
    virtual ~StudentWorld();

    virtual int init();

    virtual int move();

    virtual void cleanUp();
    
    //set the corresonding dirt to inactive and return true if the remove operation is done.
    bool remove_dirt(int x, int y);
    
    //used for boulder to check if there is any Dirt in the 4 squares immediately below it; also for squirt
    // also for other objects, to check the 4 squares in front of the object.
    bool is_dirt(GraphObject* goodie, GraphObject::Direction dir);
    
    //used to check if the current location is occupied by a dirt.
    bool is_occupied_by_dirt(int x, int y);
    
    //precondition: the 16 squares is within the dirt bound.
    // check if one of the upper and right 4*4 square is occupied by a dirt
    bool is_current_16square_occupied_by_dirt(int x, int y);
    
    // determine if the player is within the distance
    bool is_player_near(GraphObject* goodie, double distance);
    
    // determine if the goodie is facing the player
    bool is_facing_player(GraphObject* goodie);
    
    //used by "is_boulder" to check if the current location is occupied by a boulder
    bool is_occupied_by_boulder(int x, int y);
    
    // function used by objects to check if there is boulders around within the distance
    bool is_within_radius_of_boulder(int x, int y, double radius);
    
    //function used by objects to check if there is boulders around
    bool is_boulder(GraphObject * goodie = nullptr, GraphObject::Direction dir = GraphObject::none, double distance = -1.0, int x = -1, int y = -1);
    
    // set the player to dead.
    void set_player_dead();
    
    // update x and y if moving one step in the current dir.
    void my_next_coordinate(int &x, int &y, GraphObject::Direction dir);
    
    // move the goodie for one step in the current direction.
    void move_current_dir(GraphObject * goodie, GraphObject::Direction dir);
    
    bool annoy_protesters(GraphObject* goodie, double distance);
    // if there are protesters within the distance, reduce the points amount of hit_points and
    // if the protester die, add some points points for the frackman.
    // return true if at least one protester is annoyed.
    // 1. boulder annoye protesters, hitpoints = 100, bonus_point = 500
    // 2. squirt annoy protesters, hitpoints = 2, bonuspoint = 100 (regular), bonus_point = 250(hardcore)
    
    // used by frackman to check whether it's in a straight line of sight to frackman without dirt ir boulder
    bool can_move_straight_to_frackman(GraphObject * object, GraphObject::Direction &dir);
    
    // create a squirt object for frackman, new a Squirt and insert it to the vector
    void create_squirt(int x, int y, GraphObject::Direction dir);
    
    // create a temporary gold for frackman with lifetime 100.
    void create_temporary_gold(int x, int y);
    
    // create a Hardcore/regular protester for the world.
    void create_protester();
    
    // create a sonar at the left corner of the world;
    void create_sonar();
    
    // create a pool at a random dirt-less spot;
    void create_pool();
    
    //set all hidden game object(gold&barrel) with in the radius of 12(<=12)to visible
    void use_sonar(FrackMan * frackman);
    
    void get_player_position(int &x, int &y)
    {
        x = player -> getX();
        y = player -> getY();
    }
    
    // annoy the protester by the goodie.
    void annoy_frackman(int point)
    {
        player -> get_annoyed(point);
    }
    
    // bribe the protester.
    // only need to check is there s protester nearby and call protester's get_bribed function,
    // return 0 if not bribe any protester, return 1 if bribe a regular protester, return 2 is bribe a hardcore protester
    int bribe_protester(GraphObject * goodie, double distance);
    
    void Pick_Barrel()
    {
        current_barrel_num++;
    }
    
    void Inc_player_gold()
    {
        player -> Inc_gold();
    }
    
    void Inc_player_sonar()
    {
        player -> Inc_sonar();
    }
    
    void Inc_player_water()
    {
        player -> Inc_water();
    }
    
    // get the max value among x and y
    int max(int x, int y);
    
    // get the min value among x and y
    int min(int x, int y);
    
    // check if there is an object within the distance of 6 from (x, y), used for distributing object in init().
    bool is_object_within_distance_six(int x, int y);
    
    // update the text appear at the top of the game window
    void updateDisplayText();
    
    // Determine should the created protester be hardcore or regular at this moment by calculating probability
    bool Hardcore_or_Regular();
    
    // Determine should a sonar or pool be created at this moment by calculating probability
    int Sonar_or_Pool();
    
    //return the hit_point of the health
    int getCurrentHealth()
    {
        return player -> my_hit_point();
    }
    
    int getSquirtsLeftInSquirtGun()
    {
        return player -> my_squirt();
    }
    
    int getPlayerSonarChargeCount()
    {
        return player -> my_sonar();
    }
    
    int getPlayerGoldCount()
    {
        return player -> my_gold();
    }
    
    int getNumOfBarrelRemained()
    {
        return (num_of_Barrel - current_barrel_num);
    }

    void protester_die()
    {
        num_of_protesters--;
    }
    
    // if facing the dir direction, get my left direction
    GraphObject::Direction my_left(GraphObject::Direction dir);
    
    // if facing the dir direction, get my right direction
    GraphObject::Direction my_right(GraphObject::Direction dir);
    
    // transfer int to string and filled the empty bits with zeros
    std::string int_to_string_with_leading_zeros(int x, int digits);
    
    // transfer int to string and filled the empty bits with spaces
    std::string int_to_string_with_leading_space(int x, int digits);
    
    // delete the dead objects in the vector
    void removeDeadGameObject();
    
private:
    FrackMan* player;
    //dirt_map[y][x]
    Dirt* dirt_map[DIRT_HEIGHT][DIRT_WIDTH];
    
    std::vector<Actor*> live_object;
    
    // number of barrel that is picked up by the player. If all picked up, advance to the nect level
    int current_barrel_num;
    Actor *object; // created for testing
    
    // number of boulders, gold, barrel in each level, initialized in init()
    int num_of_Boulders, num_of_Golds, num_of_Barrel;
    //T is the number of ticks before a new protester; P is the number of protesters in the oil field.
    int num_of_ticks_between_protesters, max_num_of_protesters;
    int probabilityofHardcore, probabilityofPoolSonar;
    int tick_for_adding_protester;
    int current_tick;
    int num_of_protesters;
};

#endif // STUDENTWORLD_H_
