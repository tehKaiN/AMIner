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

static UWORD s_uwAccountingCost;

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
		if(bButtonCurr == 0) {
			g_pVehicles[0].lCash -= s_uwAccountingCost;
			if(randUwMinMax(&g_sRand, 1, 100) > heatGetPercent()) {
				warehouseNextPlan(NEXT_PLAN_REASON_FULFILLED_ACCOUNTING);
			}
			else {
				gameAddRebuke();
			}

			heatTryIncrease(6);
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
		buttonInitOk("Back");
	}
	else {
		s_uwAccountingCost = planGetRemainingCost() / 2;

		uwPosY += commDrawMultilineText(
			"I can do some Creative Acccounting for you and fulfill your plan instantly."
			" For a price, of course.", 0, uwPosY
		) * ubLineHeight;
		char szBfr[150];
		uwPosY += ubLineHeight / 2;
		sprintf(
			szBfr, "There is %hhu%% chance that we will get caught, which would result in instantly getting a rebuke.",
			heatGetPercent()
		);
		uwPosY += commDrawMultilineText(szBfr,  0, uwPosY) * ubLineHeight;
		sprintf(szBfr, "It will cost you %hu\x1F.", s_uwAccountingCost);
		uwPosY += commDrawMultilineText(szBfr, 0, uwPosY) * ubLineHeight;

		buttonInitAcceptDecline("Accept", "Decline");
	}

	buttonDrawAll(commGetDisplayBuffer());
}

void pageAccountingReset(void) {
}

void pageAccountingSave(tFile *pFile) {
	saveWriteHeader(pFile, "ACTG");
}

UBYTE pageAccountingLoad(tFile *pFile) {
	if(!saveReadHeader(pFile, "ACTG")) {
		return 0;
	}
	return 1;
}
