#include <ultra64.h>
#include "constants.h"
#include "game/chraction.h"
#include "game/chrai.h"
#include "game/chraicommands.h"
#include "bss.h"
#include "lib/rng.h"
#include "lib/ailist.h"
#include "lib/model.h"
#include "data.h"
#include "types.h"

bool (*g_CommandPointers[])(void) = {
	/*0x0000*/ aiGoToNext,
	/*0x0001*/ aiGoToFirst,
	/*0x0002*/ aiLabel,
	/*0x0003*/ aiYield,
	/*0x0004*/ aiEndList,
	/*0x0005*/ aiSetList,
	/*0x0006*/ aiSetReturnList,
	/*0x0007*/ aiSetShotList,
	/*0x0008*/ aiReturn,
	/*0x0009*/ aiStop,
	/*0x000a*/ aiKneel,
	/*0x000b*/ aiChrDoAnimation,
	/*0x000c*/ aiIfIdle,
	/*0x000d*/ aiBeSurprisedOneHand,
	/*0x000e*/ aiBeSurprisedLookAround,
	/*0x000f*/ aiTrySidestep,
	/*0x0010*/ aiTryJumpOut,
	/*0x0011*/ aiTryRunSideways,
	/*0x0012*/ aiTryAttackWalk,
	/*0x0013*/ aiTryAttackRun,
	/*0x0014*/ aiTryAttackRoll,
	/*0x0015*/ aiTryAttackStand,
	/*0x0016*/ aiTryAttackKneel,
	/*0x0017*/ aiTryModifyAttack,
	/*0x0018*/ aiFaceEntity,
	/*0x0019*/ ai0019,
	/*0x001a*/ aiChrDamageChr,
	/*0x001b*/ aiConsiderGrenadeThrow,
	/*0x001c*/ aiDropItem,
	/*0x001d*/ aiJogToPad,
	/*0x001e*/ aiGoToPadPreset,
	/*0x001f*/ aiWalkToPad,
	/*0x0020*/ aiRunToPad,
	/*0x0021*/ aiSetPath,
	/*0x0022*/ aiStartPatrol,
	/*0x0023*/ aiIfPatrolling,
	/*0x0024*/ aiSurrender,
	/*0x0025*/ aiFadeOut,
	/*0x0026*/ aiRemoveChr,
	/*0x0027*/ aiTryStartAlarm,
	/*0x0028*/ aiActivateAlarm,
	/*0x0029*/ aiDeactivateAlarm,
	/*0x002a*/ aiTryRunFromTarget,
	/*0x002b*/ aiTryJogToTargetProp,
	/*0x002c*/ aiTryWalkToTargetProp,
	/*0x002d*/ aiTryRunToTargetProp,
	/*0x002e*/ aiTryGoToCoverProp,
	/*0x002f*/ aiTryJogToChr,
	/*0x0030*/ aiTryWalkToChr,
	/*0x0031*/ aiTryRunToChr,
	/*0x0032*/ aiIfStopped,
	/*0x0033*/ aiIfChrDead,
	/*0x0034*/ aiIfChrDeathAnimationFinished,
	/*0x0035*/ aiIfCanSeeTarget,
	/*0x0036*/ aiRandom,
	/*0x0037*/ aiIfRandomLessThan,
	/*0x0038*/ aiIfRandomGreaterThan,
	/*0x0039*/ aiIfCanHearAlarm,
	/*0x003a*/ aiIfAlarmActive,
	/*0x003b*/ aiIfGasActive,
	/*0x003c*/ aiIfHearsTarget,
	/*0x003d*/ aiIfSawInjury,
	/*0x003e*/ aiIfSawDeath,
	/*0x003f*/ aiIfLosToTarget,
	/*0x0040*/ aiIfTargetNearlyInSight,
	/*0x0041*/ aiIfNearlyInTargetsSight,
	/*0x0042*/ aiSetPadPresetToPadOnRouteToTarget,
	/*0x0043*/ aiIfSawTargetRecently,
	/*0x0044*/ aiIfHeardTargetRecently,
	/*0x0045*/ aiIfLosToChr,
	/*0x0046*/ aiIfNeverBeenOnScreen,
	/*0x0047*/ aiIfOnScreen,
	/*0x0048*/ aiIfChrInOnScreenRoom,
	/*0x0049*/ aiIfRoomIsOnScreen,
	/*0x004a*/ aiIfTargetAimingAtMe,
	/*0x004b*/ aiIfNearMiss,
	/*0x004c*/ aiIfSeesSuspiciousItem,
	/*0x004d*/ aiIfTargetInFovLeft,
	/*0x004e*/ aiIfCheckFovWithTarget,
	/*0x004f*/ aiIfTargetOutOfFovLeft,
	/*0x0050*/ aiIfTargetInFov,
	/*0x0051*/ aiIfTargetOutOfFov,
	/*0x0052*/ aiIfDistanceToTargetLessThan,
	/*0x0053*/ aiIfDistanceToTargetGreaterThan,
	/*0x0054*/ aiIfChrDistanceToPadLessThan,
	/*0x0055*/ aiIfChrDistanceToPadGreaterThan,
	/*0x0056*/ aiIfDistanceToChrLessThan,
	/*0x0057*/ aiIfDistanceToChrGreaterThan,
	/*0x0058*/ ai0058,
	/*0x0059*/ aiIfDistanceFromTargetToPadLessThan,
	/*0x005a*/ aiIfDistanceFromTargetToPadGreaterThan,
	/*0x005b*/ aiIfChrInRoom,
	/*0x005c*/ aiIfTargetInRoom,
	/*0x005d*/ aiIfChrHasObject,
	/*0x005e*/ aiIfWeaponThrown,
	/*0x005f*/ aiIfWeaponThrownOnObject,
	/*0x0060*/ aiIfChrHasWeaponEquipped,
	/*0x0061*/ aiIfGunUnclaimed,
	/*0x0062*/ aiIfObjectHealthy,
	/*0x0063*/ aiIfChrActivatedObject,
	/*0x0064*/ NULL,
	/*0x0065*/ aiObjInteract,
	/*0x0066*/ aiDestroyObject,
	/*0x0067*/ ai0067,
	/*0x0068*/ aiChrDropItems,
	/*0x0069*/ aiChrDropWeapon,
	/*0x006a*/ aiGiveObjectToChr,
	/*0x006b*/ aiObjectMoveToPad,
	/*0x006c*/ aiOpenDoor,
	/*0x006d*/ aiCloseDoor,
	/*0x006e*/ aiIfDoorState,
	/*0x006f*/ aiIfObjectIsDoor,
	/*0x0070*/ aiLockDoor,
	/*0x0071*/ aiUnlockDoor,
	/*0x0072*/ aiIfDoorLocked,
	/*0x0073*/ aiIfObjectiveComplete,
	/*0x0074*/ aiIfObjectiveFailed,
	/*0x0075*/ ai0075,
	/*0x0076*/ aiSetPadPresetToTargetQuadrant,
	/*0x0077*/ aiIfDifficultyLessThan,
	/*0x0078*/ aiIfDifficultyGreaterThan,
	/*0x0079*/ aiIfStageTimerLessThan,
	/*0x007a*/ aiIfStageTimerGreaterThan,
	/*0x007b*/ aiIfStageIdLessThan,
	/*0x007c*/ aiIfStageIdGreaterThan,
	/*0x007d*/ aiIfNumArghsLessThan,
	/*0x007e*/ aiIfNumArghsGreaterThan,
	/*0x007f*/ aiIfNumCloseArghsLessThan,
	/*0x0080*/ aiIfNumCloseArghsGreaterThan,
	/*0x0081*/ aiIfChrHealthGreaterThan,
	/*0x0082*/ aiIfChrHealthLessThan,
	/*0x0083*/ aiIfInjured,
	/*0x0084*/ aiSetMorale,
	/*0x0085*/ aiAddMorale,
	/*0x0086*/ aiChrAddMorale,
	/*0x0087*/ aiSubtractMorale,
	/*0x0088*/ aiIfMoraleLessThan,
	/*0x0089*/ aiIfMoraleLessThanRandom,
	/*0x008a*/ aiSetAlertness,
	/*0x008b*/ aiAddAlertness,
	/*0x008c*/ aiChrAddAlertness,
	/*0x008d*/ aiSubtractAlertness,
	/*0x008e*/ aiIfAlertness,
	/*0x008f*/ aiIfChrAlertnessLessThan,
	/*0x0090*/ aiIfAlertnessLessThanRandom,
	/*0x0091*/ aiNoOp0091,
	/*0x0092*/ aiSetHearDistance,
	/*0x0093*/ aiSetViewDistance,
	/*0x0094*/ aiSetGrenadeProbability,
	/*0x0095*/ aiSetChrNum,
	/*0x0096*/ aiSetMaxDamage,
	/*0x0097*/ aiAddHealth,
	/*0x0098*/ aiSetReactionSpeed,
	/*0x0099*/ aiSetRecoverySpeed,
	/*0x009a*/ aiSetAccuracy,
	/*0x009b*/ aiSetFlag,
	/*0x009c*/ aiUnsetFlag,
	/*0x009d*/ aiIfHasFlag,
	/*0x009e*/ aiChrSetFlag,
	/*0x009f*/ aiChrUnsetFlag,
	/*0x00a0*/ aiIfChrHasFlag,
	/*0x00a1*/ aiSetStageFlag,
	/*0x00a2*/ aiUnsetStageFlag,
	/*0x00a3*/ aiIfStageFlagEq,
	/*0x00a4*/ aiSetChrflag,
	/*0x00a5*/ aiUnsetChrflag,
	/*0x00a6*/ aiIfHasChrflag,
	/*0x00a7*/ aiChrSetChrflag,
	/*0x00a8*/ aiChrUnsetChrflag,
	/*0x00a9*/ aiIfChrHasChrflag,
	/*0x00aa*/ aiSetObjFlag,
	/*0x00ab*/ aiUnsetObjFlag,
	/*0x00ac*/ aiIfObjHasFlag,
	/*0x00ad*/ aiSetObjFlag2,
	/*0x00ae*/ aiUnsetObjFlag2,
	/*0x00af*/ aiIfObjHasFlag2,
	/*0x00b0*/ aiSetChrPreset,
	/*0x00b1*/ aiSetChrTarget,
	/*0x00b2*/ aiSetPadPreset,
	/*0x00b3*/ aiChrSetPadPreset,
	/*0x00b4*/ aiChrCopyPadPreset,
	/*0x00b5*/ aiPrint,
	/*0x00b6*/ aiRestartTimer,
	/*0x00b7*/ aiResetTimer,
	/*0x00b8*/ aiPauseTimer,
	/*0x00b9*/ aiResumeTimer,
	/*0x00ba*/ aiIfTimerStopped,
	/*0x00bb*/ aiIfTimerGreaterThanRandom,
	/*0x00bc*/ aiIfTimerLessThan,
	/*0x00bd*/ aiIfTimerGreaterThan,
	/*0x00be*/ aiShowCountdownTimer,
	/*0x00bf*/ aiHideCountdownTimer,
	/*0x00c0*/ aiSetCountdownTimerValue,
	/*0x00c1*/ aiStopCountdownTimer,
	/*0x00c2*/ aiStartCountdownTimer,
	/*0x00c3*/ aiIfCountdownTimerStopped,
	/*0x00c4*/ aiIfCountdownTimerLessThan,
	/*0x00c5*/ aiIfCountdownTimerGreaterThan,
	/*0x00c6*/ aiSpawnChrAtPad,
	/*0x00c7*/ aiSpawnChrAtChr,
	/*0x00c8*/ aiTryEquipWeapon,
	/*0x00c9*/ aiTryEquipHat,
	/*0x00ca*/ aiDuplicateChr,
	/*0x00cb*/ aiShowHudmsg,
	/*0x00cc*/ aiShowHudmsgTopMiddle,
	/*0x00cd*/ aiSpeak,
	/*0x00ce*/ aiPlaySound,
	/*0x00cf*/ aiSetObjectSoundPlaying,
	/*0x00d0*/ aiPlayRepeatingSoundFromPad,
	/*0x00d1*/ aiSetObjectSoundVolume,
	/*0x00d2*/ aiSetObjectSoundVolumeByDistance,
	/*0x00d3*/ aiAudioMuteChannel,
	/*0x00d4*/ aiIfObjectSoundVolumeLessThan,
	/*0x00d5*/ aiHovercarBeginPath,
	/*0x00d6*/ aiSetVehicleSpeed,
	/*0x00d7*/ aiSetRotorSpeed,
	/*0x00d8*/ aiNoOp00d8,
	/*0x00d9*/ aiNoOp00d9,
	/*0x00da*/ aiSetObjImage,
	/*0x00db*/ aiNoOp00db,
	/*0x00dc*/ aiEndLevel,
	/*0x00dd*/ ai00dd,
	/*0x00de*/ aiWarpJoToPad,
	/*0x00df*/ ai00df,
	/*0x00e0*/ aiRevokeControl,
	/*0x00e1*/ aiGrantControl,
	/*0x00e2*/ aiChrMoveToPad,
	/*0x00e3*/ ai00e3,
	/*0x00e4*/ ai00e4,
	/*0x00e5*/ aiIfColourFadeComplete,
	/*0x00e6*/ NULL,
	/*0x00e7*/ NULL,
	/*0x00e8*/ aiSetDoorOpen,
	/*0x00e9*/ ai00e9,
	/*0x00ea*/ aiIfNumPlayersLessThan,
	/*0x00eb*/ aiIfChrAmmoQuantityLessThan,
	/*0x00ec*/ aiChrDrawWeapon,
	/*0x00ed*/ aiChrDrawWeaponInCutscene,
	/*0x00ee*/ ai00ee,
	/*0x00ef*/ aiIfObjInRoom,
	/*0x00f0*/ ai00f0,
	/*0x00f1*/ aiIfAttacking,
	/*0x00f2*/ aiSwitchToAltSky,
	/*0x00f3*/ aiChrSetInvincible,
	/*0x00f4*/ ai00f4,
	/*0x00f5*/ ai00f5,
	/*0x00f6*/ ai00f6,
	/*0x00f7*/ aiIfAllObjectivesComplete,
	/*0x00f8*/ aiIfPlayerIsInvincible,
	/*0x00f9*/ aiPlayXTrack,
	/*0x00fa*/ aiStopXTrack,
	/*0x00fb*/ aiChrExplosions,
	/*0x00fc*/ aiIfKillCountGreaterThan,
	/*0x00fd*/ ai00fd,
	/*0x00fe*/ aiKillBond,
	/*0x00ff*/ aiBeSurprisedSurrender,
	/*0x0100*/ aiNoOp0100,
	/*0x0101*/ aiNoOp0101,
	/*0x0102*/ aiSetLights,
	/*0x0103*/ aiIfPropPresetIsBlockingSightToTarget,
	/*0x0104*/ aiRemoveObjectAtPropPreset,
	/*0x0105*/ aiIfPropPresetHeightLessThan,
	/*0x0106*/ aiSetTarget,
	/*0x0107*/ aiIfPresetsTargetIsNotMyTarget,
	/*0x0108*/ aiIfChrTarget,
	/*0x0109*/ aiSetChrPresetToChrNearSelf,
	/*0x010a*/ aiSetChrPresetToChrNearPad,
	/*0x010b*/ aiChrSetTeam,
	/*0x010c*/ aiIfCompareChrPresetsTeam,
	/*0x010d*/ aiNoOp010d,
	/*0x010e*/ aiSetShield,
	/*0x010f*/ aiIfChrShieldLessThan,
	/*0x0110*/ aiIfChrShieldGreaterThan,
	/*0x0111*/ aiSetCameraAnimation,
	/*0x0112*/ aiObjectDoAnimation,
	/*0x0113*/ aiIfInCutscene,
	/*0x0114*/ aiEnableChr,
	/*0x0115*/ aiDisableChr,
	/*0x0116*/ aiEnableObj,
	/*0x0117*/ aiDisableObj,
	/*0x0118*/ aiSetObjFlag3,
	/*0x0119*/ aiUnsetObjFlag3,
	/*0x011a*/ aiIfObjHasFlag3,
	/*0x011b*/ aiChrSetHiddenFlag,
	/*0x011c*/ aiChrUnsetHiddenFlag,
	/*0x011d*/ aiIfChrHasHiddenFlag,
	/*0x011e*/ aiIfHuman,
	/*0x011f*/ aiIfSkedar,
	/*0x0120*/ aiIfSafety2LessThan,
	/*0x0121*/ aiFindCover,
	/*0x0122*/ aiFindCoverWithinDist,
	/*0x0123*/ aiFindCoverOutsideDist,
	/*0x0124*/ aiGoToCover,
	/*0x0125*/ aiCheckCoverOutOfSight,
	/*0x0126*/ aiIfPlayerUsingCmpOrAr34,
	/*0x0127*/ aiDetectEnemyOnSameFloor,
	/*0x0128*/ aiDetectEnemy,
	/*0x0129*/ aiIfSafetyLessThan,
	/*0x012a*/ aiIfTargetMovingSlowly,
	/*0x012b*/ aiIfTargetMovingCloser,
	/*0x012c*/ aiIfTargetMovingAway,
	/*0x012d*/ NULL,
	/*0x012e*/ NULL,
	/*0x012f*/ ai012f,
	/*0x0130*/ aiSayQuip,
	/*0x0131*/ aiIncreaseSquadronAlertness,
	/*0x0132*/ aiSetAction,
	/*0x0133*/ aiSetTeamOrders,
	/*0x0134*/ aiIfOrders,
	/*0x0135*/ aiIfHasOrders,
	/*0x0136*/ aiRetreat,
	/*0x0137*/ aiIfChrInSquadronDoingAction,
	/*0x0138*/ aiIfChannelFree,
	/*0x0139*/ ai0139,
	/*0x013a*/ aiSetChrPresetToUnalertedTeammate,
	/*0x013b*/ aiSetSquadron,
	/*0x013c*/ aiFaceCover,
	/*0x013d*/ aiIfDangerousObjectNearby,
	/*0x013e*/ ai013e,
	/*0x013f*/ aiIfHeliWeaponsArmed,
	/*0x0140*/ aiIfHoverbotNextStep,
	/*0x0141*/ aiShuffleInvestigationTerminals,
	/*0x0142*/ aiSetPadPresetToInvestigationTerminal,
	/*0x0143*/ aiHeliArmWeapons,
	/*0x0144*/ aiHeliUnarmWeapons,
	/*0x0145*/ aiRebuildTeams,
	/*0x0146*/ aiRebuildSquadrons,
	/*0x0147*/ aiIfSquadronIsDead,
	/*0x0148*/ aiChrSetListening,
	/*0x0149*/ aiIfChrListening,
	/*0x014a*/ aiIfTrue,
	/*0x014b*/ aiIfNotListening,
	/*0x014c*/ NULL,
	/*0x014d*/ NULL,
	/*0x014e*/ NULL,
	/*0x014f*/ NULL,
	/*0x0150*/ NULL,
	/*0x0151*/ NULL,
	/*0x0152*/ aiIfNumChrsInSquadronGreaterThan,
	/*0x0153*/ NULL,
	/*0x0154*/ NULL,
	/*0x0155*/ NULL,
	/*0x0156*/ NULL,
	/*0x0157*/ aiSetTintedGlassEnabled,
	/*0x0158*/ NULL,
	/*0x0159*/ NULL,
	/*0x015a*/ NULL,
	/*0x015b*/ aiPlayTrackIsolated,
	/*0x015c*/ aiPlayDefaultTracks,
	/*0x015d*/ NULL,
	/*0x015e*/ NULL,
	/*0x015f*/ NULL,
	/*0x0160*/ NULL,
	/*0x0161*/ NULL,
	/*0x0162*/ NULL,
	/*0x0163*/ NULL,
	/*0x0164*/ NULL,
	/*0x0165*/ aiIfChrInjured,
	/*0x0166*/ aiIfAction,
	/*0x0167*/ aiHovercopterFireRocket,
	/*0x0168*/ aiIfShieldDamaged,
	/*0x0169*/ aiIfNaturalAnim,
	/*0x016a*/ aiIfY,
	/*0x016b*/ aiPlayRepeatingSoundFromObject,
	/*0x016c*/ aiNoOp016c,
	/*0x016d*/ aiChrAdjustMotionBlur,
	/*0x016e*/ aiDamageChrByAmount,
	/*0x016f*/ aiIfChrHasGun,
	/*0x0170*/ aiDoGunCommand,
	/*0x0171*/ aiIfDistanceToGunLessThan,
	/*0x0172*/ aiRecoverGun,
	/*0x0173*/ aiChrCopyProperties,
	/*0x0174*/ aiIfCutsceneButtonPressed,
	/*0x0175*/ ai0175,
	/*0x0176*/ aiIfBotRespawning,
	/*0x0177*/ aiPlayerAutoWalk,
	/*0x0178*/ aiIfPlayerAutoWalkFinished,
	/*0x0179*/ aiPlaySoundFromEntity,
	/*0x017a*/ aiIfLosToAttackTarget,
	/*0x017b*/ aiIfChrKnockedOut,
	/*0x017c*/ aiAssignSound,
	/*0x017d*/ aiPlayCutsceneTrack,
	/*0x017e*/ aiStopCutsceneTrack,
	/*0x017f*/ aiPlayTemporaryTrack,
	/*0x0180*/ aiStopAmbientTrack,
	/*0x0181*/ aiIfPlayerLookingAtObject,
	/*0x0182*/ aiPunchOrKick,
	/*0x0183*/ aiIfTargetIsPlayer,
	/*0x0184*/ ai0184,
	/*0x0185*/ aiMpInitSimulants,
	/*0x0186*/ aiIfSoundTimer,
	/*0x0187*/ aiSetTargetToEyespyIfInSight,
	/*0x0188*/ aiIfLiftStationary,
	/*0x0189*/ aiLiftGoToStop,
	/*0x018a*/ aiIfLiftAtStop,
	/*0x018b*/ aiConfigureRain,
	/*0x018c*/ aiChrToggleModelPart,
	/*0x018d*/ aiActivateLift,
	/*0x018e*/ aiMiniSkedarTryPounce,
	/*0x018f*/ aiIfObjectDistanceToPadLessThan,
	/*0x0190*/ aiSetSavefileFlag,
	/*0x0191*/ aiUnsetSavefileFlag,
	/*0x0192*/ aiIfSavefileFlagIsSet,
	/*0x0193*/ aiIfSavefileFlagIsUnset,
	/*0x0194*/ NULL,
	/*0x0195*/ NULL,
	/*0x0196*/ NULL,
	/*0x0197*/ NULL,
	/*0x0198*/ NULL,
	/*0x0199*/ NULL,
	/*0x019a*/ NULL,
	/*0x019b*/ NULL,
	/*0x019c*/ NULL,
	/*0x019d*/ NULL,
	/*0x019e*/ aiIfObjHealthLessThan,
	/*0x019f*/ aiSetObjHealth,
	/*0x01a0*/ aiSetChrSpecialDeathAnimation,
	/*0x01a1*/ aiSetRoomToSearch,
	/*0x01a2*/ aiSayCiStaffQuip,
	/*0x01a3*/ aiDoPresetAnimation,
	/*0x01a4*/ aiShowHudmsgMiddle,
	/*0x01a5*/ aiIfUsingLift,
	/*0x01a6*/ aiIfTargetYDifferenceLessThan,
	/*0x01a7*/ aiIfChrNotTalking,
	/*0x01a8*/ NULL,
	/*0x01a9*/ NULL,
	/*0x01aa*/ ai01aa,
	/*0x01ab*/ aiIfNumKnockedOutChrs,
	/*0x01ac*/ NULL,
	/*0x01ad*/ aiReleaseObject,
	/*0x01ae*/ aiClearInventory,
	/*0x01af*/ aiChrGrabObject,
	/*0x01b0*/ NULL,
	/*0x01b1*/ aiShuffleRuinsPillars,
	/*0x01b2*/ aiSetWindSpeed,
	/*0x01b3*/ aiToggleP1P2,
#if VERSION >= VERSION_NTSC_1_0
	/*0x01b4*/ ai01b4,
#else
	/*0x01b4*/ NULL,
#endif
	/*0x01b5*/ aiChrSetP1P2,
	/*0x01b6*/ aiConfigureSnow,
	/*0x01b7*/ aiChrSetCloaked,
	/*0x01b8*/ aiSetAutogunTargetTeam,
	/*0x01b9*/ aiShufflePelagicSwitches,
	/*0x01ba*/ aiTryAttackLie,
	/*0x01bb*/ aiNoOp01bb,
	/*0x01bc*/ ai01bc,
	/*0x01bd*/ aiIfTrainingPcHolographed,
	/*0x01be*/ aiIfPlayerUsingDevice,
	/*0x01bf*/ aiChrBeginOrEndTeleport,
	/*0x01c0*/ aiIfChrTeleportFullWhite,
	/*0x01c1*/ aiSetPunchDodgeList,
	/*0x01c2*/ aiSetShootingAtMeList,
	/*0x01c3*/ aiSetDarkRoomList,
	/*0x01c4*/ aiSetPlayerDeadList,
	/*0x01c5*/ aiAvoid,
	/*0x01c6*/ aiSetDodgeRating,
	/*0x01c7*/ aiSetUnarmedDodgeRating,
	/*0x01c8*/ aiTitleInitMode,
	/*0x01c9*/ aiTryExitTitle,
	/*0x01ca*/ aiChrSetCutsceneWeapon,
	/*0x01cb*/ aiFadeScreen,
	/*0x01cc*/ aiIfFadeComplete,
	/*0x01cd*/ aiSetChrHudpieceVisible,
	/*0x01ce*/ aiSetPassiveMode,
	/*0x01cf*/ aiChrSetFiringInCutscene,
	/*0x01d0*/ aiSetPortalFlag,
	/*0x01d1*/ aiObjSetModelPartVisible,
	/*0x01d2*/ aiChrEmitSparks,
	/*0x01d3*/ aiSetDrCarollImages,
	/*0x01d4*/ aiSetRoomFlag,
	/*0x01d5*/ aiShowCutsceneChrs,
	/*0x01d6*/ aiConfigureEnvironment,
	/*0x01d7*/ aiIfDistanceToTarget2LessThan,
	/*0x01d8*/ aiIfDistanceToTarget2GreaterThan,
	/*0x01d9*/ aiPlaySoundFromProp,
	/*0x01da*/ aiPlayTemporaryPrimaryTrack,
	/*0x01db*/ aiChrKill,
	/*0x01dc*/ aiRemoveWeaponFromInventory,
	/*0x01dd*/ aiIfMusicEventQueueIsEmpty,
	/*0x01de*/ aiIfCoopMode,
#if VERSION >= VERSION_NTSC_1_0
	/*0x01df*/ aiIfChrSameFloorDistanceToPadLessThan,
	/*0x01e0*/ aiRemoveReferencesToChr,
#endif
};

