#include "Actor.h"
#include "StudentWorld.h"
#include "GameController.h"
#include <queue>

//operations performed after the goodie is picked up, add bonus point to the player and play the sound.
void Goodie::pick_up(int bonus_point, int soundID)
{
    set_me_inactive();
    GameController::getInstance().playSound(soundID);
    getWorld() -> increaseScore(bonus_point);
}

// Boulder's doSomething
void Boulder::doSomething()
{
    if (!is_active())
        return;
    else
    {
        if (current_state() == stable)
        {
            if (!getWorld() -> is_dirt(this, down))
                set_state(waiting);
        }
        else if (current_state() == waiting)
        {
            if (!dec_lifetime()) // decrement lifetime by 1 for 30 ticks
            {
                set_state(falling);
                GameController::getInstance().playSound(SOUND_FALLING_ROCK);
            }
        }
        else if (current_state() == falling)
        {
            if (getWorld() -> is_player_near(this, 3.0))
            {
                getWorld() -> set_player_dead();
                return;
            }
            else
                getWorld() -> annoy_protesters(this, 3.0);
            // determine whether to move downward one square
            if (this->getY() == 0) // hit the bottom of the oil field
                set_me_inactive();
            else if (getWorld() -> is_boulder(this, down) == 1)
                set_me_inactive();
            else if (getWorld() -> is_dirt(this, down))
                set_me_inactive();
            else moveTo(getX(), getY()-1);
        }
    }
}

// Squirt's doSomething
void Squirt::doSomething()
{
    if (!is_active())
        return;
    if (getWorld()->annoy_protesters(this, 3.0))
        set_me_inactive();
    else if (travel_distance == 0)
        set_me_inactive();
    else if (getWorld()->is_dirt(this, getDirection()) || getWorld() -> is_boulder(this,getDirection()))
        set_me_inactive();
    else
    {
        getWorld() -> move_current_dir(this, getDirection());
        travel_distance--;
    }
}

// Barrel's doSomething
void Barrel::doSomething()
{
    if (!is_active())
        return;
    else if (getWorld() -> is_player_near(this, 4.0) && (!getWorld() -> is_player_near(this, 3.0)))
    {
        set_me_visible();
        return;
    }
    else if (getWorld() -> is_player_near(this, 3.0)) // activate the barrel
    {
        pick_up(1000, SOUND_FOUND_OIL);
        getWorld() -> Pick_Barrel();
    }
}

// Gold's doSomething
void Gold::doSomething()
{
    if (!is_active())
        return;
    else if (current_state() == permanent && getWorld() -> is_player_near(this, 4.0) && (!getWorld() -> is_player_near(this, 3.0)))
    {
        set_me_visible();
        return;
    }
    else if (current_state() == permanent && getWorld() -> is_player_near(this, 3.0))
    {
        pick_up(10, SOUND_GOT_GOODIE);
        getWorld() -> Inc_player_gold();
    }
    else if (current_state() == temporary)
    {
        int bribe_result = getWorld() -> bribe_protester(this, 3.0);
        if (bribe_result == 1)
            pick_up(25, SOUND_PROTESTER_FOUND_GOLD);
        else if (bribe_result == 2)
            pick_up(50, SOUND_PROTESTER_FOUND_GOLD);
        else
        {
            if (!dec_lifetime())
                set_me_inactive();
        }
    }
}

// Temporary_Goodie's constructor
Temporary_Goodie::Temporary_Goodie(StudentWorld * world, int imageID, int startX, int startY):Goodie_with_lifetime_and_state(world, imageID, startX, startY, right, 1.0, 2)
{
    set_me_visible();
    set_lifetime(getWorld()-> max(100, (300-10*getWorld()->getLevel())));
}

// Temporary_Goodie's doSomething
void Temporary_Goodie::doSomething()
{
    if (!is_active())
        return;
    else
    {
        if (getWorld() -> is_player_near(this, 3.0))
        {
            update_world();
        }
        else
        {
            if (!dec_lifetime())
                set_me_inactive();
        }
    }
}

