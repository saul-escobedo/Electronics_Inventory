#include "settings/Settings.hpp"
#include <fstream>
#include <iostream>
#include <cstdlib>

namespace ecim {

SettingsManager::SettingsManager() {
    const char* home = std::getenv("HOME");
    if (home) {
        m_configPath = std::filesystem::path(home) / ".config" / "ecim" / "settings.json";
    } else {
        m_configPath = "settings.json";
    }
    
    m_settings.databaseDirectory = (std::filesystem::path(home) / ".local" / "share" / "ecim" / "database").string();
}

SettingsManager& SettingsManager::instance() {
    static SettingsManager instance;
    return instance;
}

nlohmann::json SettingsManager::settingsToJson() const {
    nlohmann::json j;
    j["databaseDirectory"] = m_settings.databaseDirectory;
    j["autoSave"] = m_settings.autoSave;
    
    switch (m_settings.backupFrequency) {
        case BackupFrequency::Always: j["backupFrequency"] = "always"; break;
        case BackupFrequency::Daily: j["backupFrequency"] = "daily"; break;
        case BackupFrequency::Weekly: j["backupFrequency"] = "weekly"; break;
        case BackupFrequency::Monthly: j["backupFrequency"] = "monthly"; break;
        case BackupFrequency::Never: j["backupFrequency"] = "never"; break;
    }
    
    j["uiScaling"] = m_settings.uiScaling;
    j["language"] = m_settings.language;
    j["digikeyApiKey"] = m_settings.digikeyApiKey;
    j["lcscApiKey"] = m_settings.lcscApiKey;
    j["autoFetchPartData"] = m_settings.autoFetchPartData;
    j["customIntegrations"] = m_settings.customIntegrations;
    
    return j;
}

Settings SettingsManager::jsonToSettings(const nlohmann::json& j) const {
    Settings settings;
    
    if (j.contains("databaseDirectory")) {
        settings.databaseDirectory = j["databaseDirectory"].get<std::string>();
    }
    if (j.contains("autoSave")) {
        settings.autoSave = j["autoSave"].get<bool>();
    }
    if (j.contains("backupFrequency")) {
        std::string freq = j["backupFrequency"].get<std::string>();
        if (freq == "always") settings.backupFrequency = BackupFrequency::Always;
        else if (freq == "daily") settings.backupFrequency = BackupFrequency::Daily;
        else if (freq == "weekly") settings.backupFrequency = BackupFrequency::Weekly;
        else if (freq == "monthly") settings.backupFrequency = BackupFrequency::Monthly;
        else if (freq == "never") settings.backupFrequency = BackupFrequency::Never;
    }
    if (j.contains("uiScaling")) {
        settings.uiScaling = j["uiScaling"].get<double>();
    }
    if (j.contains("language")) {
        settings.language = j["language"].get<std::string>();
    }
    if (j.contains("digikeyApiKey")) {
        settings.digikeyApiKey = j["digikeyApiKey"].get<std::string>();
    }
    if (j.contains("lcscApiKey")) {
        settings.lcscApiKey = j["lcscApiKey"].get<std::string>();
    }
    if (j.contains("autoFetchPartData")) {
        settings.autoFetchPartData = j["autoFetchPartData"].get<bool>();
    }
    if (j.contains("customIntegrations")) {
        settings.customIntegrations = j["customIntegrations"].get<std::map<std::string, std::string>>();
    }
    
    return settings;
}

bool SettingsManager::load() {
    std::ifstream file(m_configPath);
    if (!file.is_open()) {
        return false;
    }
    
    try {
        nlohmann::json j;
        file >> j;
        m_settings = jsonToSettings(j);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading settings: " << e.what() << std::endl;
        return false;
    }
}

bool SettingsManager::save() {
    std::filesystem::create_directories(m_configPath.parent_path());
    
    std::ofstream file(m_configPath);
    if (!file.is_open()) {
        return false;
    }
    
    try {
        nlohmann::json j = settingsToJson();
        file << j.dump(4);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving settings: " << e.what() << std::endl;
        return false;
    }
}

const Settings& SettingsManager::getSettings() const {
    return m_settings;
}

void SettingsManager::setSettings(const Settings& settings) {
    m_settings = settings;
}

void SettingsManager::updateSettings(const Settings& settings) {
    m_settings = settings;
}

std::filesystem::path SettingsManager::getConfigPath() const {
    return m_configPath;
}

void SettingsManager::setConfigPath(const std::filesystem::path& path) {
    m_configPath = path;
}

} // namespace ecim
