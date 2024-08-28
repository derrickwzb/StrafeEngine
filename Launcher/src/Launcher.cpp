

#include "strafepch.h"
#include "Strafe.h"

class Launcher : public strafe::Application
{
public:
	Launcher() = default;
	~Launcher() override = default;

	void Init(const ApplicationConfig& config) override
	{
		Application::Init(config);

	}
};
/**
 * \brief Creates the editor application
 * \return The editor application
 */
strafe::Application* strafe::CreateApplication()
{
	return new Launcher();
}
