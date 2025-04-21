

#ifndef __IPROTOCOLBASE_H__
#define __IPROTOCOLBASE_H__


#include <HCatAny.h>
#include <HCatNetInfo.h>
#include <HCatSignal.h>
#include <ICatEventApi.h>
#include <ICatEventData.h>
#include <HCatBuffer.h>


namespace cat
{


///< 发送文件传递给协议的数据
struct HSendProtocolFileInfo
{
    SendFileListType fileList;
    HCatAny userData;

    void Clear() { fileList.clear(); userData.Clear(); }
    HSendProtocolFileInfo() {}
    explicit HSendProtocolFileInfo(const SendFileListType &fileList_) : fileList(fileList_) {}
    explicit HSendProtocolFileInfo(const SendFileListType &fileList_, const HCatAny &userData_) : fileList(fileList_), userData(userData_) {}
};


class IProtocolBase : public HCatEventObject
{
public:
    ///< 传输给协议处理的指令
    enum eProtocolCmd {
        kInit,                  ///< 要求初始化全部数据, 通知重置后在Disponse返回false时需要重新初始化
        kReset,                 ///< 在返回false时, 初始化完成后将通知重新协商协议
        kSendFaildAsk,          ///< 通知协议发送失败
        kConnect,               ///< 进入连接状态, 开始协商
        kHeartBeat,             ///< 进行一次心跳包
        kReadData,              ///< 读取数据走协议处理
        kSendSDK,               ///< 发送SDK数据
        kSendFile,              ///< 发送文件, 参数: 文件名, 文件类型(int)
    };

    ///< 通知外部处理的指令
    enum eNotifyCmd {
        kNotofyConnect,         ///< 通知完成协商连接
        kSendData,              ///< 发送数据
        kReadyRead,             ///< 处理完毕的数据
        kUploadFileProgress,    ///< 上传文件进度 格式不包括括号:(文件名;;发送大小;;全部大小;;状态) 状态0正常, 1失败, 2完成
        kSendFaildAnswer,       ///< 通知发送失败
    };

public:
    HSignal<void(const std::string &log)> DebugLog;

    ///< 通知指令, 要求外部关联着执行对应的指令
    HSignal<void(eNotifyCmd cmd, const HCatBuffer &data, const HCatAny &userData)> NotifyCmd;

public:
    explicit IProtocolBase(const EventCoreWeakPtr &core) : core_(core) {}
    virtual ~IProtocolBase() {}

    /**
     * @brief Dispose 协议处理
     * @param cmd 指令
     * @param data 数据
     * @param userData 自定义数据
     * @return true成功, false失败
     */
    virtual bool Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData = HCatAny()) = 0;

    virtual std::string Protocol() const = 0;

protected:
    struct delaySend
    {
        hint32 cmd;
        HCatBuffer data;
        HCatAny userData;

        delaySend(delaySend &&other) : cmd(std::move(other.cmd)), data(std::move(other.data)), userData(std::move(other.userData)) {}
        delaySend(hint32 cmd_, const HCatBuffer &data_, const HCatAny &user) : cmd(cmd_), data(data_), userData(user) {}

        delaySend &operator =(delaySend &&other) {
            this->cmd = std::move(other.cmd);
            this->data = std::move(other.data);
            this->userData = std::move(other.userData);
            return *this;
        }
    };


protected:
    EventCoreWeakPtr core_;
};


}


#endif // __IPROTOCOLBASE_H__
