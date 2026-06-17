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

