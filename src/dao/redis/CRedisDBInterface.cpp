//#include <chrono>

#include "CRedisDBInterface.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>

#define  MAX_RECONNEC_TIMES 3

#define REDIS_REPLY_STRING_TYPE		0x01
#define REDIS_REPLY_ARRAY_TYPE		0x02
#define REDIS_REPLY_INTEGER_TYPE	0x04
#define REDIS_REPLY_NIL_TYPE		0x08
#define REDIS_REPLY_STATUS_TYPE	    0x10
#define REDIS_REPLY_ERROR_TYPE		0x20

#define REDIS_STRING_PREFIX  ""
#define REDIS_HASH_PREFIX    ""
#define REDIS_QUE_PREFIX     ""
#define REDIS_SET_PREFIX     ""

#define DO_REDIS_PUB_FUN(__FUN__){	\
	int nCount = 0; \
	EMRStatus emRest = EMRStatus::EM_KVDB_ERROR;  \
	while (true)\
	{\
		emRest = __FUN__; \
		if ((EMRStatus::EM_KVDB_ERROR == emRest) && !IsConnected() && nCount < MAX_RECONNEC_TIMES)\
		{\
			ReConnectDB(); \
			nCount++; \
		}\
		else{\
			break; \
		}\
	}\
	return emRest; \
}

CRedisDBInterface::CRedisDBInterface()
        : CDBInterface()
        , m_pRedisDBContex(nullptr)
{

}

CRedisDBInterface::~CRedisDBInterface() {
    if (nullptr != m_pRedisDBContex) {
        redisFree(m_pRedisDBContex);
        m_pRedisDBContex = nullptr;
    }
}

bool CRedisDBInterface::ConnectDB(const std::string &strIp, int32_t nPort) {
    m_strIp = strIp;
    m_nPort = nPort;
    if (nullptr != m_pRedisDBContex) {
        redisFree(m_pRedisDBContex);
        m_pRedisDBContex = nullptr;
    }

    scope_logger<> scopeLogger("ConnectRedis", 10);
    m_pRedisDBContex = redisConnect(strIp.c_str(), nPort);
    if (0 != m_pRedisDBContex->err) {
        BOOST_LOG_TRIVIAL(warning) << "CRedisDBInterface::ConnectDB " << strIp << ":" << nPort << " failed";
        return false;
    }
    else {
        BOOST_LOG_TRIVIAL(info) << "CRedisDBInterface::ConnectDB " << strIp << ":" << nPort << " success";
        // set blocked read/write timeout
        struct timeval tv{3, 0};
        int ret = redisSetTimeout(m_pRedisDBContex, tv);
        if (ret != REDIS_OK) {
            BOOST_LOG_TRIVIAL(warning) << "CRedisDBInterface::ConnectDB, SetTimeout Error: [" << m_pRedisDBContex->errstr << "]";
            redisFree(m_pRedisDBContex);
            m_pRedisDBContex = nullptr;
            return false;
        }

        if (!m_authPassword.empty() && EMRStatus::EM_KVDB_SUCCESS != AuthPrivate(m_authPassword)) {
            BOOST_LOG_TRIVIAL(warning) << "CRedisDBInterface::ConnectDB, Auth Error.";
            redisFree(m_pRedisDBContex);
            m_pRedisDBContex = nullptr;
            return false;
        }
        return true;
    }
}

bool CRedisDBInterface::ReConnectDB() {
    return ConnectDB(m_strIp, m_nPort);
}

bool CRedisDBInterface::setAuthPassword(const std::string &password) {
    m_authPassword = password;
    return true;
}

EMRStatus CRedisDBInterface::Auth(const std::string &password) {
    DO_REDIS_PUB_FUN(AuthPrivate(password));
}

EMRStatus CRedisDBInterface::DelKey(const std::string &strKey) {
    DO_REDIS_PUB_FUN(DelKeyPrivate(strKey));
}


EMRStatus CRedisDBInterface::IncrSeq(const std::string &strName, int64_t &unItem) {
    DO_REDIS_PUB_FUN(IncrSeqPrivate(strName, unItem));
}

EMRStatus CRedisDBInterface::Keys(const std::string & pattern, std::list<std::string> &keyItems) {
    DO_REDIS_PUB_FUN(KeysPrivate(pattern, keyItems));
}
EMRStatus CRedisDBInterface::QueLRange(const std::string &strName, int64_t start, int64_t end, std::vector<std::string> &strItems) {
    DO_REDIS_PUB_FUN(QueLRangePrivate(REDIS_QUE_PREFIX + strName, start, end, strItems));
}
EMRStatus CRedisDBInterface::QueRPop(const std::string &strName, std::string &strItem) {
    DO_REDIS_PUB_FUN(QueRPopPrivate(REDIS_QUE_PREFIX + strName, strItem));
}


