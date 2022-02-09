#include <XiaoTuNetBox/VideoCapture.h>

#include <string>
#include <cstring>
#include <map>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


std::map<uint32_t, std::string> __gCtrlId = {
    { V4L2_CID_EXPOSURE_AUTO,     "V4L2_CID_EXPOSURE_AUTO, 曝光类型"},
    { V4L2_CID_EXPOSURE_ABSOLUTE, "V4L2_CID_EXPOSURE_ABSOLUTE, 曝光时间, 单位100us"},
    { V4L2_CID_BRIGHTNESS,        "V4L2_CID_BRIGHTNESS, 亮度"},
    { V4L2_CID_CONTRAST,          "V4L2_CID_CONTRAST, 对比度"},
    { V4L2_CID_SATURATION,        "V4L2_CID_SATURATION, 饱和度"},
    { V4L2_CID_HUE,               "V4L2_CID_HUE, 色度"},
    { V4L2_CID_EXPOSURE,          "V4L2_CID_EXPOSURE, 曝光"},
    { V4L2_CID_AUTOGAIN,          "V4L2_CID_AUTOGAIN, 自动增益/曝光"},
    { V4L2_CID_GAIN,              "V4L2_CID_GAIN, 增益"},
};

std::map<uint32_t, std::string> __gCtrlType = {
    { V4L2_CTRL_TYPE_INTEGER, "INTEGER"},
    { V4L2_CTRL_TYPE_BOOLEAN, "BOOLEAN"},
    { V4L2_CTRL_TYPE_MENU,    "MENU"},
};

std::map<uint32_t, std::string> __gExposureType = {
    { V4L2_EXPOSURE_AUTO,              "V4L2_EXPOSURE_AUTO, 自动曝光时间, 自动光圈"},
    { V4L2_EXPOSURE_MANUAL,            "V4L2_EXPOSURE_MANUAL, 手动曝光时间, 手动光圈"},
    { V4L2_EXPOSURE_SHUTTER_PRIORITY,  "V4L2_EXPOSURE_SHUTTER_PRIORITY, 手动曝光时间, 自动光圈"},
    { V4L2_EXPOSURE_APERTURE_PRIORITY, "V4L2_EXPOSURE_SHUTTER_PRIORITY, 自动曝光时间, 手动光圈"},
};

std::string __gExposeTypeErrorMsg = "无法获取曝光类型";

namespace xiaotu {
namespace net {

