#include "includes.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/types.h>
    #include <dirent.h>
#endif

#ifdef __VXWORKS__
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

/* 0 - sucess -1 - failed */
static int vxRemoveDir
    (
    const char * dirName
    )
    {
    char            fileName [PATH_MAX];
    DIR *           odir;
    struct dirent * ent;
    int             fd;
    struct stat     fsST;
   
    if (dirName == nullptr) {
        return -1;
        }

    odir = opendir (dirName);
    if (odir == nullptr) {
        return -1;
        }

    while ((ent = readdir (odir)) != nullptr) {
        if ((strncmp(ent->d_name, ".", PATH_MAX) == 0) || 
            (strncmp(ent->d_name, "..", PATH_MAX) == 0)) {
            continue;
            }
        (void) sprintf (fileName, "%s/%s", dirName, ent->d_name);

        fd = open (fileName, O_RDONLY, 0777);
        if (fd == -1) {
            (void) closedir (odir);
            return -1;
            }
        
        (void) fstat (fd, &fsST);
        (void) close (fd);
        if (S_ISDIR (fsST.st_mode)) {
            if (vxRemoveDir (fileName) == -1) {
                return -1;
                }
            }
        // remove this file
        (void) remove (fileName);
        }
    (void) closedir (odir);
    rmdir (dirName);

    return 0;
    }
#endif

void prepare_logdir() {
    spdlog::drop_all();
#ifdef _WIN32
    system("rmdir /S /Q test_logs");
#elif __VXWORKS__
    // Sometimes, the directory test_logs is not exist, vxRemoveDir return -1.
    // So don't check return value. 
    (void) vxRemoveDir ("test_logs"); 
#else
    auto rv = system("rm -rf test_logs");
    if (rv != 0) {
        throw std::runtime_error("Failed to rm -rf test_logs");
    }
#endif
}

std::string file_contents(const std::string &filename) {
    std::ifstream ifs(filename, std::ios_base::binary);
    if (!ifs) {
        throw std::runtime_error("Failed open file ");
    }
    return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
}

std::size_t count_lines(const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs) {
        throw std::runtime_error("Failed open file ");
    }

    std::string line;
    size_t counter = 0;
    while (std::getline(ifs, line)) counter++;
    return counter;
}

void require_message_count(const std::string &filename, const std::size_t messages) {
    if (strlen(spdlog::details::os::default_eol) == 0) {
        REQUIRE(count_lines(filename) == 1);
    } else {
        REQUIRE(count_lines(filename) == messages);
    }
}

std::size_t get_filesize(const std::string &filename) {
    std::ifstream ifs(filename, std::ifstream::ate | std::ifstream::binary);
    if (!ifs) {
        throw std::runtime_error("Failed open file ");
    }

    return static_cast<std::size_t>(ifs.tellg());
}

// source: https://stackoverflow.com/a/2072890/192001
bool ends_with(std::string const &value, std::string const &ending) {
    if (ending.size() > value.size()) {
        return false;
    }
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

#ifdef _WIN32
// Based on: https://stackoverflow.com/a/37416569/192001
std::size_t count_files(const std::string &folder) {
    size_t counter = 0;
    WIN32_FIND_DATAA ffd;

    // Start iterating over the files in the folder directory.
    HANDLE hFind = ::FindFirstFileA((folder + "\\*").c_str(), &ffd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do  // Managed to locate and create an handle to that folder.
        {
            if (ffd.cFileName[0] != '.') counter++;
        } while (::FindNextFileA(hFind, &ffd) != 0);
        ::FindClose(hFind);
    } else {
        throw std::runtime_error("Failed open folder " + folder);
    }

    return counter;
}
#else
// Based on: https://stackoverflow.com/a/2802255/192001
std::size_t count_files(const std::string &folder) {
    size_t counter = 0;
    DIR *dp = opendir(folder.c_str());
    if (dp == nullptr) {
        throw std::runtime_error("Failed open folder " + folder);
    }

    struct dirent *ep = nullptr;
    while ((ep = readdir(dp)) != nullptr) {
        if (ep->d_name[0] != '.') counter++;
    }
    (void)closedir(dp);
    return counter;
}
#endif