EMRStatus CRedisDBInterface::QueLPop(const std::string &strName, std::string &strItem) {
    DO_REDIS_PUB_FUN(QueLPopPrivate(REDIS_QUE_PREFIX + strName, strItem));
}


EMRStatus CRedisDBInterface::QueLPush(const std::string &strName, const std::string &strItem) {
    DO_REDIS_PUB_FUN(QueLPushPrivate(REDIS_QUE_PREFIX + strName, strItem));
}

EMRStatus CRedisDBInterface::QueRPush(const std::string &strName, const std::string &strItem) {
    DO_REDIS_PUB_FUN(QueRPushPrivate(REDIS_QUE_PREFIX + strName, strItem));
}

EMRStatus CRedisDBInterface::QueSize(const std::string &strName, int64_t& nRet) {
    DO_REDIS_PUB_FUN(QueSizePrivate(REDIS_QUE_PREFIX + strName, nRet));
}

EMRStatus CRedisDBInterface::IsStringKeyExits(const std::string &strKey, bool &bIsExit) {
    DO_REDIS_PUB_FUN(IsKeyExits(strKey, bIsExit));
}

EMRStatus CRedisDBInterface::Set(const std::string &strKey, const std::string &strVal) {
    DO_REDIS_PUB_FUN(SetPrivate(strKey, strVal));
}

EMRStatus CRedisDBInterface::Setex(const std::string &strKey, uint64_t nNum, const std::string &strVal) {
    DO_REDIS_PUB_FUN(SetexPrivate(strKey, nNum, strVal));
}

EMRStatus CRedisDBInterface::Get(const std::string &strKey, std::string &strVal) {
    DO_REDIS_PUB_FUN(GetPrivate(strKey, strVal));
}

EMRStatus CRedisDBInterface::GetSetItemCount(const std::string &strName, int64_t& nRet) {
    DO_REDIS_PUB_FUN(GetSetItemCountPrivate(REDIS_SET_PREFIX + strName, nRet));
}

EMRStatus CRedisDBInterface::GetSetItems(const std::string &strName, std::set<std::string> &rgItem) {
    DO_REDIS_PUB_FUN(GetSetItemsPrivate(REDIS_SET_PREFIX + strName, rgItem));
}
EMRStatus CRedisDBInterface::SetInter(const std::vector<std::string> &sets, std::set<std::string>& resultSet) {
    DO_REDIS_PUB_FUN(SetInterPrivate(sets, resultSet));
}

EMRStatus CRedisDBInterface::IsItemInSet(const std::string &strName, const std::string &strItem, bool &bIsExit) {
    DO_REDIS_PUB_FUN(IsItemInSetPrivate(REDIS_SET_PREFIX + strName, strItem, bIsExit));
}

EMRStatus CRedisDBInterface::SetAdd(const std::string &strKey, const std::string &strItem) {
    DO_REDIS_PUB_FUN(SetAddPrivate(REDIS_SET_PREFIX + strKey, strItem));
}


EMRStatus CRedisDBInterface::ZSetAdd(const std::string &strKey, int64_t score, const  std::string &strItem) {
    DO_REDIS_PUB_FUN(ZSetAddPrivate(strKey, score, strItem));
}

EMRStatus CRedisDBInterface::SetRemove(const std::string &strKey, const std::string &strItem) {
    DO_REDIS_PUB_FUN(SetRemovePrivate(REDIS_SET_PREFIX + strKey, strItem));
}

EMRStatus CRedisDBInterface::ZSetRemove(const std::string &strKey, const std::string &strItem) {
    DO_REDIS_PUB_FUN(ZSetRemovePrivate(strKey, strItem));
}

EMRStatus CRedisDBInterface::ZScore(const std::string &strKey, const  std::string &strItem, int64_t& nRet) {
    DO_REDIS_PUB_FUN(ZScorePrivate(strKey, strItem, nRet));
}

EMRStatus CRedisDBInterface::ZCard(const std::string &strKey, int64_t& nLen) {
    DO_REDIS_PUB_FUN(ZCardPrivate(strKey, nLen));
}

EMRStatus CRedisDBInterface::ZRangeWithScore(const std::string &strKey,
                                             std::list<std::string> &topicItem, int64_t from, int64_t to, std::list<std::string> &scoreItem)
{
    DO_REDIS_PUB_FUN(ZRangeWithScorePrivate(strKey, topicItem, from, to, scoreItem));
}

EMRStatus CRedisDBInterface::ZRange(const std::string &strKey, std::vector<std::string> &topicItem, int64_t from, int64_t to) {
    DO_REDIS_PUB_FUN(ZRangePrivate(strKey, topicItem, from, to));
}

EMRStatus CRedisDBInterface::ZRangeByScore(const std::string &strKey, int64_t from, int64_t to, std::vector<std::string> &memberItem) {
    DO_REDIS_PUB_FUN(ZRangeByScorePrivate(strKey, from, to, memberItem));
}

