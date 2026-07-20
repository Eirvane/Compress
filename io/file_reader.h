#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <fstream>

//  静态方法 + 全量读写
class FileReader {
public:
    static std::vector<uint8_t> readAll(const std::string& path);  // 避免路径字符串拷贝，只读引用
};

class FileWriter {
public:
    static bool writeAll(const std::string& path, const std::vector<uint8_t>& data);
};