// the part that sonar and water pool implements differently, which is called by doSomething()
void Sonar::update_world()
{
    pick_up(75, SOUND_GOT_GOODIE);
    getWorld() -> Inc_player_sonar();
}

// the part that sonar and water pool implements differently, which is called by doSomething()
void Pool::update_world()
{
    pick_up(100, SOUND_GOT_GOODIE);
    getWorld() -> Inc_player_water();
}

// do what a protester should do when get annoyed by a squirt or a boulder
void Protester::get_annoyed(int reduced_points)
{
    if (my_state == leave_the_oil_field)
        return;
    else
        reduce_hit_point(reduced_points);
    if (my_state != leave_the_oil_field)
    {
        GameController::getInstance().playSound(SOUND_PROTESTER_ANNOYED);
        set_me_rest();
    }
    else
    {
        GameController::getInstance().playSound(SOUND_PROTESTER_GIVE_UP);
        resting_tick = 0;
    }
}

// Protester's constructor
Protester::Protester(StudentWorld * world, int startX, int startY, int hit_point, int imageID):People(world, imageID, startX, startY, hit_point,left, 1.0, 0), my_state(non_resting), current_tick(0), resting_tick(0), shouting_tick(0), perpendicular_tick(200), stared_tick(0)
{
    //set numSquaresToMove
    numSquaresToMove = rand()%53 + 8; //set numSquaresToMove to 8 - 60
    set_me_visible();
    ticksToWaitBetweenMoves = getWorld() -> max(0, 3 - ((getWorld()->getLevel())/4));
}

// Set this protester staring at the frackman after yelling to the frackman
void Protester::set_me_stared()
{
    stared_tick = getWorld() -> max(50, 100 - 10*getWorld()->getLevel());
}

// set this protester to a rest state after bribed
void Protester::set_me_rest()
{
    my_state = resting;
    resting_tick = getWorld() -> max(50, 100 - 10*getWorld()->getLevel());
}

// check to see if this protester's is at rest state.
bool Protester::is_rest()
{
    if (my_state == resting && resting_tick != 0)
        return true;
    else if (my_state == resting && resting_tick == 0)
    {
        my_state = non_resting;
        return false;
    }
    else  return false;
}

// shout at the frackman
void Protester::shout()
{
    shouting_tick = 15;
    getWorld() -> annoy_frackman(2);
}

// set the condition to avoid perpendicular turn in the following 200 ticks
void Protester::turn_perpendicular()
{
    perpendicular_tick = 200;
}

// do what a protester should do when get bribed by a gold, different for regular protester and hardcore protester
void Protester::get_bribed()
{
    my_state = leave_the_oil_field;
}

// function to implement bfs and return the current node which contains next step's information
Protester::maze_coordinate Protester::breath_first_search(int startX, int startY, int destX, int destY, int &steps)
{
    bool visiting_map[DIRT_HEIGHT+1][DIRT_HEIGHT+1];
    for (int i = 0; i <= DIRT_HEIGHT;i++) // rows
    {
        for (int j = 0; j <= DIRT_HEIGHT; j++) // columns
        {
            if (getWorld() -> is_current_16square_occupied_by_dirt(j, i))
                visiting_map[i][j] = 0; // can not be added to queue
            else if (getWorld() -> is_within_radius_of_boulder(j, i, 3.0))
                visiting_map[i][j] = 0; // can not be added to queue;
            else visiting_map[i][j] = 1; // can be added to queue.
        }
    }
    
    std::queue<maze_coordinate> my_queue;
    my_queue.push(maze_coordinate(startX,startY,-1, -1));
    visiting_map[startY][startX] = 0;
    int my_prevX = -1;
    int my_prevY = -1;
    while (!my_queue.empty())
    {
        maze_coordinate coord = my_queue.front();
        if (my_prevX != coord.prevX || my_prevY != coord.prevY)
        {
            steps++;
            my_prevX = coord.prevX;
            my_prevY = coord.prevY;
        }
        if (coord.currentX == destX && coord.currentY == destY)
            return coord;
        my_queue.pop();
        int my_x = coord.currentX;
        int my_y = coord.currentY;
        // check for four directions, if the next square is within the bound, dirtless and also not close to a boulder, then add the point to queue.
        //x+1, y right
        if(my_x + 1 <= DIRT_HEIGHT && visiting_map[my_y][my_x + 1] == 1)
        {
            my_queue.push(maze_coordinate(my_x+1,my_y,my_x, my_y));
            visiting_map[my_y][my_x + 1] = 0;
        }
        //x-1, y left
        if(my_x - 1 >=0  && visiting_map[my_y][my_x - 1] == 1)
        {
            my_queue.push(maze_coordinate(my_x-1,my_y,my_x, my_y));
            visiting_map[my_y][my_x - 1] = 0;
        }
        //x, y+1 up
        if(my_y + 1 <= DIRT_HEIGHT  && visiting_map[my_y + 1][my_x] == 1)
        {
            my_queue.push(maze_coordinate(my_x,my_y+1,my_x, my_y));
            visiting_map[my_y + 1][my_x] = 0;
        }
        //x, y-1 down
        if(my_y - 1 >=0  && visiting_map[my_y - 1][my_x] == 1)
        {
            my_queue.push(maze_coordinate(my_x,my_y-1,my_x, my_y));
            visiting_map[my_y - 1][my_x] = 0;
        }
    }
    return maze_coordinate(-1,-1,-1,-1); // can not find the path
 }

