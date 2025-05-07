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
    windEnabled = false;
    windIntensity = 1.0f;

    // helper function for indexing
    auto indexOf = [clothSize](int row, int col) {
        return row * clothSize + col;
    };

    float spacing = 0.2f;

    
    // BUILD YOUR PARTICLE SYSTEM 
    for (int row = 0; row < clothSize; ++row) {
        for (int col = 0; col < clothSize; ++col) {

            particles.push_back(glm::vec4(col * spacing, 0.0f, row * spacing, 0.0f));

        }   
        
    }

    // Structural springs
    for (int row = 0; row < clothSize; ++row) {
        for (int col = 0; col < clothSize; ++col) {
            int current = indexOf(row, col);
            if (col < clothSize - 1) {
                int right = indexOf(row, col + 1);
                springs.push_back(glm::vec4(current, right, spacing, 10.0f));
            }
            if (row < clothSize - 1) {
                int below = indexOf(row + 1, col);
                springs.push_back(glm::vec4(current, below, spacing, 10.0f));
            }
        }
    }

    // Shear Springs
    for (int row = 0; row < clothSize - 1; ++row) {
        for (int col = 0; col < clothSize; ++col) {
            if (col < clothSize - 1) {
                int current = indexOf(row, col);
                int diagRight = indexOf(row + 1, col + 1);
                springs.push_back(glm::vec4(current, diagRight, sqrt(2.0f) * spacing, 10.0f));
            }
            if (col > 0) {
                int current = indexOf(row, col);
                int diagLeft = indexOf(row + 1, col - 1);
                springs.push_back(glm::vec4(current, diagLeft, sqrt(2.0f) * spacing, 10.0f));
            }
        }
    }

    // Flexion Springs
    for (int row = 0; row < clothSize; ++row) {
        for (int col = 0; col < clothSize; ++col) {
            if (col < clothSize - 2) {
                int current = indexOf(row, col);
                int skipRight = indexOf(row, col + 2);
                springs.push_back(glm::vec4(current, skipRight, 2.0f * spacing, 10.0f));
            }
            if (row < clothSize - 2) {
                int current = indexOf(row, col);
                int skipBelow = indexOf(row + 2, col);
                springs.push_back(glm::vec4(current, skipBelow, 2.0f * spacing, 10.0f));
            }
        }
    }

    // faces
    for (int row = 0; row < clothSize - 1; ++row) {
        for (int col = 0; col < clothSize - 1; ++col) {
            int a = indexOf(row, col);
            int b = indexOf(row, col + 1);
            int c = indexOf(row + 1, col);
            int d = indexOf(row + 1, col + 1);
            
            // first triangle (a, c, b)
            faces.push_back(glm::uvec3(a, c, b));
            
            // second triangle (b, c, d)
            faces.push_back(glm::uvec3(b, c, d));
        }
    }


    setupParticles(particles, springs, faces);

}


