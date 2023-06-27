#include "../../DCS_Core/include/internal.h"
#include "../../DCS_Core/include/DCS_ModuleCore.h"
#include "../../DCS_Utils/include/DCS_ModuleUtils.h"

#include <chrono>
#include <unordered_map>

int main()
{
    DCS::RDB::OpenDatabase();
    DCS::RDB::CreateTables();
    
    // DCS::RDB::WriteVariableSys("Geometric_AngleOffsetC1", 120.637650 + 180, "Calibration offset for the crystal 1 engine encoder (deg).");
    // DCS::RDB::WriteVariableSys("Geometric_AngleOffsetC2",  90.526640 + 180, "Calibration offset for the crystal 2 engine encoder (deg).");
    // DCS::RDB::WriteVariableSys("Geometric_AngleOffsetT",  120.0, "Calibration offset for the table angle (deg).");
    // DCS::RDB::WriteVariableSys("Geometric_AngleOffsetD",    0.0, "Calibration offset for the detector angle (deg).");

    DCS::RDB::CloseDatabase();

    // DCS::Utils::String string("split,me,on,char");
    // auto vec = string.split(',');
    // LOG_DEBUG("%d", vec.size());
    // for(int i = 0; i < vec.size(); i++)
    // {
    //     LOG_DEBUG("%s", vec[i].c_str());
    // }
    
    return 0;
}