// protester's doSomething
void Protester::doSomething()
{
    current_tick++;
    bool is_resting_tick_updated = 0;
    if (resting_tick > 0)
    {
        resting_tick--;
        is_resting_tick_updated = 1;
    }
    if (stared_tick > 0)
        stared_tick--;
    if (current_tick%(ticksToWaitBetweenMoves+1) == 0)
    {
        if (is_resting_tick_updated == 1)
            resting_tick++;
        if (shouting_tick > 0)
            shouting_tick--;
        if (perpendicular_tick > 0)
            perpendicular_tick--;
        Actual_doSomething();
    }
}

// Protester's doSomething when it is at a non-resting tick
void Protester::Actual_doSomething()
{
    if (!is_active())
        return;
    else
    {
        if (is_rest() || stared_tick!=0 ) // condition2
        {
            return;
        }
        else if (my_state == leave_the_oil_field)
        {
            if (getX() == OBJECT_LOCATION_LIMIT && getY() == OBJECT_LOCATION_LIMIT)
                set_me_inactive();
            else
            {
                // step one square towards the exit point. a queue based maze searching algorithm
                int steps = 0;
                maze_coordinate current_coord = breath_first_search(60, 60, getX(), getY(), steps);
                // path not find or already at the destination, in this case, impossible
                if (current_coord.prevX == -1 && current_coord.prevY == -1)
                    return;
                else
                {
                    moveTo(current_coord.prevX, current_coord.prevY);
                    if (current_coord.currentX == current_coord.prevX - 1)
                        setDirection(right);
                    else if (current_coord.currentX == current_coord.prevX + 1)
                        setDirection(left);
                    else if (current_coord.currentY == current_coord.prevY - 1)
                        setDirection(up);
                    else if (current_coord.currentY == current_coord.prevY + 1)
                        setDirection(down);
                }
            }
        }
        else // now the protester must be at non_resting state.
        {
            if (getWorld()->is_player_near(this, 4.0) && getWorld()->is_facing_player(this))
            {
                if (shouting_tick == 0) // hasn't shout during the last 15 ticks.
                {
                    GameController::getInstance().playSound(SOUND_PROTESTER_YELL);
                    shout();
                    set_me_rest();
                    return;
                }
            }
            else
            {
                //hardcore protester condition5
                if (!getWorld()->is_player_near(this, 4.0))
                {
                    if (find_frackman_and_move())
                    return;
                }
                //condition5, can see the frackman directly
                Direction new_dir;
                if (getWorld() -> can_move_straight_to_frackman(this, new_dir) && !getWorld() -> is_player_near(this, 4.0))
                {
                    setDirection(new_dir);
                    getWorld() -> move_current_dir(this, getDirection());
                    numSquaresToMove = 0;
                    return;
                }
                // condition6, can not see the frackman directly.
                else //if (!getWorld() -> can_move_straight_to_frackman(this, new_dir))
                {
                    numSquaresToMove--;
                    
                    if (numSquaresToMove <= 0)
                    {
                        int i = rand()%4;
                        Direction d_array[4] = { up, down, left, right};
                        while (getWorld() -> is_dirt(this, d_array[i]) || getWorld() -> is_boulder(this,d_array[i]))
                        {
                            i = (i+1)%4;
                        }
                        setDirection(d_array[i]);
                        numSquaresToMove = rand()%53 + 8;
                    }
                    //getWorld() -> move_current_dir(this, getDirection()); // condition8
                    else // condition 7
                    {
                        if (perpendicular_tick == 0)
                        {
                            //if sitting at an intersection
                            Direction m_left = getWorld() -> my_left(getDirection());
                            Direction m_right = getWorld() -> my_right(getDirection());
                            bool can_turn_left = !(getWorld() -> is_dirt(this, m_left) || getWorld() -> is_boulder(this,m_left));
                            bool can_turn_right = !(getWorld() -> is_dirt(this, m_right) || getWorld() -> is_boulder(this,m_right));
                            if (can_turn_left || can_turn_right)
                            {
                                numSquaresToMove = rand()%53 + 8;
                                perpendicular_tick = 200;
                            }
                            if (can_turn_left && !can_turn_right)
                                setDirection(m_left);
                            else if (!can_turn_left && can_turn_right)
                                setDirection(m_right);
                            else if (can_turn_left && can_turn_right)
                            {
                                int  i = rand()%2;
                                if (i == 0)
                                    setDirection(m_left);
                                else setDirection(m_right);
                            }
                        }
                    }
                //condition8
                    if (!getWorld() -> is_dirt(this, getDirection()) && !getWorld() -> is_boulder(this,getDirection()))
                        getWorld() -> move_current_dir(this, getDirection());
                    else numSquaresToMove = 0;
                }
            }
        }
    }
}

