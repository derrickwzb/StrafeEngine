
#pragma once

namespace strafe
{
	struct Timestep
	{

	public:
		Timestep(double _time = 0.f)
			:m_DeltaTime(_time)
		{

		}

		operator double() const { return m_DeltaTime; }
		double GetSeconds() const { return m_DeltaTime; }
		double GetMilliseconds() const { return m_DeltaTime * 1000.f; }

	private:
		double m_DeltaTime;
	};
}
