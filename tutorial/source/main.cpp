#include <iostream>
#include <memory>

#include <ip/core/UnreferencedParam.h>

#include <vulkan-dev/tutorial/TutorialApplication.h>

int main(int argc, char* argv[]) 
{
    IP_UNREFERENCED_PARAM(argc);
    IP_UNREFERENCED_PARAM(argv);

    std::unique_ptr<TutorialApplication> app = nullptr;

    try
    {
        app = std::make_unique<TutorialApplication>();
        app->Initialize();
        app->Run();
    } 
    catch (const std::exception &e)
    {
        std::cerr << "Exception while running tutorial: " << e.what() << std::endl;

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