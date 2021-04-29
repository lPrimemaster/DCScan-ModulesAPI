#include "../../DCS_Core/include/internal.h"
#include "../include/DCS_Assert.h"

int test()
{
	DCS_START_TEST;
	
	DCS::DB::LoadDefaultDB();

    DCS::DB::LoadUsers();

    DCS::DB::AddUser({ "frank", "fw#37" });

    DCS_ASSERT_EQ(std::string(DCS::DB::GetUser("frank").u), "frank");
    DCS_ASSERT_EQ(std::string(DCS::DB::GetUser("frank").p), "fw#37");
    DCS_ASSERT_EQ(std::string(DCS::DB::GetUser("ErrorUser").u), "INVALID USER");

    DCS::DB::CloseDB();

	DCS_RETURN_TEST;
}

int main()
{
	return test();
}
