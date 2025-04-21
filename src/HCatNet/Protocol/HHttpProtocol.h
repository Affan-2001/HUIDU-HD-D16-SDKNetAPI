

#ifndef __HHTTPPROTOCOL_H__
#define __HHTTPPROTOCOL_H__



#include <Protocol/IProtocolBase.h>
#include <Protocol/HStruct.h>


class FileCtl;

namespace cat
{


class HHttpProtocol : public IProtocolBase
{
public:
    explicit HHttpProtocol(const EventCoreWeakPtr &core);
    virtual ~HHttpProtocol();

    virtual bool Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData = HCatAny()) override;
    virtual std::string Protocol() const override { return Protocol::HHttpProtocol; }

    static HHttpRequest CreateRequest(HHttpRequest::eType type, const std::string &url);
    static HHttpRequest &AppRequest(HHttpRequest &dest, const HHttpRequest &src);
    static HCatBuffer ToHttpHeader(const HHttpRequest &header);

    static HHttpResponse ToHttpResponseHeader(const HCatBuffer &data);
    static HCatBuffer ExtractHeader(const HCatBuffer &data);

private:
    void Init();
    bool SendData(const HCatBuffer &data, const HCatAny &userData = HCatAny());

    bool ParseSendData(const HCatBuffer &data, const HCatAny &userData);
    bool ParseReadData(const HCatBuffer &data);

    ///< 通知信号
    void NotifyReadyData(const HCatBuffer &data, const HCatAny &userData = HCatAny());
    void NotifyReset();

    ///< 处理日志
    void ParseLog(const std::string &log);

    ///< 判断http属性是否存在, type小写
    bool IsHeaderType(const AttributeTable &table, std::string key, std::string valus);

    bool DelaySend();

private:
    HHttpResponse response_;
    HCatBuffer readData_;
    std::shared_ptr<FileCtl> tempFile_;
    std::list<delaySend> delaySend_;
    hint64 DataLen_;
    hint64 downLen_;
    bool sendfaild_;
    bool negotiate_;
};


}


#endif // __HHTTPPROTOCOL_H__
