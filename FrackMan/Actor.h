#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
class StudentWorld;


//base class for all objects in the game, including Dirt
class Actor:public GraphObject
{
public:
    Actor(StudentWorld* world, int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0):GraphObject(imageID, startX, startY, dir, size, depth), active(1), my_world(world){}
    
    virtual ~Actor(){}
    
    // doSomething function
    virtual void doSomething(){}
    
    // set this object to be visible
    void set_me_visible()
    {
        setVisible(true);
    }
    
    // set this object to dead and also make it invisible
    void set_me_inactive()
    {
        active = false;
        setVisible(false);
    }
    
    // check if the current object is alive
    bool is_active()
    {
        return active;
    }
    
    // return the pointer to the world where this object exists
    StudentWorld* getWorld()
    {
        return my_world;
    }
    
private:
    //indicator for active or inactive during the current tick.
    bool active;
    // the pointer to the world
    StudentWorld* my_world;
};




//Dirt class
class Dirt:public Actor
{
public:
    Dirt(int startX, int startY):Actor(nullptr, IID_DIRT, startX, startY, right, 0.25, 3)
    {
        set_me_visible();
    }
    
    virtual ~Dirt(){}
};



//base class for all pickupable objects in the game
class Goodie:public Actor
{
public:
    Goodie(StudentWorld * world, int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0):Actor(world, imageID, startX, startY, dir, size, depth){}
    
    virtual ~Goodie(){}
    
    //operations performed after the goodie is picked up, add bonus point to the player and play the sound.
    void pick_up(int bonus_point, int soundID);
};


// base classes for objects with lifetime and state in the game
class Goodie_with_lifetime_and_state:public Goodie
{
public:
    // stable, waiting and falling describes the state of a boulder, permanent and temporary describes the state of a gold
    enum State {stable, waiting, falling, permanent, temporary, others};
    Goodie_with_lifetime_and_state(StudentWorld * world, int imageID, int startX, int startY, Direction dir = right, double size = 1.0, unsigned int depth = 0, int time = -1, State s = temporary):Goodie(world, imageID, startX, startY, dir, size, depth), lifetime(time), my_state(s){}
    
    virtual ~Goodie_with_lifetime_and_state(){}
   
    // decrement the lifetime if allowed
    bool dec_lifetime()
    {
        if (lifetime >= 1)
        {
            lifetime--;
            return true;
        }
        else return false;
    }
    
    // set the lifetime
    void set_lifetime(int time)
    {
        lifetime = time;
    }
    
    // return my current state
    State current_state()
    {
        return my_state;
    }
    // set my current state
    void set_state(State s)
    {
        my_state = s;
    }
private:
    // record my current lifetime
    int lifetime;
    // indicate my current state
    State my_state;
};


// Boulder class
class Boulder:public Goodie_with_lifetime_and_state
{
public:
    //startX and startY here nust be specified by studentworld correctly, from (0,0) to (60,56)
    Boulder(StudentWorld * world, int startX, int startY):Goodie_with_lifetime_and_state(world, IID_BOULDER, startX, startY, down, 1.0, 0, 30, stable)
    {
        set_me_visible();
    }
    
    virtual ~Boulder(){}
    
    // boulder's doSomething
    virtual void doSomething();
};


// Squirt class
class Squirt:public Goodie
{
public:
    Squirt(StudentWorld * world, int startX, int startY, Direction dir):Goodie(world, IID_WATER_SPURT, startX, startY, dir, 1.0, 1),travel_distance(4)
    {
        set_me_visible();
    }
    
    virtual ~Squirt(){}
    
    // Squirt's doSomething
    virtual void doSomething();
    
private:
    // record squirt's travel distance (0-4)
    int travel_distance;
};


//Barrel class
class Barrel:public Goodie
{
public:
    Barrel(StudentWorld * world, int startX, int startY):Goodie(world, IID_BARREL, startX, startY, right, 1.0, 2){}
    
    virtual ~Barrel(){}
    
    // Barrel's doSomething
    virtual void doSomething();
};

// Gold class
class Gold:public Goodie_with_lifetime_and_state
{
public:
    Gold(StudentWorld *world, int startX, int startY, State s = permanent):Goodie_with_lifetime_and_state(world, IID_GOLD, startX, startY, right, 1.0, 2, 100, s)
    {
        if (current_state() == temporary)
            set_me_visible();
    }
    
    virtual ~Gold(){}
    
    // Gold's doSomething
    virtual void doSomething();
};


//base class for sonar and water pool
class Temporary_Goodie:public Goodie_with_lifetime_and_state
{
public:
    Temporary_Goodie(StudentWorld * world, int imageID, int startX, int startY);
    
    virtual ~Temporary_Goodie(){}
    
    virtual void doSomething();
    
private:
    // the part that sonar and water pool implements differently, which is called by doSomething()
    virtual void update_world() = 0;
};


// Sonar class
class Sonar:public Temporary_Goodie
{
public:
    Sonar(StudentWorld * world, int startX, int startY):Temporary_Goodie(world, IID_SONAR, startX, startY){}
    
    virtual ~Sonar(){}
    
private:
    virtual void update_world();
};


// Water Pool class
class Pool:public Temporary_Goodie
{
public:
    Pool(StudentWorld * world, int startX, int startY):Temporary_Goodie(world, IID_WATER_POOL, startX, startY){}
    
    virtual ~Pool(){}
    
private:
    virtual void update_world();
};


//base class for all kinds of people
class People:public Actor
{
public:
    People(StudentWorld * world, int imageID, int startX, int startY, int point, Direction dir = right, double size = 1.0, unsigned int depth = 0):Actor(world, imageID, startX, startY, dir, size, depth), hit_point(point){}
    