// when hardcore protester is more than 4.0 units away from the frackman,
// find frackman's position and step one square closer, BFS
bool Hardcore_Protester::find_frackman_and_move()
{
    int max_steps = 16 + (getWorld() -> getLevel()) * 2;
    int player_x, player_y;
    getWorld() -> get_player_position(player_x, player_y);
    int steps = 0;
    maze_coordinate current_coord = breath_first_search(player_x, player_y, getX(), getY(), steps);
    if (steps <= max_steps)
    {
        if (current_coord.prevX == -1 && current_coord.prevY == -1)
            return true; // player and the protester overlaps, in this case is impossible
        else
        {
            if (current_coord.currentX == current_coord.prevX - 1)
                setDirection(right);
            else if (current_coord.currentX == current_coord.prevX + 1)
                setDirection(left);
            else if (current_coord.currentY == current_coord.prevY - 1)
                setDirection(up);
            else if (current_coord.currentY == current_coord.prevY + 1)
                setDirection(down);
            moveTo(current_coord.prevX, current_coord.prevY);
        }
        return true;
    }
    return false;
}

// hardcore protester's version of get_bribed
void Hardcore_Protester::get_bribed()
{
    set_me_stared();
}

// Frackman's doSomething
void FrackMan::doSomething()
{
    if ((!is_active()))
        return;
    else
    {
        int if_removed = 0;
        for (int i= 0 ; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                int y = getY()+i;
                int x = getX()+j;
                if (y >=0 && y <60 && x >= 0 && x <64)
                {
                    if (getWorld() -> remove_dirt(x, y))
                        if_removed = 1;
                }
            }
        }
        if (if_removed)
            GameController::getInstance().playSound(SOUND_DIG);
        else
        {
            int ch;
            Direction new_dir;
            if (getWorld() -> getKey(ch) == true)
            {
                switch (ch)
                {
                    case KEY_PRESS_ESCAPE:
                    {
                        set_me_inactive();
                        break;
                    }
                    //case spacebar -- squirt
                    case KEY_PRESS_SPACE:
                    {
                        if (squirt > 0)
                        {
                            squirt--;
                            GameController::getInstance().playSound(SOUND_PLAYER_SQUIRT);
                            int my_squirt_x = getX();
                            int my_squirt_y = getY();
                            // the squirt position is not occupied by a dirt or near a boulder
                            if (find_my_squirt_position(my_squirt_x, my_squirt_y, getDirection()))
                                getWorld()->create_squirt(my_squirt_x, my_squirt_y, getDirection());
                        }
                        break;
                    }
                    case KEY_PRESS_LEFT:
                    {
                        new_dir = left;
                        if (new_dir != getDirection())
                            setDirection(new_dir);
                        else if (can_move_forward())
                            moveTo(getX()-1, getY());
                        break;
                    }
                    case KEY_PRESS_RIGHT:
                    {
                        new_dir = right;
                        if (new_dir != getDirection())
                            setDirection(new_dir);
                        else if (can_move_forward())
                            moveTo(getX()+1, getY());
                        break;
                    }
                    case KEY_PRESS_UP:
                    {
                        new_dir = up;
                        if (new_dir != getDirection())
                            setDirection(new_dir);
                        else if (can_move_forward())
                            moveTo(getX(), getY()+1);
                        break;
                    }
                    case KEY_PRESS_DOWN:
                    {
                        new_dir = down;
                        if (new_dir != getDirection())
                            setDirection(new_dir);
                        else if (can_move_forward())
                            moveTo(getX(), getY()-1);
                        break;
                    }
                    // case "Z" sonar
                    case 'Z':
                    {
                        if (sonar > 0)
                        {
                            sonar--;
                            getWorld() -> use_sonar(this);
                        }
                        break;
                    }
                    case 'z':
                    {
                        if (sonar > 0)
                        {
                            sonar--;
                            getWorld() -> use_sonar(this);
                        }
                        break;
                    }
                    case KEY_PRESS_TAB:
                    {
                        if (gold > 0)
                        {
                            gold--;
                            getWorld() -> create_temporary_gold(getX(), getY());
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
}

// check to see if I can fire a squirt at current location and update x, y to the position to start a squirt if true.
bool FrackMan::find_my_squirt_position(int &x, int &y, Direction dir)
{
    bool is_dirt_infront;
    if (dir == up)
    {
        y = y + 4;
        is_dirt_infront = getWorld() -> is_dirt(this, up);
    }
    else if (dir == down)
    {
        y = y - 4;
        is_dirt_infront = getWorld() -> is_dirt(this, down);
    }
    else if (dir == left)
    {
        x = x - 4;
        is_dirt_infront = getWorld() -> is_dirt(this, left);
    }
    else
    {
        x = x + 4;
        is_dirt_infront = getWorld() -> is_dirt(this, right);
    }
    
    if ( x >=0 && x < DIRT_WIDTH && y >= DIRT_HEIGHT)
        return true;
    else if (x >=0 && x < DIRT_WIDTH && y >= 0 && y < DIRT_HEIGHT)
    {
        if (is_dirt_infront)
            return false;
        else if (getWorld() -> is_boulder(nullptr, none, 3.0, x, y))
            return false;
        else return true;
    }
    else return false;
    // location out of bound.
}

// check to see if I move one square forward in current direction
bool FrackMan::can_move_forward()
{
    int next_x = getX();
    int next_y = getY();
    if (getDirection() == up)
        next_y++;
    else if (getDirection() == down)
        next_y--;
    else if (getDirection() == right)
        next_x++;
    else if (getDirection() == left)
        next_x--;
    if (next_x >=0 && next_x <= DIRT_HEIGHT && next_y >= 0 && next_y <= DIRT_HEIGHT)
    {
        //if there is no boulder in the way, return true
        if (!getWorld() -> is_boulder(nullptr, none, 3.0, next_x, next_y))
        return true;
        else return false;
    }
    else return false;
}

// frackman's version of get_annoyed
void FrackMan::get_annoyed(int reduced_points)
{
    reduce_hit_point(reduced_points);
}
