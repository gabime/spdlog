#include <iostream>
#include "business_logger.h"

int main() {
    try {
        auto& logger = BusinessLogger::getInstance();

        logger.init("business_logs");

        std::cout << "业务日志系统初始化完成！" << std::endl;
        std::cout << "开始记录不同业务的日志..." << std::endl;

        logger.info(BusinessType::RECORD_SCREEN, "开始录制屏幕，分辨率: {}x{}", 1920, 1080);
        logger.info(BusinessType::RECORD_SCREEN, "正在录制第 {} 帧", 100);
        logger.warn(BusinessType::RECORD_SCREEN, "存储空间不足，仅剩 {} MB", 50);

        logger.info(BusinessType::OPEN_DESKTOP, "用户打开桌面");
        logger.debug(BusinessType::OPEN_DESKTOP, "桌面分辨率: {}x{}", 3840, 2160);

        logger.info(BusinessType::RECORD_KEYBOARD, "键盘录制开始");
        logger.debug(BusinessType::RECORD_KEYBOARD, "按下键码: {}", 65);
        logger.debug(BusinessType::RECORD_KEYBOARD, "释放键码: {}", 65);

        logger.info(BusinessType::RECORD_AUDIO, "音频录制开始，采样率: {} Hz", 44100);
        logger.debug(BusinessType::RECORD_AUDIO, "音频缓冲区已写入 {} 字节", 4096);

        for (int i = 0; i < 5; ++i) {
            logger.info(BusinessType::RECORD_SCREEN, "录制进度: {}%", (i + 1) * 20);
        }

        logger.info(BusinessType::RECORD_SCREEN, "屏幕录制完成");
        logger.info(BusinessType::RECORD_KEYBOARD, "键盘录制完成");
        logger.info(BusinessType::RECORD_AUDIO, "音频录制完成");

        std::cout << "日志记录完成！请查看 business_logs 目录下的日志文件。" << std::endl;

        spdlog::shutdown();

    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "日志初始化失败: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
