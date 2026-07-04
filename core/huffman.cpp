#include "huffman.h"
#include <queue>
#include <algorithm>
#include <iostream>

// 构建霍夫曼树
// 统计 256 种字节各自的频率，时间复杂度 O(n)，n 为数据长度。
void HuffmanCodec::build(const std::vector<uint8_t>& data) {
    std::vector<uint64_t> freq(256, 0);
    for (uint8_t b : data) freq[b]++;

    // 使用最小堆（priority_queue + 自定义比较器），堆顶是频率最小的节点。
    auto cmp = [](const std::shared_ptr<HuffmanNode>& a, const std::shared_ptr<HuffmanNode>& b) {
        return a->freq > b->freq;
        };
    std::priority_queue<std::shared_ptr<HuffmanNode>, std::vector<std::shared_ptr<HuffmanNode>>, decltype(cmp)> pq(cmp);

    // 只将出现过的字节加入堆，未出现的字节不参与建树。
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            pq.push(std::make_shared<HuffmanNode>(static_cast<uint8_t>(i), freq[i]));
        }
    }

    // 空输入处理：pq.empty() 时设 root = nullptr，则安全。
    if (pq.empty()) {
        root = nullptr;
        return;
    }

    if (pq.size() == 1) {
        root = pq.top();
    }
    else {
        while (pq.size() > 1) {
            auto left = pq.top(); pq.pop();
            auto right = pq.top(); pq.pop();
            pq.push(std::make_shared<HuffmanNode>(left, right));
        }
        root = pq.top();
    }

    // 递归生成编码表
    std::vector<bool> path;
    buildCodeTable(root, path);
}

// 前序遍历生成编码
void HuffmanCodec::buildCodeTable(std::shared_ptr<HuffmanNode> node, std::vector<bool>& path) {
    if (!node) return;
    if (node->isLeaf) {
        codeTable[node->byte] = path;
        return;
    }
    path.push_back(false);               // 左分支 = 0
    buildCodeTable(node->left, path);
    path.pop_back();

    path.push_back(true);                // 右分支 = 1
    buildCodeTable(node->right, path);   
    path.pop_back();
}

//  编码数据.查表编码，时间复杂度 O(n × 平均码长)
void HuffmanCodec::encode(BitWriter& writer, const std::vector<uint8_t>& data) {
    for (uint8_t b : data) {
        const auto& code = codeTable[b];
        for (bool bit : code) {
            writer.writeBit(bit);
        }
    }
}

//  解码数据.遍历霍夫曼树，时间复杂度 O(n × 平均码长)
std::vector<uint8_t> HuffmanCodec::decode(BitReader& reader, uint64_t originalSize) {
    std::vector<uint8_t> result;
    result.reserve(originalSize);

    if (!root) return result;

    if (root->isLeaf) {
        result.assign(originalSize, root->byte);
        return result;
    }

    // 从根出发，按位遍历树，到达叶子即输出。
    for (uint64_t i = 0; i < originalSize; ++i) {
        auto node = root;
        while (!node->isLeaf) {
            bool bit = reader.readBit();
            node = bit ? node->right : node->left;
        }
        result.push_back(node->byte);
    }

    return result;
}

// 序列化树。采用 前序遍历+标记位 的序列化方式。
// 格式：1 位类型标记 +（如果是叶子）8 位字节值
void HuffmanCodec::serializeTree(BitWriter& writer) {
    serializeNode(writer, root);
}

void HuffmanCodec::serializeNode(BitWriter& writer, std::shared_ptr<HuffmanNode> node) {
    if (!node) return;
    if (node->isLeaf) {
        writer.writeBit(true);           // 1 位标记：叶子
        writer.writeByte(node->byte);    // 8 位：字节值
    }
    else {
        writer.writeBit(false);          // 1 位标记：内部节点
        serializeNode(writer, node->left);
        serializeNode(writer, node->right);
    }
}

// 反序列化树。采用 前序遍历+标记位 的反序列化方式。
// 先清空旧编码表，再重建。重建后重新调用 buildCodeTable，确保 encode/decode 可用。
void HuffmanCodec::deserializeTree(BitReader& reader) {
    if (reader.eof()) {
        root = nullptr;
    }
    else {
        root = deserializeNode(reader);
    }
    codeTable.clear();
    if (root) {
        std::vector<bool> path;
        buildCodeTable(root, path);
    }
}

// 与序列化逻辑对称，递归重建树。
// 叶子节点的频率设为 0 . 反序列化后只用于解码，频率信息不再需要
std::shared_ptr<HuffmanNode> HuffmanCodec::deserializeNode(BitReader& reader) {
    bool isLeaf = reader.readBit();
    if (isLeaf) {
        uint8_t byte = reader.readByte();
        return std::make_shared<HuffmanNode>(byte, 0);
    }
    else {
        auto left = deserializeNode(reader);
        auto right = deserializeNode(reader);
        return std::make_shared<HuffmanNode>(left, right);
    }
}