    virtual ~People(){}
    
    // pure virtual function, do what a people should do when get annoyed.
    virtual void get_annoyed(int reduced_points) = 0;
    
    // return this people's current hit points
    int my_hit_point()
    {
        return hit_point;
    }
    
protected:
    // reduce this people's hit point by a certain ammount, update the people's state is needed(this people give up when hit points drops to <=0).
    void reduce_hit_point(int reduced_point)
    {
        hit_point = hit_point - reduced_point;
        if (hit_point <= 0)
            give_up();
    }
    
private:
    // do what this people should do when hit points drops to 0 or below
    virtual void give_up() = 0;
    // record my current hit point
    int hit_point;
};


// Regular Protester class
class Protester:public People
{
public:
    // struct used for the breath_first_search function to find the shortest path
    struct maze_coordinate
    {
        int currentX;
        int currentY;
        int prevX;
        int prevY;
        maze_coordinate(int x1, int y1, int x2, int y2):currentX(x1), currentY(y1), prevX(x2), prevY(y2){}
    };
    
    // describes the state of the protester
    enum ProtesterState {leave_the_oil_field, resting, non_resting};
    
    Protester(StudentWorld * world, int startX, int startY, int hit_point = 5, int imageID = IID_PROTESTER);
    
    virtual ~Protester(){}
    
    // protester's doSomething
    virtual void doSomething();
    
    // do what a protester should do when get bribed by a gold, different for regular protester and hardcore protester, thus, virtual
    virtual void get_bribed();
    
    // do what a protester should do when get annoyed by a squirt or a boulder
    virtual void get_annoyed(int reduced_points);
    
    // Set this protester staring at the frackman after yelling to the frackman
    void set_me_stared();
    
    // check if this protester is in the leave_the_oil_field state
    bool is_in_leaving_state()
    {
        return (my_state == leave_the_oil_field);
    }
    
    // function to implement bfs and return the current node which contains next step's information
    maze_coordinate breath_first_search(int startX, int startY, int destX, int destY, int &steps);
    
private:
    // since this is a regular protester, it can not find the frackman as the hardcore protester does.
    virtual bool find_frackman_and_move()
    {
        return false;
    }
    
    // make a perpendicular turn if allowed
    void turn_perpendicular();
    
    // protester's version give_up, which means set my state the leave_the_oil_field and leave.
    virtual void give_up()
    {
        my_state = leave_the_oil_field;
    }
    
    // shout at the frackman
    void shout();
    // set this protester to a rest state after bribed
    void set_me_rest();
    // check to see if this protester's is at rest state.
    bool is_rest();
    // Protester's doSomething when it is at a non-resting tick
    void Actual_doSomething();
    // variable to indicate this protester's state
    ProtesterState my_state;
    // variable to indicate how many steps can this protester move in its current direction
    int numSquaresToMove;
    // vairable to indicate between how many tick can this protester doSomething
    int ticksToWaitBetweenMoves;
    // variable to record the current tick since the game begins
    int current_tick;
    // variabe to indicate how many ticks does this protester left do exit the resting state
    int resting_tick;
    // variable to indicate how many ticks does this protester left to allow it shouting at the frackman again
    int shouting_tick;
    // variable to indicate how many ticks does this protester left to allow it making a perpendicular turn again.
    int perpendicular_tick;
    // used by the hardcore protester to check the staring status, stared_tick !=0 means at staring status.
    int stared_tick;
};


// Hardcore protester class
class Hardcore_Protester:public Protester
{
public:
    Hardcore_Protester(StudentWorld *world, int startX, int startY):Protester(world, startX, startY, 20, IID_HARD_CORE_PROTESTER){}
    
    virtual ~Hardcore_Protester(){}
    
    // hardcore protester's version of get_bribed
    virtual void get_bribed();
    
private:
    // if the frackman is close to the hardcore protester, use bfs to find the path and move one square closer.
    virtual bool find_frackman_and_move();
};


// Frackman class
class FrackMan:public People
{
public:
    FrackMan(StudentWorld *world):People(world, IID_PLAYER, 30, 60, 10, right, 1.0, 0)
    {
        set_me_visible();
        squirt = 5;
        sonar = 1;
        gold = 0;
    }
    
    virtual ~FrackMan(){}
    
    // Frackman's doSomething
    virtual void doSomething();
    
    // return the number of squirt left
    int my_squirt()
    {
        return squirt;
    }
    
    // return the number of sonars left
    int my_sonar()
    {
        return sonar;
    }
    
    // return the number of golds left
    int my_gold()
    {
        return gold;
    }
    
    // update the gold number after picking up a gold
    void Inc_gold()
    {
        gold++;
    }
   
    // update the sonar number after picking up a sonar
    void Inc_sonar()
    {
        sonar = sonar + 2;
    }
    
    // update the squirt number after picking up a water pool
    void Inc_water()
    {
        squirt = squirt + 5;
    }
    
    // frackman's version of get_annoyed
    virtual void get_annoyed(int reduced_points);
    
private:
    // check to see if I can fire a squirt at current location and update x, y to the position to start a squirt if true.
    bool find_my_squirt_position(int &x, int &y, Direction dir);
    // check to see if I move one square forward in current direction
    bool can_move_forward();
    // frackman's version of give_up, which is to set it to dead.
    virtual void give_up()
    {
        set_me_inactive();
    }
    // record the number of squirts, sonars, and golds left
    int squirt;
    int sonar;
    int gold;
};

#endif // ACTOR_H_
