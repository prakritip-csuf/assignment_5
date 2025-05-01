#include "SimplePendulum.h"
#include <vector>

SimplePendulum::SimplePendulum(float x, float y, float z, float scale, int colorIndex, int id, float length, float mass)
    : PendulumSystem(x, y, z, scale, colorIndex, id, 2), m_length(length), m_mass(mass) {

    shapeType = "Simple Pendulum";  // Set the type as "Simple Pendulum"
    m_length = length;
    m_mass = mass;

    // TODO: Set up the initial state


















    setupParticles(particles, springs, faces);
    
}

SimplePendulum::~SimplePendulum() {

}



