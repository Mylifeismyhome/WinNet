#pragma once
#include <Net/Net.h>
#include <assets/manager/logmanager.h>

#define DIRMANAGER Net::manager::dirmanager
#define CREATEDIR Net::manager::dirmanager::createDir
#define DELETEDIR Net::manager::dirmanager::deleteDir
#define SCANDIR Net::manager::dirmanager::scandir
#define CURRENTDIR(x) Net::manager::dirmanager::currentDir(x)

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(manager)
BEGIN_NAMESPACE(dirmanager)
extern "C" NET_API bool createDir(char*);
extern "C" NET_API bool deleteDir(char*, bool = true);

std::vector<char*> scandir(char*);

extern "C" NET_API void currentDir(char*&);
END_NAMESPACE
END_NAMESPACE
END_NAMESPACE