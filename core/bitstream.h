#pragma once
#include <iostream>
#include <cstdint>

class BitWriter {
public:
    explicit BitWriter(std::ostream& out);
    void writeBit(bool bit);
    void writeBits(uint32_t value, int n);  // 写入 n 个位
    void writeByte(uint8_t byte);           //  直接写入字节
    void flush();                           //  刷新缓冲区,如果 buffer 中有剩余位（bitCount > 0），补零后输出
    uint64_t getBitsWritten() const { return bitsWritten; }
 
// 位缓冲
// 数据按位写入 buffer，当累积满 8 位（1字节）时，一次性输出到流
private:
    std::ostream& out;
    uint8_t buffer = 0;        // 8位缓冲区,预读的字节缓冲
    int bitCount = 0;          // 当前缓冲了位数
    uint64_t bitsWritten = 0;  // 统计总写入位数
};

class BitReader {
public:
    explicit BitReader(std::istream& in);
    bool readBit();      // 读取一个位，
    uint32_t readBits(int n);
    uint8_t readByte();  // 读取完整字节。当 bitCount==0 时可以直接返回 readBits(8)，或优化为直接从流读取
    bool eof() const;    // 检查是否到文件尾
    uint64_t getBitsRead() const { return bitsRead; }
private:
    std::istream& in;
    uint8_t buffer = 0;
    int bitCount = 0;
    uint64_t bitsRead = 0;
    bool endOfFile = false;
    
};
