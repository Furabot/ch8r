#include <cstdint>

class CHIP8
{
public:

	void emulateCycle();
	bool loadROM(const char * file);
	bool drawFlag;

	void CLIRender();

	std::uint8_t gfx[64 * 32];
	std::uint8_t key[16];



private:
	std::uint16_t opcode;
	std::uint8_t memory[4096];
	std::uint8_t V[16];
	std::uint16_t I;
	std::uint16_t pc;
	std::uint16_t stack[16];
	std::uint16_t sp;
	std::uint8_t delayTimer;
	std::uint8_t soundTimer;

	void reset();
};
