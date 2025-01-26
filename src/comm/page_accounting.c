/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "page_accounting.h"
#include <comm/comm.h>
#include <comm/page_office.h>
#include <comm/button.h>
#include <comm/gs_shop.h>
#include "../core.h"
#include "../game.h"
#include "../warehouse.h"
#include "../vehicle.h"
#include "../save.h"
#include "../heat.h"
#include "../achievement.h"

#define ACCOUNTING_SLACKER_THRESHOLD 10

static UWORD s_uwAccountingStreak;
static UBYTE s_ubAccountingUses;

static void pageAccountingProcess(void) {
	BYTE bButtonPrev = buttonGetSelected(), bButtonCurr = bButtonPrev;
	BYTE bButtonCount = buttonGetCount();
	if(commNavUse(DIRECTION_RIGHT)) {
		bButtonCurr = MIN(bButtonCurr + 1, bButtonCount - 1);
	}
	else if(commNavUse(DIRECTION_LEFT)) {
		bButtonCurr = MAX(bButtonCurr - 1, 0);
	}
	if(bButtonPrev != bButtonCurr) {
		buttonSelect(bButtonCurr);
		buttonDrawAll(commGetDisplayBuffer());
	}

	if(commNavExUse(COMM_NAV_EX_BTN_CLICK)) {
		if(planManagerGet()->isPlanActive) {
			if(bButtonCurr == 0) {
				++s_ubAccountingUses;
				if(randUwMinMax(&g_sRand, 1, 100) > heatGetPercent()) {
					if(++s_uwAccountingStreak >= ACCOUNTING_SLACKER_THRESHOLD) {
						achievementUnlock(ACHIEVEMENT_SLACKER);
					}
					warehouseNextPlan(NEXT_PLAN_REASON_FULFILLED_ACCOUNTING);
				}
				else {
					s_uwAccountingStreak = 0;
					gameAddRebuke();
				}

				heatTryIncrease(5);
			}
		}
		commShopGoBack();
	}
}

void pageAccountingCreate(void) {
	commRegisterPage(pageAccountingProcess, 0);
	UWORD uwPosY = 0;
	UBYTE ubLineHeight = commGetLineHeight();
	if (!planManagerGet()->isPlanActive) {
		uwPosY += commDrawMultilineText(
			"You have no active plan! What do you want me to do?", 0, uwPosY
		) * ubLineHeight;
		buttonInitOk(g_pMsgs[MSG_PAGE_BACK]);
	}
	else {
		uwPosY += commDrawMultilineText(
			"I can do some Creative Acccounting for you and fulfill your plan instantly.",
			0, uwPosY
		) * ubLineHeight;
		char szBfr[150];
		uwPosY += ubLineHeight / 2;
		sprintf(
			szBfr, "There is %hhu%% chance that we will get caught, which would result in instantly getting a rebuke.",
			heatGetPercent()
		);
		uwPosY += commDrawMultilineText(szBfr,  0, uwPosY) * ubLineHeight;

		buttonInitAcceptDecline("Accept", "Decline");
	}

	buttonDrawAll(commGetDisplayBuffer());
}

void pageAccountingReset(void) {
	s_uwAccountingStreak = 0;
	s_ubAccountingUses = 0;
}

void pageAccountingSave(tFile *pFile) {
	saveWriteTag(pFile, SAVE_TAG_ACCOUNTING);
	fileWrite(pFile, &s_uwAccountingStreak, sizeof(s_uwAccountingStreak));
	fileWrite(pFile, &s_ubAccountingUses, sizeof(s_ubAccountingUses));
	saveWriteTag(pFile, SAVE_TAG_ACCOUNTING_END);
}

UBYTE pageAccountingLoad(tFile *pFile) {
	if(!saveReadTag(pFile, SAVE_TAG_ACCOUNTING)) {
		return 0;
	}
	fileRead(pFile, &s_uwAccountingStreak, sizeof(s_uwAccountingStreak));
	fileRead(pFile, &s_ubAccountingUses, sizeof(s_ubAccountingUses));
	return saveReadTag(pFile, SAVE_TAG_ACCOUNTING_END);
}

UBYTE pageAccountingGetUses(void) {
	return s_ubAccountingUses;
}
