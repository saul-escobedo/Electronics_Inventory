#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include <filesystem>

namespace ecim {

enum class BackupFrequency {
    Always,
    Daily,
    Weekly,
    Monthly,
    Never
};

struct Settings {
    std::string databaseDirectory;
    bool autoSave = true;
    BackupFrequency backupFrequency = BackupFrequency::Daily;
    double uiScaling = 1.0;
    std::string language = "en_US";
    std::string digikeyApiKey = "";
    std::string lcscApiKey = "";
    bool autoFetchPartData = false;
    std::map<std::string, std::string> customIntegrations; // name -> script path
};

class SettingsManager {
public:
    static SettingsManager& instance();

    bool load();
    bool save();

    const Settings& getSettings() const;
    void setSettings(const Settings& settings);
    void updateSettings(const Settings& settings);

    std::filesystem::path getConfigPath() const;
    void setConfigPath(const std::filesystem::path& path);

private:
    SettingsManager();
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    Settings m_settings;
    std::filesystem::path m_configPath;

    nlohmann::json settingsToJson() const;
    Settings jsonToSettings(const nlohmann::json& j) const;
};

} // namespace ecim
