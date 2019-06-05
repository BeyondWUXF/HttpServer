#ifndef __DB_INTERFACE_IINCLUDE_H__
#define __DB_INTERFACE_IINCLUDE_H__
#include <string>
#include <set>
#include <list>
#include <vector>

enum class EMRStatus
{
    EM_KVDB_SUCCESS = 0,
    EM_KVDB_RNULL = -1,
    EM_KVDB_ERROR = -2
};

class CDBInterface
{
public:
    CDBInterface() : m_nPort(0){};
    virtual	~CDBInterface(){};
    virtual bool ConnectDB(const std::string &strIp, int32_t port) = 0;
    virtual bool ReConnectDB() = 0;
    virtual bool IsConnected() const = 0;

    //---------------原子序列号
    virtual EMRStatus IncrSeq(const std::string &strName, int64_t &unItem) = 0;
    virtual EMRStatus IncrSeqBy(const std::string &strName, int64_t &unItem, uint64_t nNum) = 0;

    //---------队列操作
    virtual EMRStatus QueRPop(const std::string &strName, std::string &strItem) = 0;
    virtual EMRStatus QueLPop(const std::string &strName, std::string &strItem) = 0;
    virtual EMRStatus QueLPush(const std::string &strName, const std::string &strItem) = 0;
    virtual EMRStatus QueRPush(const std::string &strName, const std::string &strItem) = 0;
    virtual EMRStatus QueIndexElement(const std::string &strName, int32_t nIndex, std::string &strItem) = 0;
    virtual EMRStatus QueSize(const std::string &strName, int64_t & ret) = 0;

    //----------string操作
    virtual EMRStatus IsStringKeyExits(const std::string &strKey, bool &bIsExit) = 0;
    virtual EMRStatus Set(const std::string &strKey, const std::string &strVal) = 0;
    virtual EMRStatus Setex(const std::string &strKey, uint64_t nNum, const std::string &strVal) = 0;
    virtual EMRStatus Get(const std::string &strKey, std::string &strVal) = 0;

    //------------集合操作----------------
    //获得集合中元素的数量
    virtual EMRStatus GetSetItemCount(const std::string &strName, int64_t& nRet) = 0;
    //获得集合中的所有元素
    virtual EMRStatus GetSetItems(const std::string &strName, std::set<std::string> &rgItem) = 0;
    //向集合中插入一条数据
    virtual EMRStatus SetAdd(const std::string &strKey, const std::string &strItem) = 0;
    //从集合中移除指定的数据
    virtual EMRStatus SetRemove(const std::string &strKey, const std::string &strItem) = 0;

    //-----------哈希表操作--------------
    virtual EMRStatus DelHashKey(const std::string &strHashTableName, const std::string &strKeyName) = 0;
    virtual EMRStatus SetHashItem(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField, const std::string &strItem) = 0;
    virtual EMRStatus DelHashField(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField) = 0;
    virtual EMRStatus GetHashItem(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField, std::string &strItem) = 0;
protected:
    std::string m_strIp;
    int32_t m_nPort;
};



#endif
