#include "SimpleChain.h"
#include <vector>

SimpleChain::SimpleChain(float x, float y, float z, float scale, int colorIndex, int id, float length, float mass)
    : PendulumSystem(x, y, z, scale, colorIndex, id, 4) {  // 2 particles: anchor and bob 

    shapeType = "Simple Chain";  // Set the type as "Simple Chain"
    m_length = length;
    m_mass = mass;

    // TODO: Set up the initial state

    wireframe_ON = true;
    structSprings_ON = true;
    bool particles_ON = true;
    
    particles.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    particles.push_back(glm::vec4(1.0f, 2.0f, 0.0f, 0.0f));
    particles.push_back(glm::vec4(-1.0f, 2.0f, -4.0f, 0.0f));
    particles.push_back(glm::vec4(1.0f, -2.0f, 16.0f, 0.0f));

    m_numParticles = 4;

    springs.push_back(glm::vec4(0.0f, 1.0f, 1.0f, 15.0f)); 





    setupParticles(particles, springs, faces);

}


