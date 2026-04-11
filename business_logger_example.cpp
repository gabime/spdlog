#include "business_logger.h"
#include <iostream>

int main() {
    try {
        std::cout << "=== Business Logger System Demo ===\n\n";

        BusinessLogger logger("logs", 30 * 1024 * 1024, 3);
        logger.init();

        std::cout << "1. Recording screen business logs...\n";
        logger.info(BusinessType::RECORD_SCREEN, "Started screen recording");
        logger.info(BusinessType::RECORD_SCREEN, "Resolution: 1920x1080");
        logger.debug(BusinessType::RECORD_SCREEN, "Frame rate: 30 FPS");
        logger.warn(BusinessType::RECORD_SCREEN, "Low disk space, please clean up");

        std::cout << "\n2. Opening desktop business logs...\n";
        logger.info(BusinessType::OPEN_DESKTOP, "User logged in successfully");
        logger.info(BusinessType::OPEN_DESKTOP, "Desktop environment loaded");
        logger.debug(BusinessType::OPEN_DESKTOP, "Number of plugins loaded: 15");

        std::cout << "\n3. Recording keyboard business logs...\n";
        logger.info(BusinessType::RECORD_KEYBOARD, "Started keyboard recording");
        logger.debug(BusinessType::RECORD_KEYBOARD, "Key: Ctrl+C");
        logger.debug(BusinessType::RECORD_KEYBOARD, "Key: Ctrl+V");
        logger.error(BusinessType::RECORD_KEYBOARD, "Keyboard input timeout");

        std::cout << "\n4. Recording audio business logs...\n";
        logger.info(BusinessType::RECORD_AUDIO, "Started audio recording");
        logger.info(BusinessType::RECORD_AUDIO, "Sample rate: 44100 Hz");
        logger.debug(BusinessType::RECORD_AUDIO, "Bit rate: 128 kbps");
        logger.warn(BusinessType::RECORD_AUDIO, "Microphone volume too low");

        logger.flush_all();

        std::cout << "\n=== Logging completed, check the logs directory\n";

    } catch (const spdlog::spdlog_ex &ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
