#include "test.h"

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <opal.h>
#include <opal/manager.h>
#include <opal/endpoint.h>
#include <opal/ep/localep.h>
#include <opal/ep/pcss.h>
#include <opal/sip/sipep.h>
#include <opal/patch.h>
#include <opal/transcoders.h>
#include <opal/codec/opalplugin.h>
#include <opal/codec/vidcodec.h>

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <shared_mutex>
#include <queue>
#include <condition_variable>

#include <cstring>

namespace vgw
{

using frame_data_t = std::vector<std::uint8_t>;
using frame_queue_t = std::queue<frame_data_t>;
using cond_t = std::condition_variable;
using mutex_t = std::shared_mutex;


template<class T>
using lock_t = std::lock_guard<T>;

template<class T>
using shared_lock_t = std::shared_lock<T>;

template<class T>
using unique_lock_t = std::unique_lock<T>;

class frame_queue
{
    constexpr static std::size_t max_queue_size = 5;

    mutable mutex_t         m_safe_mutex;
    cond_t                  m_signal;
    frame_queue_t           m_frames;

public:
    frame_queue()
    {

    }

    ~frame_queue()
    {

    }

    void push_frame(const void* frame_data, std::size_t frame_size)
    {
        lock_t lock(m_safe_mutex);
        m_frames.emplace(static_cast<const frame_data_t::value_type*>(frame_data)
                         , static_cast<const frame_data_t::value_type*>(frame_data) + frame_size);

        while(m_frames.size() > max_queue_size)
        {
            m_frames.pop();
        }
        m_signal.notify_one();
    }

    std::size_t pop_data(void* frame_data, std::size_t frame_size)
    {
        std::size_t result = 0;

        lock_t lock(m_safe_mutex);

        if (!m_frames.empty())
        {
            const auto& frame = m_frames.front();
            result = std::min(frame.size(), frame_size);

            if (result > 0)
            {
                std::memcpy(frame_data
                            , frame.data()
                            , result);
            }

            m_frames.pop();
        }

        return result;
    }

    bool wait_frame(std::uint32_t timeout_ms)
    {
        if (pending() > 0)
        {
            return true;
        }
        else
        {
            std::mutex signal_mutex;
            unique_lock_t lock(signal_mutex);
            m_signal.wait_for(lock
                              , std::chrono::milliseconds(timeout_ms));
        }

        return pending() > 0;
    }

    std::size_t pending() const
    {
        shared_lock_t lock(m_safe_mutex);
        return m_frames.size();
    }

};

class process : public PProcess
{
public:
    process()
    {

    }

    ~process()
    {

    }

    void Main() override
    {
        std::cout << "From PProcess Main" << std::endl;
    }
};

class call_manager : public OpalManager
{

public:

    call_manager()
    {

        bool ok = false;
        {
            PVideoDevice::OpenArgs video = GetVideoOutputDevice();
            video.deviceName = "OutDevice";
            ok = SetVideoOutputDevice(video);
        }
        {
            PVideoDevice::OpenArgs video = GetVideoInputDevice();
            video.deviceName = "InputDevice";
            ok = SetVideoInputDevice(video);
        }

        return;
    }

    ~call_manager()
    {

    }

    std::vector<std::string> get_codecs() const
    {

        std::vector<std::string> codecs;

        OpalMediaFormatList allFormats(OpalMediaFormat::GetAllRegisteredMediaFormats());
        PStringArray order(GetMediaFormatOrder());
        allFormats.Reorder(order);

        for (int i = 0; i < order.GetSize(); i++)
        {
            const PString& codec_name = order[i];
            codecs.push_back(codec_name);

            continue;
            for (int j = 0; j < allFormats.GetSize(); j++)
            {
                const OpalMediaFormat& fmt = allFormats[j];
                if (fmt.GetName() == codec_name)
                {
                    codecs.push_back(codec_name);
                    break;
                }
            }
        }

        return codecs;
    }

    void OnClearedCall(OpalCall& call) override
    {
        std::cout << "on clear call: " << call.GetLocalName() << std::endl;
        OpalManager::OnClearedCall(call);
    }

    PBoolean OnIncomingConnection(OpalConnection & connection
                                  , unsigned options
                                  , OpalConnection::StringOptions * stringOptions) override
    {
        std::cout << "on incoming connection" << std::endl;
        return OpalManager::OnIncomingConnection(connection
                                                , options
                                                , stringOptions);
    }

    PBoolean OnOpenMediaStream(OpalConnection& aConnection, OpalMediaStream& aStream) override
    {
        std::cout << "on open media stream" << std::endl;

        return OpalManager::OnOpenMediaStream(aConnection
                                              , aStream);
    }

    void OnClosedMediaStream(const OpalMediaStream& aStream) override
    {
        std::cout << "on closed media stream" << std::endl;
        return OpalManager::OnClosedMediaStream(aStream);
    }