u16 g_CommandLengths[] = {
	/*0x0000*/ 3,  /*0x0001*/ 3,  /*0x0002*/ 3,  /*0x0003*/ 2,
	/*0x0004*/ 2,  /*0x0005*/ 5,  /*0x0006*/ 5,  /*0x0007*/ 4,
	/*0x0008*/ 2,  /*0x0009*/ 2,  /*0x000a*/ 2,  /*0x000b*/ 12,
	/*0x000c*/ 3,  /*0x000d*/ 2,  /*0x000e*/ 2,  /*0x000f*/ 3,
	/*0x0010*/ 3,  /*0x0011*/ 3,  /*0x0012*/ 3,  /*0x0013*/ 3,
	/*0x0014*/ 3,  /*0x0015*/ 7,  /*0x0016*/ 7,  /*0x0017*/ 7,
	/*0x0018*/ 7,  /*0x0019*/ 8,  /*0x001a*/ 5,  /*0x001b*/ 7,
	/*0x001c*/ 6,  /*0x001d*/ 4,  /*0x001e*/ 3,  /*0x001f*/ 4,
	/*0x0020*/ 4,  /*0x0021*/ 3,  /*0x0022*/ 2,  /*0x0023*/ 3,
	/*0x0024*/ 2,  /*0x0025*/ 2,  /*0x0026*/ 3,  /*0x0027*/ 5,
	/*0x0028*/ 2,  /*0x0029*/ 2,  /*0x002a*/ 3,  /*0x002b*/ 3,
	/*0x002c*/ 3,  /*0x002d*/ 3,  /*0x002e*/ 3,  /*0x002f*/ 4,
	/*0x0030*/ 4,  /*0x0031*/ 4,  /*0x0032*/ 3,  /*0x0033*/ 4,
	/*0x0034*/ 4,  /*0x0035*/ 3,  /*0x0036*/ 2,  /*0x0037*/ 4,
	/*0x0038*/ 4,  /*0x0039*/ 3,  /*0x003a*/ 3,  /*0x003b*/ 3,
	/*0x003c*/ 3,  /*0x003d*/ 4,  /*0x003e*/ 4,  /*0x003f*/ 3,
	/*0x0040*/ 7,  /*0x0041*/ 7,  /*0x0042*/ 3,  /*0x0043*/ 3,
	/*0x0044*/ 3,  /*0x0045*/ 4,  /*0x0046*/ 3,  /*0x0047*/ 3,
	/*0x0048*/ 4,  /*0x0049*/ 5,  /*0x004a*/ 3,  /*0x004b*/ 3,
	/*0x004c*/ 3,  /*0x004d*/ 4,  /*0x004e*/ 6,  /*0x004f*/ 4,
	/*0x0050*/ 4,  /*0x0051*/ 4,  /*0x0052*/ 5,  /*0x0053*/ 5,
	/*0x0054*/ 8,  /*0x0055*/ 8,  /*0x0056*/ 6,  /*0x0057*/ 6,
	/*0x0058*/ 5,  /*0x0059*/ 7,  /*0x005a*/ 7,  /*0x005b*/ 7,
	/*0x005c*/ 5,  /*0x005d*/ 5,  /*0x005e*/ 4,  /*0x005f*/ 5,
	/*0x0060*/ 5,  /*0x0061*/ 5,  /*0x0062*/ 4,  /*0x0063*/ 5,
	/*0x0064*/ 1,  /*0x0065*/ 3,  /*0x0066*/ 3,  /*0x0067*/ 3,
	/*0x0068*/ 3,  /*0x0069*/ 3,  /*0x006a*/ 4,  /*0x006b*/ 5,
	/*0x006c*/ 3,  /*0x006d*/ 3,  /*0x006e*/ 5,  /*0x006f*/ 4,
	/*0x0070*/ 4,  /*0x0071*/ 4,  /*0x0072*/ 5,  /*0x0073*/ 4,
	/*0x0074*/ 4,  /*0x0075*/ 4,  /*0x0076*/ 4,  /*0x0077*/ 4,
	/*0x0078*/ 4,  /*0x0079*/ 5,  /*0x007a*/ 5,  /*0x007b*/ 4,
	/*0x007c*/ 4,  /*0x007d*/ 4,  /*0x007e*/ 4,  /*0x007f*/ 4,
	/*0x0080*/ 4,  /*0x0081*/ 5,  /*0x0082*/ 5,  /*0x0083*/ 4,
	/*0x0084*/ 3,  /*0x0085*/ 3,  /*0x0086*/ 4,  /*0x0087*/ 3,
	/*0x0088*/ 4,  /*0x0089*/ 3,  /*0x008a*/ 3,  /*0x008b*/ 3,
	/*0x008c*/ 4,  /*0x008d*/ 3,  /*0x008e*/ 5,  /*0x008f*/ 5,
	/*0x0090*/ 3,  /*0x0091*/ 2,  /*0x0092*/ 4,  /*0x0093*/ 3,
	/*0x0094*/ 3,  /*0x0095*/ 3,  /*0x0096*/ 5,  /*0x0097*/ 4,
	/*0x0098*/ 3,  /*0x0099*/ 3,  /*0x009a*/ 3,  /*0x009b*/ 7,
	/*0x009c*/ 7,  /*0x009d*/ 9,  /*0x009e*/ 8,  /*0x009f*/ 8,
	/*0x00a0*/ 9,  /*0x00a1*/ 6,  /*0x00a2*/ 6,  /*0x00a3*/ 8,
	/*0x00a4*/ 6,  /*0x00a5*/ 6,  /*0x00a6*/ 7,  /*0x00a7*/ 7,
	/*0x00a8*/ 7,  /*0x00a9*/ 8,  /*0x00aa*/ 7,  /*0x00ab*/ 7,
	/*0x00ac*/ 8,  /*0x00ad*/ 7,  /*0x00ae*/ 7,  /*0x00af*/ 8,
	/*0x00b0*/ 3,  /*0x00b1*/ 4,  /*0x00b2*/ 4,  /*0x00b3*/ 5,
	/*0x00b4*/ 4,  /*0x00b5*/ 0,  /*0x00b6*/ 2,  /*0x00b7*/ 2,
	/*0x00b8*/ 2,  /*0x00b9*/ 2,  /*0x00ba*/ 3,  /*0x00bb*/ 3,
	/*0x00bc*/ 6,  /*0x00bd*/ 6,  /*0x00be*/ 2,  /*0x00bf*/ 2,
	/*0x00c0*/ 4,  /*0x00c1*/ 2,  /*0x00c2*/ 2,  /*0x00c3*/ 3,
	/*0x00c4*/ 5,  /*0x00c5*/ 5,  /*0x00c6*/ 13, /*0x00c7*/ 12,
	/*0x00c8*/ 10, /*0x00c9*/ 9,  /*0x00ca*/ 10, /*0x00cb*/ 5,
	/*0x00cc*/ 6,  /*0x00cd*/ 9,  /*0x00ce*/ 5,  /*0x00cf*/ 6,
	/*0x00d0*/ 7,  /*0x00d1*/ 7,  /*0x00d2*/ 7,  /*0x00d3*/ 3,
	/*0x00d4*/ 6,  /*0x00d5*/ 3,  /*0x00d6*/ 6,  /*0x00d7*/ 6,
	/*0x00d8*/ 3,  /*0x00d9*/ 3,  /*0x00da*/ 5,  /*0x00db*/ 3,
	/*0x00dc*/ 2,  /*0x00dd*/ 2,  /*0x00de*/ 4,  /*0x00df*/ 7,
	/*0x00e0*/ 4,  /*0x00e1*/ 3,  /*0x00e2*/ 7,  /*0x00e3*/ 3,
	/*0x00e4*/ 3,  /*0x00e5*/ 4,  /*0x00e6*/ 1,  /*0x00e7*/ 1,
	/*0x00e8*/ 3,  /*0x00e9*/ 4,  /*0x00ea*/ 4,  /*0x00eb*/ 6,
	/*0x00ec*/ 4,  /*0x00ed*/ 4,  /*0x00ee*/ 5,  /*0x00ef*/ 6,
	/*0x00f0*/ 3,  /*0x00f1*/ 3,  /*0x00f2*/ 2,  /*0x00f3*/ 3,
	/*0x00f4*/ 14, /*0x00f5*/ 2,  /*0x00f6*/ 3,  /*0x00f7*/ 3,
	/*0x00f8*/ 4,  /*0x00f9*/ 5,  /*0x00fa*/ 3,  /*0x00fb*/ 3,
	/*0x00fc*/ 4,  /*0x00fd*/ 4,  /*0x00fe*/ 2,  /*0x00ff*/ 2,
	/*0x0100*/ 3,  /*0x0101*/ 3,  /*0x0102*/ 11, /*0x0103*/ 3,
	/*0x0104*/ 2,  /*0x0105*/ 5,  /*0x0106*/ 5,  /*0x0107*/ 3,
	/*0x0108*/ 6,  /*0x0109*/ 6,  /*0x010a*/ 8,  /*0x010b*/ 4,
	/*0x010c*/ 4,  /*0x010d*/ 2,  /*0x010e*/ 4,  /*0x010f*/ 6,
	/*0x0110*/ 6,  /*0x0111*/ 4,  /*0x0112*/ 8,  /*0x0113*/ 3,
	/*0x0114*/ 3,  /*0x0115*/ 3,  /*0x0116*/ 3,  /*0x0117*/ 3,
	/*0x0118*/ 7,  /*0x0119*/ 7,  /*0x011a*/ 8,  /*0x011b*/ 7,
	/*0x011c*/ 7,  /*0x011d*/ 8,  /*0x011e*/ 4,  /*0x011f*/ 4,
	/*0x0120*/ 4,  /*0x0121*/ 5,  /*0x0122*/ 9,  /*0x0123*/ 9,
	/*0x0124*/ 3,  /*0x0125*/ 3,  /*0x0126*/ 3,  /*0x0127*/ 3,
	/*0x0128*/ 4,  /*0x0129*/ 4,  /*0x012a*/ 4,  /*0x012b*/ 3,
	/*0x012c*/ 3,  /*0x012d*/ 1,  /*0x012e*/ 1,  /*0x012f*/ 2,
	/*0x0130*/ 10, /*0x0131*/ 3,  /*0x0132*/ 4,  /*0x0133*/ 4,
	/*0x0134*/ 5,  /*0x0135*/ 3,  /*0x0136*/ 4,  /*0x0137*/ 4,
	/*0x0138*/ 4,  /*0x0139*/ 8,  /*0x013a*/ 5,  /*0x013b*/ 3,
	/*0x013c*/ 3,  /*0x013d*/ 4,  /*0x013e*/ 2,  /*0x013f*/ 3,
	/*0x0140*/ 5,  /*0x0141*/ 9,  /*0x0142*/ 4,  /*0x0143*/ 2,
	/*0x0144*/ 2,  /*0x0145*/ 2,  /*0x0146*/ 2,  /*0x0147*/ 4,
	/*0x0148*/ 4,  /*0x0149*/ 6,  /*0x014a*/ 6,  /*0x014b*/ 3,
	/*0x014c*/ 1,  /*0x014d*/ 1,  /*0x014e*/ 1,  /*0x014f*/ 1,
	/*0x0150*/ 1,  /*0x0151*/ 1,  /*0x0152*/ 5,  /*0x0153*/ 1,
	/*0x0154*/ 1,  /*0x0155*/ 1,  /*0x0156*/ 1,  /*0x0157*/ 3,
	/*0x0158*/ 1,  /*0x0159*/ 1,  /*0x015a*/ 1,  /*0x015b*/ 3,
	/*0x015c*/ 2,  /*0x015d*/ 1,  /*0x015e*/ 1,  /*0x015f*/ 1,
	/*0x0160*/ 1,  /*0x0161*/ 1,  /*0x0162*/ 1,  /*0x0163*/ 1,
	/*0x0164*/ 1,  /*0x0165*/ 4,  /*0x0166*/ 4,  /*0x0167*/ 3,
	/*0x0168*/ 4,  /*0x0169*/ 4,  /*0x016a*/ 7,  /*0x016b*/ 10,
	/*0x016c*/ 2,  /*0x016d*/ 5,  /*0x016e*/ 5,  /*0x016f*/ 5,
	/*0x0170*/ 4,  /*0x0171*/ 5,  /*0x0172*/ 4,  /*0x0173*/ 4,
	/*0x0174*/ 3,  /*0x0175*/ 3,  /*0x0176*/ 3,  /*0x0177*/ 9,
	/*0x0178*/ 4,  /*0x0179*/ 11, /*0x017a*/ 3,  /*0x017b*/ 4,
	/*0x017c*/ 5,  /*0x017d*/ 3,  /*0x017e*/ 2,  /*0x017f*/ 3,
	/*0x0180*/ 2,  /*0x0181*/ 5,  /*0x0182*/ 4,  /*0x0183*/ 3,
	/*0x0184*/ 4,  /*0x0185*/ 2,  /*0x0186*/ 6,  /*0x0187*/ 3,
	/*0x0188*/ 4,  /*0x0189*/ 4,  /*0x018a*/ 5,  /*0x018b*/ 3,
	/*0x018c*/ 4,  /*0x018d*/ 4,  /*0x018e*/ 7,  /*0x018f*/ 8,
	/*0x0190*/ 3,  /*0x0191*/ 3,  /*0x0192*/ 4,  /*0x0193*/ 4,
	/*0x0194*/ 1,  /*0x0195*/ 1,  /*0x0196*/ 1,  /*0x0197*/ 1,
	/*0x0198*/ 1,  /*0x0199*/ 1,  /*0x019a*/ 1,  /*0x019b*/ 1,
	/*0x019c*/ 1,  /*0x019d*/ 1,  /*0x019e*/ 6,  /*0x019f*/ 5,
	/*0x01a0*/ 4,  /*0x01a1*/ 2,  /*0x01a2*/ 4,  /*0x01a3*/ 3,
	/*0x01a4*/ 6,  /*0x01a5*/ 3,  /*0x01a6*/ 4,  /*0x01a7*/ 4,
	/*0x01a8*/ 1,  /*0x01a9*/ 1,  /*0x01aa*/ 3,  /*0x01ab*/ 5,
	/*0x01ac*/ 1,  /*0x01ad*/ 3,  /*0x01ae*/ 3,  /*0x01af*/ 4,
	/*0x01b0*/ 1,  /*0x01b1*/ 18, /*0x01b2*/ 3,  /*0x01b3*/ 3,
	/*0x01b4*/ VERSION >= VERSION_NTSC_1_0 ? 3 : 1, /*0x01b5*/ 4, /*0x01b6*/ 3, /*0x01b7*/ 5,
	/*0x01b8*/ 4,  /*0x01b9*/ 2,  /*0x01ba*/ 7,  /*0x01bb*/ 4,
	/*0x01bc*/ 4,  /*0x01bd*/ 3,  /*0x01be*/ 5,  /*0x01bf*/ 5,
	/*0x01c0*/ 4,  /*0x01c1*/ 4,  /*0x01c2*/ 4,  /*0x01c3*/ 4,
	/*0x01c4*/ 4,  /*0x01c5*/ 2,  /*0x01c6*/ 4,  /*0x01c7*/ 3,
	/*0x01c8*/ 3,  /*0x01c9*/ 3,  /*0x01ca*/ 5,  /*0x01cb*/ 8,
	/*0x01cc*/ 3,  /*0x01cd*/ 4,  /*0x01ce*/ 3,  /*0x01cf*/ 4,
	/*0x01d0*/ 5,  /*0x01d1*/ 5,  /*0x01d2*/ 3,  /*0x01d3*/ 5,
	/*0x01d4*/ 6,  /*0x01d5*/ 3,  /*0x01d6*/ 6,  /*0x01d7*/ 5,
	/*0x01d8*/ 5,  /*0x01d9*/ 11, /*0x01da*/ 3,  /*0x01db*/ 3,
	/*0x01dc*/ 3,  /*0x01dd*/ 4,  /*0x01de*/ 3,
#if VERSION >= VERSION_NTSC_1_0
	/*0x01df*/ 8,
	/*0x01e0*/ 2,
#endif
};

