#include "SimpleCloth.h"
#include <vector>

SimpleCloth::SimpleCloth(float x, float y, float z, float scale, int colorIndex, int id, float length, float mass, int clothSize)
    : PendulumSystem(x, y, z, scale, colorIndex, id, clothSize * clothSize) {

    shapeType = "Simple Cloth";  // Set the type as "Simple Cloth"
    m_length = length;
    m_mass = mass;

    // TODO: Set up the initial state


















    setupParticles(particles, springs, faces);

}


