/* ************************************************************************
   SmarDTV

   Description :

   Historique :
   - COF   - Iwedia  - v 0    - 05/2005 - Creation
   - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
   ************************************************************************ */


#include <Tools/Tools.h>
#include <Tools/Xml.h>

#include "UNT.h"
#include "DESC.h"

/* ========================================================================
   Constructeur / Destructeur
   ======================================================================== */


dmsSSU_UNT_Data::dmsSSU_UNT_Data(dmsMPEG_Section* section) : dmsData()
{
    HDR_INIT(OUI,                  24);
    HDR_INIT(ProcessingOrder,      8);
    HDR_INIT(CommonDescriptorLoop, 0);
    HDR_INIT(Loop,                 0);

    section->TableId = 0x4B;
    section->m_iPID  = 0x0000;
    section->SetName("UNT_Section");
    section->SetName(&section->TableIdExtension, "ActionType_OUI_Hash");

    SetName("UNT_Data");

    CommonDescriptorLoop = new dmsSSU_UNT_DescriptorLoop("Common");
    Loop                 = new dmsSSU_UNT_ItemList();

    SetLoad(&OUI,                  false);
    SetLoad(&ProcessingOrder,      false);
    SetLoad(&CommonDescriptorLoop, true);
    SetLoad(&Loop,                 true);
}



dmsSSU_UNT_Item::dmsSSU_UNT_Item() : dmsData()
{
    HDR_INIT(CompatibilityDescriptor, 0);
    HDR_INIT(PlateformLoopLength,    16);
    HDR_INIT(PlateformInfo,           0);

    CompatibilityDescriptor = new dmsSSU_UNT_CompatibilityDescriptor();
    PlateformInfo           = new dmsSSU_UNT_PlateformInfoList();

    SetLoad(&CompatibilityDescriptor, true);
    SetLoad(&PlateformInfo,           true);

    SetLenLimit(&PlateformLoopLength, &PlateformInfo);
}




dmsSSU_UNT_CompatibilityDescriptor::dmsSSU_UNT_CompatibilityDescriptor() : dmsData()
{
    HDR_INIT(CompatibilityDescriptorLength, 16);
    HDR_INIT(DescriptorCount,               16);
    HDR_INIT(DescriptorList,                 0);

    DescriptorList = new dmsSSU_UNT_CompatibilityItemList();

    SetLoad(&DescriptorList, true);

    SetLenLimit(&CompatibilityDescriptorLength, &DescriptorList);
    SetListCount(&DescriptorCount, DescriptorList);
}



dmsSSU_UNT_CompatibilityItem::dmsSSU_UNT_CompatibilityItem() : dmsData()
{
    HDR_INIT(DescriptorType,     8);
    HDR_INIT(DescriptorLength,   8);
    HDR_INIT(SpecifierType,      8);
    HDR_INIT(SpecifierData,      24);
    HDR_INIT(Model,              16);
    HDR_INIT(Version,            16);
    HDR_INIT(SubDescriptorCount, 8);
    HDR_INIT(SubDescriptorList,  0);

    SubDescriptorList = new dmsSSU_UNT_DescriptorList();

    SetLoad(&DescriptorType,    false);
    SetLoad(&SpecifierType,     false);
    SetLoad(&SpecifierData,     false);
    SetLoad(&Model,             false);
    SetLoad(&Version,           false);
    SetLoad(&SubDescriptorList, true);

    SetLenLimit(&DescriptorLength, &SubDescriptorList);
    SetListCount(&SubDescriptorCount, SubDescriptorList);
}



dmsSSU_UNT_PlateformInfo::dmsSSU_UNT_PlateformInfo() : dmsData()
{
    HDR_INIT(TargetDescriptorLoop,      0);
    HDR_INIT(OperationalDescriptorLoop, 0);

    TargetDescriptorLoop      = new dmsSSU_UNT_DescriptorLoop("Target");
    OperationalDescriptorLoop = new dmsSSU_UNT_DescriptorLoop("Operational");

    SetLoad(&TargetDescriptorLoop,      "", true);
    SetLoad(&OperationalDescriptorLoop, "", true);
}




dmsSSU_UNT_DescriptorLoop::dmsSSU_UNT_DescriptorLoop(const wxString &name) : dmsData()
{
    HDR_INIT(Reserved,                   4);
    HDR_INIT(DescriptorLoopLength, 12);
    HDR_INIT(DescriptorList,       0);

    DescriptorList = new dmsSSU_UNT_DescriptorList();

    SetLoad(&DescriptorList, name+"DescriptorList", true);
    SetName(&DescriptorLoopLength, name+"DescriptorLoopLength");

    SetLenLimit(&DescriptorLoopLength, &DescriptorList);
}
