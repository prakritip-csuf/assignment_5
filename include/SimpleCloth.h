#ifndef SIMPLECLOTH_H
#define SIMPLECLOTH_H

#include "PendulumSystem.h"

class SimpleCloth : public PendulumSystem {
public:
    // Constructor
    SimpleCloth(float x, float y, float z, float scale, int colorIndex, int id, float length, float mass, int size);

};

#endif // SIMPLECLOTH_H
