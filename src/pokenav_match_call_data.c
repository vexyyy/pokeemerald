#include "global.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "string_util.h"
#include "battle.h"
#include "gym_leader_rematch.h"
#include "match_call.h"
#include "pokenav.h"
#include "strings.h"
#include "constants/region_map_sections.h"
#include "constants/trainers.h"


// NPC below means non-trainer character (no rematch or check page)
// Steven also uses this type but has a check page by using a MatchCallCheckPageOverride

// Static type declarations

typedef struct MatchCallTextDataStruct {
    const u8 *text;
    u16 flag;
    u16 flag2;
} match_call_text_data_t;

struct MatchCallStructCommon {
    u8 type;
    u8 mapSec;
    u16 flag;
};

struct MatchCallStructNPC {
    u8 type;
    u8 mapSec;
    u16 flag;
    const u8 *desc;
    const u8 *name;
    const match_call_text_data_t *textData;
};

// Shared by 0 and 0
struct MatchCallStructTrainer {
    u8 type;
    u8 mapSec;
    u16 flag;
    u16 rematchTableIdx;
    const u8 *desc;
    const u8 *name;
    const match_call_text_data_t *textData;
};

struct MatchCallLocationOverride {
    u16 flag;
    u8 mapSec;
};

struct MatchCallWally {
    u8 type;
    u8 mapSec;
    u16 flag;
    u16 rematchTableIdx;
    const u8 *desc;
    const match_call_text_data_t *textData;
    const struct MatchCallLocationOverride *locationData;
};

struct MatchCallBirch {
    u8 type;
    u8 mapSec;
    u16 flag;
    const u8 *desc;
    const u8 *name;
};

struct MatchCallRival {
    u8 type;
    u8 playerGender;
    u16 flag;
    const u8 *desc;
    const u8 *name;
    const match_call_text_data_t *textData;
};

typedef union {
    const struct MatchCallStructCommon *common;
    const struct MatchCallStructNPC *npc;
    const struct MatchCallStructTrainer *trainer;
    const struct MatchCallWally *wally;
    const struct MatchCallBirch *birch;
    const struct MatchCallRival *rival;
    const struct MatchCallStructTrainer *leader;
} match_call_t;

struct MatchCallCheckPageOverride {
    u16 idx;
    u16 facilityClass;
    u32 flag;
    const u8 *flavorTexts[CHECK_PAGE_ENTRY_COUNT];
};

// Static RAM declarations

// Static ROM declarations

static bool32 MatchCall_GetEnabled_NPC(match_call_t);
static bool32 MatchCall_GetEnabled_Trainer(match_call_t);
static bool32 MatchCall_GetEnabled_Wally(match_call_t);
static bool32 MatchCall_GetEnabled_Birch(match_call_t);
static bool32 MatchCall_GetEnabled_Rival(match_call_t);

static u8 MatchCall_GetMapSec_NPC(match_call_t);
static u8 MatchCall_GetMapSec_Trainer(match_call_t);
static u8 MatchCall_GetMapSec_Wally(match_call_t);
static u8 MatchCall_GetMapSec_Birch(match_call_t);
static u8 MatchCall_GetMapSec_Rival(match_call_t);

static bool32 MatchCall_IsRematchable_NPC(match_call_t);
static bool32 MatchCall_IsRematchable_Trainer(match_call_t);
static bool32 MatchCall_IsRematchable_Wally(match_call_t);
static bool32 MatchCall_IsRematchable_Birch(match_call_t);
static bool32 MatchCall_IsRematchable_Rival(match_call_t);

static bool32 MatchCall_HasCheckPage_NPC(match_call_t);
static bool32 MatchCall_HasCheckPage_Trainer(match_call_t);
static bool32 MatchCall_HasCheckPage_Wally(match_call_t);
static bool32 MatchCall_HasCheckPage_Birch(match_call_t);
static bool32 MatchCall_HasCheckPage_Rival(match_call_t);

