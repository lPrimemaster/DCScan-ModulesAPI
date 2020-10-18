#pragma once

#define DCS_ASSERT_EQ(x, y)   !(x == y)
#define DCS_ASSERT_NEQ(x, y)   (x == y)

#define DCS_ASSERT_LEQ(x, y)  !(x <= y)
#define DCS_ASSERT_HEQ(x, y)  !(x >= y)

#define DCS_ASSERT_LOW(x, y)  !(x < y)
#define DCS_ASSERT_HIGH(x, y) !(x > y)