EMRStatus CRedisDBInterface::ZSetGetPeerMsgs(const std::string &strKey, std::vector<std::string> &msgs, int32_t& nTotalSize) {
    DO_REDIS_PUB_FUN(ZSetGetPeerMsgsPrivate(strKey, msgs, nTotalSize));
}

EMRStatus CRedisDBInterface::ZSetRemoveByScore(const std::string &strKey, int64_t scorefrom, int64_t scoreto) {
    DO_REDIS_PUB_FUN(ZSetRemoveByScorePrivate(strKey, scorefrom, scoreto));
}

EMRStatus CRedisDBInterface::ZSetIncrby(const std::string &strKey, const std::string &strMember, const std::string &strScore, int64_t& nRet) {
    DO_REDIS_PUB_FUN(ZSetIncrbyPrivate(strKey, strMember, strScore, nRet));
}

EMRStatus CRedisDBInterface::GetOfflineMsg(const std::string& table, int start, int end, std::vector<std::string>& msgs, int32_t& nTotalSize) {
    DO_REDIS_PUB_FUN(GetOfflineMsgPrivate(table, start, end, msgs, nTotalSize));
}

EMRStatus CRedisDBInterface::DelHashKey(const std::string &strHashTableName, const std::string &strKeyName) {
    DO_REDIS_PUB_FUN(DelKeyPrivate(REDIS_HASH_PREFIX + strHashTableName + strKeyName));
}

EMRStatus CRedisDBInterface::SetHashItem(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField, const std::string &strItem) {
    DO_REDIS_PUB_FUN(SetHashItemPrivate(REDIS_HASH_PREFIX + strHashTableName + strHashItemName, strField, strItem));
}

EMRStatus CRedisDBInterface::HIncrby(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField, int64_t val) {
    DO_REDIS_PUB_FUN(HIncrbyPrivate(REDIS_HASH_PREFIX + strHashTableName + strHashItemName, strField, val));
}
EMRStatus CRedisDBInterface::HIncrby(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField, int64_t val, int64_t& curVal) {
    DO_REDIS_PUB_FUN(HIncrbyPrivate(REDIS_HASH_PREFIX + strHashTableName + strHashItemName, strField, val, curVal));
}

EMRStatus CRedisDBInterface::HLen(const std::string &strHashTable, int64_t& len) {
    DO_REDIS_PUB_FUN(HLenPrivate(REDIS_HASH_PREFIX + strHashTable, len));
}
EMRStatus CRedisDBInterface::HExist(const std::string &strHashTable, const std::string& key, bool& isExist) {
    DO_REDIS_PUB_FUN(HExistPrivate(REDIS_HASH_PREFIX + strHashTable, key, isExist));
}
EMRStatus CRedisDBInterface::HScan(const std::string &strHashTableName, const std::string &strHashItemMatch, int page_size, int& page_nums, std::list<std::string>& hashItems, std::list<std::string>& hashItemsValue) {
    DO_REDIS_PUB_FUN(HScanPrivate(REDIS_HASH_PREFIX + strHashTableName, strHashItemMatch, page_size, page_nums, hashItems, hashItemsValue));
}

EMRStatus CRedisDBInterface::DelHashField(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField) {
    DO_REDIS_PUB_FUN(DelHashFieldPrivate(REDIS_HASH_PREFIX + strHashTableName + strHashItemName, strField));
}

EMRStatus CRedisDBInterface::GetHashItem(const std::string &strHashTableName, const std::string &strHashItemName, const std::string &strField, std::string &strItem) {
    DO_REDIS_PUB_FUN(GetHashItemPrivate(REDIS_HASH_PREFIX + strHashTableName + strHashItemName, strField, strItem));
}

EMRStatus CRedisDBInterface::GetAllHashItems(const std::string &strHashTableName, const std::string &strHashItemName, std::map<std::string, std::string>& items) {
    DO_REDIS_PUB_FUN(GetAllHashItemsPrivate(REDIS_HASH_PREFIX + strHashTableName + strHashItemName, items));
}


EMRStatus CRedisDBInterface::GetMsgId(const std::string &strKey, int64_t& nRet) {
    DO_REDIS_PUB_FUN(GetMsgIdPrivate(strKey, nRet));
}

EMRStatus CRedisDBInterface::SaveGroupMessage(const std::string& table, const std::string& message, int64_t msgId) {
    DO_REDIS_PUB_FUN(SaveGroupMessagePrivate(table, message, msgId));
}

EMRStatus CRedisDBInterface::QueIndexElement(const std::string &strName, int32_t nIndex, std::string &strItem) {
    DO_REDIS_PUB_FUN(QueIndexElementPrivate(REDIS_QUE_PREFIX + strName, nIndex, strItem));
}

EMRStatus CRedisDBInterface::Expire(const std::string &strName, uint64_t nNum) {
    DO_REDIS_PUB_FUN(ExpirePrivate(strName, nNum));
}