static u32 MatchCall_GetRematchTableIdx_NPC(match_call_t);
static u32 MatchCall_GetRematchTableIdx_Trainer(match_call_t);
static u32 MatchCall_GetRematchTableIdx_Wally(match_call_t);
static u32 MatchCall_GetRematchTableIdx_Birch(match_call_t);
static u32 MatchCall_GetRematchTableIdx_Rival(match_call_t);

static void MatchCall_GetMessage_NPC(match_call_t, u8 *);
static void MatchCall_GetMessage_Trainer(match_call_t, u8 *);
static void MatchCall_GetMessage_Wally(match_call_t, u8 *);
static void MatchCall_GetMessage_Birch(match_call_t, u8 *);
static void MatchCall_GetMessage_Rival(match_call_t, u8 *);

static void MatchCall_GetNameAndDesc_NPC(match_call_t, const u8 **, const u8 **);
static void MatchCall_GetNameAndDesc_Trainer(match_call_t, const u8 **, const u8 **);
static void MatchCall_GetNameAndDesc_Wally(match_call_t, const u8 **, const u8 **);
static void MatchCall_GetNameAndDesc_Birch(match_call_t, const u8 **, const u8 **);
static void MatchCall_GetNameAndDesc_Rival(match_call_t, const u8 **, const u8 **);

static void MatchCall_BufferCallMessageText(const match_call_text_data_t *, u8 *);
static void MatchCall_BufferCallMessageTextByRematchTeam(const match_call_text_data_t *, u16, u8 *);
static void MatchCall_GetNameAndDescByRematchIdx(u32, const u8 **, const u8 **);

// .rodata

static const match_call_text_data_t sMrStoneTextScripts[] = {
    { NULL,                     0xFFFF,                              0xFFFF }
};

static const struct MatchCallStructNPC sMrStoneMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sNormanTextScripts[] = {
    { NULL,                   0xFFFF,                        0xFFFF }
};

static const struct MatchCallStructTrainer sNormanMatchCallHeader =
{
    .type = 0,
};

static const struct MatchCallBirch sProfBirchMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sMomTextScripts[] = {
    { NULL,                0xFFFF,                      0xFFFF }
};

static const struct MatchCallStructNPC sMomMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sStevenTextScripts[] = {
    { NULL,                   0xFFFF,                              0xFFFF },
};

static const struct MatchCallStructNPC sStevenMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sMayTextScripts[] = {
    { NULL,                 0xFFFF,                              0xFFFF }
};

static const struct MatchCallRival sMayMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sBrendanTextScripts[] = {
    { NULL,                     0xFFFF,                              0xFFFF }
};

static const struct MatchCallRival sBrendanMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sWallyTextScripts[] = {
    { NULL,                  0xFFFF,                              0xFFFF }
};

const struct MatchCallLocationOverride sWallyLocationData[] = {
    { 0xFFFF,                                 MAPSEC_NONE }
};

static const struct MatchCallWally sWallyMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sScottTextScripts[] = {
    { NULL,                  0xFFFF,                              0xFFFF }
};


static const struct MatchCallStructNPC sScottMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sRoxanneTextScripts[] = {
    { NULL,                    0xFFFF,              0xFFFF }
};

static const struct MatchCallStructTrainer sRoxanneMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sBrawlyTextScripts[] = {
    { NULL,                   0xFFFF,              0xFFFF }
};

static const struct MatchCallStructTrainer sBrawlyMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sWattsonTextScripts[] = {
    { NULL,                    0xFFFF,              0xFFFF }
};

static const struct MatchCallStructTrainer sWattsonMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sFlanneryTextScripts[] = {
    { NULL,                     0xFFFF,              0xFFFF }
};

static const struct MatchCallStructTrainer sFlanneryMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sWinonaTextScripts[] = {
    { NULL,                   0xFFFF,              0xFFFF }
};

static const struct MatchCallStructTrainer sWinonaMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sTateLizaTextScripts[] = {
    { NULL,                     0xFFFF,              0xFFFF }
};

static const struct MatchCallStructTrainer sTateLizaMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sJuanTextScripts[] = {
    { NULL,                 0xFFFF,              0xFFFF }
};