    virtual PBoolean CreateVideoInputDevice(
      const OpalConnection & connection,    ///<  Connection needing created video device
      const OpalMediaFormat & mediaFormat,  ///<  Media format for stream
      PVideoInputDevice * & device,         ///<  Created device
      PBoolean & autoDelete                     ///<  Flag for auto delete device
    )  override
    {
        return OpalManager::CreateVideoInputDevice(connection
                                                    , mediaFormat
                                                   , device
                                                   , autoDelete);
    }

    /**Create an PVideoOutputDevice for a sink media stream or the preview
       display for a source media stream.
      */
    virtual PBoolean CreateVideoOutputDevice(
      const OpalConnection & connection,    ///<  Connection needing created video device
      const OpalMediaFormat & mediaFormat,  ///<  Media format for stream
      PBoolean preview,                         ///<  Flag indicating is a preview output
      PVideoOutputDevice * & device,        ///<  Created device
      PBoolean & autoDelete                     ///<  Flag for auto delete device
    ) override
    {
        return OpalManager::CreateVideoOutputDevice(connection
                                                    , mediaFormat
                                                   , preview
                                                   , device
                                                   , autoDelete);
    }

    /**Create a PVideoInputDevice for a source media stream.
      */
    virtual bool CreateVideoInputDevice(
      const OpalConnection & connection,    ///<  Connection needing created video device
      const PVideoDevice::OpenArgs & args,  ///< Device to change to
      PVideoInputDevice * & device,         ///<  Created device
      PBoolean & autoDelete                     ///<  Flag for auto delete device
    ) override
    {
        return OpalManager::CreateVideoInputDevice(connection
                                                   , args
                                                   , device
                                                   , autoDelete);
    }

    virtual bool CreateVideoOutputDevice(
      const OpalConnection & connection,    ///<  Connection needing created video device
      const PVideoDevice::OpenArgs & args,  ///< Device to change to
      PVideoOutputDevice * & device,        ///<  Created device
      PBoolean & autoDelete                     ///<  Flag for auto delete device
    ) override
    {
        return OpalManager::CreateVideoOutputDevice(connection
                                                    , args
                                                   , device
                                                   , autoDelete);
    }

    virtual bool SetVideoInputDevice(
      const PVideoDevice::OpenArgs & deviceArgs, ///<  Full description of device
      OpalVideoFormat::ContentRole role = OpalVideoFormat::eNoRole  ///< Role for video stream to set
    ) override
    {
        return true;
    }

    virtual PBoolean SetVideoOutputDevice(
      const PVideoDevice::OpenArgs & deviceArgs, ///<  Full description of device
      OpalVideoFormat::ContentRole role = OpalVideoFormat::eNoRole  ///< Role for video stream to set
    ) override
    {
        return true;
    }
};

class call_endpoint : public OpalLocalEndPoint
{
    frame_queue m_frame_queue_1;
    frame_queue m_frame_queue_2;
/*
    class input_video_device: public PVideoInputDevice
    {


        // PVideoDevice interface
    public:
        PStringArray GetDeviceNames() const override;
        PBoolean Open(const PString &deviceName, PBoolean startImmediate) override;
        PBoolean IsOpen() override;
        PBoolean Start() override;
        PBoolean Stop() override;
        PINDEX GetMaxFrameBytes() override;

        // PVideoInputDevice interface
    public:
        PBoolean IsCapturing() override;
        PBoolean GetFrameData(BYTE *buffer, PINDEX *bytesReturned) override;
        PBoolean GetFrameDataNoDelay(BYTE *buffer, PINDEX *bytesReturned) override;
    };*/

public:
    call_endpoint(OpalManager& manager)
        : OpalLocalEndPoint(manager
                            , OPAL_LOCAL_PREFIX
                            , true)
    {
        // m_useCallback[OpalMediaType::Video()] = NoCallbacks;
    }

    ~call_endpoint()
    {

    }

    frame_queue* get_frame_queue(std::uint8_t session_id)
    {
        switch(session_id)
        {
            case 1:
                return &m_frame_queue_1;
            break;
            case 2:
                return &m_frame_queue_2;
            break;
        }

        return nullptr;
    }

    virtual bool OnReadMediaFrame(
      const OpalLocalConnection & connection, ///<  Connection for media
      const OpalMediaStream & mediaStream,    ///<  Media stream data is required for
      RTP_DataFrame & frame                   ///<  RTP frame for data
    )
    {
        /*
        std::cout << "Session #" << mediaStream.GetSessionID() <<  " read frame: size: " << frame.GetPacketSize()
                  << ", seq: " << frame.GetSequenceNumber()
                  << ", pt: " << frame.GetPayloadType() << std::endl;*/
        return OpalLocalEndPoint::OnReadMediaFrame(connection
                                                   , mediaStream
                                                   , frame);
    }

    virtual bool OnWriteMediaFrame(
      const OpalLocalConnection & connection, ///<  Connection for media
      const OpalMediaStream & mediaStream,    ///<  Media stream data is required for
      RTP_DataFrame & frame                   ///<  RTP frame for data
    )
    {
        /*
        std::cout << "Session #" << mediaStream.GetSessionID() << " write frame: size: " << frame.GetPacketSize()
                  << ", seq: " << frame.GetSequenceNumber()
                  << ", pt: " << frame.GetPayloadType() << std::endl;*/
        return OpalLocalEndPoint::OnWriteMediaFrame(connection
                                                   , mediaStream
                                                   , frame);
    }



