#include "SimpleCloth.h"
#include <vector>

SimpleCloth::SimpleCloth(float x, float y, float z, float scale, int colorIndex, int id, float length, float mass, int clothSize)
    : PendulumSystem(x, y, z, scale, colorIndex, id, clothSize * clothSize) {

    shapeType = "Simple Cloth";  // Set the type as "Simple Cloth"
    m_length = length;
    m_mass = mass;
    m_numParticles = clothSize * clothSize;

    // TODO: Set up the initial state
       
        wireframe_ON = false;
        faces_ON = true;
        isCloth = true;

        // BUILD YOUR PARTICLE SYSTEM 

    for (int row = 0; row < clothSize; ++row) {
        for (int col = 0; col < clothSize; ++col) {


            particles.push_back(glm::vec4(col, 0.0f, row, 0.0f));




        }   
        
    }

      // this is creating springs
            // structural
            // check first
           // row < clothSize -1;

            //check next
            // col < clothSize -1;

            //sheer
            // check both if your row is 

            // row < clothSize -1 and col < clothSize -1 

            //Flexion

            //check first
           // row < clothSize -2;

            //check next
            //col < clothSize -2;

            //diagonal


    // you need your faces too



    setupParticles(particles, springs, faces);

}


