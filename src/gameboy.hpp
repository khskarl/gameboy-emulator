#ifndef GAMEBOY_HPP
#define GAMEBOY_HPP

#include "cpu.hpp"
#include "mmu.hpp"
#include "ppu.hpp"
#include "rom.hpp"

class GameBoy {
private:
	CPU cpu;
	MMU mmu;
	PPU ppu;

	// const uint32_t clockSpeed = 4194304; // Hz or Cycles per second
	// uint32_t clockFrequency = 4096;

	uint32_t timerCounter = 1024; // aka TIMA

	bool isHalted = true;

	bool joypad[8] = {0, 0, 0, 0,
										0, 0, 0, 0};

public:
	GameBoy ();
	~GameBoy ();

	void Initialize ();

	void LoadRom  (Rom rom);

	void StepEmulation (const uint32_t cyclesThisUpdate = 69905);
	void StepInstruction ();
	void StepTimers (uint32_t cycles);

	void ResetClockFrequency ();
	bool IsClockEnabled ();

	bool*    GetJoypadBuffer ();
	uint8_t* GetDisplayBuffer ();
	CPU* GetCPU ();
	MMU* GetMMU ();

	bool GetHalt ();
	void SetHalt (bool state);
	void ToggleHalt ();
};

#endif
