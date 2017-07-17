#include <iostream>
#include <algorithm>

#include "ppu.hpp"
#include "cpu.hpp"
#include "debug.hpp"

PPU::PPU () {}
PPU::~PPU () {}

void PPU::Initialize(CPU* _cpu, MMU* _mmu) {
	assert("CPU reference invalid" && _cpu != nullptr);
	assert("MMU reference invalid" && _mmu != nullptr);
	cpu = _cpu;
	mmu = _mmu;

	// FeedRandomToBackground ();
	// FeedPatternToBackground ();
}

void PPU::SetDisplayStatus () {
	uint8_t status = mmu->ReadByte(LCDSTAT);
	// If display is disabled, set mode to 1 and reset scanline
	if (IsDisplayEnabled() == false) {
		ResetScanline();
		ResetScanlineCounter();

		status &= 252;
		status |= 0x1;
		mmu->WriteByte(LCDSTAT, status);
		return;
	}

	uint8_t currLine = mmu->ReadByte(CURLINE);
	uint8_t currMode = status & 0x3;

	uint8_t nextMode = 0;
	bool requestInterrupt = false;

	/* If in VBLANK, set mode to 1 */
	if (currLine >= 144) {
		nextMode = 1;
		status = (status & ~0x3) | 0x1;
		requestInterrupt = ((status & 0x10) == 0x10);
	}
	/* Mode 2 */
	else if (scanlineCounter >= 376) {
		nextMode = 2;
		status = (status & ~0x3) | 0x2;
		requestInterrupt = ((status & 0x20) == 0x20);
	}
	/* Mode 3 */
	else if (scanlineCounter >= 204) {
		nextMode = 3;
		status = (status & ~0x3) | 0x3;
	}
	/* Mode 0 */
	else {
		nextMode = 0;
		status = (status & ~0x3);
		requestInterrupt = ((status & 0x8) == 0x8);
	}

	// Request an interrupt if we changed modes
	if (requestInterrupt && nextMode != currMode) {
		cpu->RequestInterrupt(1);
	}

	// Coincidence Flag
	if (currLine == mmu->ReadByte(CMPLINE)) {
		status |= 0x4;
		if ((status & 0x20) == 0x20)
			cpu->RequestInterrupt(1);
	}
	else {
		status = (status & ~0x4);
	}

	mmu->WriteByte(LCDSTAT, status);
}

void PPU::StepUpdate (uint16_t cycles) {
	SetDisplayStatus();

	if (IsDisplayEnabled() == false)
		return;

	scanlineCounter -= cycles;

	if (scanlineCounter > 0)
		return;

	uint8_t currLine = NextScanline();

	ResetScanlineCounter();

	if (currLine == 144)
		cpu->RequestInterrupt(0);

	if (currLine > 153)
		ResetScanline();
	else if (currLine < 144) {
		DrawScanline(currLine);
	}
}

uint8_t* PPU::GetDisplayBuffer () {
	return &displayBuffer[0];
}

uint8_t CalculatePixelColorID (uint16_t line, uint8_t numBit) {
	return (line & (0x8000 >> numBit)) >> (14 - numBit) |
	       (line & (0x80   >> numBit)) >> (7  - numBit);
}

void PPU::DrawScanline (uint8_t line) {
	DrawBackground(line);
	DrawSprites(line);
}


void PPU::DrawBackground (uint8_t line) {
	uint8_t scrollY = mmu->ReadByte(SCROLLY);
	uint8_t scrollX = mmu->ReadByte(SCROLLX);
	uint16_t bgTilesMapAddress = GetBackgroundTilesAddress();
	uint16_t tilesAddress = GetTilesAddress();

	uint8_t iScrolled = scrollY + line;
	uint8_t iTile = (iScrolled) / 8;

	for (size_t jPixel = 0; jPixel < 160; jPixel += 1) {
		uint8_t jScrolled = scrollX + jPixel;
		uint8_t jTile = (jScrolled) / 8;

		uint16_t tileIdAddress = bgTilesMapAddress + iTile * 32 + jTile;
		uint8_t  rawTileID = mmu->ReadByte(tileIdAddress);

		uint8_t  tileID;
		uint16_t tileLocation;
		if (tilesAddress == 0x8000) {
			tileID = rawTileID;
			tileLocation = tilesAddress + tileID * 16;
		} else {
			tileID = reinterpret_cast<int8_t&>(rawTileID);
			tileLocation = tilesAddress + (tileID + 128) * 16;
		}

		uint16_t tileData = mmu->ReadWord(tileLocation + (iScrolled % 8) * 2);

		uint8_t currentBitColor = CalculatePixelColorID(tileData, jScrolled % 8);
		displayBuffer[line * 160 + jPixel] = GetShadeFromBGP(currentBitColor);
	}
}

void PPU::DrawSprites (uint8_t line) {

}

void PPU::FeedRandomToDisplay () {
	for (size_t i = 0; i < 160 * 144; i += 1) {
		int luminosity = rand() % 4;
		displayBuffer[i] = luminosity;
	}
}

uint16_t PPU::GetTilesAddress () {
	uint8_t lcdControl = mmu->ReadByte(LCDCTRL);
	if ((lcdControl & 0x10) == 0x10) {
		return 0x8000;
	} else {
		return 0x8800;
	}
}

uint16_t PPU::GetBackgroundTilesAddress () {
	uint8_t lcdControl = mmu->ReadByte(LCDCTRL);
	if ((lcdControl & 0x8) == 0x8) {
		return 0x9C00;
	} else {
		return 0x9800;
	}
}

uint8_t PPU::GetShadeFromBGP(uint8_t colorID) {
	uint8_t backgroundPalette = mmu->ReadByte(BGP);
	uint8_t offset = 2 * colorID;
	uint8_t mask = 0b11 << offset;
	return (backgroundPalette & mask) >> offset;
}

uint8_t PPU::NextScanline () {
	uint8_t* pScanline = mmu->GetMemoryRef(CURLINE);
	(*pScanline) += 1;
	return *pScanline;
}

void PPU::ResetScanline () {
	(*mmu->GetMemoryRef(CURLINE)) = 0;
}

void PPU::ResetScanlineCounter () {
	scanlineCounter = 456;
}

bool PPU::IsDisplayEnabled () {
	return (mmu->ReadByte(LCDCTRL) & 0x80) == 0x80;
}
