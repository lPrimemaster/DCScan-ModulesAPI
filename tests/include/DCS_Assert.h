#pragma once

#define _DCS_RET_ERROR(result, expected, c) std::cout << "Sub-test [at " << __FILE__ << ":" << __LINE__ <<"] failed...\n"\
	"Expected --> (" << #result << " " << #c << " " << expected << ")\n"\
	"Got      --> (" << #result << " == " << result << ")\n"

#define DCS_START_TEST int _dcs_ltest_c = 0; int _dcs__test_result = 0
#define DCS_RETURN_TEST if(_dcs__test_result) std::cout << "Failed a total of " << _dcs__test_result << " sub-test(s).\n"; return _dcs__test_result

/// \todo Add a syntax of type (when failed) -> Expected x, got y.
#define DCS_ASSERT_EQ(x, y)   _dcs_ltest_c = !(x == y); _dcs__test_result += _dcs_ltest_c; if(_dcs_ltest_c) _DCS_RET_ERROR(x, y, ==)
#define DCS_ASSERT_NEQ(x, y)  _dcs_ltest_c =  (x == y); _dcs__test_result += _dcs_ltest_c; if(_dcs_ltest_c) _DCS_RET_ERROR(x, y, !=)

#define DCS_ASSERT_LEQ(x, y)  _dcs_ltest_c = !(x <= y); _dcs__test_result += _dcs_ltest_c; if(_dcs_ltest_c) _DCS_RET_ERROR(x, y, <=)
#define DCS_ASSERT_HEQ(x, y)  _dcs_ltest_c = !(x => y); _dcs__test_result += _dcs_ltest_c; if(_dcs_ltest_c) _DCS_RET_ERROR(x, y, >=)

#define DCS_ASSERT_LOW(x, y)  _dcs_ltest_c = !(x < y); _dcs__test_result += _dcs_ltest_c; if(_dcs_ltest_c) _DCS_RET_ERROR(x, y, <)
#define DCS_ASSERT_HIGH(x, y) _dcs_ltest_c = !(x > y); _dcs__test_result += _dcs_ltest_c; if(_dcs_ltest_c) _DCS_RET_ERROR(x, y, >)
