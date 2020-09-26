#include "PendulumIntegrator.h"


// We get the ancor position of the pendulum.
PendulumIntegrator::PendulumIntegrator(float anchorPoint[3])
{
	m_anchorPoint[0] = anchorPoint[0];
	m_anchorPoint[1] = anchorPoint[1];
	m_anchorPoint[2] = anchorPoint[2];

	m_currentPendulumPosition[0] = anchorPoint[0];
	m_currentPendulumPosition[1] = anchorPoint[1];
	m_currentPendulumPosition[2] = anchorPoint[2];

	m_currentPendulumVelocity[0] = 0.0f;
	m_currentPendulumVelocity[1] = 0.0f;
	m_currentPendulumVelocity[2] = 0.0f;
}

	
// Sets the position of the pendulum and resets velocity.
void PendulumIntegrator::SetPendulumPosition(float position[3])
{
	m_currentPendulumPosition[0] = position[0];
	m_currentPendulumPosition[1] = position[1];
	m_currentPendulumPosition[2] = position[2];

	m_currentPendulumVelocity[0] = 0.0f;
	m_currentPendulumVelocity[1] = 0.0f;
	m_currentPendulumVelocity[2] = 0.0f;
}

// Updates the simulation.
void PendulumIntegrator::UpdateSimulation(float deltaTime)
{
	float acceleration[3];
	ComputeCurrentAcceleration(acceleration);

	m_currentPendulumPosition[0] += deltaTime * m_currentPendulumVelocity[0];
	m_currentPendulumPosition[1] += deltaTime * m_currentPendulumVelocity[1];
	m_currentPendulumPosition[2] += deltaTime * m_currentPendulumVelocity[2];

	m_currentPendulumVelocity[0] += deltaTime * acceleration[0];
	m_currentPendulumVelocity[1] += deltaTime * acceleration[1];
	m_currentPendulumVelocity[2] += deltaTime * acceleration[2];

}


// Obtains the current position of the pendulum.
void PendulumIntegrator::ObtainCurrentPosition(float position[3])
{
	position[0] = m_currentPendulumPosition[0];
	position[1] = m_currentPendulumPosition[1];
	position[2] = m_currentPendulumPosition[2];
}


// Gets the current acceleration vector.
void PendulumIntegrator::ComputeCurrentAcceleration(float acceleration[3])
{
	const float earthAcceleration = -9.81f;
	const float invMass = 2.0f;
	const float dampingVelocity = 0.05f;
	const float springConstant = 0.5f;

	acceleration[0] = 0.0f;
	acceleration[1] = earthAcceleration;
	acceleration[2] = 0.0f;

	acceleration[0] += invMass * (-m_currentPendulumVelocity[0] * dampingVelocity + springConstant * (m_anchorPoint[0] - m_currentPendulumPosition[0]));
	acceleration[1] += invMass * (-m_currentPendulumVelocity[1] * dampingVelocity + springConstant * (m_anchorPoint[1] - m_currentPendulumPosition[1]));
	acceleration[2] += invMass * (-m_currentPendulumVelocity[2] * dampingVelocity + springConstant * (m_anchorPoint[2] - m_currentPendulumPosition[2]));
}