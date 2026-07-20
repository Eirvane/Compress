#include <iostream>
#include <cstring>
#include <vector>
#include <cstdint>
#include <fstream>     // 文件流操作
#include <sstream>
#include "core/bitstream.h"
#include "core/huffman.h"
#include "io/file_reader.h"

const char* MAGIC = "MYHM";  // 识别文件格式

// 先以二进制模式打开文件验证可读性，然后立即关闭。
bool compress(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream check(inputPath, std::ios::binary);    
    if (!check.is_open()) {
        std::cerr << "Cannot read input file: " << inputPath << "\n";
        return false;
    }
    check.close();

    auto data = FileReader::readAll(inputPath);
    std::cout << "Original size: " << data.size() << " bytes\n";

    // 根据原始数据统计频率，构建霍夫曼树和编码表
    HuffmanCodec codec;
    codec.build(data);

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Cannot create output file: " << outputPath << "\n";
        return false;    // 以二进制模式创建并覆盖输出文件。
    }

    // 写入文件头
    outFile.write(MAGIC, 4);
    uint8_t version = 1;
    outFile.write(reinterpret_cast<const char*>(&version), 1);
    uint64_t origSize = data.size();
    outFile.write(reinterpret_cast<const char*>(&origSize), 8);  // 原始文件大小，用于解压时知道要解码多少字节

    // 序列化霍夫曼树
    std::ostringstream treeStream;     // 内存输出流，用于收集序列化后的位数据
    {
        BitWriter treeWriter(treeStream);
        codec.serializeTree(treeWriter);
        treeWriter.flush();      // 确保缓冲区中剩余的位被写入
    }
    std::string treeStr = treeStream.str();
    uint32_t treeBytes = static_cast<uint32_t>(treeStr.size());
    outFile.write(reinterpret_cast<const char*>(&treeBytes), 4);
    outFile.write(treeStr.data(), treeStr.size());    // 序列化后的树占用的字节数

    // 编码数据
    std::ostringstream dataStream;
    {
		BitWriter dataWriter(dataStream);  // 内存输出流，用于收集编码后的位数据
        codec.encode(dataWriter, data);
        dataWriter.flush();
    }
    std::string dataStr = dataStream.str();
    uint64_t dataBytes = dataStr.size();
	outFile.write(reinterpret_cast<const char*>(&dataBytes), 8);   // 编码后的数据占用的字节数
    outFile.write(dataStr.data(), dataStr.size());

    // 总文件大小 = 魔数 + 版本号 + 原始大小 + 树大小 + 树数据 + 编码数据大小 + 编码数据
	uint64_t totalSize = 4 + 1 + 8 + 4 + treeBytes + 8 + dataBytes;  
	std::cout << "Compressed size: " << totalSize << " bytes\n";  // 输出压缩后的总文件大小
    std::cout << "Tree bytes: " << treeBytes << ", Data bytes: " << dataBytes << "\n";
    if (data.size() > 0) {
		std::cout << "Ratio: " << (100.0 * totalSize / data.size()) << "%\n";  // 输出压缩比
    }

    return true;
}

// 解压缩函数
bool decompress(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile) {
        std::cerr << "Cannot read input file: " << inputPath << "\n";   // 输出错误信息
        return false;
    }

    char magic[4];
    inFile.read(magic, 4);
	if (std::memcmp(magic, MAGIC, 4) != 0) {  // 验证文件头的魔数是否匹配，确保是正确的压缩文件格式
        std::cerr << "Invalid file format\n";
        return false;
    }

    uint8_t version;
	inFile.read(reinterpret_cast<char*>(&version), 1);  // 读取版本号
    if (version != 1) {
        std::cerr << "Unsupported version\n";    // 
        return false;
    }

	// 读取原始文件大小、霍夫曼树大小、霍夫曼树数据、编码数据大小和编码数据
    uint64_t origSize;
    inFile.read(reinterpret_cast<char*>(&origSize), 8);

    uint32_t treeBytes;
    inFile.read(reinterpret_cast<char*>(&treeBytes), 4);

    std::string treeData(treeBytes, '\0');
    inFile.read(treeData.data(), treeBytes);

    uint64_t dataBytes;
    inFile.read(reinterpret_cast<char*>(&dataBytes), 8);

    std::string compData(dataBytes, '\0');
    inFile.read(compData.data(), dataBytes);

	// 反序列化霍夫曼树
    std::istringstream treeStream(treeData);
    BitReader treeReader(treeStream);
    HuffmanCodec codec;
    codec.deserializeTree(treeReader);

	// 解码数据
    std::istringstream dataStream(compData);
    BitReader dataReader(dataStream);
    auto result = codec.decode(dataReader, origSize);

	// 将解码后的数据写入输出文件
    if (!FileWriter::writeAll(outputPath, result)) {
        std::cerr << "Cannot write output file: " << outputPath << "\n";
        return false;
    }

    std::cout << "Decompressed size: " << result.size() << " bytes\n";
    return true;
}

// 主函数，处理命令行参数，调用压缩或解压缩函数
int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage:\n";
        std::cout << "  " << argv[0] << " -c <input> <output>   Compress\n";
        std::cout << "  " << argv[0] << " -d <input> <output>   Decompress\n";
        return 1;
    }

	// 解析命令行参数，确定操作模式和输入输出文件路径
    std::string mode = argv[1];
    std::string input = argv[2];
    std::string output = argv[3];

    if (mode == "-c") {
        if (compress(input, output)) {
            std::cout << "Compression successful.\n";
            return 0;
        }
    }
    else if (mode == "-d") {
        if (decompress(input, output)) {
            std::cout << "Decompression successful.\n";
            return 0;
        }
    }
    else {
        std::cerr << "Unknown mode: " << mode << "\n";
    }

    return 1;
}


