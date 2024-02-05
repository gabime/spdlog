
// swift-tools-version: 5.8

import PackageDescription

let package = Package(
    name: "spdlog",
    platforms: [
        .macOS(.v11),
        .iOS(.v13)
    ],
    products: [
        .library(
            name: "spdlog",
            targets: ["spdlog"]),
    ],
    targets: [
        .target(name: "spdlog",
                path: "",
                exclude:[
                ],
                sources:[
                    "src"
                ],
                publicHeadersPath:"include",
                cSettings: [
                    .define("SPDLOG_COMPILED_LIB"),
                ]
               )
    ],
    cxxLanguageStandard: .cxx20
)
