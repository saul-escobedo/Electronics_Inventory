#include "settings/Settings.hpp"

#include <gtest/gtest.h>
#include <iostream>

TEST(SettingsLogic, Settings) {
    auto& manager = ecim::SettingsManager::instance();

    // Get current settings (with defaults)
    const auto& settings = manager.getSettings();

    std::cout << "Settings configuration path: " << manager.getConfigPath() << std::endl;
    std::cout << "Database directory: " << settings.databaseDirectory << std::endl;
    std::cout << "Auto-save: " << (settings.autoSave ? "true" : "false") << std::endl;
    std::cout << "UI scaling: " << settings.uiScaling << std::endl;
    std::cout << "Language: " << settings.language << std::endl;

    // Save to generate the config file
    if (manager.save()) {
        std::cout << "Settings saved successfully to: " << manager.getConfigPath() << std::endl;
    } else {
        std::cout << "Failed to save settings" << std::endl;
    }
}