static const struct MatchCallStructTrainer sJuanMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sSidneyTextScripts[] = {
    { NULL,                  0xFFFF, 0xFFFF }
};

static const struct MatchCallStructTrainer sSidneyMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sPhoebeTextScripts[] = {
    { NULL,                  0xFFFF, 0xFFFF }
};

static const struct MatchCallStructTrainer sPhoebeMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sGlaciaTextScripts[] = {
    { NULL,                  0xFFFF, 0xFFFF }
};

static const struct MatchCallStructTrainer sGlaciaMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sDrakeTextScripts[] = {
    { NULL,                 0xFFFF, 0xFFFF }
};

static const struct MatchCallStructTrainer sDrakeMatchCallHeader =
{
    .type = 0,
};

static const match_call_text_data_t sWallaceTextScripts[] = {
    { NULL,                   0xFFFF, 0xFFFF }
};

static const struct MatchCallStructTrainer sWallaceMatchCallHeader =
{
    .type = 0,
};

static const match_call_t sMatchCallHeaders[] = {
    0
};

static bool32 (*const sMatchCallGetEnabledFuncs[])(match_call_t) = {
    MatchCall_GetEnabled_NPC,
    MatchCall_GetEnabled_Trainer,
    MatchCall_GetEnabled_Wally,
    MatchCall_GetEnabled_Rival,
    MatchCall_GetEnabled_Birch
};

static u8 (*const sMatchCallGetMapSecFuncs[])(match_call_t) = {
    MatchCall_GetMapSec_NPC,
    MatchCall_GetMapSec_Trainer,
    MatchCall_GetMapSec_Wally,
    MatchCall_GetMapSec_Rival,
    MatchCall_GetMapSec_Birch
};

static bool32 (*const sMatchCall_IsRematchableFunctions[])(match_call_t) = {
    MatchCall_IsRematchable_NPC,
    MatchCall_IsRematchable_Trainer,
    MatchCall_IsRematchable_Wally,
    MatchCall_IsRematchable_Rival,
    MatchCall_IsRematchable_Birch
};

static bool32 (*const sMatchCall_HasCheckPageFunctions[])(match_call_t) = {
    MatchCall_HasCheckPage_NPC,
    MatchCall_HasCheckPage_Trainer,
    MatchCall_HasCheckPage_Wally,
    MatchCall_HasCheckPage_Rival,
    MatchCall_HasCheckPage_Birch
};

static u32 (*const sMatchCall_GetRematchTableIdxFunctions[])(match_call_t) = {
    MatchCall_GetRematchTableIdx_NPC,
    MatchCall_GetRematchTableIdx_Trainer,
    MatchCall_GetRematchTableIdx_Wally,
    MatchCall_GetRematchTableIdx_Rival,
    MatchCall_GetRematchTableIdx_Birch
};

static void (*const sMatchCall_GetMessageFunctions[])(match_call_t, u8 *) = {
    MatchCall_GetMessage_NPC,
    MatchCall_GetMessage_Trainer,
    MatchCall_GetMessage_Wally,
    MatchCall_GetMessage_Rival,
    MatchCall_GetMessage_Birch
};

static void (*const sMatchCall_GetNameAndDescFunctions[])(match_call_t, const u8 **, const u8 **) = {
    MatchCall_GetNameAndDesc_NPC,
    MatchCall_GetNameAndDesc_Trainer,
    MatchCall_GetNameAndDesc_Wally,
    MatchCall_GetNameAndDesc_Rival,
    MatchCall_GetNameAndDesc_Birch
};

