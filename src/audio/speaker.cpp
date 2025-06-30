#include "speaker.h"

namespace audio {

Speaker::Speaker() {
    Pa_Initialize();
}

Speaker::~Speaker() {
    close();
    Pa_Terminate();
}

bool Speaker::open(int sampleRate, int channels, unsigned long framesPerBuffer) {
    PaStreamParameters outputParams;
    outputParams.device = Pa_GetDefaultOutputDevice();
    if (outputParams.device == paNoDevice) return false;
    outputParams.channelCount = channels;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    return Pa_OpenStream(&stream_, nullptr, &outputParams, sampleRate, framesPerBuffer, paNoFlag, nullptr, nullptr) == paNoError
        && Pa_StartStream(stream_) == paNoError;
}

void Speaker::play(const std::vector<float>& samples) {
    if (stream_) {
        Pa_WriteStream(stream_, samples.data(), samples.size());
    }
}

void Speaker::close() {
    if (stream_) {
        Pa_StopStream(stream_);
        Pa_CloseStream(stream_);
        stream_ = nullptr;
    }
}

} // namespace audio