    // OpalLocalEndPoint interface
public:
    bool OnReadMediaData(const OpalLocalConnection &connection
                         , const OpalMediaStream &mediaStream
                         , void *data
                         , PINDEX size
                         , PINDEX &length) override
    {
        if (auto queue = get_frame_queue(mediaStream.GetSessionID()))
        {
            // auto format = mediaStream.GetMediaFormat();
            if (size > 0)
            {
                if (queue->wait_frame(100))
                {
                    length = queue->pop_data(data, size);
                    std::cout << "Session #" << mediaStream.GetSessionID() << ", read frame size: " << length
                              << ", pending: " << queue->pending()
                              << ", source: " << mediaStream.IsSource()
                              << std::endl;


                    if (mediaStream.GetMediaFormat().GetMediaType() == OpalMediaType::Video())
                    {
                        auto& frame_header = *static_cast<PluginCodec_Video_FrameHeader*>(data);
                        void* frame_data = static_cast<std::uint8_t*>(data) + sizeof(PluginCodec_Video_FrameHeader);
                        std::size_t frame_size = length - sizeof(PluginCodec_Video_FrameHeader);

                        // frame_header.width /= 2;
                        // frame_header.height /= 2;

                        frame_size = (frame_header.width * frame_header.height * 3) / 2;
                        length = frame_size + sizeof(PluginCodec_Video_FrameHeader);

                        std::cout << "Read video frame:"
                                  << " x: " << frame_header.x
                                  << ", y: " << frame_header.y
                                  << ", width: " << frame_header.width
                                  << ", height: " << frame_header.height
                                  << ", frame_size: " << frame_size
                                  << std::endl;

                    }
                    return true;

                }

            }
        }

        return false;
        /*
        return OpalLocalEndPoint::OnReadMediaData(connection
                                                  , mediaStream
                                                  , data
                                                  , size
                                                  , length);*/
    }

    bool OnWriteMediaData(const OpalLocalConnection &connection
                          , const OpalMediaStream &mediaStream
                          , const void *data
                          , PINDEX length
                          , PINDEX &written) override
    {

        if (auto queue = get_frame_queue(mediaStream.GetSessionID()))
        {
            if (length > 0)
            {
                queue->push_frame(data, length);
                written = length;
                auto format = mediaStream. GetMediaFormat();
                // auto format.GetOptionInteger()
                if (format.GetMediaType() == OpalMediaType::Audio())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
                else
                {
                    const auto& frame_header = *static_cast<const PluginCodec_Video_FrameHeader*>(data);
                    const void* frame_data = static_cast<const std::uint8_t*>(data) + sizeof(PluginCodec_Video_FrameHeader);
                    std::size_t frame_size = length - sizeof(PluginCodec_Video_FrameHeader);


                    std::cout << "Write video frame:"
                              << " x: " << frame_header.x
                              << ", y: " << frame_header.y
                              << ", width: " << frame_header.width
                              << ", height: " << frame_header.height
                              << ", frame_size: " << frame_size
                              << std::endl;
                }
            }
            return true;
        }

        return false;
        /*
        std::cout << "Session #" << mediaStream.GetSessionID() << " write frame: size: " << length << std::endl;
        return OpalLocalEndPoint::OnWriteMediaData(connection
                                                   , mediaStream
                                                   , data
                                                   , length
                                                   , written);*/
    }


    // OpalLocalEndPoint interface
public:
    bool CreateVideoInputDevice(const OpalConnection &connection
                                , const OpalMediaFormat &mediaFormat
                                , PVideoInputDevice *&device
                                , bool &autoDelete) override
    {
        return OpalLocalEndPoint::CreateVideoInputDevice(connection
                                                          , mediaFormat
                                                          , device
                                                          , autoDelete);
    }

    bool CreateVideoOutputDevice(const OpalConnection &connection
                                 , const OpalMediaFormat &mediaFormat
                                 , bool preview
                                 , PVideoOutputDevice *&device
                                 , bool &autoDelete) override
    {
        return OpalLocalEndPoint::CreateVideoOutputDevice(connection
                                                          , mediaFormat
                                                          , preview
                                                          , device
                                                          , autoDelete);
    }
};

void test1()
{
    std::cout << "Begin test!" << std::endl;
    process proc;
    proc.PreInitialise(1, nullptr);
    proc.InternalMain();


    call_manager    manager;
    call_endpoint   endpoint(manager);
    SIPEndPoint     sip(manager);

    std::cout << "Supported codecs: " << std::endl;

    for (const auto& f : manager.get_codecs())
    {
        std::cout << f << std::endl;
    }

    manager.AddRouteEntry("sip.*:.* = local:");
    manager.AddRouteEntry("local:.* = si:<da>");

    if (sip.StartListeners(PStringArray()))
    {
        std::this_thread::sleep_for(std::chrono::minutes(10));
    }


    std::cout << "End test!" << std::endl;

    return ;
}

void test()
{
    test1();
}

}
