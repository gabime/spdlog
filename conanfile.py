from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy, rmdir
from conan.tools.microsoft import is_msvc_static_runtime
from conan.tools.scm import Version
import os

required_conan_version = ">=1.53.0"

# Inherited from https://github.com/conan-io/conan-center-index/blob/master/recipes/spdlog/all/conanfile.py
class TessonicsSpdlogConan(ConanFile):
    name = "tessonics-spdlog"
    version = "1.12.1"
    package_type = "library"
    description = "Fast C++ logging library"
    url = "https://github.com/tessonics/spdlog"
    topics = ("logger", "logging", "log-filtering", "file sink", "header-only")
    license = "MIT"
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "header_only": [True, False],
        "wchar_support": [True, False],
        "wchar_filenames": [True, False],
        "wchar_console": [True, False],
        "no_exceptions": [True, False],
        "use_std_fmt": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "header_only": False,
        "wchar_support": False,
        "wchar_filenames": False,
        "wchar_console": False,
        "no_exceptions": False,
        "use_std_fmt": False,
    }

    exports_sources = (
        # Directories
        ".github/*",
        "bench/*",
        "cmake/*",
        "example/*",
        "include/*",
        "logos/*",
        "scripts/*",
        "src/*",
        "tests/*",
        # Files
        ".clang-format",
        ".clang-tidy",
        ".gitattributes",
        ".gitignore",
        "CMakeLists.txt",
        "INSTALL",
        "LICENSE",
        "README.md",
    )
    no_copy_source = True

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
        else:
            del self.options.wchar_support
            del self.options.wchar_filenames
            del self.options.wchar_console
        if Version(self.version) < "1.10.0":
            del self.options.use_std_fmt
        if Version(self.version) < "1.15.0":
            self.options.rm_safe("wchar_console")

    def configure(self):
        if self.options.get_safe("shared") or self.options.header_only:
            self.options.rm_safe("fPIC")
        if self.options.header_only:
            self.options.rm_safe("shared")

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        if not self.options.get_safe("use_std_fmt"):
            self.requires(f"fmt/11.1.3", transitive_headers=True, transitive_libs=True)

    def package_id(self):
        if self.info.options.header_only:
            self.info.clear()

    @property
    def _std_fmt_compilers_minimum_version(self):
        return {
            "gcc": "13",
            "clang": "14",
            "apple-clang": "15",
            "Visual Studio": "16",
            "msvc": "192",
        }

    def validate(self):
        if self.options.get_safe("use_std_fmt"):
            check_min_cppstd(self, 20)
        else:
            check_min_cppstd(self, 11)
        if self.options.get_safe("shared") and is_msvc_static_runtime(self):
            raise ConanInvalidConfiguration("Visual Studio build for shared library with MT runtime is not supported")

        if self.options.get_safe("use_std_fmt"):
            compiler_name = str(self.settings.compiler)
            minimum_version = self._std_fmt_compilers_minimum_version.get(compiler_name, False)
            if not minimum_version:
                self.output.warning(f"{self.name} recipe lacks information about the {compiler_name} compiler support.")
            elif Version(self.settings.compiler.version) < minimum_version:
                raise ConanInvalidConfiguration(
                    f"{self.ref} using std::fmt requires std::fmt, which your compiler does not support."
                )

    # def source(self):
    #     get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        if not self.options.header_only:
            tc = CMakeToolchain(self)
            tc.cache_variables["SPDLOG_BUILD_EXAMPLE"] = False
            tc.cache_variables["SPDLOG_BUILD_EXAMPLE_HO"] = False
            tc.cache_variables["SPDLOG_BUILD_TESTS"] = False
            tc.cache_variables["SPDLOG_BUILD_TESTS_HO"] = False
            tc.cache_variables["SPDLOG_BUILD_BENCH"] = False
            if not self.options.get_safe("use_std_fmt"):
                fmt = self.dependencies["fmt"]
                tc.cache_variables["SPDLOG_FMT_EXTERNAL"] = not fmt.options.header_only
                tc.cache_variables["SPDLOG_FMT_EXTERNAL_HO"] = fmt.options.header_only
            tc.cache_variables["SPDLOG_BUILD_SHARED"] = not self.options.header_only and self.options.shared
            tc.cache_variables["SPDLOG_WCHAR_SUPPORT"] = self.options.get_safe("wchar_support", False)
            tc.cache_variables["SPDLOG_WCHAR_FILENAMES"] = self.options.get_safe("wchar_filenames", False)
            if Version(self.version) >= "1.15.0":
                tc.cache_variables["SPDLOG_WCHAR_CONSOLE"] = self.options.get_safe("wchar_console", False)
            tc.cache_variables["SPDLOG_INSTALL"] = True
            tc.cache_variables["SPDLOG_NO_EXCEPTIONS"] = self.options.no_exceptions
            tc.cache_variables["SPDLOG_USE_STD_FORMAT"] = self.options.get_safe("use_std_fmt")
            if self.settings.os in ("iOS", "tvOS", "watchOS"):
                tc.cache_variables["SPDLOG_NO_TLS"] = True
            tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0091"] = "NEW"
            tc.generate()
        cmake_deps = CMakeDeps(self)
        cmake_deps.generate()


    def build(self):
        if not self.options.header_only:
            cmake = CMake(self)
            cmake.configure()
            cmake.build()

    def package(self):
        copy(self, "LICENSE", dst=os.path.join(self.package_folder, "licenses"), src=self.source_folder)
        if self.options.header_only:
            copy(self,
                 src=os.path.join(self.source_folder, "include"),
                 pattern="*.h", dst=os.path.join(self.package_folder, "include"),
                 # Unvendor bundled dependencies https://github.com/gabime/spdlog/commit/18495bf25dad3a4e8c2fe3777a5f79acecde27e3
                 excludes=("spdlog/fmt/bundled/*"))
        else:
            cmake = CMake(self)
            cmake.install()
            rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))
            rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
            rmdir(self, os.path.join(self.package_folder, "lib", "spdlog", "cmake"))

    def package_info(self):
        target = "spdlog_header_only" if self.options.header_only else "spdlog"
        self.cpp_info.set_property("cmake_file_name", "spdlog")
        self.cpp_info.set_property("cmake_target_name", f"spdlog::{target}")
        self.cpp_info.set_property("pkg_config_name", "spdlog")

        # TODO: back to global scope in conan v2 once legacy generators removed
        self.cpp_info.components["libspdlog"].set_property("cmake_target_name", f"spdlog::{target}")
        if self.options.get_safe("use_std_fmt"):
            self.cpp_info.components["libspdlog"].defines.append("SPDLOG_USE_STD_FORMAT")
        else:
            self.cpp_info.components["libspdlog"].requires = ["fmt::fmt"]
            self.cpp_info.components["libspdlog"].defines.append("SPDLOG_FMT_EXTERNAL")

        if self.options.header_only:
            self.cpp_info.components["libspdlog"].libdirs = []
        else:
            suffix = "d" if self.settings.build_type == "Debug" else ""
            self.cpp_info.components["libspdlog"].libs = [f"spdlog{suffix}"]
            self.cpp_info.components["libspdlog"].defines.append("SPDLOG_COMPILED_LIB")
        if self.options.get_safe("wchar_support"):
            self.cpp_info.components["libspdlog"].defines.append("SPDLOG_WCHAR_TO_UTF8_SUPPORT")
        if self.options.get_safe("wchar_filenames"):
            self.cpp_info.components["libspdlog"].defines.append("SPDLOG_WCHAR_FILENAMES")
        if self.options.get_safe("wchar_console"):
            self.cpp_info.components["libspdlog"].defines.append("SPDLOG_UTF8_TO_WCHAR_CONSOLE")
        if self.options.no_exceptions:
            self.cpp_info.components["libspdlog"].defines.append("SPDLOG_NO_EXCEPTIONS")
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["libspdlog"].system_libs = ["pthread"]
        if self.options.header_only and self.settings.os in ("iOS", "tvOS", "watchOS"):
            self.cpp_info.components["libspdlog"].defines.append("SPDLOG_NO_TLS")
