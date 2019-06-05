#ifndef __REDIS_DB_INTERFACE_IINCLUDE_H__
#define __REDIS_DB_INTERFACE_IINCLUDE_H__

#include <map>

#include "CDBInterface.h"
#include "hiredis/hiredis.h"
#include "string_util.h"
#include "scope_logger.hpp"
#include "vendor_boost.h"

#define	FORMAT_STREAM(__STROUPUT__, __INPUT__)\
{\
	try	\
{	\
	std::ostringstream  strSourceStream;	\
	strSourceStream << __INPUT__;	\
	__STROUPUT__ = strSourceStream.str();	\
}	\
	catch (std::exception& e)	\
{	\
	BOOST_LOG_TRIVIAL(error) << "FORMAT_STREAM ERROR, file:" << __FILE__ << ", line:" << __LINE__ << ", reason:" << e.what();	\
}	\
}


class CRedisDBInterface : public CDBInterface
{
public:
    CRedisDBInterface();
    virtual ~CRedisDBInterface();

    //-----------------检查当前数据库是否断开连接-------------
    virtual bool IsConnected() const;
    virtual bool ConnectDB(const std::string &strIp, int32_t nPort);
    virtual bool ReConnectDB();

    virtual bool setAuthPassword(const std::string &password);
    virtual EMRStatus Auth(const std::string &password);
    virtual EMRStatus DelKey(const std::string &strKey);
    virtual EMRStatus IncrSeq(const std::string &strName, int64_t &unItem);
    virtual EMRStatus IncrSeqBy(const std::string &strName, int64_t &unItem, uint64_t nNum);
    virtual EMRStatus Keys(const std::string & pattern, std::list<std::string> &keyItems);
    virtual EMRStatus Expire(const std::string &strName, uint64_t nNum);

    //-----------------队列操作------------------------------
    virtual EMRStatus QueRPop(const std::string &strName, std::string &strItem);
    virtual EMRStatus QueLPop(const std::string &strName, std::string &strItem);
    virtual EMRStatus QueLPush(const std::string &strName, const std::string &strItem);
    virtual EMRStatus QueRPush(const std::string &strName, const std::string &strItem);
    virtual EMRStatus QueIndexElement(const std::string &strName, int32_t nIndex, std::string &strItem);
    virtual EMRStatus QueSize(const std::string &strName, int64_t& nRet);
    virtual EMRStatus QueLRange(const std::string &strName, int64_t start, int64_t end, std::vector<std::string> &strItems);

    //------------------字符串操作--------------------------
    virtual EMRStatus IsStringKeyExits(const std::string &strKey, bool &bIsExit);
    virtual EMRStatus Set(const std::string &strKey, const std::string &strVal);
    virtual EMRStatus Setex(const std::string &strKey, uint64_t nNum, const std::string &strVal);
    virtual EMRStatus Get(const std::string &strKey, std::string &strVal);

    //-----------------集合操作-----------------------------
    virtual EMRStatus GetSetItemCount(const std::string &strName, int64_t& nRet);
    virtual EMRStatus GetSetItems(const std::string &strName, std::set<std::string> &rgItem);
    virtual EMRStatus IsItemInSet(const std::string &strName, const std::string &strItem, bool &bIsExit);
    virtual EMRStatus SetAdd(const std::string &strKey, const  std::string &strItem);
    virtual EMRStatus SetRemove(const std::string &strKey, const std::string &strItem);
    // 返回一个集合的全部成员，该集合是所有给定集合的交集。
    virtual EMRStatus SetInter(const std::vector<std::string> &sets, std::set<std::string>& resultSet);


    virtual EMRStatus ZSetAdd(const std::string &strKey, int64_t score, const  std::string &strItem);
    virtual EMRStatus ZSetRemove(const std::string &strKey, const std::string &strItem);
    virtual EMRStatus ZScore(const std::string &strKey, const  std::string &strItem, int64_t& nRet);
    virtual EMRStatus ZCard(const std::string &strKey, int64_t& nLen);
    //    //---------------------------key score value-----------
    virtual EMRStatus ZRangeWithScore(const std::string &strKey, std::list<std::string> &topicItem, int64_t from, int64_t to, std::list<std::string> &scoreItem);
    virtual EMRStatus ZRange(const std::string &strKey, std::vector<std::string> &topicItem, int64_t from, int64_t to);
    virtual EMRStatus ZRangeByScore(const std::string &strKey, int64_t from, int64_t to, std::vector<std::string> &memberItem);
    virtual EMRStatus GetOfflineMsg(const std::string& table, int start, int end, std::vector<std::string>& msgs, int32_t& nTotalSize);
    virtual EMRStatus ZSetGetPeerMsgs(const std::string &strKey, std::vector<std::string> &msgs, int32_t& nTotalSize);	// range scope 0 -1
    virtual EMRStatus ZSetRemoveByScore(const std::string &strKey, int64_t scorefrom, int64_t scoreto);
    virtual EMRStatus ZSetIncrby(const std::string &strKey, const std::string &strMember, const std::string &strScore, int64_t& nRet);

