#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

namespace garden {

class ErrorPrinter : public google::protobuf::compiler::MultiFileErrorCollector {
public:
    void AddError(std::string const& filename, int line, int column, 
            std::string const& msg) override {
        std::cerr << "compile protobuf error:" << filename << ":" << line << ":" << column 
                << ":" << msg << std::endl;
    }
};

class DynamicImporter {
public:
    struct ImporterInter {
        std::shared_ptr<::google::protobuf::compiler::Importer> importer;
        std::shared_ptr<::google::protobuf::DynamicMessageFactory> factory;
    };
    typedef std::shared_ptr<ImporterInter> ImporterFactory;

public:
    DynamicImporter(const std::string& dir);
    
    bool load();

    bool reload();

    // 通过名字获得messag实例的入口
    bool get_message_by_name(
            const std::string& name, std::shared_ptr<::google::protobuf::Message>& msg);

private:
    static std::vector<std::string> find_files_by_dir(const std::string& dir);

private:
    // proto文件存放的目录
    std::string _dir;

    ::google::protobuf::compiler::DiskSourceTree _source_tree;
    ErrorPrinter _err_printer;

    ImporterFactory _importer;
    // 保存旧版本的工厂，避免message生命周期问题
    std::vector<ImporterFactory> _importer_history;
};

} // namespace garden
