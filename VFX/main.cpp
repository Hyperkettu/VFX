#include "FoxRenderer.h"

int main(int argc, char* argv[]) {
   
    {
        Fox::Platform::Vulkan::Application app("Fox Renderer", 1920 * 2, 1080 * 2);

        if (!app.Initialize()) {
            exit(1);
        }

        app.Run();
    }

    return 0;
}