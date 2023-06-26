#include "../../DCS_Core/include/internal.h"

int main()
{
    DCS::RDB::OpenDatabase();
    DCS::RDB::CreateTables();
    
    DCS::RDB::WriteVariableSys("Geometric_AngleOffsetC1", -120.637650 - 180, "Calibration offset for the crystal 1 engine encoder (deg).");
    DCS::RDB::WriteVariableSys("Geometric_AngleOffsetC2", - 90.526640 - 180, "Calibration offset for the crystal 2 engine encoder (deg).");

    // std::string output = DCS::RDB::ReadVariable("TestVar");
    // LOG_DEBUG("%s", output.c_str());

    // DCS::RDB::WriteVariableSys("TestVar", 1, "A test var descriptor.");

    // output = DCS::RDB::GetVariableDescriptor("TestVar");
    // LOG_DEBUG("%s", output.c_str());

    // output = DCS::RDB::ReadVariable("TestVar_Fail");
    // LOG_DEBUG("%s", output.c_str());

    // output = DCS::RDB::GetVariableDescriptor("TestVar_Fail");
    // LOG_DEBUG("%s", output.c_str());

    // DCS::RDB::DeleteVariableSys("TestVar");
    // DCS::RDB::DeleteVariableSys("TestVar_Fail");

    // DCS::RDB::AddUser("prime", "alfa77");

    // DCS::RDB::AddUser("prime2", "alfa78");

    // LOG_DEBUG("%s", DCS::RDB::GetUser("prime").u);
    // LOG_DEBUG("%s", DCS::RDB::GetUser("prime2").u);

    // DCS::RDB::RemoveUser("prime2");

    DCS::RDB::CloseDatabase();
    
    return 0;
}
