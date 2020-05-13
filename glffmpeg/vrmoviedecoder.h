#ifndef _MOVIE_DECODER_H_
#define _MOVIE_DECODER_H_
#include <list>
#include <map>
#include <mutex>
#include <vector>
#include <string>
using namespace std;

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
	//#include <sys/time.h>
}

typedef enum {

	vrMovieErrorNone,
	vrMovieErrorOpenFile,
	vrMovieErrorStreamInfoNotFound,
	vrMovieErrorStreamNotFound,
	vrMovieErrorCodecNotFound,
	vrMovieErrorOpenCodec,
	vrMovieErrorAllocateFrame,
	vrMovieErrorSetupScaler,
	vrMovieErrorReSampler,
	vrMovieErrorUnsupported,

} vrMovieError;

class vrmoviedecoder
{
public:
	vrmoviedecoder();
	~vrmoviedecoder();

public:
	// 打开路径指定的媒体文件，perror返回错误码
	bool vr_movie_decoder_open_file(const char* path, int* perror);
	bool vr_movie_decoder_open_screen(bool save);

	void vr_movie_decoder_init_avframe_rgb24();

	int vr_movie_decoder_width();
	int vr_movie_decoder_height();

	bool vr_movie_decoder_read_frame();

private:
	// 初始化ffmpeg context，打开path指定的媒体文件
	vrMovieError vr_movie_decoder_open_input(const char* path);

	// 
	vrMovieError vr_movie_decoder_open_video_stream();
	vrMovieError vr_movie_decoder_open_audio_stream();

	vrMovieError vr_movie_decoder_open_video_stream_by_idx();
	vrMovieError vr_movie_decoder_open_audio_stream_by_idx();

public:
	// 查找指定媒体类型的数据
	static unsigned int vr_movie_decoder_collect_streams_idx(AVFormatContext* fmt_ctx, AVMediaType codec_type);

private:

	AVFormatContext* fmt_ctx;
	AVStream*		 video_stream;
	AVCodecContext*  video_codec_ctx;
	AVCodec*         video_decoder;
	AVFrame*         av_frame;
	AVFrame*         gl_frame;
	AVPacket*        packet;
	struct SwsContext* sws_ctx;
	int              video_streams_idx;




	int              audio_streams_idx;

	char* _path;

	AVStream* audio_stream;
	AVCodecContext* audio_codec_ctx;
	AVCodec* audio_decoder;

	vector<unsigned int> _video_streams_idx;

	float _sampleRate;
	int   _frameWidth;
	int   _frameHeight;

	bool save_file;
	FILE* fp_rgb;
};

#endif // !_MOVIE_DECODER_H_

