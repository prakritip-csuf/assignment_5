#include "PendulumSystem.h"
#include "SimpleCloth.h"
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

    if (faceVAO) glDeleteVertexArrays(1, &faceVAO);
    if (faceVBO) glDeleteBuffers(1, &faceVBO);
    if (faceEBO) glDeleteBuffers(1, &faceEBO);

    int clothSize = static_cast<int>(sqrt(m_numParticles));

    // Helper to access index
    auto indexOf = [clothSize](int row, int col) {
        return row * clothSize + col;
    };

    glm::vec3 normal(0.0f, 1.0f, 0.0f);

    for (int row = 0; row < clothSize; ++row) {
        for (int col = 0; col < clothSize; ++col) {
            int idx = indexOf(row, col);
            glm::vec3 pos = m_state[2 * idx];
            faceVertices.push_back(pos.x);
            faceVertices.push_back(pos.y);
            faceVertices.push_back(pos.z);

            faceVertices.push_back(normal.x);
            faceVertices.push_back(normal.y);
            faceVertices.push_back(normal.z);
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

    // Helper to get index in grid
    auto indexOf = [clothSize](int row, int col) {
        return row * clothSize + col;
    };

    // Get current positions
    std::vector<glm::vec3> positions(m_numParticles);
    std::vector<glm::vec3> normals(m_numParticles, glm::vec3(0.0f));

    if (m_state.size() < 2 * m_numParticles) {
        std::cerr << "m_state size too small! Expected: " << 2 * m_numParticles << ", got: " << m_state.size() << std::endl;
        return;
    }
    

    for (int i = 0; i < m_numParticles; ++i) {
        positions[i] = m_state[2 * i];
    }

    // Calculate face normals and accumulate to vertices
    for (const auto& face : faces) {
        int i0 = face.x;
        int i1 = face.y;
        int i2 = face.z;

        glm::vec3 v0 = positions[i0];
        glm::vec3 v1 = positions[i1];
        glm::vec3 v2 = positions[i2];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        normals[i0] += faceNormal;
        normals[i1] += faceNormal;
        normals[i2] += faceNormal;
    }

    // Normalize accumulated normals
    for (int i = 0; i < m_numParticles; ++i) {
        normals[i] = glm::normalize(normals[i]);
    }

    // Fill faceVertices with updated position and smooth normal
    for (int i = 0; i < m_numParticles; ++i) {
        glm::vec3 pos = positions[i];
        glm::vec3 norm = normals[i];

        faceVertices.push_back(pos.x);
        faceVertices.push_back(pos.y);
        faceVertices.push_back(pos.z);

        faceVertices.push_back(norm.x);
        faceVertices.push_back(norm.y);
        faceVertices.push_back(norm.z);
    }

    // Update dVBO
   
    glBindBuffer(GL_ARRAY_BUFFER, faceVBO);

    GLint currentBufferSize = 0;
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &currentBufferSize);
    GLsizeiptr requiredSize = faceVertices.size() * sizeof(float);

    if (requiredSize > currentBufferSize) {
        glBufferData(GL_ARRAY_BUFFER, requiredSize, faceVertices.data(), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, requiredSize, faceVertices.data());
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// TODO: implement evalF
// for a given state, evaluate f(X,t)

std::vector<glm::vec3> PendulumSystem::evalF(const std::vector<glm::vec3>& state) {
    std::vector<glm::vec3> f;
    int clothSize = static_cast<int>(sqrt(m_numParticles));

    std::vector<glm::vec3> positions(m_numParticles);
    std::vector<glm::vec3> normals(m_numParticles, glm::vec3(0.0f));

    // Calculate normals for wind
    for (const auto& face : faces) {
        int i0 = face.x, i1 = face.y, i2 = face.z;
        glm::vec3 v0 = positions[i0], v1 = positions[i1], v2 = positions[i2];
        glm::vec3 edge1 = v1 - v0, edge2 = v2 - v0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
        normals[i0] += faceNormal;
        normals[i1] += faceNormal;
        normals[i2] += faceNormal;
    }
    for (auto& n : normals) {
        n = glm::normalize(n);
    }

    for (unsigned int i = 0; i < m_numParticles; i++) {
        glm::vec3 pos = state[2 * i];
        glm::vec3 vel = state[2 * i + 1];
        f.push_back(vel);

        // GRAVITY
        glm::vec3 f_Gravity = glm::vec3(0.0f, m_gravity * m_mass, 0.0f);

        // DRAG
        glm::vec3 f_Drag = -m_drag * vel;

        // NET FORCE
        glm::vec3 f_Net = f_Gravity + f_Drag;

        // SPRING FORCES
        for (const auto& spring : springs) {
            int i0 = static_cast<int>(spring[0]);
            int i1 = static_cast<int>(spring[1]);
            if (i0 != i && i1 != i) continue;

            glm::vec3 p0 = state[2 * i0];
            glm::vec3 p1 = state[2 * i1];
            float restLength = spring[2], stiffness = spring[3];

            glm::vec3 dir = p1 - p0;
            float len = glm::length(dir);

            if (len > 0.0f) {
                glm::vec3 springForce = -stiffness * (len - restLength) * glm::normalize(dir);
                if (i0 == i) f_Net -= springForce;
                if (i1 == i) f_Net += springForce;
            }
        }

        // wind forces
        SimpleCloth* cloth = dynamic_cast<SimpleCloth*>(this);
        if (isCloth && cloth && particles[i].w != 1.0f) {
            float time = glfwGetTime();
            int row = i / clothSize;
            int col = i % clothSize;

            float waveFrequency = 10.0f;   
            float waveAmplitude = 4.0f;   
            float waveLength = 3.0f;     

            float flutterFrequency = 8.0f;
            float flutterAmplitude = 0.08f;

            if (cloth->getMovement()) {
                float sidewaysWave = sin(time * waveFrequency + col / waveLength + row * 0.1f) * waveAmplitude;
                
                float verticalFlutter = sin(time * flutterFrequency + col * 0.4f + row * 0.8f) * flutterAmplitude;

                f_Net += glm::vec3(sidewaysWave, verticalFlutter, 0.0f);
            }

            if (cloth->getWind()) {
                glm::vec3 windDirNorm = glm::normalize(cloth->getWindDirection());
                glm::vec3 windForce = cloth->getWindIntensity() * windDirNorm;
                f_Net += windForce;
            }
        }

        if (particles[i].w == 1.0f) {
            f.push_back(glm::vec3(0.0f));
        } else {
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
        glDrawElements(GL_TRIANGLES, faces.size() * 6, GL_UNSIGNED_INT, 0);
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

