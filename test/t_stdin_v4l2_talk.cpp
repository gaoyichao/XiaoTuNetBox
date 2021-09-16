#include <string>
#include <iostream>
#include <vector>

#include <XiaoTuNetBox/Connection.h>
#include <XiaoTuNetBox/PollLoop.h>
#include <XiaoTuNetBox/VideoCapture.h>

xiaotu::net::VideoCapture::BufferPtr gBuffer;

void OnStdinRawMsg(xiaotu::net::VideoCapturePtr capture, xiaotu::net::RawMsgPtr const & msg) {
    std::cout << "msg.size = " << msg->size() << std::endl;

    switch ((*msg)[0]) {
        case 'a':
            std::cout << capture->mCap << std::endl;
            std::cout << capture->mFmt << std::endl;
            capture->EnumFmt();
            capture->ReleaseBufferInUse();
            break;
        case 'g':
            std::cout << "---- V4L2_CID_BRIGHTNESS -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_BRIGHTNESS);
            std::cout << "---- V4L2_CID_CONTRAST -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_CONTRAST);
            std::cout << "---- V4L2_CID_SATURATION -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_SATURATION);
            std::cout << "---- V4L2_CID_HUE -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_HUE);

            std::cout << "---- 关于曝光 -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_EXPOSURE_AUTO);
            capture->QueryCtrl(V4L2_CID_EXPOSURE_ABSOLUTE);
            capture->QueryCtrl(V4L2_CID_IRIS_ABSOLUTE);

            std::cout << "---- V4L2_CID_AUTOGAIN -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_AUTOGAIN);

            std::cout << "---- V4L2_CID_GAIN -----" << std::endl;
            capture->QueryCtrl(V4L2_CID_GAIN);

            std::cout << "Gain:" << capture->GetCtrlInteger(V4L2_CID_GAIN) << std::endl;
            std::cout << "亮度:" << capture->GetCtrlInteger(V4L2_CID_BRIGHTNESS) << std::endl;
            std::cout << "对比度:" << capture->GetCtrlInteger(V4L2_CID_CONTRAST) << std::endl;
            std::cout << "饱和度:" << capture->GetCtrlInteger(V4L2_CID_SATURATION) << std::endl;
            std::cout << "色度:" << capture->GetCtrlInteger(V4L2_CID_HUE) << std::endl;

            break;
        case 'h':
            capture->SetCtrlInteger(V4L2_CID_GAIN, 10);
            break;
        case 'H':
            capture->SetCtrlInteger(V4L2_CID_GAIN, 64);
            break;
        case 'i':
            capture->SetCtrlInteger(V4L2_CID_BRIGHTNESS, -32);
            break;
        case 'I':
            capture->SetCtrlInteger(V4L2_CID_BRIGHTNESS, 32);
            break;
        case 'j':
            capture->SetCtrlInteger(V4L2_CID_CONTRAST, 10);
            break;
        case 'J':
            capture->SetCtrlInteger(V4L2_CID_CONTRAST, 90);
            break;
        case 'k':
            capture->SetCtrlInteger(V4L2_CID_SATURATION, 10);
            break;
        case 'K':
            capture->SetCtrlInteger(V4L2_CID_SATURATION, 110);
            break;
        case 'l':
            capture->SetCtrlInteger(V4L2_CID_HUE, -110);
            break;
        case 'L':
            capture->SetCtrlInteger(V4L2_CID_HUE, 110);
            break;
        default:
            break;
    }
}

void OnRead(xiaotu::net::VideoCapture::BufferPtr const & buffer)
{
    gBuffer = buffer;
}

int main(int argc, char *argv[])
{
    if (2 != argc) {
        std::cout << "./t_stdin_v4l2_talk /dev/video0" << std::endl;
        exit(-1);
    }

    std::string dev_path(argv[1]);
    gBuffer.reset();

    xiaotu::net::VideoCapturePtr capture(new xiaotu::net::VideoCapture(dev_path));
    capture->SetCaptureCB(std::bind(&OnRead, std::placeholders::_1));
	//capture->SetImageSizeOrDie(720, 1280);
	capture->SetImageSizeOrDie(1080, 1920);
    capture->PrepareBufferOrDie(4);

    std::cout << "曝光模式:" << capture->GetExposureType() << std::endl;
    std::cout << "曝光时间:" << capture->GetCtrlInteger(V4L2_CID_EXPOSURE_ABSOLUTE) * 0.1 << "ms" << std::endl;
    std::cout << "帧率:" << capture->GetFrameRate() << std::endl;
    capture->SetCtrlInteger(V4L2_CID_EXPOSURE_ABSOLUTE, 100);
    capture->SetCtrlInteger(V4L2_CID_GAIN, 128);
    capture->SetCtrlInteger(V4L2_CID_BRIGHTNESS, 0);
    capture->SetFrameRate(30, 1);
    std::cout << "曝光时间:" << capture->GetCtrlInteger(V4L2_CID_EXPOSURE_ABSOLUTE) * 0.1 << "ms" << std::endl;
    std::cout << "帧率:" << capture->GetFrameRate() << std::endl;

    capture->SetExposureType(V4L2_EXPOSURE_MANUAL);

    capture->StartOrDie();


    xiaotu::net::ConnectionPtr stdin_conn = xiaotu::net:: ConnectionPtr(new xiaotu::net::Connection(0, "标准输入:stdin:0"));
    stdin_conn->SetRecvRawCallBk(std::bind(&OnStdinRawMsg, capture, std::placeholders::_1));

    xiaotu::net::PollLoopPtr gLoop = xiaotu::net::CreatePollLoop();
    xiaotu::net::ApplyOnLoop(capture, gLoop);
    xiaotu::net::ApplyOnLoop(stdin_conn, gLoop);

    while (1) {
        gLoop->LoopOnce(0);

        if (nullptr != gBuffer) {
            std::cout << "gBuffe id:" << gBuffer->id << std::endl;
            gBuffer.reset();
            capture->ReleaseBufferInUse();
        }
    }
    return 0;
}


