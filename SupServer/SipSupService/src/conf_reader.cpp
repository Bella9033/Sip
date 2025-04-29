#include "conf_reader.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <sstream>

// 辅助函数：字符串编辑距离（Levenshtein distance），用于拼写建议
static size_t editDistance(std::string_view a, std::string_view b) 
{
    size_t m = a.size(), n = b.size();
    std::vector<size_t> dp(n + 1);
    for (size_t j = 0; j <= n; ++j) 
    {
        dp[j] = j;
    }
    for (size_t i = 1; i <= m; ++i) 
    {
        size_t pre = dp[0]; dp[0] = i;
        for (size_t j = 1; j <= n; ++j) 
        {
            size_t tmp = dp[j];
            if (a[i-1] == b[j-1]) dp[j] = pre;
            else dp[j] = std::min({dp[j-1], dp[j], pre}) + 1;
            pre = tmp;
        }
    }
    return dp[n];
}

ConfReader::ConfReader(const std::string& filename)
    : filename_(filename) 
{
    parse();
}

std::optional<std::string> ConfReader::getString(std::string_view section, 
    std::string_view key, std::string* errMsg) const
{
    auto secIt = sections_.find(std::string(section));
    if (secIt == sections_.end()) {
        if (errMsg) 
        {
            std::ostringstream oss;
            oss << "Section [" << section << "] not found.";
            auto suggestions = suggestKeys(section, getSections());
            if (!suggestions.empty()) 
            {
                oss << " Did you mean: ";
                for (const auto& s : suggestions) 
                {oss << "[" << s << "] ";}
            }
            *errMsg = oss.str();
        }
        return std::nullopt;
    }
    const auto& kv = secIt->second;
    auto keyIt = kv.find(std::string(key));
    if (keyIt == kv.end()) 
    {
        if (errMsg) 
        {
            std::ostringstream oss;
            oss << "Key [" << key << "] not found in section [" << section << "].";
            auto suggestions = suggestKeys(key, getKeys(section));
            if (!suggestions.empty()) 
            {
                oss << " Did you mean: ";
                for (const auto& k : suggestions) oss << "[" << k << "] ";
            }
            *errMsg = oss.str();
        }
        return std::nullopt;
    }
    return keyIt->second;
}

std::optional<int> ConfReader::getInt(std::string_view section, 
    std::string_view key, std::string* errMsg) const
{
    std::string innerErr;
    auto valOpt = getString(section, key, &innerErr);
    if (!valOpt) {
        if (errMsg) *errMsg = innerErr;
        return std::nullopt;
    }
    try {
        size_t idx = 0;
        int v = std::stoi(*valOpt, &idx);
        if (idx != valOpt->size()) 
        {
            if (errMsg) *errMsg = "Value for [" + std::string(section) + "." 
                + std::string(key) + "] is not a valid integer: [" + *valOpt + "]";
            { return std::nullopt; }
        }
        return v;
    } catch (...) {
        if (errMsg) *errMsg = "Value for [" + std::string(section) + "." 
            + std::string(key) + "] is not a valid integer: [" + *valOpt + "]";
        { return std::nullopt; }
    }
}

std::vector<std::string> ConfReader::getSections() const 
{
    std::vector<std::string> out;
    for (const auto& kv : sections_) 
    { 
        out.push_back(kv.first); 
    }
    return out;
}

std::vector<std::string> ConfReader::getKeys(std::string_view section) const {
    std::vector<std::string> out;
    auto it = sections_.find(std::string(section));
    if (it != sections_.end()) 
    {
        for (const auto& kv : it->second) 
        { out.push_back(kv.first); }
    }
    return out;
}

void ConfReader::parse() 
{
    if (parsed_) return;
    std::ifstream ifs(filename_);
    if (!ifs) return;

    std::string line, current_section;
    while (std::getline(ifs, line)) 
    {
        auto trimmed_sv = trim(line);
        if (trimmed_sv.empty() || trimmed_sv.front() == '#') continue;
        std::string_view trimmed = trimmed_sv;

        if (trimmed.front() == '[' && trimmed.back() == ']') 
        {
            current_section = std::string(trim(trimmed.substr(1, trimmed.size() - 2)));
            continue;
        }
        size_t pos = trimmed.find('=');
        if (pos == std::string::npos) { continue; }

        std::string_view key = trim(trimmed.substr(0, pos));
        std::string_view value = trim(trimmed.substr(pos + 1));
        if (!current_section.empty() && !key.empty())
            sections_[current_section][std::string(key)] = std::string(value);
    }
    parsed_ = true;
}

std::string_view ConfReader::trim(std::string_view sv) 
{
    while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.front()))) 
        { sv.remove_prefix(1); }
    while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.back()))) 
        { sv.remove_suffix(1); }
    return sv;
}

std::vector<std::string> ConfReader::suggestKeys(std::string_view input, 
    const std::vector<std::string>& candidates, size_t maxSuggest) 
{
    std::vector<std::pair<size_t, std::string>> scored;
    for (const auto& s : candidates) 
    {
        size_t ed = editDistance(input, s);
        if (s.find(std::string(input)) != std::string::npos || ed < 4) // 提高容错
            {scored.emplace_back(ed, s);}
    }
    std::sort(scored.begin(), scored.end());
    std::vector<std::string> out;
    for (size_t i = 0; i < std::min(maxSuggest, scored.size()); ++i)
        {out.push_back(scored[i].second);}
    return out;
}