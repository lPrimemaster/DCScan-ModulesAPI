#include "../../DCS_Core/include/internal.h"
#include "../include/DCS_Assert.h"

int test()
{
	DCS_START_TEST;
	
	DCS::DB::LoadDefaultDB();

    DCS::DB::LoadUsers();

    DCS::DB::AddUser("frank", "fw#37");

    DCS::u8 h[32];
    DCS::Auth::SHA256Str("fw#37", h);

    DCS_ASSERT_EQ(std::string(DCS::DB::GetUser("frank").u), "frank");

    for(int i = 0; i < 32; i++)
    {
        DCS_ASSERT_EQ(DCS::DB::GetUser("frank").p[i], h[i]);
    }
    
    DCS_ASSERT_EQ(std::string(DCS::DB::GetUser("ErrorUser").u), "INVALID_USER");

    DCS::DB::CloseDB();

	DCS_RETURN_TEST;
}

int main()
{
	return test();
}
