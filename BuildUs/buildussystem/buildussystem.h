#pragma once

#include <boost/filesystem.hpp>
#include "yaml-cpp/yaml.h"
#include <string>
using boost::filesystem::path;

//
// structs for the different .buildus nodes
//

// File: contains the file path to compile and the file output path
struct File {
    std::string filePath;
    std::string fileOutputPath;
};

// Lib: contains the environment variable of the library directory
// and the list of the libs to use in this directory
struct Lib {
    std::string enVarLibDirectory;
    std::vector<std::string> libList;
};

// BuildConfigs: contains all the nodes of the .buildus file
// including the exeName that we set with it's path in the tmp dir.
// a list of File that we have to compile,
// an environment variable of the include directory we have to use
// and a node Lib
struct BuildConfigs {
    std::string exePath;
    std::vector<File> filesToCompile;
    std::string enVarDepsInclude; // env var
    Lib depsLibrary;
};


BuildConfigs ParseConfigFile(std::string configFilePath);
void Clean();
void Build(BuildConfigs configs);
void Compile(BuildConfigs configs);
void Link(BuildConfigs configs);
std::string Exec(const char* cmd);



void PathDoesntExistError(std::string path);
void SimpleErrorOccurMessage();

void operator >> (const YAML::Node& node, BuildConfigs& config);
void operator >> (const YAML::Node& node, Lib& lib);
void operator >> (const YAML::Node& node, File& file);


// Methods manipulating files and directories
bool FileExists(std::string filePath);
bool IsBuildUsExt(std::string filePathGiven);
bool IsCompileNeeded(path filePath, path outFilePath);
bool IsInTmp(path outFilePath);
path GetRootPath();
void CreateTmpDirectory();
void DeleteOldFile(BuildConfigs configs);
void RemoveTmpFiles();


// Constants
const std::string CLEAN_COMMAND = "clean";
const std::string BUILDUS_EXTENSION = ".buildus";
const std::string PROJECT_NODE = "project";
const std::string COMPILE_NODE = "compile";
const std::string DEPS_INCLUDE_NODE = "deps_include";
const std::string DEPS_LIBRARY_NODE = "deps_library";
const std::string LIBS_NODE = "libs";
const std::string VAR_NODE = "var";
const std::string GPP_COMPILE = "g++ -c";
const std::string GPP_LINK = "g++";
const std::string BUILDUS_FOLDER = "BuildUs";
const std::string TMP_FOLDER = "intermediate";
const std::string OUT_EXT = ".o";
const std::string OUT_NAME = " -o ";
const std::string INC_DIR = " -I";
const std::string INC_LIB_DIR = " -L";
const std::string INC_LIB = " -l";
const auto TMP_FOLDER_PATH = GetRootPath() / TMP_FOLDER;
