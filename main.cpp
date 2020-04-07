// MyPlayer.cpp: 定义应用程序的入口点。
//

#include <iostream>
using namespace std;

#define _STDC_CONSTANT_MACROS
#define SDL_MAIN_HANDLED
extern "C" {
#include "SDL/SDL.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
};

// Refresh Event
#define SFM_REFRESH_EVENT (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT (SDL_USEREVENT + 2)

int thread_exit = 0;

int refresh_thread(void* opaque) {
  thread_exit = 0;
  while (!thread_exit) {
    SDL_Event event;
    event.type = SFM_REFRESH_EVENT;
    SDL_PushEvent(&event);
    SDL_Delay(40);
  }
  thread_exit = 0;
  // Break
  SDL_Event event;
  event.type = SFM_BREAK_EVENT;
  SDL_PushEvent(&event);

  return 0;
}

int main() {
  AVFormatContext* pFormatCtx = nullptr;
  int i = -1, videoIndex = -1;
  AVCodecParameters* parser = nullptr;
  AVCodecContext* pCodecCtx = nullptr;
  AVCodec* pCodec = nullptr;
  uint8_t* out_buffer = nullptr;
  AVFrame* pFrame = av_frame_alloc();
  AVPacket* packet;
  int ret;

  //输入文件路径
  char filepath[] = "D:/c++workspace/VideoPlayer/Titanic.ts";

  int frame_cnt;

  avformat_network_init();  //加载socket库以及网络加密协议相关的库，为后续使用网络相关提供支持
  pFormatCtx = avformat_alloc_context();  //封装格式上下文结构体

  // start init AVCodecContext
  if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) !=
      0) {  //打开输入视频文件
    printf("Couldn't open input stream.\n");
    return -1;
  }
  if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {  //获取视频文件信息
    printf("Couldn't find stream information.\n");
    return -1;
  }
  for (i = 0; i < pFormatCtx->nb_streams; i++)
    if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoIndex = i;
      break;
    }
  if (videoIndex == -1) {
    printf("Didn't find a audio stream.\n");
    return -1;
  }

  parser = pFormatCtx->streams[videoIndex]->codecpar;

  pCodec = avcodec_find_decoder(parser->codec_id);  //视频编解码器
  if (pCodec == NULL) {
    printf("Codec not found.\n");
    return -1;
  }
  pCodecCtx = avcodec_alloc_context3(pCodec);
  if (!pCodecCtx) {
    printf("Could not allocate video codec context\n");
    return -1;
  }
  if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {  //打开解码器
    printf("Could not open codec.\n");
    return -1;
  }
  // init AVCodecContext over

  // SDL start
  int screen_w, screen_h;
  SDL_Window* screen;
  SDL_Renderer* sdlRenderer;
  SDL_Texture* sdlTexture;
  SDL_Rect sdlRect;
  SDL_Thread* video_tid;
  SDL_Event event;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
    printf("Could not initialize SDL - %s\n", SDL_GetError());
    return -1;
  }
  screen_w = parser->width;
  screen_h = parser->height;
  cout << screen_w << "::" << screen_h << endl;
  screen = SDL_CreateWindow("Simplest ffmpeg player's Window",
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            screen_w, screen_h, SDL_WINDOW_OPENGL);
  if (!screen) {
    printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
    return -1;
  }
  sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
  sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV,
                                 SDL_TEXTUREACCESS_STREAMING, parser->width,
                                 parser->height);
  sdlRect.x = 0;
  sdlRect.y = 0;
  sdlRect.w = screen_w;
  sdlRect.h = screen_h;

  packet = (AVPacket*)av_malloc(sizeof(AVPacket));

  video_tid = SDL_CreateThread(refresh_thread, NULL, NULL);
  // SDL End

  while (1) {
    SDL_WaitEvent(&event);
    if (event.type == SFM_REFRESH_EVENT) {
      //只读取视频帧
      while (1) {
        if (av_read_frame(pFormatCtx, packet) < 0) {
          cout << "can't read packet" << endl;
          return -1;
        }
        if (packet->stream_index == videoIndex) break;
      }
      if (avcodec_send_packet(pCodecCtx, packet) < 0) {
        cout << "send packet error" << endl;
        return -1;
      }
      // TODO 前两帧有问题
      cout << avcodec_receive_frame(pCodecCtx, pFrame) << endl;

      // SDL---------------------------
      SDL_UpdateYUVTexture(sdlTexture, NULL, pFrame->data[0],
                           pFrame->linesize[0], pFrame->data[1],
                           pFrame->linesize[1], pFrame->data[2],
                           pFrame->linesize[2]);
      SDL_RenderClear(sdlRenderer);
      // SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );
      SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
      SDL_RenderPresent(sdlRenderer);
      // SDL End-----------------------

      av_packet_unref(packet);

    } else if (event.type == SDL_WINDOWEVENT) {
      SDL_GetWindowSize(screen, &screen_w, &screen_h);
    } else if (event.type == SDL_QUIT) {
      thread_exit = 1;
    } else if (event.type == SFM_BREAK_EVENT) {
      break;
    }
  }
}