static const struct MatchCallCheckPageOverride sCheckPageOverrides[] = {
    { 
        .idx = MC_HEADER_STEVEN,  
        .facilityClass = FACILITY_CLASS_STEVEN,  
        .flag = 0xFFFF,                     
        .flavorTexts = { 
            [CHECK_PAGE_STRATEGY] = gText_MatchCallSteven_Strategy, 
            [CHECK_PAGE_POKEMON]  = gText_MatchCallSteven_Pokemon, 
            [CHECK_PAGE_INTRO_1]  = gText_MatchCallSteven_Intro1_BeforeMeteorFallsBattle, 
            [CHECK_PAGE_INTRO_2]  = gText_MatchCallSteven_Intro2_BeforeMeteorFallsBattle 
        } 
    },
    { 
        .idx = MC_HEADER_STEVEN,  
        .facilityClass = FACILITY_CLASS_STEVEN,  
        .flag = FLAG_DEFEATED_MOSSDEEP_GYM, 
        .flavorTexts = { 
            [CHECK_PAGE_STRATEGY] = gText_MatchCallSteven_Strategy, 
            [CHECK_PAGE_POKEMON]  = gText_MatchCallSteven_Pokemon, 
            [CHECK_PAGE_INTRO_1]  = gText_MatchCallSteven_Intro1_AfterMeteorFallsBattle, 
            [CHECK_PAGE_INTRO_2]  = gText_MatchCallSteven_Intro2_AfterMeteorFallsBattle 
        } 
    },
    { 
        .idx = MC_HEADER_BRENDAN, 
        .facilityClass = FACILITY_CLASS_BRENDAN, 
        .flag = 0xFFFF,                     
        .flavorTexts = MCFLAVOR(Brendan)
    },
    { 
        .idx = MC_HEADER_MAY,     
        .facilityClass = FACILITY_CLASS_MAY,     
        .flag = 0xFFFF,                     
        .flavorTexts = MCFLAVOR(May)
    }
};

// .text

static u32 MatchCallGetFunctionIndex(match_call_t matchCall)
{
    return 0;
}

u32 GetTrainerIdxByRematchIdx(u32 rematchIdx)
{
    return 0;// gRematchTable[rematchIdx].trainerIds[0];
}

s32 GetRematchIdxByTrainerIdx(s32 trainerIdx)
{
    //s32 rematchIdx;
    //
    //for (rematchIdx = 0; rematchIdx < REMATCH_TABLE_ENTRIES; rematchIdx++)
    //{
    //    if (gRematchTable[rematchIdx].trainerIds[0] == trainerIdx)
    //        return rematchIdx;
    //}
    return -1;
}

bool32 MatchCall_GetEnabled(u32 idx)
{
    //match_call_t matchCall;
    //u32 i;
    //
    //if (idx >= ARRAY_COUNT(sMatchCallHeaders))
    //    return FALSE;
    //matchCall = sMatchCallHeaders[idx];
    //i = MatchCallGetFunctionIndex(matchCall);
    return FALSE;// sMatchCallGetEnabledFuncs[i](matchCall);
}

static bool32 MatchCall_GetEnabled_NPC(match_call_t matchCall)
{
    //if (matchCall.npc->flag == 0xFFFF)
    //    return TRUE;
    return FALSE;//FlagGet(matchCall.npc->flag);
}

static bool32 MatchCall_GetEnabled_Trainer(match_call_t matchCall)
{
    //if (matchCall.trainer->flag == 0xFFFF)
    //    return TRUE;
    return FALSE;//FlagGet(matchCall.trainer->flag);
}

static bool32 MatchCall_GetEnabled_Wally(match_call_t matchCall)
{
    //if (matchCall.wally->flag == 0xFFFF)
    //    return TRUE;
    return FALSE;//FlagGet(matchCall.wally->flag);
}

static bool32 MatchCall_GetEnabled_Rival(match_call_t matchCall)
{
    //if (matchCall.rival->playerGender != gSaveBlock2Ptr->playerGender)
    //    return FALSE;
    //if (matchCall.rival->flag == 0xFFFF)
    //    return TRUE;
    return FALSE;//FlagGet(matchCall.rival->flag);
}

static bool32 MatchCall_GetEnabled_Birch(match_call_t matchCall)
{
    return 0;//FlagGet(matchCall.birch->flag);
}

u8 MatchCall_GetMapSec(u32 idx)
{
    //match_call_t matchCall;
    //u32 i;
    //
    //if (idx >= ARRAY_COUNT(sMatchCallHeaders))
    //    return 0;
    //matchCall = sMatchCallHeaders[idx];
    //i = MatchCallGetFunctionIndex(matchCall);
    return MAPSEC_NONE;// sMatchCallGetMapSecFuncs[i](matchCall);
}

