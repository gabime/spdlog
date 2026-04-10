#include "spdlog/spdlog.h"
#include "spdlog/business_logger.h"
#include <iostream>

int main() {
    try {
        std::cout << "=== Business Logger Example ===" << std::endl;
        std::cout << std::endl;

        spdlog::business::BusinessLogger::init();
        std::cout << "Business logger initialized" << std::endl;
        std::cout << std::endl;

        std::cout << "--- Testing Screen Recording ---" << std::endl;
        LOG_SCREEN_RECORDING_INFO("Started screen recording, resolution: 1920x1080, FPS: 30");
        LOG_SCREEN_RECORDING_DEBUG("Codec: H.264, Bitrate: 5000kbps");
        LOG_SCREEN_RECORDING_INFO("Recording has been running for {} seconds", 60);

        std::cout << std::endl;

        std::cout << "--- Testing Desktop Opening ---" << std::endl;
        LOG_DESKTOP_OPENING_INFO("User logged in, opening remote desktop");
        LOG_DESKTOP_OPENING_WARN("High network latency: {}ms", 150);
        LOG_DESKTOP_OPENING_INFO("Desktop successfully displayed");

        std::cout << std::endl;

        std::cout << "--- Testing Keyboard Recording ---" << std::endl;
        LOG_KEYBOARD_RECORDING_INFO("Keyboard recording started");
        LOG_KEYBOARD_RECORDING_DEBUG("Captured key: Ctrl + S");
        LOG_KEYBOARD_RECORDING_INFO("Recorded {} key events", 1250);

        std::cout << std::endl;

        std::cout << "--- Testing Audio Recording ---" << std::endl;
        LOG_AUDIO_RECORDING_INFO("Audio recording started, sample rate: 44100Hz");
        LOG_AUDIO_RECORDING_ERROR("Audio buffer overflow, lost {} frames", 5);
        LOG_AUDIO_RECORDING_INFO("Audio file size: {} MB", 25.5);

        std::cout << std::endl;
        std::cout << "=== Example completed ===" << std::endl;
        std::cout << "Log files saved to logs/ directory" << std::endl;

        spdlog::shutdown();
    }
    catch (const spdlog::spdlog_ex &ex) {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
