/**
 *  @file       file_methods.cpp
 *
 *  @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 *  @copyright  MIT License
 */

#include <libKitsunemimiPersistence/files/file_methods.h>

namespace Kitsunemimi
{
namespace Persistence
{

/**
 * @brief check if a path exist
 *
 * @param path path to check
 *
 * @return true, if path exist, else false
 */
bool
doesPathExist(const std::string path)
{
    return boost::filesystem::exists(path);
}

/**
 * @brief check if a path exist and is a directory
 *
 * @param filePath path to check
 *
 * @return true, if path exist and is a file, else false
 */
bool
isFile(const std::string filePath)
{
    if(boost::filesystem::exists(filePath) == false
            || boost::filesystem::is_regular(filePath) == false)
    {
        return false;
    }

    return true;
}

/**
 * @brief check if a path exist and is a directory
 *
 * @param dirPath path to check
 *
 * @return true, if path exist and is a directory, else false
 */
bool
isDir(const std::string dirPath)
{
    if(boost::filesystem::exists(dirPath) == false
            || boost::filesystem::is_directory(dirPath) == false)
    {
        return false;
    }

    return true;
}

/**
 * @brief get parent-path of a path
 *
 * @param path original path
 *
 * @return parent-path
 */
const std::string
getParent(const std::string &path)
{
    boost::filesystem::path pathObj(path);
    return pathObj.parent_path().string();
}

/**
 * @brief iterate over a directory and subdirectory to file all containing files
 *
 * @param fileList resulting string-list with the absolute path of all found files
 * @param directory directory-path where to search
 * @param withSubdirs false, to list only files in the current directory, but not files from
 *                    subdirectories
 * @param exceptions list with directory-names, which should be skipped
 */
void
getFilesInDir(std::vector<std::string> &fileList,
              const boost::filesystem::path &directory,
              const bool withSubdirs,
              const std::vector<std::string> &exceptions)
{
    boost::filesystem::directory_iterator end_itr;
    for(boost::filesystem::directory_iterator itr(directory);
        itr != end_itr;
        ++itr)
    {
        if(is_directory(itr->path()))
        {
            if(withSubdirs == true)
            {
                if(exceptions.size() != 0)
                {
                    for(uint64_t i = 0; i < exceptions.size(); i++)
                    {
                        bool found = false;
                        if(itr->path().leaf().string() == exceptions.at(i)) {
                            found = true;
                        }

                        if(found == false) {
                            getFilesInDir(fileList, itr->path(), withSubdirs, exceptions);
                        }
                    }
                }
                else
                {
                    getFilesInDir(fileList, itr->path(), withSubdirs, exceptions);
                }
            }
        }
        else
        {
            fileList.push_back(itr->path().string());
        }
    }
}

/**
 * @brief iterate over a directory and subdirectory to file all containing files
 *
 * @param fileList resulting string-list with the absolute path of all found files
 * @param path path where to search. This should be a directory. If this is a file-path, this path
 *             is the only one in the resulting list
 * @param withSubdirs false, to list only files in the current directory, but not files from
 *                    subdirectories (Default: true)
 * @param exceptions list with directory-names, which should be skipped (Default: empty list)
 */
void
listFiles(std::vector<std::string> &fileList,
          const std::string &path,
          const bool withSubdirs,
          const std::vector<std::string> &exceptions)
{
    boost::filesystem::path pathObj(path);

    if(is_directory(pathObj))
    {
        getFilesInDir(fileList,
                      pathObj,
                      withSubdirs,
                      exceptions);
    }
    else
    {
        fileList.push_back(path);
    }
}

/**
 * @brief rename a file or directory
 *
 * @param oldPath origial path
 * @param newPath new path after renaming
 * @param errorMessage reference for error-message output
 *
 * @return true, if successful, else false
 */
bool
renameFileOrDir(const std::string &oldPath,
                const std::string &newPath,
                std::string &errorMessage)
{
    if(doesPathExist(oldPath) == false)
    {
        errorMessage = "source-path " + oldPath + " doesn't exist.";
        return false;
    }

    boost::system::error_code boostError;
    boost::filesystem::rename(oldPath, newPath, boostError);

    if(boostError.value() != 0)
    {
        errorMessage = boostError.message();
        return false;
    }

    return true;
}

/**
 * @brief copy a file or directory
 *
 * @param sourcePath origial path
 * @param targetPath path of the copy
 * @param errorMessage reference for error-message output
 * @param force true to delete target, if already exist, if something exist at the target-location
 *              (Default: true)
 *
 * @return true, if successful, else false
 */
bool
copyPath(const std::string &sourcePath,
         const std::string &targetPath,
         std::string &errorMessage,
         const bool force)
{
    if(doesPathExist(sourcePath) == false)
    {
        errorMessage = "source-path " + sourcePath + " doesn't exist.";
        return false;
    }

    boost::system::error_code boostError;
    if(force) {
        boost::filesystem::remove_all(targetPath);
    }
    boost::filesystem::copy(sourcePath, targetPath, boostError);

    if(boostError.value() != 0)
    {
        errorMessage = boostError.message();
        return false;
    }

    return true;
}

/**
 * @brief create a directory
 *
 * @param path path to create
 * @param errorMessage reference for error-message output
 *
 * @return true, if successful, else false
 */
bool
createDirectory(const std::string &path,
                std::string &errorMessage)
{
    if(isFile(path))
    {
        errorMessage = "under path " + path + " a file already exist.";
        return false;
    }

    boost::system::error_code boostError;
    const bool result = boost::filesystem::create_directories(path, boostError);

    if(result == false) {
        errorMessage = boostError.message();
    }

    return result;
}

/**
 * @brief delete a path
 *
 * @param path path to delete
 * @param errorMessage reference for error-message output
 *
 * @return true, if successful, else false. Also return true, if path is already deleted.
 */
bool
deleteFileOrDir(const std::string &path,
                std::string &errorMessage)
{
    if(doesPathExist(path) == false) {
        return true;
    }

    boost::system::error_code boostError;
    const bool result = boost::filesystem::remove_all(path, boostError);

    if(result == false) {
        errorMessage = boostError.message();
    }

    return result;
}

}
}