#ifndef SIMPLECLOTH_H
#define SIMPLECLOTH_H

#include "PendulumSystem.h"

class SimpleCloth : public PendulumSystem {
public:
    // Constructor
    SimpleCloth(float x, float y, float z, float scale, int colorIndex, int id, float length, float mass, int size);

    // Movement toggling
    void enableMovement() { movementEnabled = true; }
    void disableMovement() { movementEnabled = false; }
    bool getMovement() const { return movementEnabled; }
    
    // Wind toggling
    void enableWind() { windEnabled = true; }
    void disableWind() { windEnabled = false; }
    bool getWind() const { return windEnabled; }

    // Wind direction/intensity getters/setters
    void setWindDirection(const glm::vec3& direction) { windDirection = glm::normalize(direction); }
    glm::vec3 getWindDirection() const { return windDirection; }

    void setWindIntensity(float intensity) { windIntensity = intensity; }
    float getWindIntensity() const { return windIntensity; }

private:
    bool movementEnabled = false;
    bool windEnabled = false;
    glm::vec3 windDirection = glm::vec3(1.0f, 0.0f, 0.0f);  // default +X
    float windIntensity = 1.0f;  // default strength

};

#endif // SIMPLECLOTH_H
