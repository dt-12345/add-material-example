#include "lib.hpp"
#include "nn.hpp"

#include "binaryoffsethelper.h"

#include <cstring>

#include "nn/util/util_snprintf.hpp"

#define PRINT(...)                                                  \
{                                                                   \
    int len = nn::util::SNPrintf(buf, sizeof(buf), __VA_ARGS__);    \
    svcOutputDebugString(buf, len);                                 \
}

// game version (1.0.0 = 0 ... 1.2.1 = 5)
int gGameVersion = -1;

enum class PouchCategory {
    Weapon, Bow, Arrow, Shield, Armor, Material, Food, SpecialParts, KeyItem, Rupee, Grain, SpecialPower
};

enum class PouchTab {
    Armor, Bow, Shield, Weapon, Material, Food, SpecialParts, KeyItem, System
};

enum class ModifierType {
    None, AttackUp, AttackUpPlus, DurabilityUp, DurabilityUpPlus, FinishBlow, LongThrow, RapidFire, ThreeWayZoom, FiveWay, GuardUp, GuardUpPlus
};

class PouchMgr {
public:
    // skeleton to access members
    u8 _00[0x18c];
    PouchTab mCurrentTab;
    s32 mActiveIndices[9];
    s32 mActiveRowIndices[9];
};    

using AddToPouchFunction = bool (PouchMgr* self, const char** actor_name, const char** attachment_name, PouchCategory category,
                                    int count, bool set_is_get, int unk, bool is_equip, ModifierType modifier, int modifier_value,
                                    int life, int attachment_life, int attachment_extra_life, int record_extra_life, int* out_index, bool increment_ms_counter);
AddToPouchFunction* addToPouch = nullptr;

static constexpr u64 cConsumeMaterialOffsets[] = {
    0x01a2304c, // 1.0.0
    0x01a7b7dc, // 1.1.0
    0x01a790e8, // 1.1.1
    0x01a6fcc8, // 1.1.2
    0x01a618c4, // 1.2.0
    0x01a6cc4c // 1.2.1
};

static constexpr u64 cAddToPouchOffsets[] = {
    0x00c93dc0, // 1.0.0
    0x00cfbff0, // 1.1.0
    0x00c97a50, // 1.1.1
    0x00cf0f90, // 1.1.2
    0x00bf8b30, // 1.2.0
    0x00ce0eec // 1.2.1
};

// our hook to inject our new code
HOOK_DEFINE_INLINE(OnConsumeMaterial) {
    /*
        This is the function that will be called when the game reaches our hook
        Figuring out what is what at this point requires some RE so we'll just skip that for now
    */
    static void Callback(exl::hook::InlineCtx* ctx) {
        // x1 holds a pointer to the material name
        const char** item_name = reinterpret_cast<const char**>(ctx->X[1]);

        // if the material name is invalid, return
        if (item_name == nullptr || *item_name == nullptr)
            return;

        const char* material_to_add = "MagicP_00"; // material to add
        const char* attachment = ""; // materials don't have fuses so just leave this blank
        
        // x20 holds a pointer to the global PouchMgr instance
        PouchMgr* pouch_mgr = reinterpret_cast<PouchMgr*>(ctx->X[20]);

        if (strncmp(*item_name, "Item_Magic_05", sizeof("Item_Magic_05")) == 0) {
            addToPouch(pouch_mgr, &material_to_add, &attachment, PouchCategory::KeyItem, 100, true, 0, false, ModifierType::None, -1, -1, -1, -1, -1, nullptr, false);
        } else if (strncmp(*item_name, "Item_Magic_06", sizeof("Item_Magic_06")) == 0) {
            addToPouch(pouch_mgr, &material_to_add, &attachment, PouchCategory::KeyItem, 200, true, 0, false, ModifierType::None, -1, -1, -1, -1, -1, nullptr, false);
        } else if (strncmp(*item_name, "Item_Magic_08", sizeof("Item_Magic_08")) == 0) {
            addToPouch(pouch_mgr, &material_to_add, &attachment, PouchCategory::KeyItem, 400, true, 0, false, ModifierType::None, -1, -1, -1, -1, -1, nullptr, false);
        }
        pouch_mgr->mCurrentTab = PouchTab::Material;
    }
};

extern "C" void exl_main(void* x0, void* x1) {

    char buf[500];

    /* Setup hooking enviroment */
    exl::hook::Initialize();

    /* Determine game version */
    PRINT("Getting app version...");
    gGameVersion = InitializeAppVersion();
    if (gGameVersion == -1 || gGameVersion > 5) {
        PRINT("Error getting version");
        return;
    }
    PRINT("Version index %d", gGameVersion);

    /* Get the function to add to the inventory */
    addToPouch = reinterpret_cast<AddToPouchFunction*>(exl::util::modules::GetTargetOffset(cAddToPouchOffsets[gGameVersion]));

    /* Install our hook */
    OnConsumeMaterial::InstallAtOffset(cConsumeMaterialOffsets[gGameVersion]);

    return;
}

extern "C" NORETURN void exl_exception_entry() {
    /* TODO: exception handling */
    EXL_ABORT(0x420);
}