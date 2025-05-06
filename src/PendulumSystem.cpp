#include "PendulumSystem.h"
#include <cmath> // For math functions like sin, cos, sqrt
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>


PendulumSystem::PendulumSystem(float x, float y, float z, float scale, int colorIndex, int id, int numParticles)
    : ParticleSystem(x, y, z, scale, colorIndex, id), windDirection(1.0f, 0.0f, 0.0f), windIntensity(1.0f) { 

    m_numParticles = numParticles;
    m_gravity = -9.81f;
    m_drag = 1.0f;
    m_mass = 1.0f;

    wireframe_ON = false;
    wind_ON = false;
    sinusoidMove_ON = false;
    faces_ON = true;

    particles_ON = false;
    structSprings_ON = false;

    isCloth = false;

    degree = 0;

    float radius = 0.025f;
    int sectorCount = 8;
    int stackCount = 8;

    // Particle sphere rendering
    VAO, VBO, EBO = 0;
    springVAO, springVBO = 0;
    wireVAO, wireVBO = 0;
    faceVAO, faceVBO, faceEBO = 0;

    generateUnitSphereMesh( radius, sectorCount, stackCount);
}

PendulumSystem::~PendulumSystem() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    if (springVAO) glDeleteVertexArrays(1, &springVAO);
    if (springVBO) glDeleteBuffers(1, &springVBO);
    if (wireVAO) glDeleteVertexArrays(1, &wireVAO);
    if (wireVBO) glDeleteBuffers(1, &wireVBO);
    if (faceVAO) glDeleteVertexArrays(1, &faceVAO);
    if (faceVBO) glDeleteBuffers(1, &faceVBO);
    if (faceEBO) glDeleteBuffers(1, &faceEBO);
}


void PendulumSystem::generateUnitSphereMesh(float radius, int sectorCount, int stackCount) {
    unitSphereVertices.clear();
    unitSphereNormals.clear();
    unitSphereIndices.clear();

    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount;
        float xy = radius * cos(stackAngle);
        float z = radius * sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * glm::pi<float>() / sectorCount;
            float x = xy * cos(sectorAngle);
            float y = xy * sin(sectorAngle);
            glm::vec3 pos(x, y, z);
            glm::vec3 normal = glm::normalize(pos);
            unitSphereVertices.push_back(pos);
            unitSphereNormals.push_back(normal);
        }
    }

    for (int i = 0; i < stackCount; ++i) {
        for (int j = 0; j < sectorCount; ++j) {
            int first = i * (sectorCount + 1) + j;
            int second = first + sectorCount + 1;

            unitSphereIndices.push_back(first);
            unitSphereIndices.push_back(second);
            unitSphereIndices.push_back(first + 1);

            unitSphereIndices.push_back(second);
            unitSphereIndices.push_back(second + 1);
            unitSphereIndices.push_back(first + 1);
        }
    }
}


void PendulumSystem::setupParticles(const std::vector<glm::vec4>& myParticles, 
                                    const std::vector<glm::vec4>& mySprings,
                                    const std::vector<glm::vec3>& myFaces) {

    particleVertices.clear();
    particleIndices.clear();
    springVertices.clear();
    
    
    particles = myParticles;
    springs = mySprings;
    faces = myFaces;

    m_state.clear();

    // TODO: Build your initial particles and velocities for your pendulum system and 
    // populate it back to m_state. Build your buffers in particleVertices and 
    // particleIndices

    for (unsigned int i = 0; i < particles.size(); ++i) {
        m_state.push_back(glm::vec3(particles[i]));
        m_state.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    }

    m_initialState = m_state;

    int vertexOffset = 0;

    for (int p = 0; p < m_numParticles; ++p) {
      
        glm::vec3 center = m_state[2 * p]; 
        
        for (unsigned int i = 0; i < unitSphereVertices.size(); ++i) {
            
            glm::vec3 pos = unitSphereVertices[i] + center;
            glm::vec3 normal = unitSphereNormals[i];

            particleVertices.insert(particleVertices.end(), { pos.x, pos.y, pos.z });
            particleVertices.insert(particleVertices.end(), { normal.x, normal.y, normal.z });
            

        }

        for (unsigned int j = 0; j < unitSphereIndices.size(); ++j) {
            particleIndices.push_back(static_cast<unsigned int>(vertexOffset+ unitSphereIndices[j]));
        }

        vertexOffset += unitSphereVertices.size();
    
            
    }
    
    // Build the buffers for the particles

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, particleVertices.size() * sizeof(float), particleVertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, particleIndices.size() * sizeof(unsigned int), particleIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    
    setupSprings();
    setupFaces();
    setupWireframe();

}

