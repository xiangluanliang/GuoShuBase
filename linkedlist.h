
// 链表索引从0开始
// 遍历所有元素的示例:
//   for (int i=0; i<list.GetLength(); i++) {
//       <class T> *pT = list[i]
//   }

#ifndef TEMPL_LINKED_LIST_H
#define TEMPL_LINKED_LIST_H

//----------------------------------------------
// 链表模板类定义
//----------------------------------------------
template <class T>
class LinkList {
public:
    LinkList();                          // 默认构造函数
    LinkList(const LinkList<T> &sourcell); // 拷贝构造函数
    ~LinkList();                         // 析构函数

    void operator=(const LinkList<T> &sourcell); // 赋值运算符
    bool operator==(const LinkList<T> &rhs) const; // 相等比较运算符

    operator T*();  // 转换为数组运算符(调用者需负责释放内存)

    // 添加元素方法
    void Append(const T &item);          // 添加单个元素
    void Append(const LinkList<T> &sourcell); // 添加链表

    // 运算符重载(更直观的添加方式)
    LinkList<T> operator+(const LinkList<T> &sourcell) const;
    LinkList<T> operator+(const T &element) const;
    void operator+=(const LinkList<T> &sourcell);
    void operator+=(const T &element);

    void Delete(int index);              // 删除指定位置元素
    void Erase();                        // 清空链表

    // 访问方法(返回元素指针，调用者不应删除返回的指针)
    T* Get(int index);                   // 获取指定位置元素
    T* operator[](int index);            // 下标运算符重载
    int GetLength() const;               // 获取链表长度

protected:
    // 链表内部节点结构
    struct InternalNode {
        T Data;                     // 节点数据
        InternalNode *next;         // 后向指针
        InternalNode *previous;     // 前向指针
    };

    int iLength;                    // 链表长度
    InternalNode *pnHead;           // 头节点指针
    InternalNode *pnTail;           // 尾节点指针
    InternalNode *pnLastRef;        // 最后访问节点指针(缓存优化)
    int iLastRef;                   // 最后访问位置索引

    void Setnullptr();              // 初始化成员变量
};

//----------------------------------------------
// 成员函数实现
//----------------------------------------------

// 初始化成员变量
template <class T>
inline void LinkList<T>::Setnullptr() {
    pnHead = nullptr;
    pnTail = nullptr;
    pnLastRef = nullptr;
    iLength = 0;
    iLastRef = -1;
}

// 默认构造函数
template <class T>
inline LinkList<T>::LinkList() {
    Setnullptr();
}

// 拷贝构造函数
template <class T>
LinkList<T>::LinkList(const LinkList<T> &sourcell) {
    Setnullptr();
    if (sourcell.iLength == 0) return;

    InternalNode *n = sourcell.pnHead;
    while (n != nullptr) {
        Append(n->Data);
        n = n->next;
    }
    pnLastRef = pnHead;
}

// 析构函数
template <class T>
inline LinkList<T>::~LinkList() {
    Erase();
}

// 赋值运算符
template <class T>
void LinkList<T>::operator=(const LinkList<T> &sourcell) {
    Erase(); // 先清空原链表
    InternalNode *pnTemp = sourcell.pnHead;
    while (pnTemp != nullptr) {
        Append(pnTemp->Data);
        pnTemp = pnTemp->next;
    }
    pnLastRef = nullptr;
    iLastRef = -1;
}

// 相等比较运算符
template <class T>
bool LinkList<T>::operator==(const LinkList<T> &rhs) const {
    if (iLength != rhs.iLength) return false;

    InternalNode *pnLhs = this->pnHead;
    InternalNode *pnRhs = rhs.pnHead;
    while (pnLhs != nullptr && pnRhs != nullptr) {
        if (!(pnLhs->Data == pnRhs->Data)) return false;
        pnLhs = pnLhs->next;
        pnRhs = pnRhs->next;
    }
    return (pnLhs == nullptr && pnRhs == nullptr);
}

// 转换为数组运算符
template <class T>
LinkList<T>::operator T*() {
    if (iLength == 0) return nullptr;

    T *pResult = new T[iLength];
    InternalNode *pnCur = pnHead;
    T *pnCopy = pResult;

    while (pnCur != nullptr) {
        *pnCopy = pnCur->Data;
        ++pnCopy;
        pnCur = pnCur->next;
    }
    return pResult; // 调用者需负责释放内存
}

// 添加单个元素
template <class T>
inline void LinkList<T>::Append(const T &item) {
    InternalNode *pnNew = new InternalNode;
    pnNew->Data = item;
    pnNew->next = nullptr;
    pnNew->previous = pnTail;

    if (iLength == 0) {
        pnHead = pnNew;
        pnTail = pnNew;
        pnLastRef = pnNew;
    } else {
        pnTail->next = pnNew;
        pnTail = pnNew;
    }
    ++iLength;
}