    /*
     * VideoCapture - 构造函数
     * 
     * 打开设备 --> 查看设备功能 --> 设置图片格式 --> 申请缓存 --> 用户指针映射
     */
    VideoCapture::VideoCapture(std::string const & path)
    {
        mModuleFd = open(path.c_str(), O_RDWR | O_NONBLOCK); 
        if (-1 == mModuleFd) {
            fprintf(stderr, "无法打开设备 '%s': %d, %s\n", path.c_str(), errno, strerror(errno));
            exit(-1);
        }

        int re = ioctl(mModuleFd, VIDIOC_QUERYCAP, &mCap);
        if (-1 == re) {
            fprintf(stderr, "无法获取设备信息 '%s': %d, %s\n", path.c_str(), errno, strerror(errno));
            exit(-1);
        }

        if (!(mCap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            fprintf(stderr, "%s 不是 video capture 设备\n", path.c_str());
            exit(-1);
        }


        if (!(mCap.capabilities & V4L2_CAP_STREAMING)) {
            fprintf(stderr, "目前只支持 STREAMING 类型的设备\n");
            exit(-1);
        }
 
        re = ioctl(mModuleFd, VIDIOC_G_PRIORITY, &mPrio);
        assert(-1 != re);

        //////////////////////////////////////////////////////////////////////////////////
        // v4l2_format
        //////////////////////////////////////////////////////////////////////////////////

        mEventHandler = xiaotu::net::PollEventHandlerPtr(new xiaotu::net::PollEventHandler(mModuleFd));
        mEventHandler->EnableRead(true);
        mEventHandler->EnableWrite(false);
        mEventHandler->SetReadCallBk(std::bind(&VideoCapture::OnReadEvent, this));
    }

    VideoCapture::~VideoCapture()
    {
        close(mModuleFd);
    }

    void VideoCapture::SetImageSizeOrDie(int height, int width) {
        memset(&mFmt, 0, sizeof(mFmt));
        mFmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;			// 传输流类型
        mFmt.fmt.pix.width = width;                 		// 宽度
        mFmt.fmt.pix.height = height;               		// 高度
        mFmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;    // 采样类型
        mFmt.fmt.pix.field = V4L2_FIELD_INTERLACED;      // 采样区域
        if (ioctl(mModuleFd, VIDIOC_S_FMT, &mFmt) < 0) {
            close(mModuleFd);
            exit(-1);
        }
    }

    void VideoCapture::PrepareBufferOrDie(int nbuf)
    {
        mNumBuffers = nbuf;

        memset(&mBufReq, 0, sizeof(mBufReq));
        mBufReq.count = mNumBuffers;
        mBufReq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        mBufReq.memory = V4L2_MEMORY_USERPTR;
        if (-1 == ioctl(mModuleFd, VIDIOC_REQBUFS, &mBufReq)) {
            fprintf(stderr, "ERROR VIDIOC_REQBUFS: %d, %s\n", errno, strerror(errno));
            close(mModuleFd);
            exit(-1);
        }

        mBuffers.resize(mNumBuffers);
        for (int i = 0; i < mNumBuffers; i++) {
            mBuffers[i] = BufferPtr(new Buffer);
            mBuffers[i]->Alloc(mFmt.fmt.pix.sizeimage);
            mBuffers[i]->id = i + 1;
        }

        mBufferInUse = BufferPtr(new Buffer);
        mBufferInUse->Alloc(mFmt.fmt.pix.sizeimage);
        mBufferInUse->id = 0;

        mUsingBuffer = false;
    }
    

    void VideoCapture::StartOrDie()
    {
        for (int i = 0; i < mNumBuffers; i++) {
            struct v4l2_buffer buf;
            memset(&buf, 0, sizeof(buf));

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.index = i;
            buf.m.userptr = (unsigned long)mBuffers[i]->start;
            buf.length = mBuffers[i]->length;

            if (-1 == ioctl(mModuleFd, VIDIOC_QBUF, &buf)) {
                fprintf(stderr, "ERROR VIDIOC_QBUF: %d, %s\n", errno, strerror(errno));
                close(mModuleFd);
                exit(-1);
            }
        }

        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == ioctl(mModuleFd, VIDIOC_STREAMON, &type)) {
            fprintf(stderr, "ERROR VIDIOC_STREAMON: %d, %s\n", errno, strerror(errno));
            close(mModuleFd);
            exit(-1);
        }
    }

    /*
     * EnumFmt - 枚举支持的格式
     */
    void VideoCapture::EnumFmt()
    {
        struct v4l2_fmtdesc desc;
        desc.index = 0;
        desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        int re = ioctl(mModuleFd, VIDIOC_ENUM_FMT, &desc);
        while (0 == re) {
            std::cout << desc << std::endl;
            desc.index++;
            re = ioctl(mModuleFd, VIDIOC_ENUM_FMT, &desc);
        }
    }

    void VideoCapture::QueryMenu(struct v4l2_queryctrl const & ctrl)
    {
        struct v4l2_querymenu menu;
        menu.id = ctrl.id;

        for (menu.index = ctrl.minimum; menu.index <= ctrl.maximum; menu.index++) {
            if (0 == ioctl(mModuleFd, VIDIOC_QUERYMENU, &menu))
                std::cout << "    " << menu.name << std::endl;
        }
    }

    void VideoCapture::QueryCtrl(uint32_t id)
    {
        struct v4l2_queryctrl ctrl;
        ctrl.id = id;

        int re = ioctl(mModuleFd, VIDIOC_QUERYCTRL, &ctrl);
        if (-1 == re) {
            perror("不支持 VIDIOC_QUERYCTRL");
            return;
        }

        std::cout << ctrl << std::endl;
        if (V4L2_CTRL_TYPE_MENU == ctrl.type) {
            QueryMenu(ctrl);
        }
    }

