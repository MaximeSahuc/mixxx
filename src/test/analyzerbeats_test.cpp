#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "analyzer/analyzerbeats.h"
#include "analyzer/constants.h"
#include "analyzer/plugins/analyzerqueenmarybeats.h"
#include "track/beats.h"
#include "track/beatutils.h"

namespace {

const mixxx::audio::SampleRate kSampleRate = mixxx::audio::SampleRate(44100);

/// Creates a stereo signal with a short click on every beat.
std::vector<CSAMPLE> makeClickTrack(double bpm, int seconds) {
    const SINT frameCount = seconds * kSampleRate;
    std::vector<CSAMPLE> samples(frameCount * mixxx::kAnalysisChannels, 0);
    const double beatLengthFrames = 60.0 * kSampleRate / bpm;
    const SINT clickFrameCount = kSampleRate / 100; // 10 ms
    unsigned int noise = 1;
    for (double beatFrame = 0; beatFrame + clickFrameCount < frameCount;
            beatFrame += beatLengthFrames) {
        const SINT firstFrame = static_cast<SINT>(beatFrame);
        for (SINT i = 0; i < clickFrameCount; ++i) {
            // Decaying pseudo random noise
            noise = noise * 1103515245 + 12345;
            const CSAMPLE value = ((noise >> 16) / 32768.0f - 1.0f) *
                    std::exp(-5.0f * i / clickFrameCount);
            for (int channel = 0; channel < mixxx::kAnalysisChannels; ++channel) {
                samples[(firstFrame + i) * mixxx::kAnalysisChannels + channel] = value;
            }
        }
    }
    return samples;
}

mixxx::Bpm detectBpm(const std::vector<CSAMPLE>& samples,
        mixxx::Bpm minBpm,
        mixxx::Bpm maxBpm) {
    mixxx::AnalyzerQueenMaryBeats analyzer(minBpm, maxBpm);
    EXPECT_TRUE(analyzer.initialize(kSampleRate));
    const SINT sampleCount = static_cast<SINT>(samples.size());
    const SINT chunkSampleCount =
            mixxx::kAnalysisFramesPerChunk * mixxx::kAnalysisChannels;
    for (SINT i = 0; i < sampleCount; i += chunkSampleCount) {
        EXPECT_TRUE(analyzer.processSamples(&samples[i],
                std::min(chunkSampleCount, sampleCount - i)));
    }
    EXPECT_TRUE(analyzer.finalize());
    return BeatUtils::calculateBpm(analyzer.getBeats(), kSampleRate);
}

TEST(AnalyzerQueenMaryBeatsTest, BpmRange) {
    const auto samples = makeClickTrack(200.0, 30);
    // Only the tempo within the range is detected. The ranges exclude
    // 200 / 3 BPM, which also matches every click.
    EXPECT_NEAR(200.0,
            detectBpm(samples, mixxx::Bpm(130), mixxx::Bpm(260)).value(),
            0.5);
    EXPECT_NEAR(100.0,
            detectBpm(samples, mixxx::Bpm(70), mixxx::Bpm(140)).value(),
            0.5);
}

TEST(AnalyzerBeatsTest, ScaleBeatsIntoBpmRange) {
    const auto trackEndPosition = mixxx::audio::FramePos(180.0 * kSampleRate);
    const auto scaledBpm = [&](double bpm) {
        const auto pBeats = mixxx::Beats::fromConstTempo(
                kSampleRate, mixxx::audio::kStartFramePos, mixxx::Bpm(bpm));
        return AnalyzerBeats::scaleBeatsIntoBpmRange(pBeats,
                trackEndPosition,
                mixxx::Bpm(125),
                mixxx::Bpm(250))
                ->getBpmInRange(mixxx::audio::kStartFramePos, trackEndPosition)
                .value();
    };
    EXPECT_DOUBLE_EQ(190.0, scaledBpm(95.0));
    EXPECT_DOUBLE_EQ(240.0, scaledBpm(60.0));
    EXPECT_DOUBLE_EQ(150.0, scaledBpm(300.0));
    EXPECT_DOUBLE_EQ(200.0, scaledBpm(200.0));
    // The range borders are included
    EXPECT_DOUBLE_EQ(125.0, scaledBpm(125.0));
    EXPECT_DOUBLE_EQ(250.0, scaledBpm(250.0));
}

} // namespace
