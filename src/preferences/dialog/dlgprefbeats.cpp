#include "preferences/dialog/dlgprefbeats.h"

#include "analyzer/analyzerbeats.h"
#include "defs_urls.h"
#include "moc_dlgprefbeats.cpp"
#include "util/math.h"

DlgPrefBeats::DlgPrefBeats(QWidget* parent, UserSettingsPointer pConfig)
        : DlgPreferencePage(parent),
          m_bpmSettings(pConfig),
          m_bAnalyzerEnabled(m_bpmSettings.getBpmDetectionEnabledDefault()),
          m_bFixedTempoEnabled(m_bpmSettings.getFixedTempoAssumptionDefault()),
          m_bFastAnalysisEnabled(m_bpmSettings.getFastAnalysisDefault()),
          m_bReanalyze(m_bpmSettings.getReanalyzeWhenSettingsChangeDefault()),
          m_bReanalyzeImported(m_bpmSettings.getReanalyzeImportedDefault()),
          m_bBpmRangeEnabled(m_bpmSettings.getBpmRangeEnabledDefault()),
          m_bpmRangeMin(m_bpmSettings.getBpmRangeMinDefault()),
          m_bpmRangeMax(m_bpmSettings.getBpmRangeMaxDefault()) {
    setupUi(this);

    // The maximum is at least double the minimum, see slotBpmRangeMinChanged()
    spinBoxBpmRangeMin->setRange(BeatDetectionSettings::kBpmRangeLowerLimit,
            BeatDetectionSettings::kBpmRangeUpperLimit / 2);
    spinBoxBpmRangeMax->setRange(BeatDetectionSettings::kBpmRangeLowerLimit * 2,
            BeatDetectionSettings::kBpmRangeUpperLimit);

    m_availablePlugins = AnalyzerBeats::availablePlugins();
    for (const auto& info : std::as_const(m_availablePlugins)) {
        comboBoxBeatPlugin->addItem(info.name(), info.id());
    }

    loadSettings();

    // Connections
    connect(comboBoxBeatPlugin,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefBeats::pluginSelected);
    connect(checkBoxAnalyzerEnabled,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefBeats::analyzerEnabled);
    connect(checkBoxFixedTempo,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefBeats::fixedtempoEnabled);
    connect(checkBoxFastAnalysis,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefBeats::fastAnalysisEnabled);
    connect(checkBoxReanalyze,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefBeats::slotReanalyzeChanged);
    connect(checkBoxReanalyzeImported,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefBeats::slotReanalyzeImportedChanged);
    connect(checkBoxBpmRange,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
#else
            &QCheckBox::stateChanged,
#endif
            this,
            &DlgPrefBeats::slotBpmRangeEnabledChanged);
    connect(spinBoxBpmRangeMin,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefBeats::slotBpmRangeMinChanged);
    connect(spinBoxBpmRangeMax,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &DlgPrefBeats::slotBpmRangeMaxChanged);

    setScrollSafeGuard(comboBoxBeatPlugin);
    setScrollSafeGuard(spinBoxBpmRangeMin);
    setScrollSafeGuard(spinBoxBpmRangeMax);
}

DlgPrefBeats::~DlgPrefBeats() {
}

QUrl DlgPrefBeats::helpUrl() const {
    return QUrl(MIXXX_MANUAL_BEATS_URL);
}

void DlgPrefBeats::loadSettings() {
    m_selectedAnalyzerId = m_bpmSettings.getBeatPluginId();
    m_bAnalyzerEnabled = m_bpmSettings.getBpmDetectionEnabled();
    m_bFixedTempoEnabled = m_bpmSettings.getFixedTempoAssumption();
    m_bReanalyze =  m_bpmSettings.getReanalyzeWhenSettingsChange();
    m_bReanalyzeImported = m_bpmSettings.getReanalyzeImported();
    m_bFastAnalysisEnabled = m_bpmSettings.getFastAnalysis();
    m_bBpmRangeEnabled = m_bpmSettings.getBpmRangeEnabled();
    m_bpmRangeMin = m_bpmSettings.getBpmRangeMin();
    m_bpmRangeMax = m_bpmSettings.getBpmRangeMax();

    slotUpdate();
}

void DlgPrefBeats::slotResetToDefaults() {
    if (m_availablePlugins.size() > 0) {
        m_selectedAnalyzerId = m_availablePlugins[0].id();
    }
    m_bAnalyzerEnabled = m_bpmSettings.getBpmDetectionEnabledDefault();
    m_bFixedTempoEnabled = m_bpmSettings.getFixedTempoAssumptionDefault();
    m_bFastAnalysisEnabled = m_bpmSettings.getFastAnalysisDefault();
    m_bReanalyze = m_bpmSettings.getReanalyzeWhenSettingsChangeDefault();
    m_bReanalyzeImported = m_bpmSettings.getReanalyzeImportedDefault();
    m_bBpmRangeEnabled = m_bpmSettings.getBpmRangeEnabledDefault();
    m_bpmRangeMin = m_bpmSettings.getBpmRangeMinDefault();
    m_bpmRangeMax = m_bpmSettings.getBpmRangeMaxDefault();

    slotUpdate();
}

