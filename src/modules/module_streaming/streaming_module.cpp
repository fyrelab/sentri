/*
  streaming_module.cpp
  Copyright 2016 fyrelab
 */

#include "modules/module_streaming/streaming_module.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>
#include <map>

#include "lib_module/module.h"

#define HEARTBEAT_INTERVAL 10000
#define MODULE_STREAMING_NAME "module_streaming"


Streaming::Streaming(const std::string &sending_queue, const std::string &receiving_queue)
                     : lib_module::Module(MODULE_STREAMING_NAME, sending_queue, receiving_queue) {
}

void Streaming::start() {
  sendHeartbeat();
  setHeartbeatInterval(HEARTBEAT_INTERVAL);

  std::map<std::string, std::string> *configurationData = getConfigLoader().getConfig();

  std::string input_framerate;
  std::string capture_device_video;
  std::string capture_device_audio;
  std::string quality;
  std::string path;
  std::string sound;

  try {
    input_framerate = configurationData->at("input_framerate");
    capture_device_video = configurationData->at("video");
    capture_device_audio = configurationData->at("audio");
    quality = configurationData->at("quality");
    path = lib_module::getTempPath() + "/stream/";
    configurationData->at("sound");
  } catch (const std::out_of_range &oor) {
    log(lib_module::LogSeverity::FATAL, "Invalid configuration: " + std::string(oor.what()));
    exit(EXIT_FAILURE);
  }

  std::string audio_out_rate = "64k";
  std::string profile = "baseline";
  std::string level = "1.3";
  std::string max_bitrate = "192k";
  std::string bufsize = "1M";
  std::string frame_rate = "10";
  std::string width = "320";
  std::string height = "180";

  if (quality == "2") {
    audio_out_rate = "64k";
    profile = "baseline";
    level = "2.1";
    max_bitrate = "500k";
    bufsize = "2M";
    frame_rate = "10";
    width = "480";
    height = "272";
  } else if (quality == "3") {
    audio_out_rate = "96k";
    profile = "baseline";
    level = "3.0";
    max_bitrate = "1500K";
    bufsize = "4M";
    frame_rate = "24";
    width = "640";
    height = "360";
  } else if (quality == "4") {
    audio_out_rate = "96k";
    profile = "baseline";
    level = "3.1";
    max_bitrate = "2500K";
    bufsize = "6M";
    frame_rate = "24";
    width = "1280";
    height = "720";
  }

  int status;

  pid_t pid = fork();

  /*
  https://trac.ffmpeg.org/wiki/Encode/H.264
  https://www.ffmpeg.org/ffmpeg-formats.html#toc-hls-1
  */

  if (pid == 0) {
    #ifdef __APPLE__
      if (sound == "true") {
        execlp(
          "ffmpeg", "ffmpeg", "-nostats", "-loglevel", "panic",
          "-f", "avfoundation", "-framerate", input_framerate.c_str(), "-i",
          (capture_device_video + ":"+ capture_device_audio).c_str(),
          "-c:a", "aac", "-ac", "2", "-b:a", audio_out_rate.c_str(), "-ar", "44100",
          "-c:v", "libx264", "-x264opts", "keyint=35:min-keyint=35:scenecut=-1",
          "-pix_fmt", "yuv420p", "-profile:v", profile.c_str(), "-level", level.c_str(),
          "-maxrate", max_bitrate.c_str(), "-bufsize", bufsize.c_str(), "-crf", "18", "-r", frame_rate.c_str(),
          "-f", "hls", "-hls_time", "2", "-hls_flags", "delete_segments",
          "-hls_list_size", "10", "-start_number", "0",
          "-s", (width + "x" + height).c_str(), (path + "stream.m3u8").c_str(), NULL);
      } else {
        execlp(
          "ffmpeg", "ffmpeg", "-nostats", "-loglevel", "panic",
          "-f", "avfoundation", "-framerate", input_framerate.c_str(), "-i",
          capture_device_video.c_str(),
          "-c:v", "libx264", "-x264opts", "keyint=35:min-keyint=35:scenecut=-1",
          "-pix_fmt", "yuv420p", "-profile:v", profile.c_str(), "-level", level.c_str(),
          "-maxrate", max_bitrate.c_str(), "-bufsize", bufsize.c_str(), "-crf", "18", "-r", frame_rate.c_str(),
          "-f", "hls", "-hls_time", "2", "-hls_flags", "delete_segments",
          "-hls_list_size", "10", "-start_number", "0",
          "-s", (width + "x" + height).c_str(), (path + "stream.m3u8").c_str(), NULL);
      }
    #elif __linux__
      if (sound == "true") {
        execlp("ffmpeg", "ffmpeg", "-nostats", "-loglevel", "panic",
          "-f", "pulse", "-ac", "1", "-i", capture_device_audio.c_str(),
          "-f", "video4linux2", "-framerate", input_framerate.c_str(), "-i",
          ("/dev/video" + capture_device_video).c_str(),
          "-c:a", "aac", "-ac", "1", "-b:a", audio_out_rate.c_str(), "-ar", "44100",
          "-c:v", "libx264", "-x264opts", "keyint=35:min-keyint=35:scenecut=-1",
          "-pix_fmt", "yuv420p", "-preset", "ultrafast", "-profile:v", profile.c_str(), "-level", level.c_str(),
          "-maxrate", max_bitrate.c_str(), "-bufsize", bufsize.c_str(), "-crf", "18", "-r", frame_rate.c_str(),
          "-f", "hls", "-hls_time", "2", "-hls_flags", "delete_segments",
          "-hls_list_size", "10", "-start_number", "0",
          "-s", (width + "x" + height).c_str(), (path + "stream.m3u8").c_str(), NULL);
      } else {
        execlp("ffmpeg", "ffmpeg", "-nostats", "-loglevel", "panic",
          "-f", "video4linux2", "-framerate", input_framerate.c_str(), "-i",
          ("/dev/video" + capture_device_video).c_str(),
          "-c:v", "libx264", "-x264opts", "keyint=35:min-keyint=35:scenecut=-1",
          "-pix_fmt", "yuv420p", "-preset", "ultrafast", "-profile:v", profile.c_str(), "-level", level.c_str(),
          "-maxrate", max_bitrate.c_str(), "-bufsize", bufsize.c_str(), "-crf", "18", "-r", frame_rate.c_str(),
          "-f", "hls", "-hls_time", "2", "-hls_flags", "delete_segments",
          "-hls_list_size", "10", "-start_number", "0",
          "-s", (width + "x" + height).c_str(), (path + "stream.m3u8").c_str(), NULL);
      }
    #endif
    exit(EXIT_FAILURE);
  }

  if (pid < 0) {
    log(lib_module::FATAL, "Cannot fork module_streaming");
    exit(EXIT_FAILURE);
  }

  while (true) {
    pid_t w = waitpid(pid, &status, WNOHANG);
    if (w != 0) {
      if (WIFEXITED(status)) {
        log(lib_module::FATAL, "Terminated with exit status: " + std::to_string(WEXITSTATUS(status)));
      } else {
          log(lib_module::FATAL, "Terminated without exit status.");
      }
      exit(EXIT_FAILURE);
    }
    sendHeartbeat();
    boost::this_thread::sleep_for(boost::chrono::milliseconds(HEARTBEAT_INTERVAL - 2000));
  }
}
