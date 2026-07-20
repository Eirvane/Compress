#include "file_reader.h"

std::vector<uint8_t> FileReader::readAll(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);  // 二进制模式打开文件，打开后立即定位到文件末尾
    if (!file) return {};          // 文件打开失败时返回空向量。
    auto size = file.tellg();      // 返回当前读取位置，返回值为文件大小
    if (size <= 0) return {};      // 大小为 0 或负数时直接返回空向量
    file.seekg(0, std::ios::beg);  // 将读取位置重置到文件开头，准备读取全部内容。
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

// 以二进制模式打开文件用于写入,如果文件已存在则清空,如果不存在则新创建一个
bool FileWriter::writeAll(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;    // 打开失败时返回 false
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return file.good();
}