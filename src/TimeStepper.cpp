#include "ParticleSystem.h"
#include "TimeStepper.h"
#include "SimpleSystem.h"

// Constructor initializes the animation state
TimeStepper::TimeStepper() : animationPlaying(false), stepSize(0.02f)  {}

// Play animation
void TimeStepper::playAnimation() {
    animationPlaying = true;
}

// Stop animation
void TimeStepper::stopAnimation() {
    animationPlaying = false;
}

// Reset animation (default behavior)
void TimeStepper::resetAnimation() {
    // Derived classes can override this to reset specific states
    stopAnimation();
	
}

// Check if the animation is playing
bool TimeStepper::isAnimationPlaying() const {
    return animationPlaying;
}

// Getters and setters for step size
float TimeStepper::getStepSize() const {
    return stepSize;
}

void TimeStepper::setStepSize(float newStepSize) {
    stepSize = newStepSize;
}


// Factory Method for Creating Integrators
TimeStepper* TimeStepper::createIntegrator(IntegratorType type) {
    switch (type) {
        case IntegratorType::ForwardEuler:
            return new ForwardEuler();
        case IntegratorType::Trapezoidal:
            return new Trapezoidal();
        case IntegratorType::Midpoint:
            return new Midpoint();
        case IntegratorType::RK4:
            return new RK4();
        default:
            return nullptr;
    }
}

// Forward Euler Method
void ForwardEuler::takeStep(ParticleSystem* particleSystem, float stepSize) {

    // Euler's Method
    //
    // X(t+h) = X + hf(X, t)

    // Get the current particle system state and have the particles in the state
    // take a step of size h using Forward Euler method 













    // Call updateParticles() 
    particleSystem->updateParticles();

}

// Trapezoidal Method
void Trapezoidal::takeStep(ParticleSystem* particleSystem, float stepSize) {

    // Trapezoid Method:
    //
    // f0 = f(X, t)
    // f1 = f(X + hf0, t+h)
    // X(t+h) = X + h/2 (f0 + f1)

    // Get the current particle system state and have the particles in the state
    // take a step of size h using Trapezoid method 















    // Call updateParticles() 
    particleSystem->updateParticles();

}

// Midpoint Method
void Midpoint::takeStep(ParticleSystem* particleSystem, float stepSize) {

    // Extra Credit: Midpoint Method
















    // Call updateParticles() 
    particleSystem->updateParticles();
}


// RK4 Method
void RK4::takeStep(ParticleSystem* particleSystem, float stepSize) {
    std::vector<glm::vec3> X1 = particleSystem->getState();
    std::vector<glm::vec3> f1 = particleSystem->evalF(X1);

    std::vector<glm::vec3> X2, X3, X4;
    for (size_t i = 0; i < f1.size(); ++i) {
        X2.push_back(X1[i] + stepSize / 2.0f * f1[i]);
    }

    std::vector<glm::vec3> f2 = particleSystem->evalF(X2);
    for (size_t i = 0; i < f2.size(); ++i) {
        X3.push_back(X1[i] + stepSize / 2.0f * f2[i]);
    }

    std::vector<glm::vec3> f3 = particleSystem->evalF(X3);
    for (size_t i = 0; i < f3.size(); ++i) {
        X4.push_back(X1[i] + stepSize * f3[i]);
    }

    std::vector<glm::vec3> f4 = particleSystem->evalF(X4);

    std::vector<glm::vec3> newState;
    for (size_t i = 0; i < X1.size(); ++i) {
        newState.push_back(X1[i] + (stepSize / 6.0f) * (f1[i] + 2.0f * f2[i] + 2.0f * f3[i] + f4[i]));
    }

    particleSystem->setState(newState);
    
    // Call updateParticles() 
    particleSystem->updateParticles();

    
}
