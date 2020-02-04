#include "rsc/RResource.h"
#include "RDebug.h"

#include <fstream>
#include <sstream>
#include <regex>

using namespace Redopera;

std::string RResource::resourcesPath = "resource/";
std::mutex RResource::mutex;

const std::shared_ptr<RResource::ResourcesList> RResource::queryResourceList()
{
    // 获取ID列表后的读取都是线程安全的，若其他线程对其修改会新建一个一样的列表进行
    std::lock_guard<std::mutex> guard(mutex);
    return resourcesList();
}

std::string RResource::getTextFileContent(const std::string &path)
{
    std::string newpath = rscpath(path);

    std::string text;
    std::ifstream file;
    //若状态被置为failbit或badbit，则抛出异常
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(newpath);
        //读取文件缓冲到数据流
        std::stringstream sstream;
        sstream << file.rdbuf();

        file.close();
        text = sstream.str();
    }
    catch(...)
    {
        return "";
    }

    return text;
}

std::string RResource::rscpath(const std::string &path)
{
    std::string newpath;

    if(path[0] == ':')
        newpath = resourcesPath + path.substr(2);
    else
        newpath = path;

    return newpath;
}

void RResource::setResourcePath(const std::string &path)
{
    static std::regex r("(/|(../)+)?([-_a-z0-9][-_a-z0-9 ]*/)+", std::regex::icase|std::regex::optimize);
    if(check(!std::regex_match(path, r), "Invalid resource path: " + path))
        return;
    resourcesPath = path;
}

const std::string &RResource::getResourcePath()
{
    return resourcesPath;
}

RResource::RResource(const std::string &name, const std::string &typeName):
    name_(name),
    resourceID_(new unsigned(registerResourceID(name, typeName)), unregisterResourceID)
{

}

RResource::RResource(const RResource &rc):
    name_(rc.name_),
    resourceID_(rc.resourceID_)
{

}

RResource::RResource(const RResource &&rc):
    name_(std::move(rc.name_)),
    resourceID_(std::move(rc.resourceID_))
{

}

RResource &RResource::operator=(RResource rc)
{
    name_ = rc.name_;
    resourceID_ = rc.resourceID_;
    return *this;
}

void RResource::swap(RResource &rc) noexcept
{
    name_.swap(rc.name_);
    resourceID_.swap(rc.resourceID_);
}

RResource::~RResource()
{

}

RResource::ResourceID RResource::resourceID() const
{
    return *resourceID_;
}

const std::string &RResource::name() const
{
    return name_;
}

std::string RResource::nameAndID() const
{
    return '(' + std::to_string(*resourceID_) + ')' + name_;
}

void RResource::rename(const std::string &name)
{
    name_ = name;
}

RResource::ResourceID RResource::registerResourceID(const std::string &name, const std::string &typeName)
{
    ResourceID i = 0;
    std::lock_guard<std::mutex> guard(mutex);

    while(resourcesList()->count(i))
        ++i;

    // 若有其他线程在查询资源ID列表 则新建一个ID列表
    if(!resourcesList().unique())
        resourcesList() = std::make_shared<ResourcesList>(*resourcesList());
    resourcesList()->emplace(i, ResourceInfo{ name, typeName });
    return i;
}

std::shared_ptr<RResource::ResourcesList> &RResource::resourcesList()
{
    static std::shared_ptr<ResourcesList> RESOURCE_LIST(new ResourcesList);
    return RESOURCE_LIST;
}

void RResource::unregisterResourceID(unsigned *ID)
{
    std::lock_guard<std::mutex> guard(mutex);
    assert(resourcesList()->count(*ID));

    // 若有其他线程在查询资源ID列表 则新建一个ID列表
    if(!resourcesList().unique())
        resourcesList() = std::make_shared<ResourcesList>(*resourcesList());
    resourcesList()->erase(*ID);

    delete ID;
}

void swap(RResource &rc1, RResource &rc2)
{
    rc1.swap(rc2);
}
