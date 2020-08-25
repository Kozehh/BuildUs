#include <iostream>

#include "buildussystem/buildussystem.h"

int main(int argc, char *argv[])
{
    // If there is only one command following the call of the app (./BuildUs)
    if (argc == 2)
    {
        std::string appArgument(argv[1]);
        // If the argument is 'clean' execute the clean command
        if (appArgument.compare(CLEAN_COMMAND) == 0)
        {
            Clean();
        }
        // Else, verify the config file path given
        else
        {
            if (!FileExists(appArgument))
            {
                std::cout << "Error: The config file " << appArgument << " doesn't exist\n";
            }
            else if(!IsBuildUsExt(appArgument))
            {
                std::cout << "Error: The config file extension has to be .buildus\n";
            }
            else
            {
                BuildConfigs configs = ParseConfigFile(appArgument);
                Build(configs);
            }
        }
    }
    else
    {
        std::cout << "Error. Wrong number of arguments..\nUsage BuildUs:\nBuildUs <configFilePath>\t-The file extension has to be .buildus\nBuildUs clean\t\t\t-Eliminate all the temporary compiled files on the disk\n";
    }
    return 0;
}