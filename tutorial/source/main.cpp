#include <iostream>

#include <ip/core/logging/LoggingMacros.h>
#include <ip/core/logging/LogSystem.h>
#include <ip/core/UnreferencedParam.h>

#include <vulkan-dev/tutorial/TutorialApplication.h>

#include <assert.h>

int main(int argc, char* argv[]) 
{
    IP_UNREFERENCED_PARAM(argc);
    IP_UNREFERENCED_PARAM(argv);

    const char *layer_path = getenv("VK_LAYER_PATH");
    assert(layer_path != nullptr);

    IP::Logging::SetLogLevel(IP::Logging::LogLevel::Debug);
    DECLARE_BACKGROUND_FILE_LOGGER(logScope, "Tutorial", ".")

    IP::UniquePtr<TutorialApplication> app = nullptr;

    try
    {
        app = IP::MakeUnique<TutorialApplication>(MEMORY_TAG);
        app->Start();
    } 
    catch (const std::exception &e)
    {
        LOG_FATAL("Exception while running tutorial: " << e.what());

        if (app)
        {
            app->Stop();
            app = nullptr;
        }

        return EXIT_FAILURE;
    }

    app->Stop();

    return EXIT_SUCCESS;
}