void PendulumSystem::setupSprings() {

    springVertices.clear();


    // TODO: Build your initial spring pairs positions for your pendulum system and 
    // build your buffers in springVertices. This helps to draw all spring structures between 
    // particles
    
    for ( const auto& spring : springs) {
        
        int index1 = static_cast<int>(spring[0]);
        int index2 = static_cast<int>(spring[1]);

        glm::vec3 p0 = m_state[2 * index1]; // particle positions
        glm::vec3 p1 = m_state[2 * index2];

        springVertices.insert(springVertices.end(), { p0.x, p0.y, p0.z });
        springVertices.insert(springVertices.end(), { p1.x, p1.y, p1.z });


    }
    
    // Build the buffers for the springs
    
    
    for (const auto& spring : springs) {
        int i0 = static_cast<int>(spring[0]);
        int i1 = static_cast<int>(spring[1]);

        glm::vec3 p0 = m_state[2 * i0]; // particle positions
        glm::vec3 p1 = m_state[2 * i1];

        springVertices.insert(springVertices.end(), { p0.x, p0.y, p0.z });
        springVertices.insert(springVertices.end(), { p1.x, p1.y, p1.z });
    }
    
    glGenVertexArrays(1, &springVAO);
    glGenBuffers(1, &springVBO);

    glBindVertexArray(springVAO);
    glBindBuffer(GL_ARRAY_BUFFER, springVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * springs.size(), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    updateSprings();

}


void PendulumSystem::setupWireframe() {
    wireVertices.clear();

    // TODO: Build your initial spring pairs positions for your pendulum system wireframe and build your buffers in wireVertices. This helps to draw spring structures between 
    // particles. The wireframe are only the structural springs that are horizontally and vertically linking particles. This is only used in SimpleCloth

    int clothSize = static_cast<int>(sqrt(m_numParticles));

    // Helper function for indexing
    auto indexOf = [clothSize](int row, int col) {
        return row * clothSize + col;
    };


    for (int row = 0; row < clothSize - 1; ++row) {
        for (int col = 0; col < clothSize - 1; ++col) {
            int a = indexOf(row, col);
            int b = indexOf(row, col + 1);
            int c = indexOf(row + 1, col);
            int d = indexOf(row + 1, col + 1);

            glm::vec3 posA = m_state[2 * a];
            glm::vec3 posB = m_state[2 * b];
            glm::vec3 posC = m_state[2 * c];
            glm::vec3 posD = m_state[2 * d];

            // line a-b
            wireVertices.insert(wireVertices.end(), { posA.x, posA.y, posA.z });
            wireVertices.insert(wireVertices.end(), { posB.x, posB.y, posB.z });

            // line b-d
            wireVertices.insert(wireVertices.end(), { posB.x, posB.y, posB.z });
            wireVertices.insert(wireVertices.end(), { posD.x, posD.y, posD.z });

            // line d-c
            wireVertices.insert(wireVertices.end(), { posD.x, posD.y, posD.z });
            wireVertices.insert(wireVertices.end(), { posC.x, posC.y, posC.z });

            // line c-a
            wireVertices.insert(wireVertices.end(), { posC.x, posC.y, posC.z });
            wireVertices.insert(wireVertices.end(), { posA.x, posA.y, posA.z });
        }
    }

    // Generate VAO and VBO if not already created
    glGenVertexArrays(1, &wireVAO);
    glGenBuffers(1, &wireVBO);

    // Bind VAO and VBO
    glBindVertexArray(wireVAO);
    glBindBuffer(GL_ARRAY_BUFFER, wireVBO);

    // Upload wireframe vertex data
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * wireVertices.size(), nullptr, GL_DYNAMIC_DRAW);

    
    // Position attribute (3 floats per vertex)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // Unbind VAO
    
    updateWireframe();    
   
}




