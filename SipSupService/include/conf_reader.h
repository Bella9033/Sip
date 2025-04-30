#pragma once

#include "common.h"
#include <string_view>
#include <unordered_map>
#include <optional>
#include <vector>

class ConfReader 
{
public:
    explicit ConfReader(const std::string& filename);
    ~ConfReader() = default;

    ConfReader(const ConfReader&) = delete;
    ConfReader& operator=(const ConfReader&) = delete;

    // 获取字符串配置，若查不到会给出详细报错和建议
    std::optional<std::string> getString(std::string_view section, 
        std::string_view key, std::string* errMsg = nullptr) const;

    // 获取整型配置，类型校验，支持详细错误提示
    std::optional<int> getInt(std::string_view section, 
        std::string_view key, std::string* errMsg = nullptr) const;

    // 获取所有section和key，便于做拼写建议
    std::vector<std::string> getSections() const;
    std::vector<std::string> getKeys(std::string_view section) const;

    std::string getFilename() const { return filename_; }
private:
    std::string filename_;
    std::unordered_map<std::string, 
        std::unordered_map<std::string, std::string>> sections_;

    void parse();
    static std::string_view trim(std::string_view sv);
    static std::vector<std::string> suggestKeys(std::string_view input, 
        const std::vector<std::string>& candidates, size_t maxSuggest = 3);

    bool parsed_{false};
};