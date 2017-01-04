#include "gameboy.hpp"

GameBoy::GameBoy () {}
GameBoy::~GameBoy () {}

void GameBoy::Initialize () {
	cpu.Initialize(&mmu);
	mmu.Initialize();
	ppu.Initialize(&mmu);
}

void GameBoy::LoadBios (Rom bios) {
	mmu.WriteBios(bios.GetData());
}

void GameBoy::LoadRom (Rom rom) {
	mmu.WriteBufferToRom(rom.GetData(), rom.GetSize());
}

void GameBoy::StepUpdate () {
	size_t cyclesThisUpdate = 0;
	const int maxCycles = 4194304 / 65536;
	while (cyclesThisUpdate < maxCycles)
	{
		StepCycle();
		cyclesThisUpdate += cpu.clockCycles;
	}

	ppu.StepUpdate();
}

void GameBoy::StepCycle () {
	cpu.EmulateCycle();
}

uint8_t* GameBoy::GetDisplayBuffer () {
	return ppu.GetDisplayBuffer();
}
