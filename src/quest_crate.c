/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "quest_crate.h"
#include <comm/page_office.h>
#include <comm/page_questioning.h>
#include <comm/inbox.h>
#include <save.h>
#include <hud.h>

static UBYTE s_ubCrateCount;
static UBYTE s_isAgentTriggered;
static UBYTE s_isScientistUnlocked;
static tCapsuleState s_eCapsuleState;

static void questCrateOnQuestioningEnd(
	tQuestioningBit eQuestioningBit, UBYTE isReportedOrCaught
) {
	switch(eQuestioningBit) {
		case QUESTIONING_BIT_TELEPORT_PARTS:
			if(isReportedOrCaught) {
				s_ubCrateCount = 0;
			}
			break;
		case QUESTIONING_BIT_AGENT:
			if(isReportedOrCaught) {
				pageOfficeLockPerson(FACE_ID_AGENT);
			}
			break;
		default:
			break;
	}
}

void questCrateReset(void) {
	s_ubCrateCount = 0;
	s_isAgentTriggered = 0;
	s_isScientistUnlocked = 0;
	pageQuestioningSetHandler(QUESTIONING_BIT_TELEPORT_PARTS, questCrateOnQuestioningEnd);
	pageQuestioningSetHandler(QUESTIONING_BIT_AGENT, questCrateOnQuestioningEnd);
}

void questCrateSave(tFile *pFile) {
	saveWriteHeader(pFile, "CRTE");
	fileWrite(pFile, &s_ubCrateCount, sizeof(s_ubCrateCount));
	fileWrite(pFile, &s_isAgentTriggered, sizeof(s_isAgentTriggered));
	fileWrite(pFile, &s_isScientistUnlocked, sizeof(s_isScientistUnlocked));
	fileWrite(pFile, &s_eCapsuleState, sizeof(s_eCapsuleState));
}

UBYTE questCrateLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "CRTE")) {
		return 0;
	}

	fileRead(pFile, &s_ubCrateCount, sizeof(s_ubCrateCount));
	fileRead(pFile, &s_isAgentTriggered, sizeof(s_isAgentTriggered));
	fileRead(pFile, &s_isScientistUnlocked, sizeof(s_isScientistUnlocked));
	fileRead(pFile, &s_eCapsuleState, sizeof(s_eCapsuleState));

	return 1;
}

void questCrateProcess(void) {

}

void questCrateAdd(void) {
	s_ubCrateCount += 1;
	if(!s_isAgentTriggered) {
		s_isAgentTriggered = 1;
		pageOfficeUnlockPerson(FACE_ID_AGENT);
		pageQuestioningAddReporting(QUESTIONING_BIT_AGENT);
		pageOfficeTryUnlockPersonSubpage(FACE_ID_AGENT, COMM_SHOP_PAGE_AGENT_WELCOME);
		pageQuestioningTrySetPendingQuestioning(QUESTIONING_BIT_TELEPORT_PARTS);
		inboxPushBack(COMM_SHOP_PAGE_AGENT_WELCOME, 0);
	}
}

UBYTE questCrateGetCount(void) {
	return s_ubCrateCount;
}

UBYTE questCrateTrySell(void) {
	if(questCrateGetCount()) {
		s_ubCrateCount -= 1;
		return 1;
	}
	return 0;
}

void questCrateSetCapsuleState(tCapsuleState eNewState) {
	s_eCapsuleState = eNewState;
	if(s_eCapsuleState == CAPSULE_STATE_FOUND) {
		// pageOfficeTryUnlockPersonSubpage(FACE_ID_SCIENTIST, COMM_SHOP_PAGE_SCIENTIST_ABOUT_CAPSULE);
	}
}

tCapsuleState questCrateGetCapsuleState(void) {
	return s_eCapsuleState;
}

void questCrateProcessBase(void) {
	if(!s_isScientistUnlocked) {
		pageOfficeUnlockPerson(FACE_ID_SCIENTIST);
		pageOfficeTryUnlockPersonSubpage(FACE_ID_SCIENTIST, COMM_SHOP_PAGE_SCIENTIST_WELCOME);
		inboxPushBack(COMM_SHOP_PAGE_SCIENTIST_WELCOME, 0);
		hudShowMessage(FACE_ID_SCIENTIST, g_pMsgs[MSG_HUD_SCI_WELCOME]);
		s_isScientistUnlocked = 1;
	}
}
