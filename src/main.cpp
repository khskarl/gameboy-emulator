#include <iostream>
#include <stdint.h>

#include "context/context.hpp"
#include "gameboy.hpp"
#include "disassembler.hpp"
#include "rom.hpp"

int main(int argc, char const *argv[]) {
	if (argc < 2) {
		std::cout << "<*Error*> You didn't supply a ROM path as argument\n" <<
		             "Usage example: ./build/gbemu.out roms/tetris.gb\n";
	}

	const char* filepath = argv[1];

	Rom rom(filepath);
	if (rom.isLoaded == false) {
		std::cout << "<*Error*> Failed to load: " << filepath << " \n";
		return 1;
	}

	if (rom.isLoaded) {
		std::cout << "[ROM DATA]" << '\n' <<
		             "Name: " << rom.GetName() << '\n' <<
		             "Size: " << rom.GetSize() << " B\n";
		// std::cout << DisassembleRom(rom) << '\n';
	}

	Rom bios("roms/bios.gb");

	GameBoy gameBoy;
	gameBoy.Initialize();
	gameBoy.LoadBios(bios);
	gameBoy.LoadRom(rom);

	Context::SetupContext(3);
	Context::SetTitle("MyGameBoy " + rom.GetName());
	Context::SetDisplayBuffer(gameBoy.ppu.GetDisplayBuffer());

	bool isHalted = true;
	while (Context::IsOpen()) {
		Context::HandleEvents();

		// Toggle halt state
		if (Context::ShouldHalt())
			isHalted ^= true;

		if (isHalted && Context::ShouldStep() == true)
			gameBoy.StepCycle();
		else if (isHalted && Context::ShouldStep() == false)
			continue;
		else
			gameBoy.StepUpdate();

		Context::RenderDisplay();
	}

	Context::DestroyContext();

	return 0;
}