EMRStatus CRedisDBInterface::IncrSeqBy(const std::string &strName, int64_t &unItem, uint64_t nNum) {
    DO_REDIS_PUB_FUN(IncrSeqByPrivate(strName, unItem, nNum));
}

EMRStatus CRedisDBInterface::IncrSeqPrivate(const std::string &strName, int64_t &unItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "IncrSeqPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "INCR %s", strName.c_str());
    EMRStatus emRet = CheckReply("INCR " + strName, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        unItem = pReply->integer;
    }
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::AuthPrivate(const std::string &password) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "AuthPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }
    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "Auth %s", password.c_str());
    EMRStatus emRet = CheckReply("Auth " + password, pReply);
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::KeysPrivate(const std::string & pattern, std::list<std::string> &keyItems) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "KeysPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "KEYS %s", pattern.c_str());
    EMRStatus emRet = CheckReply("KEYS " + pattern, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        keyItems.clear();
        for(uint32_t nIndex = 0; nIndex < pReply->elements; nIndex++) {
            keyItems.emplace_back(std::string(pReply->element[nIndex]->str, pReply->element[nIndex]->len));
        }
    }
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::QueLRangePrivate(const std::string &strName, int64_t from, int64_t to, std::vector<std::string> &strItems) {
    if(!IsConnected()){
        BOOST_LOG_TRIVIAL(error) << "LRange error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "LRange %s %ld %ld", strName.c_str(), from, to);
    std::string strFormat; FORMAT_STREAM(strFormat, "LRange " << strName << " " << from << " " << to);
    BOOST_LOG_TRIVIAL(debug) << "call: " << strFormat;
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        for(uint32_t nIndex = 0; nIndex < pReply->elements; nIndex++) {
            strItems.emplace_back(std::string(pReply->element[nIndex]->str, pReply->element[nIndex]->len));
        }
    }

    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::QueRPopPrivate(const std::string &strName, std::string &strItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "QuePushPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "RPOP %s", strName.c_str());
    EMRStatus emRet = CheckReply("RPOP " + strName, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        strItem.append(pReply->str, pReply->len);
    }

    freeReplyObject(pReply);
    return emRet;
}


EMRStatus CRedisDBInterface::QueLPopPrivate(const std::string &strName, std::string &strItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "QuePushPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "LPOP %s", strName.c_str());
    EMRStatus emRet = CheckReply("LPOP " + strName, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        strItem.append(pReply->str, pReply->len);
    }

    freeReplyObject(pReply);
    return emRet;
}


EMRStatus CRedisDBInterface::QueLPushPrivate(const std::string &strName, const std::string &strItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "QuePushPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "LPUSH  %s %b", strName.c_str(), strItem.c_str(), strItem.size());
    EMRStatus emRet = CheckReply("LPUSH " + strName, pReply);
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::QueRPushPrivate(const std::string &strName, const std::string &strItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "QuePushFrontPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "RPUSH %s %b", strName.c_str(), strItem.c_str(), strItem.size());
    EMRStatus emRet = CheckReply("RPUSH " + strName, pReply);
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::QueSizePrivate(const std::string &strName, int64_t& nRet) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "QueSizePrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "LLEN %s", strName.c_str());
    EMRStatus emRet = CheckReply("LLEN " + strName, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        nRet = pReply->integer;
    }
    freeReplyObject(pReply);
    return emRet;
}


EMRStatus CRedisDBInterface::GetMsgIdPrivate(const std::string &strKey, int64_t& nRet) {
    if (!IsConnected()){
        BOOST_LOG_TRIVIAL(error) << "GetMsgIdPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "INCR %s", strKey.c_str());
    EMRStatus emRet = CheckReply("INCR " + strKey, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet)
    {
        nRet = pReply->integer;
    }
    freeReplyObject(pReply);

    return emRet;
}

EMRStatus CRedisDBInterface::SetPrivate(const std::string &strKey, const std::string &strVal) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "SetPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "SET %s %b", strKey.c_str(), strVal.c_str(), strVal.size());
    EMRStatus emRet = CheckReply("SET " + strKey, pReply);
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::SetexPrivate(const std::string &strKey, uint64_t nNum, const std::string &strVal) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "SetPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "SETEX %s %ld %b", strKey.c_str(), nNum, strVal.c_str(), strVal.size());
    EMRStatus emRet = CheckReply("SETEX " + strKey, pReply);
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::GetPrivate(const std::string &strKey, std::string &strVal) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "GetPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "GET %s", strKey.c_str());
    EMRStatus emRet = CheckReply("GET " + strKey, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        strVal.append(pReply->str, pReply->len);
    }
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::GetSetItemCountPrivate(const std::string &strName, int64_t& nRet) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "GetSetItemCountPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "SCARD %s", strName.c_str());
    EMRStatus emRet = CheckReply("SCARD " + strName, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        nRet = pReply->integer;
    }
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::GetSetItemsPrivate(const std::string &strName, std::set<std::string> &rgItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "GetSetItemsPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    rgItem.clear();
    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "SMEMBERS %s", strName.c_str());
    EMRStatus emRet = CheckReply("SMEMBERS " + strName, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        for (uint32_t nIndex = 0; nIndex < pReply->elements; nIndex++) {
            redisReply *pReplyTmp = *(pReply->element+nIndex);
            rgItem.emplace(std::string(pReplyTmp->str, pReplyTmp->len));
        }
    }
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::SetInterPrivate(const std::vector <std::string> &sets, std::set<std::string> &resultSet) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "SetInterPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }
    if (sets.empty()) {
        return EMRStatus::EM_KVDB_RNULL;
    }

    std::string setString = "SINTER ";
    for(auto& s : sets) {
        setString += s + " ";
    }
    setString.resize(setString.size() -1);

/*	vector<char*> argv;
	argv.resize(sets.size() + 1);
	vector<size_t> argvlen;
	argv[0] = "SINTER";
	redisReply *pReply = (redisReply*)redisCommandArgv(m_pRedisDBContex, sets.size()+1, &argv[0], &argvlen[0]);
 */
    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, setString.c_str());
    EMRStatus emRet = CheckReply("SINTER " + setString, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
//		BOOST_LOG_TRIVIAL(debug) << "SINTER " + setString << " result:" << (int)emRet << ", count:" << pReply->elements;
        for (uint32_t nIndex = 0; nIndex < pReply->elements; nIndex++) {
            redisReply *pReplyTmp = *(pReply->element+nIndex);
            resultSet.emplace(std::string(pReplyTmp->str, pReplyTmp->len));
        }
    }
    freeReplyObject(pReply);

/*
	redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "SINTERSTORE set_temp \"%s\"", setString.c_str());
	EMRStatus emRet = CheckReply("SINTERSTORE set_temp " + setString, pReply);
	freeReplyObject(pReply);

	GetSetItems("set_temp", resultSet);
*/
    return emRet;

}

EMRStatus CRedisDBInterface::IsItemInSetPrivate(const std::string &strName, const std::string &strItem, bool &bIsExit) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "GetSetItemsPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "SISMEMBER  %s %s", strName.c_str(), strItem.c_str());
    EMRStatus emRet = CheckReply("SISMEMBER " + strName + " " + strItem, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        if (0 < pReply->integer) {
            bIsExit = true;
        }
        else {
            bIsExit = false;
        }
    }
    freeReplyObject(pReply);
    return emRet;
}



