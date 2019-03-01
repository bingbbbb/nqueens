/**
 * Buffer类，作用是在non-blocking网络通信中，避免程序阻塞在read()或write()
 * 函数中，应尽快返回控制权。IO线程只能阻塞在IO multiplexing函数上，如epoll_wait
 * 
 * 使用circular Buffer结构，减少数据拷贝
 * 用malloc代替new，减少不必要的init操作
 * readerIndex_使用下标而非指针，避免了内存重新分配指针失效的问题
 */

#ifndef BUFFER_H
#define BUFFER_H
#include <string>           //std::string
#include <stdlib.h>         //malloc(),free()
#include <assert.h>         //assert()

namespace nqueens
{
class Buffer
{
public:
    explicit Buffer (size_t initCap = 0)
        : readerIndex_(0), writerIndex_(0), size_(0), capacity_(initCap ? initCap : SPARE_CAPACITY_)
    {
        pObjs_ = static_cast<char*>(malloc(capacity_ * sizeof(char)));
        assert(pObjs_ != nullptr);
    }
    Buffer(const Buffer &) = delete;
    const Buffer &operator=(const Buffer &) = delete;
    ~Buffer()
    {
        free(pObjs_);
    }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    void reserve(size_t newCap) //重新分配数组容量
    {
        if (newCap <= size_)
            return;
        char* pOld = pObjs_;
        pObjs_ = static_cast<char*>(malloc(newCap * sizeof(char)));
        assert(pObjs_ != nullptr);
        for (size_t i = 0;i < size_;++i)    //拷贝数据,从下标0开始填充
        {
            pObjs_[i] = pOld[(readerIndex_ + i) % capacity_];
        }
        readerIndex_ = 0;                   //新的下标及容量
        writerIndex_ = size_;
        capacity_ = newCap;
    }
    void append(const char* pData, size_t len)  //将pData数组追加到writeable区间
    {
        if (size_ + len > capacity_)            //空间不够扩容
            reserve((size_ + len) * 2);
        for (size_t i = 0;i < len;++i)
        {
            pObjs_[writerIndex_] = pData[i];
            writerIndex_ = (++writerIndex_) % capacity_;    //更新writerIndex_
        }
        size_ += len;
    }
    void append(const std::string &data)        //将string中的字符追加到writeable区间
    {
        if (size_ + data.size() > capacity_)            //空间不够扩容
            reserve((size_ + data.size()) * 2);
        for (size_t i = 0;i < data.size();++i)
        {
            pObjs_[writerIndex_] = data[i];
            writerIndex_ = (++writerIndex_) % capacity_;    //更新writerIndex_
        }
        size_ += data.size();
    }
    void discard(size_t len)                                //已经读完len个字节，移动读下标
    {
        if (len == 0)
            return;
        assert(size_ >= len);
        readerIndex_ = (readerIndex_ + len) % capacity_;
        size_ -= len;
    }
    const char * GetPtr() const { return pObjs_ + readerIndex_; }
    size_t sizeEnd() { return capacity_ - readerIndex_; }



private:
    const static size_t SPARE_CAPACITY_ = 32;//默认初始化大小
    char*   pObjs_;                         //申请到的空间
    size_t  readerIndex_;                   //读下标
    size_t  writerIndex_;                   //写下标
    size_t  size_;                          //大小
    size_t  capacity_;                      //容量


};
}
#endif


/**
#ifndef BUFFER_H
#define BUFFER_H
#include <string>
#include "MLog.h"
namespace nqueens
{
/**
 * 类模板分离编译容易出问题，所以都在头文件中实现
 * 封装Buffer类似于封装Vector，接口差不多
 * 待改进：（1）用alloctor而不是new
 *        （2）用nHead和nTail将Buffer以环形数组实现，这样可以避免discard操作
 *
template <typename Object>
class Buffer
{
  public:
    explicit Buffer(size_t initCap = 0)
        : m_size(0), m_capacity(initCap ? initCap : SPARE_CAPACITY)
    {
        m_pObjs = new Object[m_capacity];
    }
    Buffer(const Buffer &) = delete;
    const Buffer &operator=(const Buffer &) = delete;
    ~Buffer()
    {
        delete[] m_pObjs;
        m_pObjs = nullptr;
    }
    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    void reserve(size_t newCap) //重新分配数组容量，无法截断数组
    {
        if (newCap <= m_size)
            return;
        Object *pOldObjs = m_pObjs;
        m_pObjs = new Object[newCap];
        m_capacity = newCap;
        for (size_t i = 0; i < m_size; ++i)
            m_pObjs[i] = std::move(pOldObjs[i]);
        delete[] pOldObjs;
    }
    const Object *GetPtr() const { return m_pObjs; } //有些操作需要原始的指针
    void append(const Object *pData, size_t len)     //将pData数组的字符追加到m_pObjs中
    {
        if (m_size + len > m_capacity)
            reserve(len + m_capacity);                        //空间不足，扩容
        if (m_pObjs <= pData && pData < m_pObjs + m_capacity) //源和目的数组重合
        {
            _LOG(Level::ERROR, {WHERE, "src and dst array overlap"});
            THROW_EXCEPT("src and dst array overlap");
        }
        for (size_t i = 0; i < len; ++i)
            m_pObjs[m_size + i] = pData[i];
        m_size += len;
    }
    void append(const std::string &data) //将string中的字符追加到m_pObjs中
    {
        if (m_size + data.size() > m_capacity)
            reserve(data.size() + m_capacity); //空间不足，扩容
        for (size_t i = 0; i < data.size(); ++i)
            m_pObjs[m_size + i] = data[i];
        m_size += data.size();
    }
    void discard(size_t len) //丢弃m_pObjs的前n个字符，后面的补到前面
    {
        if (len == 0)
            return;
        if (len > m_size)
        {
            _LOG(Level::ERROR, {WHERE, "no enough data to discard"});
            THROW_EXCEPT("no enough data to discard");
        }
        size_t indexSrc = len, indexDst = 0;
        while (indexSrc != m_size)
            m_pObjs[indexDst++] = std::move(m_pObjs[indexSrc++]);
        m_size = indexDst;
    }

  private:
    const static size_t SPARE_CAPACITY = 32;
    size_t m_size;     //大小
    size_t m_capacity; //容量
    Object *m_pObjs;   //数组
};
}
#endif //BUFFER_H
*/