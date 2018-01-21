#include <iostream>

#include <ip/core/logging/LoggingMacros.h>
#include <ip/core/logging/LogSystem.h>
#include <ip/core/UnreferencedParam.h>

#include <vulkan-dev/tutorial/TutorialApplication.h>

int main(int argc, char* argv[]) 
{
    IP_UNREFERENCED_PARAM(argc);
    IP_UNREFERENCED_PARAM(argv);

    IP::Logging::SetLogLevel(IP::Logging::LogLevel::Debug);
    DECLARE_BACKGROUND_FILE_LOGGER(logScope, "Tutorial", ".")

    IP::UniquePtr<TutorialApplication> app = nullptr;

    try
    {
        app = IP::MakeUnique<TutorialApplication>(MEMORY_TAG);
        app->Initialize();
        app->Run();
    } 
    catch (const std::exception &e)
    {
        LOG_FATAL("Exception while running tutorial: " << e.what());

        if (app)
        {
            app->Shutdown();
            app = nullptr;
        }

        return EXIT_FAILURE;
    }

    app->Shutdown();

    return EXIT_SUCCESS;
}