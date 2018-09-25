/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vehicle.h"
#include "game.h"
#include "tile.h"

void vehicleCreate(void) {
	logBlockBegin("vehicleCreate()");
	g_sVehicle.pFrames = bitmapCreateFromFile("data/drill.bm");
	g_sVehicle.pMask = bitmapCreateFromFile("data/drill_mask.bm");
	bobNewInit(
		&g_sVehicle.sBob, VEHICLE_WIDTH, VEHICLE_HEIGHT, 1,
		g_sVehicle.pFrames, g_sVehicle.pMask, 0, 0
	);
	g_sVehicle.ubPayloadCurr = 0;
	g_sVehicle.ubPayloadMax = 10;
	g_sVehicle.uwPayloadScore = 0;
	g_sVehicle.ulScore = 0;
	g_sVehicle.fX = 0;
	g_sVehicle.fY = 0;
	g_sVehicle.fDx = 0;
	g_sVehicle.fDy = 0;
	logBlockEnd("vehicleCreate()");
}

void vehicleDestroy(void) {
	bitmapDestroy(g_sVehicle.pFrames);
	bitmapDestroy(g_sVehicle.pMask);
}

void vehicleMove(BYTE bDirX, BYTE bDirY) {
	g_sVehicle.sSteer.bX = bDirX;
	g_sVehicle.sSteer.bY = bDirY;

	if(bDirX > 0) {
		bobNewSetBitMapOffset(&g_sVehicle.sBob, 0);
	}
	else if(bDirX < 0) {
		bobNewSetBitMapOffset(&g_sVehicle.sBob, VEHICLE_HEIGHT);
	}
}

void vehicleProcess(void) {
	UBYTE isOnGround = 0;

	if(g_sVehicle.sSteer.bX) {
		if(g_sVehicle.sSteer.bX > 0) {
			g_sVehicle.fDx = MIN(g_sVehicle.fDx + fix16_one/8, 4*fix16_one);
		}
		else {
			g_sVehicle.fDx = MAX(g_sVehicle.fDx - fix16_one/8, -4*fix16_one);
		}
	}
	else {
		if(g_sVehicle.fDx > 0) {
			g_sVehicle.fDx = MAX(0, g_sVehicle.fDx - fix16_one / 12);
		}
		else {
			g_sVehicle.fDx = MIN(0, g_sVehicle.fDx + fix16_one / 12);
		}
	}

	// Limit X movement
	g_sVehicle.fX = CLAMP(g_sVehicle.fX + g_sVehicle.fDx, 0, fix16_one * (320 - g_sVehicle.sBob.uwWidth));
	g_sVehicle.sBob.sPos.sUwCoord.uwX = fix16_to_int(g_sVehicle.fX);
	UBYTE ubAdd = g_sVehicle.sBob.sPos.sUwCoord.uwY > (1 + TILE_ROW_GRASS) * 32 ? 4 : 8;
	UBYTE ubHalfWidth = 12;

	UWORD uwCenterX = g_sVehicle.sBob.sPos.sUwCoord.uwX + g_sVehicle.sBob.uwWidth / 2;
	UWORD uwTileBottom = (g_sVehicle.sBob.sPos.sUwCoord.uwY + g_sVehicle.sBob.uwHeight + ubAdd) >> 5;
	UWORD uwTileMid = (g_sVehicle.sBob.sPos.sUwCoord.uwY + g_sVehicle.sBob.uwHeight /2) >> 5;
	UWORD uwTileCenter = uwCenterX >> 5;
	UBYTE isTouchingLeft = 0, isTouchingRight = 0;
	UWORD uwTileLeft = (uwCenterX - ubHalfWidth) >> 5;
	UWORD uwTileRight = (uwCenterX + ubHalfWidth) >> 5;

	if(tileIsSolid(uwTileLeft, uwTileMid)) {
		g_sVehicle.fX = fix16_from_int(((uwTileLeft+1) << 5) - g_sVehicle.sBob.uwWidth / 2 + ubHalfWidth);
		isTouchingLeft = 1;
	}
	else if(tileIsSolid(uwTileRight, uwTileMid)) {
		g_sVehicle.fX = fix16_from_int((uwTileRight << 5) - g_sVehicle.sBob.uwWidth / 2 - ubHalfWidth);
		isTouchingRight = 1;
	}

	if(g_sVehicle.sSteer.bY < 0) {
		g_sVehicle.fDy = MAX(-3*fix16_one, g_sVehicle.fDy - fix16_one/12);
	}
	else {
		g_sVehicle.fDy = MIN(4*fix16_one, g_sVehicle.fDy + fix16_one/8);
	}

	if(g_sVehicle.fDy < 0) {
		UWORD uwTileTop = (fix16_to_int(g_sVehicle.fY) - 1) >> 5;
		// Flying
		g_sVehicle.fY = MAX(0, g_sVehicle.fY + g_sVehicle.fDy);
		if(tileIsSolid(uwTileCenter, uwTileTop)) {
			g_sVehicle.fY = fix16_from_int((uwTileTop+1) << 5);
			g_sVehicle.fDy = 0;
		}
	}
	else {
		if(!tileIsSolid(uwTileCenter, uwTileBottom)) {
			// Gravity
			g_sVehicle.fY += g_sVehicle.fDy;
		}
		else {
			// Collision with ground
			isOnGround = 1;
			g_sVehicle.fY = fix16_from_int((uwTileBottom << 5) - g_sVehicle.sBob.uwHeight - ubAdd);
			g_sVehicle.fDy = 0;
		}
	}
	g_sVehicle.sBob.sPos.sUwCoord.uwY = fix16_to_int(g_sVehicle.fY);

	if(isOnGround) {
		if(g_sVehicle.sSteer.bX > 0 && isTouchingRight) {
			// Drilling right
			tileExcavate(&g_sVehicle, uwTileRight, uwTileMid);
			if(uwTileMid == TILE_ROW_GRASS + 1) {
				// Drilling beneath a grass - refresh it
				tileRefreshGrass(uwTileRight);
			}
		}
		else if(g_sVehicle.sSteer.bX < 0 && isTouchingLeft) {
			// Drilling left
			tileExcavate(&g_sVehicle, uwTileLeft, uwTileMid);
			if(uwTileMid == TILE_ROW_GRASS + 1) {
				// Drilling beneath a grass - refresh it
				tileRefreshGrass(uwTileLeft);
			}
		}
		else if(
			g_sVehicle.sSteer.bY > 0 && tileIsSolid(uwTileCenter, uwTileBottom)
		) {
			// Drilling down
			// Move to center of tile
			g_sVehicle.fX = fix16_from_int(uwTileCenter << 5);
			tileExcavate(&g_sVehicle, uwTileCenter, uwTileBottom);
			g_sVehicle.fY += fix16_from_int(16);
			if(uwTileBottom == TILE_ROW_GRASS + 1) {
				// Drilling beneath a grass - refresh it
				tileRefreshGrass(uwTileCenter);
			}
		}
	}
	bobNewPush(&g_sVehicle.sBob);
}
