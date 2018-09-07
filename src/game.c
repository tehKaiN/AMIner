/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "game.h"
#include <ace/managers/key.h> // Keyboard processing
#include <ace/managers/game.h> // For using gameClose
#include <ace/managers/system.h> // For systemUnuse and systemUse
#include <ace/managers/viewport/simplebuffer.h> // Simple buffer
#include <ace/utils/palette.h>
#include <ace/utils/custom.h>
#include <ace/managers/blit.h>
#include <ace/managers/rand.h>
#include "bob_new.h"
#include "vehicle.h"

// All variables outside fns are global - can be accessed in any fn
// Static means here that given var is only for this file, hence 's_' prefix
// You can have many variables with same name in different files and they'll be
// independent as long as they're static
// * means pointer, hence 'p' prefix
static tView *s_pView; // View containing all the viewports
static tVPort *s_pVpScore; // Viewport for score
static tSimpleBufferManager *s_pScoreBuffer;
static tVPort *s_pVpMain; // Viewport for playfield
static tSimpleBufferManager *s_pMainBuffer;

static tBitMap *s_pTiles;

#define TILE_ROCK 0
#define TILE_GOLD 1
#define TILE_COPPER 2
#define TILE_COAL 3
#define TILE_DIAMOND 4
#define TILE_RUBY 5
#define TILE_DIRT 6

// TODO sapphire, emerald, topaz

// 32px: 1 << 5
#define TILE_SIZE 5

void gameGsCreate(void) {
  s_pView = viewCreate(0,
    TAG_VIEW_GLOBAL_CLUT, 1,
  TAG_END);

  s_pVpScore = vPortCreate(0,
    TAG_VPORT_VIEW, s_pView,
    TAG_VPORT_BPP, 4,
    TAG_VPORT_HEIGHT, 32,
  TAG_END);
  s_pScoreBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpScore,
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
  TAG_END);

  s_pVpMain = vPortCreate(0,
    TAG_VPORT_VIEW, s_pView,
    TAG_VPORT_BPP, 4,
  TAG_END);
  s_pMainBuffer = simpleBufferCreate(0,
    TAG_SIMPLEBUFFER_VPORT, s_pVpMain,
    TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_BOUND_HEIGHT, 1024,
		TAG_SIMPLEBUFFER_IS_DBLBUF, 1,
  TAG_END);

	s_pTiles = bitmapCreateFromFile("data/tiles.bm");
	paletteLoad("data/aminer.plt", s_pVpScore->pPalette, 16);
	paletteLoad("data/aminer.plt", s_pVpMain->pPalette, 16);

  // We don't need anything from OS anymore
  systemUnuse();
	randInit(2184);

	for(UBYTE x = 0; x < 10; ++x) {
		blitCopyAligned(
			s_pTiles, 0, TILE_DIRT << TILE_SIZE, s_pMainBuffer->pBack,
			x << TILE_SIZE, 2 << TILE_SIZE, 32, 32
		);
		blitCopyAligned(
			s_pTiles, 0, TILE_DIRT << TILE_SIZE, s_pMainBuffer->pFront,
			x << TILE_SIZE, 2 << TILE_SIZE, 32, 32
		);
		for(UBYTE y = 3; y < 32; ++y) {
			blitCopyAligned(
				s_pTiles, 0, TILE_ROCK << TILE_SIZE, s_pMainBuffer->pBack,
				x << TILE_SIZE, y << TILE_SIZE, 32, 32
			);
			blitCopyAligned(
				s_pTiles, 0, TILE_ROCK << TILE_SIZE, s_pMainBuffer->pFront,
				x << TILE_SIZE, y << TILE_SIZE, 32, 32
			);
		}
	}

	bobNewManagerCreate(
		1, VEHICLE_HEIGHT * (VEHICLE_WIDTH/16 + 1),
		s_pMainBuffer->pFront, s_pMainBuffer->pBack
	);
	vehicleCreate();

  // Load the view
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
	bobNewPushingDone();
	bobNewEnd();

	cameraCenterAt(
		s_pMainBuffer->pCameraManager,
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

  // This will also destroy all associated viewports and viewport managers
  viewDestroy(s_pView);
}