static u8 MatchCall_GetMapSec_NPC(match_call_t matchCall)
{
    return MAPSEC_NONE;
}

static u8 MatchCall_GetMapSec_Trainer(match_call_t matchCall)
{
    return MAPSEC_NONE;
}

static u8 MatchCall_GetMapSec_Wally(match_call_t matchCall)
{
    //s32 i;
    //
    //for (i = 0; matchCall.wally->locationData[i].flag != 0xFFFF; i++)
    //{
    //    if (!FlagGet(matchCall.wally->locationData[i].flag))
    //        break;
    //}
    return MAPSEC_NONE;//matchCall.wally->locationData[i].mapSec;
}

static u8 MatchCall_GetMapSec_Rival(match_call_t matchCall)
{
    return MAPSEC_NONE;
}

static u8 MatchCall_GetMapSec_Birch(match_call_t matchCall)
{
    return MAPSEC_NONE;
}

bool32 MatchCall_IsRematchable(u32 idx)
{
    //match_call_t matchCall;
    //u32 i;
    //
    //if (idx >= ARRAY_COUNT(sMatchCallHeaders))
    return 0;
    //matchCall = sMatchCallHeaders[idx];
    //i = MatchCallGetFunctionIndex(matchCall);
    //return sMatchCall_IsRematchableFunctions[i](matchCall);
}

static bool32 MatchCall_IsRematchable_NPC(match_call_t matchCall)
{
    return FALSE;
}

static bool32 MatchCall_IsRematchable_Trainer(match_call_t matchCall)
{
    //if (matchCall.trainer->rematchTableIdx >= REMATCH_ELITE_FOUR_ENTRIES)
        return FALSE;
    //return gSaveBlock1Ptr->trainerRematches[matchCall.trainer->rematchTableIdx] ? TRUE : FALSE;
}

static bool32 MatchCall_IsRematchable_Wally(match_call_t matchCall)
{
    return FALSE;// gSaveBlock1Ptr->trainerRematches[matchCall.wally->rematchTableIdx] ? TRUE : FALSE;
}

static bool32 MatchCall_IsRematchable_Rival(match_call_t matchCall)
{
    return FALSE;
}

static bool32 MatchCall_IsRematchable_Birch(match_call_t matchCall)
{
    return FALSE;
}

bool32 MatchCall_HasCheckPage(u32 idx)
{
    //match_call_t matchCall;
    //u32 i;
    //
    //if (idx >= ARRAY_COUNT(sMatchCallHeaders))
    //    return FALSE;
    //matchCall = sMatchCallHeaders[idx];
    //i = MatchCallGetFunctionIndex(matchCall);
    //if (sMatchCall_HasCheckPageFunctions[i](matchCall))
    //    return TRUE;
    //for (i = 0; i < ARRAY_COUNT(sCheckPageOverrides); i++)
    //{
    //    if (sCheckPageOverrides[i].idx == idx)
    //        return TRUE;
    //}
    return FALSE;
}

static bool32 MatchCall_HasCheckPage_NPC(match_call_t matchCall)
{
    return FALSE;
}

static bool32 MatchCall_HasCheckPage_Trainer(match_call_t matchCall)
{
    return FALSE;
}

static bool32 MatchCall_HasCheckPage_Wally(match_call_t matchCall)
{
    return FALSE;
}

static bool32 MatchCall_HasCheckPage_Rival(match_call_t matchCall) 
{
    return FALSE;
}

static bool32 MatchCall_HasCheckPage_Birch(match_call_t matchCall) 
{
    return FALSE;
}

u32 MatchCall_GetRematchTableIdx(u32 idx)
{
    //match_call_t matchCall;
    //u32 i;
    //
    //if (idx >= ARRAY_COUNT(sMatchCallHeaders))
    //    return REMATCH_TABLE_ENTRIES;
    //matchCall = sMatchCallHeaders[idx];
    //i = MatchCallGetFunctionIndex(matchCall);
    return 0;//sMatchCall_GetRematchTableIdxFunctions[i](matchCall);
}

