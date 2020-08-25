#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "buildussystem.h"
#include <boost/filesystem.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

bool fileExist(path filePath);

TEST_CASE("YAML parsing test")
{
    const path tmpDir = GetRootPath() / TMP_FOLDER;
    auto configFilePath = GetRootPath().append("config"+BUILDUS_EXTENSION);
    BuildConfigs configs = ParseConfigFile(configFilePath.string());
    std::string awaitedExePath = (tmpDir / "appidou").string();
    std::cout << "Awaited : " << awaitedExePath << std::endl;
    std::string awaitedDepsInclude = "BOOST_INCLUDEDIR";
    std::string awaitedLibDirectory = "BOOST_LIBRARYDIR";
    std::string awaitedFilePath1 = (tmpDir / "hehe.o").string();
    std::string awaitedFilePath2 = (tmpDir / "xd.o").string();
    std::string awaitedOutputFilePath1 = GetRootPath().append("test1.cpp").string();
    std::string awaitedOutputFilePath2 = GetRootPath().append("test2.cpp").string();
    
    
    REQUIRE(configs.exePath.compare(awaitedExePath) == 0);
    REQUIRE(configs.enVarDepsInclude.compare(awaitedDepsInclude) == 0);
    REQUIRE(configs.depsLibrary.enVarLibDirectory.compare(awaitedLibDirectory) == 0);
    REQUIRE(configs.filesToCompile.at(0).filePath.compare(awaitedFilePath1));
    REQUIRE(configs.filesToCompile.at(1).filePath.compare(awaitedFilePath2));
    REQUIRE(configs.filesToCompile.at(0).fileOutputPath.compare(awaitedOutputFilePath1));
    REQUIRE(configs.filesToCompile.at(1).fileOutputPath.compare(awaitedOutputFilePath2));
}

TEST_CASE("Clear test")
{
    auto configFilePath = GetRootPath().append("config"+BUILDUS_EXTENSION);
    BuildConfigs configs = ParseConfigFile(configFilePath.string());
    Build(configs);
    Clean();

    bool oFileExist = false;
    bool exeExist = false;
    bool intermediateFolderExist = false;
    if(boost::filesystem::exists(TMP_FOLDER_PATH))
    {
        intermediateFolderExist = true;
        for (boost::filesystem::directory_iterator end_dir_it, it(TMP_FOLDER_PATH); it!=end_dir_it; ++it) {
            if((it->path().extension().string()).compare(OUT_EXT) == 0){
                oFileExist = true;
            }
            else if((it->path().string()).compare(configs.exePath) == 0){
                exeExist = true;
            }
        }        
    }
    
    REQUIRE(intermediateFolderExist);
    REQUIRE(exeExist);
    REQUIRE(!oFileExist);
}

TEST_CASE("Build test")
{
    //1.first build
    //we start without the intermediate folder
    if(boost::filesystem::exists(TMP_FOLDER_PATH))
    {
        boost::filesystem::remove_all(TMP_FOLDER_PATH);
    }
    REQUIRE(!boost::filesystem::exists(TMP_FOLDER_PATH));

    //we run the build
    auto configFilePath = GetRootPath().append("config"+BUILDUS_EXTENSION);
    BuildConfigs configs = ParseConfigFile(configFilePath.string());
    Build(configs);
    auto lastBuildTime = last_write_time(TMP_FOLDER_PATH);
    //we verifie that we have all the necesary files
    REQUIRE(boost::filesystem::exists(path(configs.exePath)));
    for (auto files : configs.filesToCompile){
        REQUIRE(boost::filesystem::exists(path(files.fileOutputPath)));
    }

    // we now compile a diffent .config containing a diffrent .cpp file
    //this test verifie 3 new things:
    //1. If we can only recompile the changed file
    //2. If we add a new file to compile, it should compile it correctly
    //3. If we remove a file to compile, we should not keep the old .o 
    auto configFilePath2 = GetRootPath().append("config2"+BUILDUS_EXTENSION);
    BuildConfigs configs2 = ParseConfigFile(configFilePath2.string());
    Build(configs2);
    //we verifie that we have all the necesary files
    REQUIRE(boost::filesystem::exists(path(configs2.exePath)));
    for (auto files : configs2.filesToCompile){
        REQUIRE(boost::filesystem::exists(path(files.fileOutputPath)));
    }
    //we verifie that hehe.o was not changed
    REQUIRE(last_write_time(path(configs2.filesToCompile.at(1).fileOutputPath)) <= lastBuildTime);
    //we verifie that our old .o is not there anymore
    REQUIRE(!boost::filesystem::exists(path(configs.filesToCompile.at(0).fileOutputPath)));
}

bool fileExist(path filePath)
{
    return boost::filesystem::exists(filePath);
}

//prend un .o ou exe et dit si il a ete compiler au dernier build
bool fileWasCompile(path filePath)
{
    auto tmpFolderPath = TMP_FOLDER_PATH;
    auto lastBuildTime = last_write_time(tmpFolderPath);
    auto lastWriteTime = last_write_time(filePath);
    if(lastBuildTime == lastWriteTime)
    {
        return true;
    }
    return false;
}