#include <iostream>
#include <cstring>
#include <vector>
#include <cstdint>
#include <fstream>
#include <sstream>
#include "core/bitstream.h"

const char* MAGIC = "MYHM";

bool compress(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream check(inputPath, std::ios::binary);   // 先以二进制模式打开文件验证可读性，然后立即关闭。
    if (!check.is_open()) {
        std::cerr << "Cannot read input file: " << inputPath << "\n";
        return false;
    }
    check.close();