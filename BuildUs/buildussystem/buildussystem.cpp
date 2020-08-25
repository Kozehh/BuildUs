#include "buildussystem.h"

#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <yaml-cpp/yaml.h>

using boost::filesystem::path;

// Delete .o file in intermediate folder (clean command)
void Clean()
{
    if(boost::filesystem::exists(TMP_FOLDER_PATH))
    {
        std::cout << "Cleaning the temporary files ..\n";
        RemoveTmpFiles();
    }
    else
    {
        std::cout << "There is nothing to clean\n"; 
    }
}

// Parsing the .buildus file the user gave
BuildConfigs ParseConfigFile(std::string configFilePath)
{
    try {
        auto doc = YAML::LoadFile(configFilePath);
        BuildConfigs configs;
        doc >> configs;
        return configs;
    } catch(YAML::ParserException& e) {
        std::cout << e.msg << std::endl;
    }
}

// The whole pipeline of building an executable
void Build(BuildConfigs configs)
{
    CreateTmpDirectory();
    Compile(configs);
    Link(configs);
    DeleteOldFile(configs);
}

void Compile(BuildConfigs configs)
{
    // Get the path of the environment variable
    std::string includeDirPath = getenv(configs.enVarDepsInclude.c_str());
    // For each files in the filesToCompile list
    for(File file : configs.filesToCompile)
    {
        // Check if the file needs to be compile
        if(IsCompileNeeded(path{file.filePath}, path{file.fileOutputPath}))
        {
            auto compileCommand = GPP_COMPILE;
            compileCommand += " " + file.filePath + OUT_NAME + file.fileOutputPath;
            compileCommand += INC_DIR + includeDirPath;
            // Executes the compile command in the terminal
            auto res = Exec(compileCommand.c_str());
            if(res.compare("") != 0){
                // Print the error if there is one 
                std::cout << res << std::endl;
            }            
            // Set the last write time of the file as of now
            boost::filesystem::last_write_time(path{file.filePath});
        }
    }
}

// Linking the .o files to build an executable
void Link(BuildConfigs configs)
{
    auto linkCommand = GPP_LINK;
    std::string incLidDir = getenv(configs.depsLibrary.enVarLibDirectory.c_str());
    for(File file : configs.filesToCompile)
    {
        linkCommand += " " + file.fileOutputPath;
    }
    linkCommand += INC_LIB_DIR + incLidDir;
    for(std::string lib : configs.depsLibrary.libList)
    {
        linkCommand += INC_LIB + lib;
    }
    linkCommand += OUT_NAME + configs.exePath;
    std::cout<<linkCommand.c_str()<<std::endl;
    auto res = Exec(linkCommand.c_str());
    if(res.compare("") != 0){
        // Print the error if there is one 
        std::cout << res << std::endl;
    }
}

void RemoveTmpFiles(){
    for (boost::filesystem::directory_iterator end_dir_it, it(TMP_FOLDER_PATH); it!=end_dir_it; ++it) {
        // Remove only the temporary files (.o)
        if((it->path().extension().string()).compare(OUT_EXT) == 0){
            boost::filesystem::remove_all(it->path());
        }        
    }
}

// Get the root path of the projects
path GetRootPath()
{
    return boost::filesystem::current_path();
}

// Create a tmp directory that will contains temporary files (.o and executable)
void CreateTmpDirectory()
{
    if (!boost::filesystem::exists(TMP_FOLDER_PATH))
    {
        boost::system::error_code ec;
        boost::filesystem::create_directory(TMP_FOLDER_PATH, ec);
		if (ec)
		{
			std::cout << ec.message() << std::endl;
		}
    }
}


//look at an uncompiled file and determine if it's .o need to be recompile
bool IsCompileNeeded(path filePath, path outFilePath)
{
    if(IsInTmp(outFilePath))
    {
        //si derniere modif du file > dernierBuild (peut checker tmp folder?)
        auto tmpFolderPath = TMP_FOLDER_PATH;
        auto lastBuildTime = last_write_time(tmpFolderPath);
        auto lastWriteTime = last_write_time(filePath);
        if(lastBuildTime < lastWriteTime)
        {
            return true;
        }
        return false;
    }
    return true;
}