EMRStatus CRedisDBInterface::SetAddPrivate(const std::string &strKey, const  std::string &strItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "SetAddPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "SADD %s %b", strKey.c_str(), strItem.c_str(), strItem.size());
    EMRStatus emRet = CheckReply("SADD " + strKey, pReply);
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::ZSetAddPrivate(const std::string &strKey, int64_t score, const  std::string &strItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "ZSetAddPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "ZADD %s %ld %b", strKey.c_str(), score, strItem.c_str(), strItem.size());
    EMRStatus emRet = CheckReply("ZADD " + strKey, pReply);
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::SetRemovePrivate(const std::string &strKey, const std::string &strItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "SetRemovePrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "SREM %s %b", strKey.c_str(), strItem.c_str(), strItem.size());
    EMRStatus emRet = CheckReply("SREM " + strKey, pReply);
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::ZSetRemovePrivate(const std::string &strKey, const std::string &strItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "ZSetRemovePrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "ZREM %s %b", strKey.c_str(), strItem.c_str(), strItem.size());
    EMRStatus emRet = CheckReply("ZREM " + strKey, pReply);
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::ZScorePrivate(const std::string &strKey, const  std::string &strItem, int64_t& nRet) {
    if (!IsConnected()){
        BOOST_LOG_TRIVIAL(error) << "ZScorePrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "ZSCORE %s %s", strKey.c_str(), strItem.c_str());
    EMRStatus emRet = CheckReply("ZSCORE " + strKey, pReply);
    if(EMRStatus::EM_KVDB_SUCCESS == emRet) {
        if (pReply->str == nullptr) {
            BOOST_LOG_TRIVIAL(warning) << "Unnormal return value.";
            return EMRStatus::EM_KVDB_RNULL;
        }
        nRet = atoi(pReply->str);
    }

    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::ZCardPrivate(const std::string &strKey, int64_t& nLen) {
    if (!IsConnected()){
        BOOST_LOG_TRIVIAL(error) << "ZScorePrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "ZCARD %s", strKey.c_str());
    EMRStatus emRet = CheckReply("ZCARD " + strKey, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        nLen = pReply->integer;
    }
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::SaveGroupMessagePrivate(const std::string& table, const std::string& message, int64_t msgId) {
    return ZSetAddPrivate(table, msgId, message);
}


/*
* from = 0, to = -1 :ger all members
*/
EMRStatus CRedisDBInterface::ZRangeWithScorePrivate(const std::string &strKey,
                                                    std::list<std::string> &rgTopicItem, int64_t from, int64_t to, std::list<std::string> &rgScoreItem)
{
    if(!IsConnected()){
        BOOST_LOG_TRIVIAL(error) << "ZRangeWithScorePrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }


    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "ZRANGE %s %ld %ld WITHSCORES", strKey.c_str(), from, to);
    std::string strFormat; FORMAT_STREAM(strFormat, "ZRANGE " << strKey << " " << from << " " << to);
    BOOST_LOG_TRIVIAL(debug) << strFormat;
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        rgTopicItem.clear();
        rgScoreItem.clear();
        for(uint32_t nIndex = 0 ; nIndex < pReply->elements;nIndex++) {
            rgTopicItem.emplace_back(std::string(pReply->element[nIndex]->str, pReply->element[nIndex]->len));
            nIndex++;
            rgScoreItem.emplace_back(std::string(pReply->element[nIndex]->str));
        }
    }
    freeReplyObject(pReply);

    return emRet;
}

EMRStatus CRedisDBInterface::ZRangePrivate(const std::string &strKey, std::vector<std::string> &rgTopicItem, int64_t from, int64_t to) {
    if (!IsConnected()){
        BOOST_LOG_TRIVIAL(error) << "ZRangePrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "ZRANGE %s %ld %ld", strKey.c_str(), from, to);
    std::string strFormat; FORMAT_STREAM(strFormat, "ZRANGE " << strKey << " " << from << " " << to);
    BOOST_LOG_TRIVIAL(debug) << strFormat;
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        rgTopicItem.clear();
        for (uint32_t nIndex = 0; nIndex < pReply->elements; nIndex++) {
            rgTopicItem.emplace_back(std::string(pReply->element[nIndex]->str, pReply->element[nIndex]->len));
        }
    }
    freeReplyObject(pReply);

    return emRet;
}
/*
ZRANGEBYSCORE
* from: from score
* to  : to score
*/
EMRStatus CRedisDBInterface::ZRangeByScorePrivate(const std::string &strKey, int64_t from, int64_t to, std::vector<std::string> &rgMemberItem) {
    if(!IsConnected()){
        BOOST_LOG_TRIVIAL(error) << "ZRangeByScore error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "ZRANGEBYSCORE %s %ld %ld", strKey.c_str(), from, to);
    std::string strFormat; FORMAT_STREAM(strFormat, "ZRANGEBYSCORE " << strKey << " " << from << " " << to);
    BOOST_LOG_TRIVIAL(debug) << "call: " << strFormat;
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        rgMemberItem.clear();
        for(uint32_t nIndex = 0 ; nIndex < pReply->elements; nIndex++) {
            rgMemberItem.emplace_back(std::string(pReply->element[nIndex]->str, pReply->element[nIndex]->len));
        }
    }

    freeReplyObject(pReply);
    return emRet;
}



EMRStatus CRedisDBInterface::ZSetGetPeerMsgsPrivate(const std::string &strKey, std::vector<std::string> &msgs, int32_t& nTotalSize) {
    if(!IsConnected()){
        BOOST_LOG_TRIVIAL(error) << "ZSetGetPeerMsgsPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }
    nTotalSize = 0;
    msgs.clear();
    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "ZRANGE %s 0 -1 WITHSCORES", strKey.c_str());
    std::string strFormat; FORMAT_STREAM(strFormat, "ZRANGE " << strKey << " 0 -1 WITHSCORES");
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        for(uint32_t nIndex = 0 ; nIndex < pReply->elements; nIndex++) {
            msgs.emplace_back(std::string(pReply->element[nIndex]->str));
            nTotalSize += pReply->element[nIndex]->len;
            nIndex++;
        }
    }

    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::ZSetRemoveByScorePrivate(const std::string &strKey, int64_t scorefrom, int64_t scoreto) {
    if(!IsConnected()){
        BOOST_LOG_TRIVIAL(error) << "ZSetRemoveByScorePrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "ZREMRANGEBYSCORE  %s %ld %ld", strKey.c_str(), scorefrom, scoreto);
    std::string strFormat; FORMAT_STREAM(strFormat, "SREM " << strKey << " " << scorefrom << " " << scorefrom);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::ZSetIncrbyPrivate(const std::string &strKey, const std::string &strMember, const std::string &strScore, int64_t& nRet) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "ZSetIncrbyPrivat error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "ZINCRBY %s %s %s", strKey.c_str(), strScore.c_str(), strMember.c_str());
    std::string strFormat; FORMAT_STREAM(strFormat, "ZINCRBY " << strKey << " " << strScore);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        nRet = atoi(pReply->str);
    }

    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::GetOfflineMsgPrivate(const std::string& strTable, int nStart, int nEnd, std::vector<std::string>& msgs, int32_t& nTotalSize) {
    if(!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "GetOfflineMsgPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }
    nTotalSize = 0;
    msgs.clear();

    redisReply* pReply = (redisReply*)redisCommand(m_pRedisDBContex, "ZREVRANGEBYSCORE %s %d %d", strTable.c_str(), nStart, nEnd);
    BOOST_LOG_TRIVIAL(debug) << "GetOffline by: ZREVRANGEBYSCORE " << strTable << " " << nStart << " " << nEnd;
    std::string strFormat; FORMAT_STREAM(strFormat, "ZREVRANGEBYSCORE " << strTable << " " << nStart << " " << nEnd);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        for (size_t nIndex = 0; nIndex < pReply->elements; ++nIndex) {
            msgs.emplace_back(std::string(pReply->element[nIndex]->str, pReply->element[nIndex]->len));
            nTotalSize += pReply->element[nIndex]->len;
        }
    }

    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::CheckReply(const std::string & strFunName, redisReply * pReply) {
    EMRStatus nRet = EMRStatus::EM_KVDB_ERROR;
    if (nullptr == pReply) {
        BOOST_LOG_TRIVIAL(warning) << "redisCommand exec '" << strFunName << "' failed, ret = NULL";
        redisFree(m_pRedisDBContex);
        m_pRedisDBContex = nullptr;
        nRet = EMRStatus::EM_KVDB_ERROR;
        return nRet;
    }

    if (REDIS_REPLY_ERROR == pReply->type) {
        BOOST_LOG_TRIVIAL(error) << "redisCommand exec '" << strFunName << "' failed, pReply->type = " << pReply->type << ", pReply->str == " << pReply->str;
        nRet = EMRStatus::EM_KVDB_ERROR;
    }
    else if (REDIS_REPLY_NIL == pReply->type) {
        BOOST_LOG_TRIVIAL(debug) << "redisCommand exec '" << strFunName << "' failed, pReply->type = " << pReply->type << " :REDIS_REPLY_NIL";
        nRet = EMRStatus::EM_KVDB_RNULL;
    }
    else {
        nRet = EMRStatus::EM_KVDB_SUCCESS;
    }

    return nRet;
}

bool CRedisDBInterface::IsConnected() const {
    return nullptr != m_pRedisDBContex;
}

EMRStatus CRedisDBInterface::SetHashItemPrivate(const std::string &strHashItemName, const std::string &strField, const std::string &strItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "SetHashItemPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "HSET %s %s %b", strHashItemName.c_str(), strField.c_str(), strItem.c_str(), strItem.size());
    std::string strFormat; FORMAT_STREAM(strFormat, "HSET " << strHashItemName << " " << strField);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::HIncrbyPrivate(const std::string &strHashItemName, const std::string &strField, int64_t val) {
    int64_t  curVal;
    return HIncrbyPrivate(strHashItemName, strField, val, curVal);
}
EMRStatus CRedisDBInterface::HIncrbyPrivate(const std::string &strHashItemName, const std::string &strField, int64_t val, int64_t& curVal) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "HIncrbyPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "HINCRBY %s %s %ld", strHashItemName.c_str(), strField.c_str(), val);
    std::string strFormat; FORMAT_STREAM(strFormat, "HINCRBY " << strHashItemName << " " << strField);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        curVal = pReply->integer;
    }
    freeReplyObject(pReply);
    return emRet;
}
EMRStatus CRedisDBInterface::HLenPrivate(const std::string &strHashTable, int64_t& val) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "HLenPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "HLEN %s", strHashTable.c_str());
    EMRStatus emRet = CheckReply("HLEN " + strHashTable, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        val = pReply->integer;
    }
    freeReplyObject(pReply);
    return emRet;
}
EMRStatus CRedisDBInterface::HExistPrivate(const std::string &table, const std::string& key, bool &bIsExist) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "HEXISTS error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "HEXISTS %s %s", table.c_str(), key.c_str());
    std::string strFormat; FORMAT_STREAM(strFormat, "HEXISTS " << table << " " << key);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        bIsExist = static_cast<bool>(pReply->integer);
    }

    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::HScanPrivate(const std::string &strHashTableName, const std::string &strHashItemMatch, int page_size, int& page_nums, std::list<std::string>& hashItems, std::list<std::string>& hashItemsValue) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "HScanPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "HSCAN %s %u MATCH %s COUNT %ld", strHashTableName.c_str(), page_nums, strHashItemMatch.c_str(), page_size);
    std::string strFormat; FORMAT_STREAM(strFormat, "HSCAN " << strHashTableName << " " << page_nums << " MATCH " << strHashItemMatch << " COUNT " << page_size);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        page_nums = atoi(pReply->element[0]->str);
        for(uint32_t nIndex = 0 ; nIndex< pReply->element[1]->elements ;nIndex++) {
            hashItems.emplace_front(std::string(pReply->element[1]->element[nIndex]->str, pReply->element[1]->element[nIndex]->len));
            nIndex++;
            hashItemsValue.emplace_front(std::string(pReply->element[1]->element[nIndex]->str));
        }
    }

    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::DelHashFieldPrivate(const std::string &strHashItemName, const std::string &strField) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "DelHashFieldPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "HDEL %s %s", strHashItemName.c_str(), strField.c_str());
    std::string strFormat; FORMAT_STREAM(strFormat, "HDEL " << strHashItemName << " " << strField);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    freeReplyObject(pReply);
    return emRet;
}


EMRStatus CRedisDBInterface::GetHashItemPrivate(const std::string &strHashItemName, const std::string &strField, std::string &strItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "GetHashItemPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "HGET %s %s", strHashItemName.c_str(), strField.c_str());
    std::string strFormat; FORMAT_STREAM(strFormat, "HGET  " << strHashItemName << " " << strField);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        strItem.append(pReply->str, pReply->len);
    }

    freeReplyObject(pReply);
    return emRet;
}

EMRStatus CRedisDBInterface::GetAllHashItemsPrivate(const std::string &strKey, std::map<std::string, std::string>& items) {
    if(!IsConnected()){
        BOOST_LOG_TRIVIAL(error) << "GetAllHashItemsPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "HGETALL %s", strKey.c_str());
    std::string strFormat; FORMAT_STREAM(strFormat, "HGETALL " << strKey);
    //BOOST_LOG_TRIVIAL(debug) << strFormat;
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        if (pReply->elements % 2 != 0) {
            BOOST_LOG_TRIVIAL(error) << "Hash map elements number is error.";
        }
        else {
            for (uint32_t nIndex = 0; nIndex < pReply->elements; nIndex += 2) {
                items[std::move(std::string(pReply->element[nIndex]->str, pReply->element[nIndex]->len))] =
                    std::move(std::string(pReply->element[nIndex + 1]->str, pReply->element[nIndex + 1]->len));
            }
        }
    }
    freeReplyObject(pReply);

    return emRet;
}


EMRStatus CRedisDBInterface::DelKeyPrivate(const std::string &strKeyName) {
    if (!IsConnected())	{
        BOOST_LOG_TRIVIAL(error) << "DelKeyPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "DEL %s", strKeyName.c_str());
    std::string strFormat; FORMAT_STREAM(strFormat, "DEL" << strKeyName);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    freeReplyObject(pReply);
    return emRet;
}


EMRStatus CRedisDBInterface::IsKeyExits(const std::string &strKey, bool &bIsExit) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "IsKeyExits error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "EXISTS  %s", strKey.c_str());
    std::string strFormat; FORMAT_STREAM(strFormat, "EXISTS " << strKey);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        bIsExit = static_cast<bool>(pReply->integer);
    }

    freeReplyObject(pReply);
    return emRet;
}


EMRStatus CRedisDBInterface::QueIndexElementPrivate(const std::string &strName, int32_t nIndex, std::string &strItem) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "QueIndexElementPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "LINDEX  %s %d", strName.c_str(), nIndex);
    std::string strFormat; FORMAT_STREAM(strFormat, "LINDEX " << strName << " " << nIndex);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        strItem.append(pReply->str, pReply->len);
    }
    freeReplyObject(pReply);
    return emRet;
}



