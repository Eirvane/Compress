#pragma once
#include <vector>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "bitstream.h"

// 不断合并频率最小的两个节点，形成新的内部节点
struct HuffmanNode {
    uint8_t byte = 0;                     // 叶子节点存储的字节值
	uint64_t freq = 0;                    // 节点（字节）出现的频率
	std::shared_ptr<HuffmanNode> left;    // 左子节点 
	std::shared_ptr<HuffmanNode> right;   // 右子节点
    bool isLeaf = false;                  // 标记是否为叶子节点

    HuffmanNode(uint8_t b, uint64_t f) : byte(b), freq(f), isLeaf(true) {}      // 构造叶子节点
    HuffmanNode(std::shared_ptr<HuffmanNode> l, std::shared_ptr<HuffmanNode> r) // 构造内部节点（合并两个子节点）
        : freq(l->freq + r->freq), left(l), right(r), isLeaf(false) {
    }
};

//  公共接口
class HuffmanCodec {
public:
    void build(const std::vector<uint8_t>& data);     // 根据输入数据统计频率，构建霍夫曼树和编码表
    void encode(BitWriter& writer, const std::vector<uint8_t>& data);  // 将原始数据按霍夫曼编码写入位流
	void encode(BitWriter& writer, const std::vector<uint8_t>& data);  // 根据编码表将数据写入比特流
    std::vector<uint8_t> decode(BitReader& reader, uint64_t originalSize);  // 从位流中解码，恢复原始数据
    void serializeTree(BitWriter& writer);
    void deserializeTree(BitReader& reader);  // 从位流反序列化，重建霍夫曼树

// 私有成员
private:
    std::shared_ptr<HuffmanNode> root;   // 霍夫曼树的根节点，解码时从根开始遍历
    std::unordered_map<uint8_t, std::vector<bool>> codeTable;  // 编码表

    void buildCodeTable(std::shared_ptr<HuffmanNode> node, std::vector<bool>& path);  // 递归遍历树，生成每个叶子节点的编码路径
    void serializeNode(BitWriter& writer, std::shared_ptr<HuffmanNode> node);  // 递归序列化单个节点
    std::shared_ptr<HuffmanNode> deserializeNode(BitReader& reader);  // 递归反序列化，重建树结构
}; 