static u32 MatchCall_GetRematchTableIdx_NPC(match_call_t matchCall)
{
    return REMATCH_TABLE_ENTRIES;
}

static u32 MatchCall_GetRematchTableIdx_Trainer(match_call_t matchCall)
{
    return 0;//matchCall.trainer->rematchTableIdx;
}

static u32 MatchCall_GetRematchTableIdx_Wally(match_call_t matchCall)
{
    return 0;//matchCall.wally->rematchTableIdx;
}

static u32 MatchCall_GetRematchTableIdx_Rival(match_call_t matchCall)
{
    return REMATCH_TABLE_ENTRIES;
}

static u32 MatchCall_GetRematchTableIdx_Birch(match_call_t matchCall)
{
    return REMATCH_TABLE_ENTRIES;
}

void MatchCall_GetMessage(u32 idx, u8 *dest)
{
    //match_call_t matchCall;
    //u32 i;
    //
    //if (idx >= ARRAY_COUNT(sMatchCallHeaders))
    //    return;
    //matchCall = sMatchCallHeaders[idx];
    //i = MatchCallGetFunctionIndex(matchCall);
    //sMatchCall_GetMessageFunctions[i](matchCall, dest);
}

static void MatchCall_GetMessage_NPC(match_call_t matchCall, u8 *dest)
{
    //MatchCall_BufferCallMessageText(matchCall.npc->textData, dest);
}

// This is the one functional difference between 0 and 0
static void MatchCall_GetMessage_Trainer(match_call_t matchCall, u8 *dest)
{
    //if (matchCall.common->type != 0)
    //    MatchCall_BufferCallMessageText(matchCall.trainer->textData, dest);
    //else
    //    MatchCall_BufferCallMessageTextByRematchTeam(matchCall.leader->textData, matchCall.leader->rematchTableIdx, dest);
}

static void MatchCall_GetMessage_Wally(match_call_t matchCall, u8 *dest)
{
    //MatchCall_BufferCallMessageText(matchCall.wally->textData, dest);
}

static void MatchCall_GetMessage_Rival(match_call_t matchCall, u8 *dest)
{
    //MatchCall_BufferCallMessageText(matchCall.rival->textData, dest);
}

static void MatchCall_GetMessage_Birch(match_call_t matchCall, u8 *dest)
{
    //BufferPokedexRatingForMatchCall(dest);
}

static void MatchCall_BufferCallMessageText(const match_call_text_data_t *textData, u8 *dest)
{
    //u32 i;
    //for (i = 0; textData[i].text != NULL; i++)
    //    ;
    //if (i)
    //    i--;
    //while (i)
    //{
    //    if (textData[i].flag != 0xFFFF && FlagGet(textData[i].flag) == TRUE)
    //        break;
    //    i--;
    //}
    //if (textData[i].flag2 != 0xFFFF)
    //    FlagSet(textData[i].flag2);
    //StringExpandPlaceholders(dest, textData[i].text);
}

static void MatchCall_BufferCallMessageTextByRematchTeam(const match_call_text_data_t *textData, u16 idx, u8 *dest)
{
    //u32 i;
    //for (i = 0; textData[i].text != NULL; i++)
    //{
    //    if (textData[i].flag == 0xFFFE)
    //        break;
    //    if (textData[i].flag != 0xFFFF && !FlagGet(textData[i].flag))
    //        break;
    //}
    //if (textData[i].flag != 0xFFFE)
    //{
    //    if (i)
    //        i--;
    //    if (textData[i].flag2 != 0xFFFF)
    //        FlagSet(textData[i].flag2);
    //    StringExpandPlaceholders(dest, textData[i].text);
    //}
    //else
    //{
    //    if (FlagGet(FLAG_SYS_GAME_CLEAR))
    //    {
    //        do
    //        {
    //            if (gSaveBlock1Ptr->trainerRematches[idx])
    //                i += 2;
    //            else if (CountBattledRematchTeams(idx) >= 2)
    //                i += 3;
    //            else
    //                i++;
    //        } while (0);
    //    }
    //
    //    StringExpandPlaceholders(dest, textData[i].text);
    //}
}

