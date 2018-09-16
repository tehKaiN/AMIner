/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/utils/palette.h>
#include <ace/utils/custom.h>
#include <ace/managers/blit.h>
#include <ace/managers/rand.h>
#include "bob_new.h"
#include "vehicle.h"
#include "hud.h"

static tView *s_pView;
static tVPort *s_pVpMain;
static tTileBufferManager *s_pMainBuffer;

static tBitMap *s_pTiles;

#define TILE_ROCK 0
#define TILE_GOLD 1
#define TILE_COPPER 2
#define TILE_COAL 3
#define TILE_DIAMOND 4
#define TILE_RUBY 5
#define TILE_DIRT 6

// TODO sapphire, emerald, topaz

void gameGsCreate(void) {
  s_pView = viewCreate(0,
    TAG_VIEW_GLOBAL_CLUT, 1,
  TAG_END);

	hudCreate(s_pView);
	s_pTiles = bitmapCreateFromFile("data/tiles.bm");

  s_pVpMain = vPortCreate(0,
    TAG_VPORT_VIEW, s_pView,
    TAG_VPORT_BPP, 4,
  TAG_END);
  s_pMainBuffer = tileBufferCreate(0,
		TAG_TILEBUFFER_VPORT, s_pVpMain,
		TAG_TILEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_TILEBUFFER_BOUND_TILE_X, 10,
		TAG_TILEBUFFER_BOUND_TILE_Y, 2047,
		TAG_TILEBUFFER_IS_DBLBUF, 1,
		TAG_TILEBUFFER_TILE_SHIFT, 5,
		TAG_TILEBUFFER_REDRAW_QUEUE_LENGTH, 100,
		TAG_TILEBUFFER_TILESET, s_pTiles,
  TAG_END);

	paletteLoad("data/aminer.plt", s_pVpMain->pPalette, 16);

  systemUnuse();
	randInit(2184);

	for(UWORD x = 0; x < s_pMainBuffer->uTileBounds.sUwCoord.uwX; ++x) {
		s_pMainBuffer->pTileData[x][0] = TILE_DIAMOND;
		s_pMainBuffer->pTileData[x][2] = TILE_DIRT;
		for(UWORD y = 3; y < s_pMainBuffer->uTileBounds.sUwCoord.uwY; ++y) {
			s_pMainBuffer->pTileData[x][y] = y % 6;
		}
	}

	bobNewManagerCreate(
		1, VEHICLE_HEIGHT * (VEHICLE_WIDTH/16 + 1),
		s_pMainBuffer->pScroll->pFront, s_pMainBuffer->pScroll->pBack,
		s_pMainBuffer->pScroll->uwBmAvailHeight
	);
	vehicleCreate();

  // Load the view
	tileBufferInitialDraw(s_pMainBuffer);
  viewLoad(s_pView);
}

static void gameProcessInput(void) {
	BYTE bDirX = 0, bDirY = 0;
	if(keyCheck(KEY_D)) {
		bDirX += 1;
	}
	if(keyCheck(KEY_A)) {
		bDirX -= 1;
	}
	if(keyCheck(KEY_S)) {
		bDirY += 1;
	}
	if(keyCheck(KEY_W)) {
		bDirY -= 1;
	}
	vehicleMove(bDirX, bDirY);
}

void gameGsLoop(void) {
	g_pCustom->color[0] = 0x800;
  if(keyCheck(KEY_ESCAPE)) {
    gameClose();
		return;
  }

	bobNewBegin();
	gameProcessInput();
	vehicleProcess();
	hudSetDepth(g_sVehicle.sBob.sPos.sUwCoord.uwY + VEHICLE_HEIGHT);
	bobNewPushingDone();
	bobNewEnd();
	hudUpdate();

	cameraCenterAt(
		s_pMainBuffer->pCamera,
		g_sVehicle.sBob.sPos.sUwCoord.uwX + VEHICLE_WIDTH/2,
		g_sVehicle.sBob.sPos.sUwCoord.uwY + VEHICLE_WIDTH/2
	);
	viewProcessManagers(s_pView);
	copProcessBlocks();
	g_pCustom->color[0] = 0x000;
	vPortWaitForEnd(s_pVpMain);
}

void gameGsDestroy(void) {
  // Cleanup when leaving this gamestate
  systemUse();

	bitmapDestroy(s_pTiles);
	vehicleDestroy();
	bobNewManagerDestroy();

  hudDestroy();
  viewDestroy(s_pView);
}