void DlgPrefBeats::pluginSelected(int i) {
    if (i == -1) {
        return;
    }
    m_selectedAnalyzerId = m_availablePlugins[i].id();
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefBeats::analyzerEnabled(Qt::CheckState state) {
    m_bAnalyzerEnabled = (state == Qt::Checked);
#else
void DlgPrefBeats::analyzerEnabled(int i) {
    m_bAnalyzerEnabled = static_cast<bool>(i);
#endif
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefBeats::fixedtempoEnabled(Qt::CheckState state) {
    m_bFixedTempoEnabled = (state == Qt::Checked);
#else
void DlgPrefBeats::fixedtempoEnabled(int i) {
    m_bFixedTempoEnabled = static_cast<bool>(i);
#endif
    slotUpdate();
}

void DlgPrefBeats::slotUpdate() {
    checkBoxFixedTempo->setEnabled(m_bAnalyzerEnabled);
    comboBoxBeatPlugin->setEnabled(m_bAnalyzerEnabled);
    checkBoxAnalyzerEnabled->setChecked(m_bAnalyzerEnabled);
    // Fast analysis cannot be combined with non-constant tempo beatgrids.
    checkBoxFastAnalysis->setEnabled(m_bAnalyzerEnabled && m_bFixedTempoEnabled);
    checkBoxReanalyze->setEnabled(m_bAnalyzerEnabled);
    checkBoxReanalyzeImported->setEnabled(m_bAnalyzerEnabled);
    checkBoxBpmRange->setEnabled(m_bAnalyzerEnabled);
    spinBoxBpmRangeMin->setEnabled(m_bAnalyzerEnabled && m_bBpmRangeEnabled);
    spinBoxBpmRangeMax->setEnabled(m_bAnalyzerEnabled && m_bBpmRangeEnabled);

    if (!m_bAnalyzerEnabled) {
        return;
    }

    if (m_availablePlugins.size() > 0) {
        bool found = false;
        for (int i = 0; i < m_availablePlugins.size(); ++i) {
            const auto& info = m_availablePlugins.at(i);
            if (info.id() == m_selectedAnalyzerId) {
                found = true;
                comboBoxBeatPlugin->setCurrentIndex(i);
                if (!m_availablePlugins[i].isConstantTempoSupported()) {
                    checkBoxFixedTempo->setEnabled(false);
                }
                break;
            }
        }
        if (!found) {
            comboBoxBeatPlugin->setCurrentIndex(0);
            m_selectedAnalyzerId = m_availablePlugins[0].id();
        }
    }

    checkBoxFixedTempo->setChecked(m_bFixedTempoEnabled);
    // Fast analysis cannot be combined with non-constant tempo beatgrids.
    checkBoxFastAnalysis->setChecked(m_bFastAnalysisEnabled && m_bFixedTempoEnabled);

    checkBoxReanalyze->setChecked(m_bReanalyze);
    checkBoxReanalyzeImported->setChecked(m_bReanalyzeImported);
    checkBoxBpmRange->setChecked(m_bBpmRangeEnabled);
    spinBoxBpmRangeMin->setValue(m_bpmRangeMin);
    spinBoxBpmRangeMax->setValue(m_bpmRangeMax);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefBeats::slotBpmRangeEnabledChanged(Qt::CheckState state) {
    m_bBpmRangeEnabled = (state == Qt::Checked);
#else
void DlgPrefBeats::slotBpmRangeEnabledChanged(int value) {
    m_bBpmRangeEnabled = static_cast<bool>(value);
#endif
    slotUpdate();
}

void DlgPrefBeats::slotBpmRangeMinChanged(int value) {
    m_bpmRangeMin = value;
    // The range must span at least one octave, so that every tempo
    // can be doubled or halved into it.
    m_bpmRangeMax = math_max(m_bpmRangeMax, 2 * m_bpmRangeMin);
    slotUpdate();
}

void DlgPrefBeats::slotBpmRangeMaxChanged(int value) {
    m_bpmRangeMax = value;
    m_bpmRangeMin = math_min(m_bpmRangeMin, m_bpmRangeMax / 2);
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefBeats::slotReanalyzeChanged(Qt::CheckState state) {
    m_bReanalyze = (state == Qt::Checked);
#else
void DlgPrefBeats::slotReanalyzeChanged(int value) {
    m_bReanalyze = static_cast<bool>(value);
#endif
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefBeats::slotReanalyzeImportedChanged(Qt::CheckState state) {
    m_bReanalyzeImported = (state == Qt::Checked);
#else
void DlgPrefBeats::slotReanalyzeImportedChanged(int value) {
    m_bReanalyzeImported = static_cast<bool>(value);
#endif
    slotUpdate();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
void DlgPrefBeats::fastAnalysisEnabled(Qt::CheckState state) {
    m_bFastAnalysisEnabled = (state == Qt::Checked);
#else
void DlgPrefBeats::fastAnalysisEnabled(int i) {
    m_bFastAnalysisEnabled = static_cast<bool>(i);
#endif
    slotUpdate();
}

void DlgPrefBeats::slotApply() {
    m_bpmSettings.setBeatPluginId(m_selectedAnalyzerId);
    m_bpmSettings.setBpmDetectionEnabled(m_bAnalyzerEnabled);
    m_bpmSettings.setFixedTempoAssumption(m_bFixedTempoEnabled);
    m_bpmSettings.setReanalyzeWhenSettingsChange(m_bReanalyze);
    m_bpmSettings.setReanalyzeImported(m_bReanalyzeImported);
    m_bpmSettings.setFastAnalysis(m_bFastAnalysisEnabled);
    m_bpmSettings.setBpmRangeEnabled(m_bBpmRangeEnabled);
    m_bpmSettings.setBpmRangeMin(m_bpmRangeMin);
    m_bpmSettings.setBpmRangeMax(m_bpmRangeMax);
}
