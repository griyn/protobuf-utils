#include "dynamic_importer.h"

#include <dirent.h>
#include <sys/types.h>
#include <fstream>
#include <streambuf>

namespace garden {

DynamicImporter::DynamicImporter(const std::string& dir) : _dir(dir) {
    _source_tree.MapPath("", _dir);
}

bool DynamicImporter::load() {
    return reload();
}

bool DynamicImporter::reload() {
    auto files = find_files_by_dir(_dir);
    
    if (files.empty()) {
        std::cerr << "files empty" << std::endl;
        return false;
    }

    ImporterFactory importer_factory(new ImporterInter);

    importer_factory->importer.reset(
            new ::google::protobuf::compiler::Importer(&_source_tree, &_err_printer));

    for (auto& file : files) {
        auto* file_desc = importer_factory->importer->Import(file);
        if (file_desc == nullptr) {
            std::cerr << "import proto file faied, file:" << file << std::endl;
            return false;
        }
    }
    importer_factory->factory.reset(
            new ::google::protobuf::DynamicMessageFactory(importer_factory->importer->pool()));

    // 原子拷贝ptr，否则要使用锁
    std::atomic_store(&_importer, importer_factory);
          
    _importer_history.emplace_back(importer_factory);

    return true;
}

bool DynamicImporter::get_message_by_name(
        const std::string& name, std::shared_ptr<::google::protobuf::Message>& msg) {
    auto importer_factory = std::atomic_load(&_importer);
    if (importer_factory == nullptr) {
        std::cerr << "importer is nullptr" << std::endl;
        return false;
    }
    
    // 用名字生成对应message实例
    auto* descriptor = importer_factory->importer->pool()->FindMessageTypeByName(name);
    if (descriptor == nullptr){
        std::cerr << "importer find message failed, name:" << std::endl;
        return false;
    }

    msg.reset(importer_factory->factory->GetPrototype(descriptor)->New());
    return true;
}

std::vector<std::string> DynamicImporter::find_files_by_dir(
        const std::string& dir_path) {
    std::vector<std::string> files;
    DIR* dir = opendir(dir_path.c_str());
    struct dirent* ent;
    if (dir != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            std::string file = ent->d_name;
            // 只使用.proto结尾的文件, c++20可以使用ends_with
            if (file.find(".proto") != std::string::npos && file != "descriptor.proto") {
                files.emplace_back(std::move(file));
            }
        }
        closedir(dir);
    }
    return files;
}

} // namespace garden