EMRStatus CRedisDBInterface::IncrSeqByPrivate(const std::string &strName, int64_t &unItem, uint64_t nNum) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "IncrSeqByPrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "INCRBY %s %ld", strName.c_str(), nNum);
    std::string strFormat; FORMAT_STREAM(strFormat, "INCRBY " << strName << nNum);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        unItem = pReply->integer;
    }
    freeReplyObject(pReply);
    return emRet;
}



EMRStatus CRedisDBInterface::ExpirePrivate(const std::string &strName, uint64_t nNum) {
    if (!IsConnected()) {
        BOOST_LOG_TRIVIAL(error) << "ExpirePrivate error,DB Is disconnect";
        return EMRStatus::EM_KVDB_ERROR;
    }

    redisReply *pReply = (redisReply*)redisCommand(m_pRedisDBContex, "EXPIRE %s %ld", strName.c_str(), nNum);
    std::string strFormat; FORMAT_STREAM(strFormat, "EXPIRE " << strName << " " << nNum);
    EMRStatus emRet = CheckReply(strFormat, pReply);
    if (EMRStatus::EM_KVDB_SUCCESS == emRet) {
        if(pReply->integer == 1) {
            emRet = EMRStatus::EM_KVDB_SUCCESS;
        }
        else {
            emRet = EMRStatus::EM_KVDB_RNULL;
        }
    }
    freeReplyObject(pReply);
    return emRet;
}



