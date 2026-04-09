# spdlog异常恢复机制测试模块

## 任务完成总结

本测试模块为spdlog日志库新增了完整的异常恢复机制测试，按照需求实现了以下功能：

### 1. 异常场景测试覆盖

#### 1.1 磁盘空间不足 (ENOSPC)
- **模拟方法**: 模拟磁盘空间不足错误
- **预期行为**: 队列缓存，重试3次后抛出spdlog_ex异常
- **恢复机制**: 队列缓存，重试3次
- **测试文件**: `test_error_recovery.cpp` - `disk_space_recovery_enospace`
- **恢复时间**: ≤2秒（队列满后抛出异常）

#### 1.2 权限错误 (EACCES)  
- **模拟方法**: 模拟权限拒绝错误
- **预期行为**: 降级到用户目录
- **恢复机制**: 1秒内自动降级到用户目录
- **测试文件**: `test_error_recovery.cpp` - `permission_error_recovery_eacces`
- **恢复时间**: ≤1秒

#### 1.3 文件锁定 (EBUSY)
- **模拟方法**: 模拟文件被锁定错误
- **预期行为**: 等待超时后创建新文件
- **恢复机制**: 等待超时(30秒)后创建新文件
- **测试文件**: `test_error_recovery.cpp` - `file_lock_recovery_ebusy`
- **恢复时间**: 30秒超时（测试版本为3秒以加快测试）

#### 1.4 路径过长 (ENAMETOOLONG)
- **模拟方法**: 260+字符路径
- **预期行为**: 路径压缩
- **恢复机制**: 保留最后255字符
- **测试文件**: `test_error_recovery.cpp` - `path_too_long_recovery`

### 2. 恢复时间要求验证

#### 2.1 临时错误恢复
- **要求**: ≤5秒恢复
- **测试**: `recovery_time_requirements` - `temporary_error_recovery`
- **实现**: 包含重试延迟，确保在5秒内完成

#### 2.2 权限错误降级
- **要求**: ≤1秒降级
- **测试**: `recovery_time_requirements` - `permission_error_fallback`
- **实现**: 快速降级到用户目录

#### 2.3 磁盘满异常抛出
- **要求**: 队列满后 ≤2秒抛出异常
- **测试**: `disk_space_recovery_enospace`
- **实现**: 重试3次后快速抛出异常

### 3. 异步错误恢复

#### 3.1 异步日志错误处理
- **测试**: `async_error_recovery`
- **功能**: 验证异步日志记录中的错误恢复机制
- **实现**: 使用线程池和错误处理器

### 4. 技术实现细节

#### 4.1 错误模拟接收器
- **类名**: `error_simulating_sink`
- **继承**: `spdlog::sinks::base_sink<std::mutex>`
- **功能**: 模拟各种文件系统错误
- **方法**: 
  - `simulate_enospace_error()`: 模拟磁盘空间不足
  - `simulate_permission_error()`: 模拟权限错误
  - `simulate_file_lock_error()`: 模拟文件锁定
  - `simulate_path_too_long_error()`: 模拟路径过长

#### 4.2 恢复机制
- **队列缓存**: 磁盘空间不足时缓存日志并重试
- **路径降级**: 权限错误时降级到用户目录
- **超时处理**: 文件锁定时等待超时后创建新文件
- **路径压缩**: 路径过长时保留最后255字符

### 5. 测试框架集成

#### 5.1 Catch2测试框架
- 使用Catch2的`TEST_CASE`和`SECTION`宏
- 使用`REQUIRE`断言验证预期行为
- 支持异常捕获和超时验证

#### 5.2 CMake集成
- 更新`tests/CMakeLists.txt`
- 添加`test_error_recovery.cpp`到测试源文件列表
- 条件编译：仅在启用异常支持时包含

### 6. 文件结构

```
spdlog/tests/
├── test_error_recovery.cpp    # 主要的异常恢复测试文件
└── CMakeLists.txt           # 更新的构建配置
```

### 7. 测试运行

测试可以通过以下方式运行：
```bash
# 构建测试
cd build
cmake .. -DSPDLOG_BUILD_TESTS=ON
cmake --build . --target spdlog-utests

# 运行测试
./spdlog-utests "[error_recovery]"
```

### 8. 关键特性

1. **完整的错误模拟**: 覆盖所有要求的文件系统异常
2. **时间要求验证**: 严格测试恢复时间限制
3. **异步支持**: 支持异步日志记录的错误恢复
4. **平台兼容**: 支持Windows和Unix-like系统
5. **可配置**: 重试次数、超时时间可配置
6. **线程安全**: 使用互斥锁确保线程安全

这个测试模块为spdlog提供了全面的异常恢复机制验证，确保在各种文件系统异常情况下都能按照预期进行恢复。