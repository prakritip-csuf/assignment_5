#include "SimpleChain.h"
#include <vector>

SimpleChain::SimpleChain(float x, float y, float z, float scale, int colorIndex, int id, float length, float mass)
    : PendulumSystem(x, y, z, scale, colorIndex, id, 4) {  // 2 particles: anchor and bob 

    shapeType = "Simple Chain";  // Set the type as "Simple Chain"
    m_length = length;
    m_mass = mass;

    // TODO: Set up the initial state


















    setupParticles(particles, springs, faces);

}


