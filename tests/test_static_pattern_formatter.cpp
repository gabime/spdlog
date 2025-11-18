#include "includes.h"
#include "test_sink.h"

using namespace spdlog_literals;

TEST_CASE("compiled flag types", "[static_pattern_formatter]") {
    // clang-format off
    constexpr auto incompletely_compiled = spdlog::details::compile_static_pattern<"abc%+%ndef%l%L%t%v%a%A%b%h%B%c%C%Y%D%x%m%d%H%I%M%S%e%f%F%E%p%r%R%T%X%z%P%^%$%@%s%g%#%!%%%u%i%o%O">();
    // clang-format on
    auto compiled = spdlog::details::init_delayed_formatters(incompletely_compiled);
    static_assert(
        std::is_same_v<
            std::decay_t<decltype(compiled)>,
            std::tuple<
                spdlog::details::static_aggregate_formatter<"abc">, spdlog::details::full_formatter,
                spdlog::details::name_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::static_aggregate_formatter<"def">,
                spdlog::details::level_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::short_level_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::t_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::v_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::a_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::A_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::b_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::b_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::B_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::c_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::C_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::Y_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::D_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::D_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::m_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::d_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::H_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::I_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::M_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::S_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::e_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::f_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::F_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::E_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::p_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::r_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::R_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::T_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::T_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::z_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::pid_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::color_start_formatter, spdlog::details::color_stop_formatter,
                spdlog::details::source_location_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::short_filename_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::source_filename_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::source_linenum_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::source_funcname_formatter<spdlog::details::null_scoped_padder>,
                spdlog::details::ch_formatter,
                spdlog::details::elapsed_formatter<spdlog::details::null_scoped_padder,
                                                   std::chrono::nanoseconds>,
                spdlog::details::elapsed_formatter<spdlog::details::null_scoped_padder,
                                                   std::chrono::microseconds>,
                spdlog::details::elapsed_formatter<spdlog::details::null_scoped_padder,
                                                   std::chrono::milliseconds>,
                spdlog::details::elapsed_formatter<spdlog::details::null_scoped_padder,
                                                   std::chrono::seconds>>>);
#ifndef SPDLOG_NO_TLS
    static_assert(std::is_same_v<
                  std::decay_t<decltype(spdlog::details::init_delayed_formatters(
                      spdlog::details::compile_static_pattern<"%&">()))>,
                  std::tuple<spdlog::details::mdc_formatter<spdlog::details::null_scoped_padder>>>);
#endif

    auto wrapped = "%+%n"_pat;
    static_assert(std::is_same_v<decltype(wrapped),
                                 std::vector<std::unique_ptr<spdlog::details::flag_formatter>>>);
}
