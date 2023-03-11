
#include "HelloTriangleApp.h"
#include "Log.h"
#include "Mesh.h"

int main()
{
    Log::Init();

    HelloTriangleApp app;

    try {
        app.Run();
    }
    catch (const std::exception& e)
    {
        LOG_CRITICAL("Exception Thrown: {0}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