void PendulumSystem::setupFaces() {

    faceVertices.clear();
    faceIndices.clear();

    // TODO: Build your initial face vertices positions and face indices  for your pendulum system used in SimpleCloth. 
    // This creates the triangles and creates the  surface of the cloth. For now, depending on your initial position of 
    // the cloth, just use an axis-aligned normal vector for the normal. This is only used in SimpleCloth

    int clothSize = static_cast<int>(sqrt(m_numParticles));

    auto indexOf = [clothSize](int row, int col) {
        return row * clothSize + col;
    };

    glm::vec3 normal(0.0f, 1.0f, 0.0f); 

    // Build faceVertices
    for (int row = 0; row < clothSize; ++row) {
        for (int col = 0; col < clothSize; ++col) {
            int idx = indexOf(row, col);
            glm::vec3 pos = m_state[2 * idx];  
            faceVertices.insert(faceVertices.end(), { pos.x, pos.y, pos.z });
            faceVertices.insert(faceVertices.end(), { normal.x, normal.y, normal.z });
        }
    }
    
    for (int row = 0; row < clothSize - 1; ++row) {
        for (int col = 0; col < clothSize - 1; ++col) {
            int a = indexOf(row, col);
            int b = indexOf(row, col + 1);
            int c = indexOf(row + 1, col);
            int d = indexOf(row + 1, col + 1);

            faceIndices.push_back(a);
            faceIndices.push_back(c);
            faceIndices.push_back(b);

            faceIndices.push_back(b);
            faceIndices.push_back(c);
            faceIndices.push_back(d);
        }
    }
    
    // Build the buffers for the particles

    glGenVertexArrays(1, &faceVAO);
    glGenBuffers(1, &faceVBO);
    glGenBuffers(1, &faceEBO);

    glBindVertexArray(faceVAO);

    glBindBuffer(GL_ARRAY_BUFFER, faceVBO);
    glBufferData(GL_ARRAY_BUFFER, faceVertices.size() * sizeof(float), faceVertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faceEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndices.size() * sizeof(unsigned int), faceIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    updateFaces();    

}


void PendulumSystem::updateParticles() {

    particleVertices.clear(); // Reset

    // TODO: Build your updates to your particles postion and velocities for your 
    // pendulum system. Update your buffers in particleVertices and particleIndices


    for (int p = 0; p < m_numParticles; ++p) {
      
        glm::vec3 center = m_state[2 * p]; 
        
        for (unsigned int i = 0; i < unitSphereVertices.size(); ++i) {
            
            glm::vec3 pos = unitSphereVertices[i] + center;
            glm::vec3 normal = unitSphereNormals[i];

            particleVertices.insert(particleVertices.end(), { pos.x, pos.y, pos.z });
            particleVertices.insert(particleVertices.end(), { normal.x, normal.y, normal.z });

        } 
    }


    // Update VBO
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * particleVertices.size(), particleVertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}


void PendulumSystem::updateSprings() {

    springVertices.clear();

    // TODO: Build your updated spring pairs positions for your pendulum system and 
    // build your buffers in springVertices. This helps to draw all spring structures between 
    // particles

    for ( const auto& spring : springs) {
        
        int index1 = static_cast<int>(spring[0]);
        int index2 = static_cast<int>(spring[1]);

        glm::vec3 p0 = m_state[2 * index1]; // particle positions
        glm::vec3 p1 = m_state[2 * index2];

        springVertices.insert(springVertices.end(), { p0.x, p0.y, p0.z });
        springVertices.insert(springVertices.end(), { p1.x, p1.y, p1.z });


    }

    // Update VBO
    
    glBindBuffer(GL_ARRAY_BUFFER, springVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * springVertices.size(), springVertices.data(), GL_DYNAMIC_DRAW); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PendulumSystem::updateWireframe() {

    wireVertices.clear();


    // TODO: Build your updated spring pairs positions for your pendulum system wireframe and build your buffers in wireVertices. This helps to draw spring structures between 
    // particles. The wireframe are only the structural springs that are horizontally and vertically linking particles. This is only used in SimpleCloth
    
    int clothSize = static_cast<int>(sqrt(m_numParticles));

    auto indexOf = [clothSize](int row, int col) {
        return row * clothSize + col;
    };

    for (int row = 0; row < clothSize - 1; ++row) {
        for (int col = 0; col < clothSize - 1; ++col) {
            int a = indexOf(row, col);
            int b = indexOf(row, col + 1);
            int c = indexOf(row + 1, col);
            int d = indexOf(row + 1, col + 1);

            glm::vec3 posA = m_state[2 * a];
            glm::vec3 posB = m_state[2 * b];
            glm::vec3 posC = m_state[2 * c];
            glm::vec3 posD = m_state[2 * d];

            // line a-b
            wireVertices.insert(wireVertices.end(), { posA.x, posA.y, posA.z });
            wireVertices.insert(wireVertices.end(), { posB.x, posB.y, posB.z });

            // line b-d
            wireVertices.insert(wireVertices.end(), { posB.x, posB.y, posB.z });
            wireVertices.insert(wireVertices.end(), { posD.x, posD.y, posD.z });

            // line d-c
            wireVertices.insert(wireVertices.end(), { posD.x, posD.y, posD.z });
            wireVertices.insert(wireVertices.end(), { posC.x, posC.y, posC.z });

            // line c-a
            wireVertices.insert(wireVertices.end(), { posC.x, posC.y, posC.z });
            wireVertices.insert(wireVertices.end(), { posA.x, posA.y, posA.z });
        }
    }

    // Update VBO
    
    glBindBuffer(GL_ARRAY_BUFFER, wireVBO);
    glBufferData(GL_ARRAY_BUFFER, wireVertices.size() * sizeof(float), wireVertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void PendulumSystem::updateFaces() {

    faceVertices.clear();
    faceIndices.clear();

    // TODO: Build your updated face vertices positions and face indices  for your pendulum system used in SimpleCloth. 
    // This creates the triangles and creates the  surface of the cloth. This is only used in SimpleCloth
    //
    // Here you would need to calculate normals. Since our shader uses a per-vertex normal input, you would need to look
    // at all adjacent faces to a vertex, calculate the faces normals, and accumulate that by summing their vectors. At 
    // the end, this creates very smooth surface through interpolation. 


    int clothSize = static_cast<int>(sqrt(m_numParticles));

    auto indexOf = [clothSize](int row, int col) {
        return row * clothSize + col;
    };

    glm::vec3 normal(0.0f, 1.0f, 0.0f);  // same normal for now

    for (int row = 0; row < clothSize; ++row) {
        for (int col = 0; col < clothSize; ++col) {
            int idx = indexOf(row, col);
            glm::vec3 pos = m_state[2 * idx];  // current position
            faceVertices.insert(faceVertices.end(), { pos.x, pos.y, pos.z });
            faceVertices.insert(faceVertices.end(), { normal.x, normal.y, normal.z });
        }
    }
    // Update VBO
    
    glBindBuffer(GL_ARRAY_BUFFER, faceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * faceVertices.size(), faceVertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// TODO: implement evalF
// for a given state, evaluate f(X,t)

std::vector<glm::vec3> PendulumSystem::evalF(const std::vector<glm::vec3>& state) {

    std::vector<glm::vec3> f;
    
    // TODO: implement evalF
    // for a given state, evaluate f(X,t). Make sure this works for simplePendulum, simpleChain
    // and simpleCloth

        for (unsigned int i = 0; i < m_numParticles; i++) {
            
            glm::vec3 pos = state[2 * i];
            glm::vec3 vel = state[2 * i + 1];

            f.push_back(vel);

            // Gravity Force
            glm::vec3 f_Gravity = glm::vec3(0.0f, m_gravity * m_mass, 0.0f);

            // Drag Force
            glm::vec3 f_Drag = -m_drag * vel;

            // Net Force
            glm::vec3 f_Net = f_Gravity + f_Drag;

            // Spring Force
            
            for ( const auto& spring : springs) {

                int index0 = static_cast<int>(spring[0]);
                int index1 = static_cast<int>(spring[1]);

                if (index0 == i || index1 == i) {

                glm::vec3 p0 = state[2 * index0]; // particle positions
                glm::vec3 p1 = state[2 * index1];

                float restLength = spring[2];
                float stiffness = spring[3];

                glm::vec3 direction = p1 - p0;
                float currentLength = glm::length(direction);

                if (currentLength > 0){
                    glm::vec3 springForce = -stiffness * (currentLength - restLength) * glm::normalize(direction);
                    if (index0 == i) f_Net -= springForce;   // if wrong swap + and -
                    if (index1 == i) f_Net += springForce;
                }
             }
            }

            // Wind force (if wind is enabled and is cloth system)
                if (isCloth && getWind()) {
                f_Net += getWindDirection() * getWindIntensity();
                }

            // movement force
            if (isCloth && getMovement()) {
                float angularSpeed = 0.5f; 
                float time = glfwGetTime();
            
                glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
                glm::vec3 offset = pos - center;
            
                glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angularSpeed * time, glm::vec3(0.0f, 1.0f, 0.0f));
                glm::vec3 targetPos = glm::vec3(rot * glm::vec4(offset, 1.0f)) + center;
            
                glm::vec3 movementForce = (targetPos - pos) * 10.0f; 
                f_Net += movementForce;
            }


                if (particles[i].w == 1.0f) {
                    f.push_back(vel);
                }
                else{
                    f.push_back(f_Net / m_mass);
                }
            }
        

    return f;

    }

// NOTE: The following flags are available to integrate into your code. Feel free to use them, as they are tied into the Imgui interface.
// We recommend you take a look at the interface so you can see what is available to you for each particle system.


// Enable wind
void PendulumSystem::enableWind() { wind_ON = true; }

// Disable wind
void PendulumSystem::disableWind() { wind_ON = false; }

// Get wind status
bool PendulumSystem::getWind() const{ return wind_ON; }

// Set wind direction
void PendulumSystem::setWindDirection(const glm::vec3& direction) {
    windDirection = glm::normalize(direction);
}

// Set wind intensity
void PendulumSystem::setWindIntensity(float intensity) {
    windIntensity = intensity;
}

// Get wind direction
glm::vec3 PendulumSystem::getWindDirection() const {
    return windDirection;
}

// Get wind intensity
float PendulumSystem::getWindIntensity() const {
    return windIntensity;
}

// Movement utilities
void PendulumSystem::enableMovement() { sinusoidMove_ON = true; }
void PendulumSystem::disableMovement() { sinusoidMove_ON = false; }
bool PendulumSystem::getMovement() const { return sinusoidMove_ON; }

// Wireframe utilities
void PendulumSystem::enableWireframe() { wireframe_ON = true; faces_ON = false; }
void PendulumSystem::disableWireframe() { wireframe_ON = false; faces_ON = true; }
bool PendulumSystem::getWireframe() const { return wireframe_ON; }

// Particles utilities
void PendulumSystem::enableParticles() { particles_ON = true; }
void PendulumSystem::disableParticles() { particles_ON = false; }
bool PendulumSystem::getParticles() const { return particles_ON; }

// Structural springs utilities
void PendulumSystem::enableStructSprings() { structSprings_ON = true; }
void PendulumSystem::disableStructSprings() { structSprings_ON = false; }
bool PendulumSystem::getStructSprings() const { return structSprings_ON; }

void PendulumSystem::setMass(float mass) { m_mass = mass; }
float PendulumSystem::getMass() const { return m_mass; }


void PendulumSystem::reset() {
    // Reset the state of the pendulum using the parent class's reset method
    ParticleSystem::reset();
    
    m_state = m_initialState;  // Restore positions and velocities
    updateParticles();         // Update buffers
    updateSprings();
    updateWireframe();
    updateFaces();
}


void PendulumSystem::draw(GLuint shaderProgram) {

    updateSprings();
    updateWireframe();
    updateFaces(); // optional

    // Use the shader program
    glUseProgram(shaderProgram);
    
    // Enable lighting for the cube
    GLint lightingLoc = glGetUniformLocation(shaderProgram, "useLighting");
    if (lightingLoc != -1) {
        glUniform1i(lightingLoc, 1); // Enable lighting for the pendulum
    }
    
    applyTransform(shaderProgram);

    // Set material color in shader
    GLint colorLoc = glGetUniformLocation(shaderProgram, "material.color");
    if (colorLoc != -1) {
        glUniform3fv(colorLoc, 1, (colorIndex == 31) ? customColor : colorPresets[colorIndex].color);
    }

    // Draw Faces (as triangles)
    
    if (faces_ON) {
    
        // Set material color in shader
        GLint colorLoc = glGetUniformLocation(shaderProgram, "material.color");
        if (colorLoc != -1) {
            glUniform3fv(colorLoc, 1, (colorIndex == 31) ? customColor : colorPresets[colorIndex].color);
        }
        
        glBindVertexArray(faceVAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(faceIndices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
            
    }

    // Wireframe

    if (wireframe_ON){

        // Draw Particles if enabled
        if (particles_ON) {
        
            GLint colorLoc = glGetUniformLocation(shaderProgram, "material.color");
            if (colorLoc != -1) {
                glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f); // white lines
            }
            
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(particleIndices.size()), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
        
            // Draw Structural Springs (as lines)
        if (structSprings_ON) {
        
            // Disable lighting in shader
            
            GLint lightingLoc = glGetUniformLocation(shaderProgram, "useLighting");
            if (lightingLoc != -1) {
                glUniform1i(lightingLoc, 1); // Disable lighting
            }
            
            glBindVertexArray(springVAO);
            glLineWidth(2.0f);
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(springVertices.size() / 3));
            glBindVertexArray(0);

        } else {
        
            // Disable lighting in shader
            
            GLint lightingLoc = glGetUniformLocation(shaderProgram, "useLighting");
            if (lightingLoc != -1) {
                glUniform1i(lightingLoc, 1); // Disable lighting
            }
            
            glBindVertexArray(wireVAO);
            glLineWidth(2.0f);
            glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(wireVertices.size() / 3));
            glBindVertexArray(0);
        
        }


    }
    
}

