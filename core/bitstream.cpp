#include "bitstream.h"

BitWriter::BitWriter(std::ostream& out) : out(out) {}  // 建立 BitWriter 与底层字节流的关联，后续所有写入操作都通过 out 输出。

void BitWriter::writeBit(bool bit) {
    buffer <<= 1;             // 将 buffer 中已有的位全部左移一位，最低位空出 0
    if (bit) buffer |= 1;     // 如果bit为true，将最低位置设置为1；否则保持 0
    bitCount++;               // 记录当前 buffer 中已经缓存了多少位
    bitsWritten++;            // 记录总共调用了多少次 writeBit
    if (bitCount == 8) {      // 当缓存满 8 位(1字节)时，立即写入底层字节流
        out.put(static_cast<char>(buffer));  // 将 buffer 作为 char（1字节）写入 ostream
        buffer = 0;           // 清空缓冲
        bitCount = 0;         // 重置计数
    }                         // 重置缓冲区，准备接收下一个字节 
}

void BitWriter::writeBits(uint32_t value, int n) {
    for (int i = n - 1; i >= 0; --i) {   // 从第 n-1 位到第 0 位，依次提取每一位并调用 writeBit
        writeBit((value >> i) & 1);
    }
}

void BitWriter::writeByte(uint8_t byte) {  // 写入一个完整的字节
    writeBits(byte, 8);                    // 调用 writeBits(byte, 8)，将 8 位数据逐位写入
}

void BitWriter::flush() {
    if (bitCount > 0) {                         // 如果缓冲区还有未写入的位
        buffer <<= (8 - bitCount);              // 则将 buffer 左移补齐到 8 位，低位补0
        out.put(static_cast<char>(buffer));     // 写入流
		buffer = 0;                             // 清空缓冲区
		bitCount = 0;                           // 重置位计数，为下一次写入做准备
    }
}

BitReader::BitReader(std::istream& in) : in(in) {
    int c = in.get();                           // 从底层流中读取第一个字节
    if (c != EOF) {                            
		buffer = static_cast<uint8_t>(c);       // 将读取的字节存入缓冲区
		bitCount = 8;                           // 初始化缓冲区计数为 8，表示缓冲区中有 8 位可供读取
    }
    else {
		endOfFile = true;                      // 流为空，标记为文件结束
    }
}

bool BitReader::readBit() {
    if (endOfFile) return false;                 // 已读完，返回 0
    bitCount--;                                  // 消耗 1 位
    bool bit = (buffer >> bitCount) & 1;         // 提取当前最高位
    bitsRead++;                                  // 统计总读取位数
    if (bitCount == 0) {                         // 当前字节已读完
		int c = in.get();                        // 从底层流中读取下一个字节
        if (c != EOF) {
            buffer = static_cast<uint8_t>(c);
			bitCount = 8;                        // 重置缓冲区计数为 8，表示缓冲区中有 8 位可供读取
        }
        else {
			endOfFile = true;                    // 标记为文件结束
        }
    }
    return bit;
}

uint32_t BitReader::readBits(int n) {
    uint32_t value = 0;                                  // 初始化为 0，用于累积读取的位
    for (int i = 0; i < n; ++i) {
        value = (value << 1) | (readBit() ? 1 : 0);      // 将已读取的位整体左移，为下一位腾出最低位
    }                                                    // 将新读到的位（0 或 1）放入 value 的最低位
    return value;
}

uint8_t BitReader::readByte() {
    return static_cast<uint8_t>(readBits(8));
}

bool BitReader::eof() const {
    return endOfFile;
}