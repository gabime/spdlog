#include <iostream>
#include "spdlog/business_logger.h"

using namespace spdlog;

int main() {
    try {
        std::cout << "业务日志管理器示例程序\n";
        std::cout << "========================\n\n";

        // 初始化业务日志管理器
        BusinessLogger::instance().init("business_logs");
        std::cout << "业务日志管理器初始化完成\n\n";

        // 示例1: 录制屏幕业务
        std::cout << "--- 录制屏幕业务日志 ---\n";
        BusinessLogger::instance().info(BusinessType::ScreenRecording, "开始录制屏幕");
        BusinessLogger::instance().debug(BusinessType::ScreenRecording, "录制参数: 分辨率 1920x1080, 帧率 30fps");
        BusinessLogger::instance().info(BusinessType::ScreenRecording, "正在录制中... 已录制 120秒");
        BusinessLogger::instance().warn(BusinessType::ScreenRecording, "存储空间即将不足");
        BusinessLogger::instance().info(BusinessType::ScreenRecording, "屏幕录制完成，文件保存为: screen_recording_20240101.mp4");

        // 示例2: 打开桌面业务
        std::cout << "\n--- 打开桌面业务日志 ---\n";
        BusinessLogger::instance().info(BusinessType::DesktopOpening, "尝试打开远程桌面连接");
        BusinessLogger::instance().debug(BusinessType::DesktopOpening, "目标地址: 192.168.1.100:3389");
        BusinessLogger::instance().info(BusinessType::DesktopOpening, "远程桌面连接成功");
        BusinessLogger::instance().info(BusinessType::DesktopOpening, "用户已登录桌面");

        // 示例3: 录制键盘业务
        std::cout << "\n--- 录制键盘业务日志 ---\n";
        BusinessLogger::instance().info(BusinessType::KeyboardRecording, "键盘录制已启动");
        BusinessLogger::instance().debug(BusinessType::KeyboardRecording, "按键: A");
        BusinessLogger::instance().debug(BusinessType::KeyboardRecording, "按键: B");
        BusinessLogger::instance().debug(BusinessType::KeyboardRecording, "按键: C");
        BusinessLogger::instance().error(BusinessType::KeyboardRecording, "检测到异常按键序列");
        BusinessLogger::instance().info(BusinessType::KeyboardRecording, "键盘录制已停止");

        // 示例4: 录制声音业务
        std::cout << "\n--- 录制声音业务日志 ---\n";
        BusinessLogger::instance().info(BusinessType::AudioRecording, "音频录制开始");
        BusinessLogger::instance().debug(BusinessType::AudioRecording, "音频采样率: 44100Hz, 声道: 立体声");
        BusinessLogger::instance().info(BusinessType::AudioRecording, "正在录制音频...");
        BusinessLogger::instance().info(BusinessType::AudioRecording, "音频录制完成，时长: 300秒");

        std::cout << "\n所有业务日志已写入完成！\n";
        std::cout << "请查看 business_logs 目录下的日志文件\n\n";

        // 关闭日志管理器
        BusinessLogger::instance().shutdown();
        std::cout << "业务日志管理器已关闭\n";

    } catch (const spdlog::spdlog_ex &ex) {
        std::cerr << "日志初始化失败: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
