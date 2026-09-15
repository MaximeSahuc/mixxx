#pragma once

#include <QHash>
#include <QList>
#include <memory>

#include "analyzer/analyzer.h"
#include "analyzer/plugins/analyzerplugin.h"
#include "preferences/beatdetectionsettings.h"
#include "preferences/usersettings.h"

class AnalyzerBeats : public Analyzer {
  public:
    explicit AnalyzerBeats(
            UserSettingsPointer pConfig,
            bool enforceBpmDetection = false);
    ~AnalyzerBeats() override = default;

    static QList<mixxx::AnalyzerPluginInfo> availablePlugins();
    static mixxx::AnalyzerPluginInfo defaultPlugin();

    /// Doubles or halves the beats until their BPM is within the range
    /// [minBpm, maxBpm], which must span at least one octave. Returns the
    /// beats unchanged if they are within the range or can't be scaled.
    static mixxx::BeatsPointer scaleBeatsIntoBpmRange(
            mixxx::BeatsPointer pBeats,
            mixxx::audio::FramePos trackEndPosition,
            mixxx::Bpm minBpm,
            mixxx::Bpm maxBpm);

    bool initialize(const AnalyzerTrack& track,
            mixxx::audio::SampleRate sampleRate,
            SINT frameLength) override;
    bool processSamples(const CSAMPLE* pIn, SINT count) override;
    void storeResults(TrackPointer tio) override;
    void cleanup() override;

  private:
    bool shouldAnalyze(TrackPointer pTrack) const;
    static QHash<QString, QString> getExtraVersionInfo(
            const QString& pluginId,
            bool bPreferencesFastAnalysis,
            mixxx::Bpm minBpm,
            mixxx::Bpm maxBpm);

    BeatDetectionSettings m_bpmSettings;
    std::unique_ptr<mixxx::AnalyzerBeatsPlugin> m_pPlugin;
    const bool m_enforceBpmDetection;
    QString m_pluginId;
    bool m_bPreferencesReanalyzeOldBpm;
    bool m_bPreferencesReanalyzeImported;
    bool m_bPreferencesFixedTempo;
    bool m_bPreferencesFastAnalysis;
    // Both invalid if the BPM range is disabled
    mixxx::Bpm m_minBpm;
    mixxx::Bpm m_maxBpm;

    mixxx::audio::SampleRate m_sampleRate;
    SINT m_maxFramesToProcess;
    SINT m_currentFrame;
};
