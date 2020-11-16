#ifndef SHIP_H
#define SHIP_H

struct ship
{
    int no; //# of ship
    int ready_time; //ready time
    int length; //length
    int processing_time; //processing time
    int weight; //weight
    int owner = 1; //owner
};

struct berth
{
    int no; //# of berth
    int length; //length
};


#endif