s32 chraiGetListIdByList(u8 *ailist, bool *is_global)
{
	s32 i;

	if (g_StageSetup.ailists) {
		for (i = 0; g_StageSetup.ailists[i].list != NULL; i++) {
			if (g_StageSetup.ailists[i].list == ailist) {
				*is_global = false;
				return g_StageSetup.ailists[i].id;
			}
		}
	}

	for (i = 0; g_GlobalAilists[i].list != NULL; i++) {
		if (g_GlobalAilists[i].list == ailist) {
			*is_global = true;
			return g_GlobalAilists[i].id;
		}
	}

	return -1;
}

u32 chraiGoToLabel(u8 *ailist, u32 aioffset, u8 label)
{
	do {
		u8 *cmd = aioffset + ailist;
		u32 type = (cmd[0] << 8) + cmd[1];

		if (type == CMD_LABEL) {
			if (label == cmd[2]) {
				return aioffset;
			}
		} else if (type == CMD_END) {
			return 0;
		}

		aioffset += chraiGetCommandLength(ailist, aioffset);
	} while (true);
}

void chraiExecute(void *entity, s32 proptype)
{
	g_Vars.chrdata = NULL;
	g_Vars.truck = NULL;
	g_Vars.heli = NULL;
	g_Vars.hovercar = NULL;
	g_Vars.ailist = NULL;
	g_Vars.aioffset = 0;

	if (proptype == PROPTYPE_CHR) {
		g_Vars.chrdata = entity;
	} else if (proptype == PROPTYPE_OBJ) {
		struct defaultobj *obj = entity;

		if (obj->type == OBJTYPE_TRUCK) {
			g_Vars.truck = entity;
		} else if (obj->type == OBJTYPE_HELI) {
			g_Vars.heli = entity;
		} else if (obj->type == OBJTYPE_HOVERCAR || obj->type == OBJTYPE_CHOPPER) {
			g_Vars.hovercar = entity;
		}
	}

	if (g_Vars.chrdata) {
		g_Vars.ailist = g_Vars.chrdata->ailist;
		g_Vars.aioffset = g_Vars.chrdata->aioffset;
	} else if (g_Vars.truck) {
		g_Vars.ailist = g_Vars.truck->ailist;
		g_Vars.aioffset = g_Vars.truck->aioffset;
	} else if (g_Vars.heli) {
		g_Vars.ailist = g_Vars.heli->ailist;
		g_Vars.aioffset = g_Vars.heli->aioffset;
	} else if (g_Vars.hovercar) {
		g_Vars.ailist = g_Vars.hovercar->ailist;
		g_Vars.aioffset = g_Vars.hovercar->aioffset;
	}

	if (g_Vars.ailist) {
		if (g_Vars.chrdata) {
			chrAddTargetToBdlist(g_Vars.chrdata);
		}

		// Check if the ailist should be switched to a different one
		if (g_Vars.chrdata && (g_Vars.chrdata->chrflags & CHRCFLAG_TRIGGERSHOTLIST)) {
			u32 animnum = modelGetAnimNum(g_Vars.chrdata->model);
			if (g_Vars.chrdata->aishotlist >= 0
					&& g_Vars.chrdata->cshield <= 0
					&& (0 <= g_Vars.chrdata->damage || g_Vars.chrdata->gunprop != NULL)
					&& animnum != ANIM_SNIPING_GETDOWN
					&& animnum != ANIM_SNIPING_GETUP
					&& animnum != ANIM_SNIPING_ONGROUND) {
				// Set shot list
				g_Vars.chrdata->chrflags &= ~CHRCFLAG_TRIGGERSHOTLIST;
				g_Vars.ailist = ailistFindById(g_Vars.chrdata->aishotlist);
				g_Vars.aioffset = 0;
			}
		} else if (g_Vars.chrdata && (g_Vars.chrdata->chrflags & CHRCFLAG_CONSIDER_DODGE)) {
			g_Vars.chrdata->chrflags &= ~CHRCFLAG_CONSIDER_DODGE;

			if (g_Vars.chrdata->aishootingatmelist >= 0
					&& ailistFindById(g_Vars.chrdata->aishootingatmelist) != g_Vars.chrdata->ailist
					&& g_Vars.chrdata->dodgerating > rngRandom() % 100
					&& chrHasFlag(g_Vars.chrdata, CHRFLAG1_INDARKROOM, BANK_1) == 0
					&& chrHasFlag(g_Vars.chrdata, CHRFLAG0_AIVSAI, BANK_0) == 0
					&& ailistFindById(g_Vars.chrdata->aishootingatmelist) != g_Vars.chrdata->ailist
					&& g_Vars.chrdata->actiontype != ACT_ATTACK
					&& g_Vars.chrdata->actiontype != ACT_ATTACKWALK
					&& g_Vars.chrdata->actiontype != ACT_DIE
					&& g_Vars.chrdata->actiontype != ACT_DEAD
					&& g_Vars.chrdata->actiontype != ACT_ARGH
					&& g_Vars.chrdata->actiontype != ACT_PREARGH
					&& g_Vars.chrdata->actiontype != ACT_ATTACKROLL) {
				// Set shooting at me list
				g_Vars.ailist = ailistFindById(g_Vars.chrdata->aishootingatmelist);
				g_Vars.aioffset = 0;
				g_Vars.chrdata->dodgerating = 0;
			} else {
				// Increase dodge rating
				g_Vars.chrdata->dodgerating *= 2;

				if (g_Vars.chrdata->dodgerating > g_Vars.chrdata->maxdodgerating) {
					g_Vars.chrdata->dodgerating = g_Vars.chrdata->maxdodgerating;
				}
			}
		} else if (g_Vars.chrdata
				&& g_Vars.chrdata->darkroomthing
				&& chrHasFlag(g_Vars.chrdata, CHRFLAG1_INDARKROOM, BANK_1) == 0
				&& ailistFindById(g_Vars.chrdata->aidarkroomlist) != g_Vars.chrdata->ailist
				&& g_Vars.stagenum != STAGE_CRASHSITE) {
			g_Vars.chrdata->darkroomthing = 0;

			if (g_Vars.chrdata->aidarkroomlist >= 0
					&& g_Vars.chrdata->actiontype != ACT_DIE
					&& g_Vars.chrdata->actiontype != ACT_DEAD
					&& g_Vars.chrdata->actiontype != ACT_ARGH) {
				// Set darkroom list
				chrSetFlags(g_Vars.chrdata, CHRFLAG1_INDARKROOM, BANK_1);
				chrSetFlags(g_Vars.chrdata, CHRFLAG1_SEARCHSAMEROOM, BANK_1);
				g_Vars.chrdata->alertness = 0;
				g_Vars.ailist = ailistFindById(g_Vars.chrdata->aidarkroomlist);
				g_Vars.aioffset = 0;
			}
		} else {
			// empty
		}

		// Iterate and execute the ailist
		while (g_Vars.ailist) {
			u8 *cmd = g_Vars.aioffset + g_Vars.ailist;
			s32 type = (cmd[0] << 8) + cmd[1];

			if (type >= 0 && type < ARRAYCOUNT(g_CommandPointers)) {
				if (g_CommandPointers[type]()) {
					break;
				}
			} else {
				// This is attempting to handle situations where the command
				// type is invalid by passing over them and continuing
				// execution. This would very likely result in a crash though.
				g_Vars.aioffset += chraiGetCommandLength(g_Vars.ailist, g_Vars.aioffset);
			}
		}
	}
}

u32 chraiGetCommandLength(u8 *ailist, u32 aioffset)
{
	u8 *cmd = aioffset + ailist;
	s32 type = (cmd[0] << 8) + cmd[1];

	if (type == CMD_PRINT) {
		u32 prop = aioffset + 2;

		while (ailist[prop] != 0) {
			++prop;
		}

		return (prop - aioffset) + 1;
	}

	if (type >= 0 && type < ARRAYCOUNT(g_CommandLengths)) {
		return g_CommandLengths[type];
	}

	return 1;
}

// used by ext_setup
u32 chraiGetAilistLength(u8* list)
{
	if (!list) return 0;

	u8* cmd = list;

	while (true) {
		u8 type = cmd[1];
		cmd += chraiGetCommandLength(cmd, 0);
		if (type == AICMD_END) break;
	}

	return (u32)(cmd - list);
}