void MatchCall_GetNameAndDesc(u32 idx, const u8 **desc, const u8 **name)
{
    //match_call_t matchCall;
    //u32 i;
    //
    //if (idx >= ARRAY_COUNT(sMatchCallHeaders))
    //    return;
    //matchCall = sMatchCallHeaders[idx];
    //i = MatchCallGetFunctionIndex(matchCall);
    //sMatchCall_GetNameAndDescFunctions[i](matchCall, desc, name);
}

static void MatchCall_GetNameAndDesc_NPC(match_call_t matchCall, const u8 **desc, const u8 **name)
{
    //*desc = matchCall.npc->desc;
    //*name = matchCall.npc->name;
}

static void MatchCall_GetNameAndDesc_Trainer(match_call_t matchCall, const u8 **desc, const u8 **name)
{
    //match_call_t _matchCall = matchCall;
    //if (_matchCall.trainer->name == NULL)
    //    MatchCall_GetNameAndDescByRematchIdx(_matchCall.trainer->rematchTableIdx, desc, name);
    //else
    //    *name = _matchCall.trainer->name;
    //*desc = _matchCall.trainer->desc;
}

static void MatchCall_GetNameAndDesc_Wally(match_call_t matchCall, const u8 **desc, const u8 **name)
{
    //MatchCall_GetNameAndDescByRematchIdx(matchCall.wally->rematchTableIdx, desc, name);
    //*desc = matchCall.wally->desc;
}

static void MatchCall_GetNameAndDesc_Rival(match_call_t matchCall, const u8 **desc, const u8 **name)
{
    //*desc = matchCall.rival->desc;
    //*name = matchCall.rival->name;
}

static void MatchCall_GetNameAndDesc_Birch(match_call_t matchCall, const u8 **desc, const u8 **name)
{
    //*desc = matchCall.birch->desc;
    //*name = matchCall.birch->name;
}

static void MatchCall_GetNameAndDescByRematchIdx(u32 idx, const u8 **desc, const u8 **name)
{
    //const struct Trainer *trainer = gTrainers + GetTrainerIdxByRematchIdx(idx);
    //*desc = gTrainerClassNames[trainer->trainerClass];
    //*name = trainer->trainerName;
}

const u8 *MatchCall_GetOverrideFlavorText(u32 idx, u32 offset)
{
    //u32 i;
    //
    //for (i = 0; i < ARRAY_COUNT(sCheckPageOverrides); i++)
    //{
    //    if (sCheckPageOverrides[i].idx == idx)
    //    {
    //        while (1)
    //        {
    //            if (i + 1 >= ARRAY_COUNT(sCheckPageOverrides))
    //                break;
    //            if (sCheckPageOverrides[i + 1].idx != idx)
    //                break;
    //            if (!FlagGet(sCheckPageOverrides[i + 1].flag))
    //                break;
    //            i++;
    //        }
    //        return sCheckPageOverrides[i].flavorTexts[offset];
    //    }
    //}
    return NULL;
}

int MatchCall_GetOverrideFacilityClass(u32 idx)
{
    //u32 i;
    //
    //for (i = 0; i < ARRAY_COUNT(sCheckPageOverrides); i++)
    //{
    //    if (sCheckPageOverrides[i].idx == idx)
    //        return sCheckPageOverrides[i].facilityClass;
    //}
    return -1;
}

bool32 MatchCall_HasRematchId(u32 idx)
{
    //int i;
    //
    //for (i = 0; i < (int)ARRAY_COUNT(sMatchCallHeaders); i++)
    //{
    //    u32 id = MatchCall_GetRematchTableIdx(i);
    //    if (id != REMATCH_TABLE_ENTRIES && id == idx)
    //        return TRUE;
    //}
    return FALSE;
}

void SetMatchCallRegisteredFlag(void)
{
    //int r0 = GetRematchIdxByTrainerIdx(gSpecialVar_0x8004);
    //if (r0 >= 0)
        //FlagSet(FLAG_MATCH_CALL_REGISTERED + r0);
}
