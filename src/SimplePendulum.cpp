#include "SimplePendulum.h"
#include <vector>

SimplePendulum::SimplePendulum(float x, float y, float z, float scale, int colorIndex, int id, float length, float mass)
    : PendulumSystem(x, y, z, scale, colorIndex, id, 2), m_length(length), m_mass(mass) {

    shapeType = "Simple Pendulum";  // Set the type as "Simple Pendulum"
    m_length = length;
    m_mass = mass;

    // TODO: Set up the initial state

    wireframe_ON = true;
    structSprings_ON = true;
    bool particles_ON = true;
    
    particles.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    particles.push_back(glm::vec4(1.0f, 2.0f, 0.0f, 0.0f));

    m_numParticles = 2;

    springs.push_back(glm::vec4(0.0f, 1.0f, 1.0f, 15.0f));  // anchor to bob
   // springs.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));  // bob to anchor


    setupParticles(particles, springs, faces);
    
}

SimplePendulum::~SimplePendulum() {

}