    std::string const & VideoCapture::GetExposureType()
    {
        struct v4l2_control ctrl;
        ctrl.id = V4L2_CID_EXPOSURE_AUTO;
        int re = ioctl(mModuleFd, VIDIOC_G_CTRL, &ctrl);
        if (-1 == re)
            return __gExposeTypeErrorMsg;

        return __gExposureType[ctrl.value];
    }

    void VideoCapture::SetExposureType(uint32_t type)
    {
        struct v4l2_control ctrl;
        ctrl.id = V4L2_CID_EXPOSURE_AUTO;
        ctrl.value = type;
        if (ioctl(mModuleFd, VIDIOC_S_CTRL, &ctrl) < 0)
            printf("设置曝光模式失败\n");
    }

    /*
     * GetCtrlInteger - 查询整型的控制指令
     */
    int VideoCapture::GetCtrlInteger(uint32_t id)
    {
        struct v4l2_control ctrl;
        ctrl.id = id;
        int re = ioctl(mModuleFd, VIDIOC_G_CTRL, &ctrl);
        if (-1 == re) {
            std::cerr << "不支持 " << __gCtrlId[id] << std::endl;
            return 0;
        }

        return ctrl.value;
    }

    /*
     * SetCtrlInteger - 设定整型的控制指令
     */
    bool VideoCapture::SetCtrlInteger(uint32_t id, int value)
    {
        struct v4l2_control ctrl;
        ctrl.id = id;
        ctrl.value = value;
        int re = ioctl(mModuleFd, VIDIOC_S_CTRL, &ctrl);
        if (-1 == re) {
            std::cerr << "不支持 " << __gCtrlId[id] << std::endl;
            return false;
        }

        return true;
    }

    /*
     * SetFrameRate - 设置帧率 rate = deo / num
     */
    void VideoCapture::SetFrameRate(int deo, int num) {
        struct v4l2_streamparm stream_parm;
        stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        stream_parm.parm.capture.timeperframe.denominator = deo;
        stream_parm.parm.capture.timeperframe.numerator = num;
        ioctl(mModuleFd, VIDIOC_S_PARM, &stream_parm);
    }

    double VideoCapture::GetFrameRate()
    {
        struct v4l2_streamparm stream_parm;
        stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(mModuleFd, VIDIOC_G_PARM, &stream_parm);
        return (double)(stream_parm.parm.capture.timeperframe.denominator) /
               (double)(stream_parm.parm.capture.timeperframe.numerator);
    }

    /*
     * IsAutoGain - 判定设备是否是自动增益
     */
    bool VideoCapture::IsAutoGain()
    {
        struct v4l2_control ctrl;
        ctrl.id = V4L2_CID_AUTOGAIN;
        int re = ioctl(mModuleFd, VIDIOC_G_CTRL, &ctrl);
        if (-1 == re) {
            perror("不支持 V4L2_CID_AUTOGAIN");
            return false;
        }

        return ctrl.value != 0;
    }

    /*
     * OnReadEvent - PollLoop循环的可读事件回调
     * 
     * 读数据 --> 替换缓存 --> 应用回调 --> 缓存重入列
     */
    void VideoCapture::OnReadEvent()
    {
        struct v4l2_buffer buf;
        unsigned int i;
        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
 
        if (-1 == ioctl(mModuleFd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
                case EAGAIN:
                    return;
                case EIO:
                    break;
                default:
                    perror("VIDIOC_DQBUF");
                    exit(-1);
            }
        }

        for (int i = 0; i < mNumBuffers; i++) {
            if (buf.m.userptr == (unsigned long)mBuffers[i]->start && buf.length == mBuffers[i]->length) {
                if (mCaptureCB && !mUsingBuffer) {
                    mBufferInUseMutex.lock();
                        mUsingBuffer = true;
                        std::swap(mBufferInUse, mBuffers[i]);
                    mBufferInUseMutex.unlock();
                    
                    mCaptureCB(mBufferInUse);

                    buf.m.userptr = (unsigned long)mBuffers[i]->start;
                    buf.length = mBuffers[i]->length;
                }
                break;
            }
        }

        if (-1 == ioctl(mModuleFd, VIDIOC_QBUF, &buf)) {
            perror("VIDIOC_QBUF");
            exit(-1);
        }
    }

