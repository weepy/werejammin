#include "portaudio.h"
#include "opus/opus.h"
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

int main(void) {
    int opusErr;
    PaError paErr;
    std::string s;

    int const channels = 1;
    int const bufferSize = 120;
    int const sampleRate = 48000;
    int const durationSeconds = 50;

    opus_int32 enc_bytes;
    opus_int32 dec_bytes;
    int framesProcessed = 0;

    std::vector<unsigned short> captured(bufferSize * channels);
    std::vector<unsigned short> decoded(bufferSize * channels);
    // * 2: byte count, 16 bit samples
    std::vector<unsigned char> encoded(bufferSize * channels * 2);

    // initialize opus
    OpusEncoder* enc = opus_encoder_create(
        sampleRate, channels, OPUS_APPLICATION_RESTRICTED_LOWDELAY, &opusErr);

    opus_encoder_ctl(enc, OPUS_SET_BITRATE(64000));
    opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(10));
    opus_encoder_ctl(enc, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));

    if (opusErr != OPUS_OK)
    {
        std::cout << "opus_encoder_create failed: " << opusErr << "\n";
        std::getline(std::cin, s);
        return 1;
    }

    OpusDecoder* dec = opus_decoder_create(
        sampleRate, channels, &opusErr);
    if (opusErr != OPUS_OK)
    {
        std::cout << "opus_decoder_create failed: " << opusErr << "\n";
        std::getline(std::cin, s);
        return 1;
    }

    // initialize portaudio
    if ((paErr = Pa_Initialize()) != paNoError)
    {
        std::cout << "Pa_Initialize failed: " << Pa_GetErrorText(paErr) << "\n";
        std::getline(std::cin, s);
        return 1;
    }

    PaStream* stream = nullptr;
    if ((paErr = Pa_OpenDefaultStream(&stream,
        channels, channels, paInt16, sampleRate,
        bufferSize, nullptr, nullptr)) != paNoError)
    {
        std::cout << "Pa_OpenDefaultStream failed: " << Pa_GetErrorText(paErr) << "\n";
        std::getline(std::cin, s);
        return 1;
    }

    // start stream
    if ((paErr = Pa_StartStream(stream)) != paNoError) 
    {
        std::cout << "Pa_StartStream failed: " << Pa_GetErrorText(paErr) << "\n";
        std::getline(std::cin, s);
        return 1;
    }

    // capture, encode, decode & render durationSeconds of audio
    while (framesProcessed < sampleRate * durationSeconds)
    {
        if ((paErr = Pa_ReadStream(stream, 
            captured.data(), bufferSize)) != paNoError)
        {
            std::cout << "Pa_ReadStream failed: " << Pa_GetErrorText(paErr) << "\n";
            std::getline(std::cin, s);
            return 1;
        }

        if ((enc_bytes = opus_encode(enc, reinterpret_cast<opus_int16 const*>(
            captured.data()), 120, encoded.data(), encoded.size())) < 0)
        {
            std::cout << "opus_encode failed: " << enc_bytes << "\n";
            std::getline(std::cin, s);
            return 1;
        }

        if ((dec_bytes = opus_decode(dec, encoded.data(), enc_bytes,
            reinterpret_cast<opus_int16*>(decoded.data()), 120, 0)) < 0)
        {
            std::cout << "opus_decode failed: " << dec_bytes << "\n";
            std::getline(std::cin, s);
            return 1;
        }

        if ((paErr = Pa_WriteStream(stream, captured.data(), bufferSize)) != paNoError)
        {
            std::cout << "Pa_WriteStream failed: " << Pa_GetErrorText(paErr) << "\n";
            std::getline(std::cin, s);
            return 1;
        }

        framesProcessed += bufferSize;
    }

    // stop stream
    if ((paErr = Pa_StopStream(stream)) != paNoError)
    {
        std::cout << "Pa_StopStream failed: " << Pa_GetErrorText(paErr) << "\n";
        std::getline(std::cin, s);
        return 1;
    }

    // cleanup portaudio
    if ((paErr = Pa_CloseStream(stream)) != paNoError) 
    {
        std::cout << "Pa_CloseStream failed: " << Pa_GetErrorText(paErr) << "\n";
        std::getline(std::cin, s);
        return 1;
    }

    if ((paErr = Pa_Terminate()) != paNoError) 
    {
        std::cout << "Pa_Terminate failed: " << Pa_GetErrorText(paErr) << "\n";
        std::getline(std::cin, s);
        return 1;
    }

    // cleanup opus
    opus_decoder_destroy(dec);
    opus_encoder_destroy(enc);


}