// Check if a temp file is in the tmp directory
bool IsInTmp(path outFilePath)
{
	for (auto entry : boost::filesystem::directory_iterator(TMP_FOLDER_PATH))
	{
		if (entry.path().compare(outFilePath) == 0)
		{
            return true;
		}
	}
    return false;
}

// Look at an uncompiled file and determine if it's .o need to be recompile
void DeleteOldFile(BuildConfigs configs)
{
    //fileToSave
    std::vector<std::string> fileToSave;
    fileToSave.push_back(configs.exePath);
    for(File file : configs.filesToCompile)
    {
        fileToSave.push_back(file.fileOutputPath);
    }
    
	for (auto entry : boost::filesystem::directory_iterator(TMP_FOLDER_PATH))
	{
        // If there is other files in tmp than the ones we are trying to compile
        if(std::find(fileToSave.begin(), fileToSave.end(),entry.path()) == std::end(fileToSave))
        {
            // Remove the file
            remove(entry.path());
        }
	}
}

// Check if the file exists
// Arg: The absolute path of the file
// Return: true if the file exists and false if not
bool FileExists(std::string filePathGiven)
{
    return boost::filesystem::exists(path {filePathGiven}) ? true : false;
}

// Checks if the config file received from the user has a .buildus extension
bool IsBuildUsExt(std::string filePathGiven)
{
    path fileName{filePathGiven};
    auto fileExt = fileName.extension().string();
    return fileExt.compare(BUILDUS_EXTENSION) == 0 ? true : false;
}

// Found this method at :
// https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
// This code executes a command in the terminal and returns the terminal output
std::string Exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    // Opens a terminal and executes the command received in argument
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    // Reads the output from the terminal and add it in a string
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

///
/// Methods for reading our yaml nodes
///

// Override operator >> method for reading a File node
void operator >> (const YAML::Node& node, File& file)
{
    // Read all the File nodes
    for(auto it=node.begin();  it!=node.end(); ++it)
    {
        // Get the file output name add set the file variable with the tmp directory path with the .o extension
        file.fileOutputPath = TMP_FOLDER_PATH.string() + "/" + it->first.as<std::string>() + OUT_EXT;
        // Get the file path of the file to compile
        file.filePath = GetRootPath().string() + "/" + it->second.as<std::string>();
    }
    
}

// Override operator >> method for reading a Lib node
void operator >> (const YAML::Node& node, Lib& lib)
{
    // Read all the Lib nodes
    lib.enVarLibDirectory = node[VAR_NODE].as<std::string>();
    YAML::Node libs;
    if(node[LIBS_NODE] != NULL)
    {
        libs = node[LIBS_NODE];
        for(size_t indexLibs = 0; indexLibs < libs.size(); indexLibs++)
        {
            std::string libName;
            libName = libs[indexLibs].as<std::string>();
            lib.libList.push_back(libName);
        }
    }
    else{
        throw YAML::ParserException(YAML::Mark::null_mark(), "\nError: There is no library listed for " + lib.enVarLibDirectory + "\n");
    }
}

// Override operator >> method for reading a BuildConfigs node
void operator >> (const YAML::Node& node, BuildConfigs& config)
{
    // Read all the config nodes
    config.exePath = (TMP_FOLDER_PATH / node[PROJECT_NODE].as<std::string>()).string();
    const auto& depsInclude = node[DEPS_INCLUDE_NODE];
    config.enVarDepsInclude = depsInclude[VAR_NODE].as<std::string>();
    const auto& depsLib = node[DEPS_LIBRARY_NODE];
    depsLib >> config.depsLibrary;
    const auto& compileList = node[COMPILE_NODE];

    if(compileList.size() != 0){
        // For each file in the compile node
        for(size_t files = 0; files < compileList.size(); files++)
        {
            File file;
            // Read the file scalar 
            compileList[files] >> file;
            // Add the file read to the list of files to compile
            config.filesToCompile.push_back(file);
        }
    }
    else{
        throw YAML::ParserException(YAML::Mark::null_mark(), "\nError: There is no files listed to compile..\n");
    }
}