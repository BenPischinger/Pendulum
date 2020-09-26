#pragma once

// The class that can integrate the position of the pendulum.
class PendulumIntegrator
{
public:
	// We get the ancor position of the pendulum.
	PendulumIntegrator(float anchorPoint[3]);

	// Sets the position of the pendulum and resets velocity.
	void SetPendulumPosition(float position[3]);

	// Updates the simulation.
	void UpdateSimulation(float deltaTime);

	// Obtains the current position of the pendulum.
	void ObtainCurrentPosition(float position[3]);

private:
	// The position where the pendulum is anchored.
	float m_anchorPoint[3];
	// The current position of the pendulum.
	float m_currentPendulumPosition[3];
	// The current velocity of the pendulum.
	float m_currentPendulumVelocity[3];
	// Gets the current acceleration vector.
	void ComputeCurrentAcceleration(float acceleration[3]);
};