// 添加链表
template <class T>
void LinkList<T>::Append(const LinkList<T> &sourcell) {
    const InternalNode *pnCur = sourcell.pnHead;
    while (pnCur != nullptr) {
        Append(pnCur->Data);
        pnCur = pnCur->next;
    }
}

// 运算符重载实现
template <class T>
inline LinkList<T> LinkList<T>::operator+(const LinkList<T> &sourcell) const {
    LinkList<T> pTempLL(*this);
    pTempLL += sourcell;
    return pTempLL;
}

template <class T>
inline LinkList<T> LinkList<T>::operator+(const T &element) const {
    LinkList<T> pTempLL(*this);
    pTempLL += element;
    return pTempLL;
}

template <class T>
void LinkList<T>::operator+=(const LinkList<T> &list) {
    const InternalNode *pnTemp;
    const int iLength = list.iLength;
    int i;
    for (pnTemp = list.pnHead, i=0; i < iLength; pnTemp = pnTemp->next, i++)
        *this += pnTemp->Data;
}

template <class T>
void LinkList<T>::operator+=(const T &element) {
    InternalNode *pnNew = new InternalNode;
    pnNew->next = nullptr;
    pnNew->Data = element;
    if (iLength++ == 0) {
        pnHead = pnNew;
        pnNew->previous = nullptr;
    } else {
        pnTail->next = pnNew;
        pnNew->previous = pnTail;
    }
    pnTail = pnNew;
}

// 删除指定位置元素
template <class T>
inline void LinkList<T>::Delete(int which) {
    if (which > iLength || which == 0) return;

    InternalNode *pnDeleteMe = pnHead;
    for (int i=1; i<which; i++)
        pnDeleteMe = pnDeleteMe->next;

    // 处理头节点情况
    if (pnDeleteMe == pnHead) {
        if (pnDeleteMe->next == nullptr) {
            delete pnDeleteMe;
            Setnullptr();
        } else {
            pnHead = pnDeleteMe->next;
            pnHead->previous = nullptr;
            delete pnDeleteMe;
            pnLastRef = pnHead;
        }
    }
        // 处理尾节点情况
    else if (pnDeleteMe == pnTail) {
        if (pnDeleteMe->previous == nullptr) {
            delete pnDeleteMe;
            Setnullptr();
        } else {
            pnTail = pnDeleteMe->previous;
            pnTail->next = nullptr;
            delete pnDeleteMe;
            pnLastRef = pnTail;
        }
    }
        // 处理中间节点情况
    else {
        pnLastRef = pnDeleteMe->next;
        pnDeleteMe->previous->next = pnDeleteMe->next;
        pnDeleteMe->next->previous = pnDeleteMe->previous;
        delete pnDeleteMe;
    }

    if (iLength != 0) --iLength;
}

// 下标运算符重载
template <class T>
inline T* LinkList<T>::operator[](int index) {
    return Get(index);
}

// 清空链表
template <class T>
inline void LinkList<T>::Erase() {
    pnLastRef = pnHead;
    while (pnLastRef != nullptr) {
        pnHead = pnLastRef->next;
        delete pnLastRef;
        pnLastRef = pnHead;
    }
    Setnullptr();
}

// 获取指定位置元素(带访问优化)
template <class T>
inline T* LinkList<T>::Get(int index) {
    int iCur;               // 搜索起始位置
    InternalNode *pnTemp;   // 搜索起始节点
    int iRelToMiddle;       // 请求位置相对于最后访问位置的偏移

    if (index < 0 || index >= iLength) return nullptr;

    index++; // 转换为1-based索引便于处理

    // 确定最优搜索起点(头节点、尾节点或上次访问节点)
    if (iLastRef == -1) {
        if (index < (iLength - index)) {
            iCur = 1;
            pnTemp = pnHead;
        } else {
            iCur = iLength;
            pnTemp = pnTail;
        }
    } else {
        if (index < iLastRef)
            iRelToMiddle = iLastRef - index;
        else
            iRelToMiddle = index - iLastRef;

        if (index < iRelToMiddle) {
            iCur = 1;
            pnTemp = pnHead;
        } else if (iRelToMiddle < (iLength - index)) {
            iCur = iLastRef;
            pnTemp = pnLastRef;
        } else {
            iCur = iLength;
            pnTemp = pnTail;
        }
    }

    // 从确定的位置开始搜索目标元素
    while (iCur != index) {
        if (iCur < index) {
            iCur++;
            pnTemp = pnTemp->next;
        } else {
            iCur--;
            pnTemp = pnTemp->previous;
        }
    }

    // 更新最后访问记录
    iLastRef = index;
    pnLastRef = pnTemp;
    return &(pnLastRef->Data);
}

// 获取链表长度
template <class T>
inline int LinkList<T>::GetLength() const {
    return iLength;
}

#endif // TEMPL_LINKED_LIST_H