    //-----------------哈希操作---key field value---------
    virtual EMRStatus DelHashKey(const std::string &strHashTableName, const std::string &strKeyName);
    virtual EMRStatus SetHashItem(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField, const std::string &strItem);
    virtual EMRStatus DelHashField(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField);
    virtual EMRStatus GetHashItem(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField, std::string &strItem);
    virtual EMRStatus GetAllHashItems(const std::string &strHashTableName, const std::string &strHashItemName, std::map<std::string, std::string>& items);
    virtual EMRStatus GetMsgId(const std::string &strKey, int64_t& nRet);
    virtual EMRStatus SaveGroupMessage(const std::string& table, const std::string& message, int64_t msgId);
    virtual EMRStatus HIncrby(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField, int64_t val = 1);
    virtual EMRStatus HIncrby(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField, int64_t val, int64_t& retVal);
    virtual EMRStatus HLen(const std::string &strHashTable, int64_t& len);
    virtual EMRStatus HExist(const std::string &hashTable, const std::string& key, bool &bIsExit);
    virtual EMRStatus HScan(const std::string &strHashTableName, const std::string &strHashItemMatch, int page_size, int& page_nums, std::list<std::string>& hashItems, std::list<std::string>& hashItemsValue);

private:
    EMRStatus CheckReply(const std::string & strFunName, redisReply * pReply);
    EMRStatus IsKeyExits(const std::string &strKey, bool &bIsExit);

    EMRStatus AuthPrivate(const std::string &password);

    EMRStatus IncrSeqPrivate(const std::string &strName, int64_t &unItem);
    EMRStatus IncrSeqByPrivate(const std::string &strName, int64_t &unItem, uint64_t nNum);
    EMRStatus KeysPrivate(const std::string & pattern, std::list<std::string> &keyItems);
    EMRStatus ExpirePrivate(const std::string &strName, uint64_t nNum);

    //队列操作
    EMRStatus QueRPopPrivate(const std::string &strName, std::string &strItem);
    EMRStatus QueLPopPrivate(const std::string &strName, std::string &strItem);
    EMRStatus QueLPushPrivate(const std::string &strName, const std::string &strItem);
    EMRStatus QueIndexElementPrivate(const std::string &strName, int32_t nIndex, std::string &strItem);
    EMRStatus QueRPushPrivate(const std::string &strName, const std::string &strItem);
    EMRStatus QueSizePrivate(const std::string &strName, int64_t& nRet);
    EMRStatus SetPrivate(const std::string &strKey, const std::string &strVal);
    EMRStatus SetexPrivate(const std::string &strKey, uint64_t nNum, const std::string &strVal);
    EMRStatus GetPrivate(const std::string &strKey, std::string &strVal);
    EMRStatus QueLRangePrivate(const std::string &strName, int64_t start, int64_t end, std::vector<std::string> &strItems);

    EMRStatus GetSetItemCountPrivate(const std::string &strName, int64_t& nRet);
    EMRStatus GetSetItemsPrivate(const std::string &strName, std::set<std::string> &rgItem);
    EMRStatus IsItemInSetPrivate(const std::string &strName, const std::string &strItem, bool &bIsExit);
    EMRStatus SetInterPrivate(const std::vector<std::string> &sets, std::set<std::string>& resultSet);

    EMRStatus SetAddPrivate(const std::string &strKey, const  std::string &strItem);
    EMRStatus SetRemovePrivate(const std::string &strKey, const std::string &strItem);
    EMRStatus ZSetAddPrivate(const std::string &strKey, int64_t score, const  std::string &strItem);
    EMRStatus ZSetRemovePrivate(const std::string &strKey, const std::string &strItem);
    EMRStatus ZScorePrivate(const std::string &strKey, const  std::string &strItem, int64_t& nRet);
    EMRStatus ZCardPrivate(const std::string &strKey, int64_t& nLen);


    EMRStatus ZRangeWithScorePrivate(const std::string &strKey, std::list<std::string> &topicItem, int64_t from, int64_t to, std::list<std::string> &scoreItem);
    EMRStatus ZRangeByScorePrivate(const std::string &strKey, int64_t from, int64_t to, std::vector<std::string> &memberItem);
    EMRStatus ZRangePrivate(const std::string &strKey, std::vector<std::string> &rgTopicItem, int64_t from, int64_t to);
    EMRStatus GetOfflineMsgPrivate(const std::string& table, int start, int end, std::vector<std::string>& msgs, int32_t& nTotalSize);
    EMRStatus ZSetGetPeerMsgsPrivate(const std::string &strKey, std::vector<std::string> &msgs, int32_t& nTotalSize);
    EMRStatus ZSetRemoveByScorePrivate(const std::string &strKey, int64_t scorefrom, int64_t scoreto);
    EMRStatus ZSetIncrbyPrivate(const std::string &strKey, const std::string &strMember, const std::string &strScore, int64_t& bRet);

    //-----------------哈希操作----------------------------
    EMRStatus DelKeyPrivate(const std::string &strKeyName);
    EMRStatus SetHashItemPrivate(const std::string &strHashItemName, const std::string &strField, const std::string &strItem);
    EMRStatus DelHashFieldPrivate(const std::string &strHashItemName, const std::string &strField);
    EMRStatus GetHashItemPrivate(const std::string &strHashItemName, const std::string &strField, std::string &strItem);
    EMRStatus GetAllHashItemsPrivate(const std::string &strKey, std::map<std::string, std::string>& items);
    EMRStatus GetMsgIdPrivate(const std::string &strKey, int64_t& nRet);
    EMRStatus SaveGroupMessagePrivate(const std::string& table, const std::string& message, int64_t msgId);
    EMRStatus HIncrbyPrivate(const std::string &strHashItemName, const std::string &strField, int64_t val);
    EMRStatus HIncrbyPrivate(const std::string &strHashItemName, const std::string &strField, int64_t val, int64_t& retVal);
    EMRStatus HScanPrivate(const std::string &strHashTableName, const std::string &strHashItemMatch, int page_size, int& page_nums, std::list<std::string>& hashItems, std::list<std::string>& hashItemsValue);
    EMRStatus HLenPrivate(const std::string &strHashTable, int64_t& len);
    EMRStatus HExistPrivate(const std::string &hashTable, const std::string& key, bool &bIsExit);

    redisContext *m_pRedisDBContex;
    std::string m_authPassword;
};

#endif