    void VideoCapture::ReleaseBufferInUse()
    {
        mBufferInUseMutex.lock();
        mUsingBuffer = false;
        mBufferInUseMutex.unlock();
    }
    /*
     * SetPriority - 设置当前应用访问访问设备的优先级
     */
    bool VideoCapture::SetPriority(enum v4l2_priority prio)
    {
        int re = ioctl(mModuleFd, VIDIOC_S_PRIORITY, &prio);
        if (-1 == re) {
            perror("设置优先级出错:");
            return false;
        }

        enum v4l2_priority tmp;
        re = ioctl(mModuleFd, VIDIOC_G_PRIORITY, &tmp);
        if (-1 == re) {
            perror("获取优先级出错:");
            return false;
        }

        mPrio = tmp;
        return mPrio == prio;
    }

}
}


    /*******************************************************************************************************/

    /*
     * __OStreamCap - 解析并输出v4l2的特性
     * 
     * @stream: 输出流对象
     * @cap: 设备特性描述字
     */
    std::ostream & __OStreamCap(std::ostream & stream, const uint32_t cap)
    {
        if (V4L2_CAP_VIDEO_CAPTURE & cap)
            stream << "    [Support] Single-planar API Video Capture!" << std::endl;
        if (V4L2_CAP_VIDEO_CAPTURE_MPLANE & cap)
            stream << "    [Support] Multi-planar API Video Capture!" << std::endl;

        if (V4L2_CAP_VIDEO_OUTPUT & cap)
            stream << "    [Support] Single-planar API Video Output!" << std::endl;
        if (V4L2_CAP_VIDEO_OUTPUT_MPLANE & cap)
            stream << "    [Support] Multi-planar API Video Output!" << std::endl;

        if (V4L2_CAP_VIDEO_M2M & cap)
            stream << "    [Support] Single-planar API Video Memory-To-Memory interface!" << std::endl;
        if (V4L2_CAP_VIDEO_M2M_MPLANE & cap)
            stream << "    [Support] Multi-planar API Video Memory-To-Memory interface!" << std::endl;

        if (V4L2_CAP_VIDEO_OVERLAY & cap)
            stream << "    [Support] Video Overlay! A video overlay device typically stores captured images directly in the video memory of a graphics card, with hardware clipping and scaling." << std::endl;

        if (V4L2_CAP_VBI_CAPTURE & cap)
            stream << "    [Support] Raw VBI Capture!" << std::endl;
        if (V4L2_CAP_VBI_OUTPUT & cap)
            stream << "    [Support] Raw VBI Output!" << std::endl;

        if (V4L2_CAP_SLICED_VBI_CAPTURE & cap)
            stream << "    [Support] Sliced VBI Capture!" << std::endl;
        if (V4L2_CAP_SLICED_VBI_OUTPUT & cap)
            stream << "    [Support] Sliced VBI Output!" << std::endl;

        if (V4L2_CAP_RDS_CAPTURE & cap)
            stream << "    [Support] RDS Capture!" << std::endl;

        if (V4L2_CAP_VIDEO_OUTPUT_OVERLAY & cap)
            stream << "    [Support] Video Output Overlay (OSD)!" << std::endl;

        if (V4L2_CAP_HW_FREQ_SEEK & cap)
            stream << "    [Support] ioctl VIDIOC_S_HW_FREQ_SEEK! hardware frequency seeking" << std::endl;

        if (V4L2_CAP_RDS_OUTPUT & cap)
            stream << "    [Support] RDS output interface" << std::endl;

        if (V4L2_CAP_TUNER & cap)
            stream << "    [Support] The device has some sort of tuner to receive RF-modulated video signals." << std::endl;

        if (V4L2_CAP_AUDIO & cap)
            stream << "    [Support] The device has audio inputs or outputs. It may or may not support audio recording or playback, in PCM or compressed formats. PCM audio support must be implemented as ALSA or OSS interface." << std::endl;

        if (V4L2_CAP_RADIO & cap)
            stream << "    [Support] This is a radio receiver." << std::endl;

        if (V4L2_CAP_MODULATOR & cap)
            stream << "    [Support] The device has some sort of modulator to emit RF-modulated video/audio signals." << std::endl;

        if (V4L2_CAP_SDR_CAPTURE & cap)
            stream << "    [Support] SDR Capture." << std::endl;

        if (V4L2_CAP_EXT_PIX_FORMAT & cap)
            stream << "    [Support] struct v4l2_pix_format extended fields." << std::endl;

        if (V4L2_CAP_SDR_OUTPUT & cap)
            stream << "    [Support] SDR Output." << std::endl;

        if (V4L2_CAP_READWRITE & cap)
            stream << "    [Support] read() write() I/O." << std::endl;

        if (V4L2_CAP_ASYNCIO & cap)
            stream << "    [Support] 异步I/O." << std::endl;

        if (V4L2_CAP_STREAMING & cap)
            stream << "    [Support] streaming I/O." << std::endl;

        if (V4L2_CAP_TOUCH & cap)
            stream << "    [Support] 触摸设备." << std::endl;

        if (V4L2_CAP_DEVICE_CAPS & cap)
            stream << "    [Support] The driver fills the device_caps field. This capability can only appear in the capabilities field and never in the device_caps field." << std::endl;

        return stream;
    }

    /*
     * operator << - 序列化输出v4l2_capability
     */
    std::ostream & operator << (std::ostream & stream, struct v4l2_capability const & cap)
    {
        stream << "driver:   " << cap.driver << std::endl;
        stream << "card:     " << cap.card << std::endl;
        stream << "bus_info: " << cap.bus_info << std::endl;

        char str[80];
        sprintf(str, "version:  %u.%u.%u", (cap.version >> 16) & 0xFF, (cap.version >> 8) & 0xFF, cap.version & 0xFF);
        stream << str << std::endl;

        stream << "整机特性:" << std::endl;
        __OStreamCap(stream, cap.capabilities) << std::endl;

        if (V4L2_CAP_DEVICE_CAPS & cap.capabilities) {
            stream << "当前打开设备的特性:" << std::endl;
            __OStreamCap(stream, cap.device_caps) << std::endl;
        }

        return stream;
    }

    char const * __InputTypeStr[] = {
        {0},
        {"TUNER"},
        {"CAMERA"},
        {"TOUCH"}
    };
    /*
     * operator << - 序列化输出v4l2_input
     */
    std::ostream & operator << (std::ostream & stream, struct v4l2_input const & input)
    {
        stream << "index:" << input.index << std::endl;
        stream << "name: " << input.name << std::endl;
        stream << "type: " << __InputTypeStr[input.type] << std::endl;

        char str[80];
        sprintf(str, "audioset:  %4x", input.audioset);
        stream << str << std::endl;

        memset(str, 0, 80);
        sprintf(str, "tuner:  %4x", input.tuner);
        stream << str << std::endl;

        memset(str, 0, 80);
        sprintf(str, "std:  %8llx", input.std);
        stream << str << std::endl;

        memset(str, 0, 80);
        sprintf(str, "status:  %4x", input.status);
        stream << str << std::endl;

        if (V4L2_IN_CAP_DV_TIMINGS & input.capabilities)
            stream << "    [support] setting video timings by using VIDIOC_S_DV_TIMINGS" << std::endl;
        if (V4L2_IN_CAP_STD & input.capabilities)
            stream << "    [support] setting TV standard by using VIDIOC_S_STD" << std::endl;
        if (V4L2_IN_CAP_NATIVE_SIZE & input.capabilities)
            stream << "    [support] setting native size by using V4L2_SEL_TGT_NATIVE_SIZE" << std::endl;


        return stream;
    }

    std::ostream & operator << (std::ostream & stream, struct v4l2_rect const & rect)
    {
        stream << "    left:  " << rect.left << std::endl;
        stream << "    top:   " << rect.top << std::endl;
        stream << "    width: " << rect.width << std::endl;
        stream << "    height:" << rect.height << std::endl;

        return stream;
    }


    std::ostream & operator << (std::ostream & stream, struct v4l2_fract const & fract)
    {
        stream << "numerator:  " << fract.numerator << std::endl;
        stream << "denominator:" << fract.denominator << std::endl;
        stream << "帧周期:" << (fract.numerator / fract.denominator) << std::endl;
        stream << "帧率:" << (fract.denominator / fract.numerator) << std::endl;

        return stream;
    }

    std::ostream & operator << (std::ostream & stream, struct v4l2_cropcap const & cap)
    {
        stream << "type: " << cap.type << std::endl;
        stream << "bounds:" << std::endl << cap.bounds;
        stream << "defrect:" << std::endl << cap.defrect;
        stream << cap.pixelaspect << std::endl;

        return stream;
    }

    char const * __FormatTypeStr[] = {
        {0},
        {"V4L2_BUF_TYPE_VIDEO_CAPTURE"},
        {"V4L2_BUF_TYPE_VIDEO_OUTPUT"},
        {"V4L2_BUF_TYPE_VIDEO_OVERLAY"},
        {"V4L2_BUF_TYPE_VBI_CAPTURE"},
        {"V4L2_BUF_TYPE_VBI_OUTPUT"},
        {"V4L2_BUF_TYPE_SLICED_VBI_CAPTURE"},
        {"V4L2_BUF_TYPE_SLICED_VBI_OUTPUT"},
        {"V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY"},
        {"V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE"},
        {"V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE"},
        {"V4L2_BUF_TYPE_SDR_CAPTURE"},
        {"V4L2_BUF_TYPE_SDR_OUTPUT"},
    };

    std::map<uint32_t, std::string> __gPixelFormat = {
        { V4L2_PIX_FMT_RGB332, "V4L2_PIX_FMT_RGB332"},
        { V4L2_PIX_FMT_ARGB444, "V4L2_PIX_FMT_ARGB444"},

        { V4L2_PIX_FMT_YUYV, "V4L2_PIX_FMT_YUYV"},
    };

    std::ostream & operator << (std::ostream & stream, struct v4l2_format const & format)
    {
        stream << "type: " << __FormatTypeStr[format.type] << std::endl;
        stream << "width: " << format.fmt.pix.width << std::endl;
        stream << "height:" << format.fmt.pix.height << std::endl;
        stream << "bytesperline:" << format.fmt.pix.bytesperline << std::endl;
        stream << "sizeimage:" << format.fmt.pix.sizeimage << std::endl;
        char str[80];
        sprintf(str, "pixelformat: %c%c%c%c",  format.fmt.pix.pixelformat & 0xFF,
                                                (format.fmt.pix.pixelformat >> 8) & 0xFF,
                                                (format.fmt.pix.pixelformat >> 16) & 0xFF,
                                                (format.fmt.pix.pixelformat >> 24) & 0xFF);
        stream << str << std::endl;
        return stream;
    }

    std::ostream & operator << (std::ostream & stream, struct v4l2_fmtdesc const & desc)
    {
        stream << "--------------------------" << std::endl;
        stream << "index: " << desc.index << std::endl;
        stream << "type: " << __FormatTypeStr[desc.type] << std::endl;
        stream << "flags: " << desc.flags << std::endl;

        stream << desc.description << std::endl;

        char str[80];
        sprintf(str, "pixelformat: %c%c%c%c", desc.pixelformat & 0xFF,
                                             (desc.pixelformat >> 8) & 0xFF,
                                             (desc.pixelformat >> 16) & 0xFF,
                                             (desc.pixelformat >> 24) & 0xFF);
        stream << str << std::endl;
   
        return stream;
    }


    std::ostream & operator << (std::ostream & stream, struct v4l2_queryctrl const & ctrl)
    {
        stream << "id:" << ctrl.id << __gCtrlId[ctrl.id] << std::endl;    
        stream << "type:" << ctrl.type << ", " << __gCtrlType[ctrl.type] << std::endl;
        stream << "name:" << ctrl.name << std::endl;
        stream << "min:" << ctrl.minimum << std::endl;
        stream << "max:" << ctrl.maximum << std::endl;
        stream << "step:" << ctrl.step << std::endl;
        stream << "default_value:" << ctrl.default_value << std::endl;
        stream << "flags:" << std::hex << ctrl.flags << std::dec << std::endl;

        return stream;
    }

