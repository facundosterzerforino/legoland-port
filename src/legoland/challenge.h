#pragma once

#include "image_sprite.h"
#include "render.h"

struct LocFile;
struct AdvisorObject;

unsigned int RenderAdvisorIcon(struct AdvisorObject *param_1);
void UpdateAppraisalPageButtons(void);
void BuildTextureNodeFromImage(struct Image *param_1, struct TextureNode *param_2);
void SetNextAdvisorAnimState(unsigned int param_1, unsigned int param_2);
void ResetReportFlags(void);
void ResetAppraisalStreak(void);
void StartAppraisalTimer(void);
void StopAppraisalTimer(void);
void ConfigureAppraisal(unsigned int param_1, unsigned int param_2);
unsigned int GetLoadedTextureCount(void);
void LoadTextureBitmaps(struct LocFile *param_1, const char *param_2);
unsigned int FUN_004443b0(unsigned int param_1, unsigned int param_2);
unsigned int FUN_004443e0(unsigned int param_1, unsigned int param_2);
unsigned int FUN_00444410(unsigned int param_1, unsigned int param_2);
void FUN_00444440(unsigned int param_1, unsigned int param_2);
void SetReportCastleScoreGoal(unsigned int param_1, unsigned int param_2);
void SetReportDrivingSchoolScoreGoal(unsigned int param_1, unsigned int param_2);
void SetReportLogFlumeScoreGoal(unsigned int param_1, unsigned int param_2);
void SetReportBoatingSchoolScoreGoal(unsigned int param_1, unsigned int param_2);
void SetReportJungleCruiseScoreGoal(unsigned int param_1, unsigned int param_2);
void SetReportType1Or3ObjectCountGoal(unsigned int param_1, unsigned int param_2);
void SetReportType1Or3ClassCountGoal(unsigned int param_1, unsigned int param_2);
void SetReportLinkedObjectPercentGoal(unsigned int param_1, unsigned int param_2);
void SetReportType2ObjectCountGoal(unsigned int param_1, unsigned int param_2);
void SetReportType2ClassCountGoal(unsigned int param_1, unsigned int param_2);
void FUN_004446f0(unsigned int param_1, unsigned int param_2);
void SetReportFoodStallObjectCountGoal(unsigned int param_1, unsigned int param_2);
void SetReportFoodStallClassCountGoal(unsigned int param_1, unsigned int param_2);
void SetReportType4ObjectCountGoal(unsigned int param_1, unsigned int param_2);
void SetReportType4ClassCountGoal(unsigned int param_1, unsigned int param_2);
void SetReportVisitorCountGoal(unsigned int param_1, unsigned int param_2);
void SetReportVisitorHappinessGoal(unsigned int param_1, unsigned int param_2);
void SetReportVisitorFullnessGoal(unsigned int param_1, unsigned int param_2);
void SetReportPowerSupplyGoal(unsigned int param_1, unsigned int param_2);
void FUN_00444930(unsigned int param_1, unsigned int param_2);
void SetReportMapTileCountGoal(unsigned int param_1, unsigned int param_2);
void LoadAdvisorAnims(void);
void FreeAdvisorAnims(void);
int CheckAppraisalDue(void);
unsigned int SaveReport(void);
unsigned int LoadReport(void);
