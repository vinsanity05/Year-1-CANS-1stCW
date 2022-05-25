
#include "stdafx.h"
#include <winsock2.h>

/*
* Author: Vince Verdadero (19009246)
* Created : 07 / 02 / 20
* Last edited : 19 / 02 / 20
* Description : Chimera - 2018 - J Emulator
*/

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "19009246"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500
#define MAX_BUFFER_SIZE   500

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer[MAX_BUFFER_SIZE];

char hex_file[MAX_BUFFER_SIZE];
char trc_file[MAX_BUFFER_SIZE];

//////////////////////////
//   Registers          //
//////////////////////////

#define FLAG_I  0x80
#define FLAG_N  0x20
#define FLAG_V  0x10
#define FLAG_Z  0x08
#define FLAG_C  0x01

#define REGISTER_M  7 //ADDED EXTRA REGISTER
#define REGISTER_A	6
#define REGISTER_H	5
#define REGISTER_L	4
#define REGISTER_E	3
#define REGISTER_D	2
#define REGISTER_C	1
#define REGISTER_B	0

WORD IndexRegister;

BYTE Registers[8]; // CHANGED FROM 7 TO 8
BYTE Flags;
WORD ProgramCounter;
WORD StackPointer;


////////////
// Memory //
////////////

#define MEMORY_SIZE	65536

BYTE Memory[MEMORY_SIZE];

#define TEST_ADDRESS_1  0x01FA
#define TEST_ADDRESS_2  0x01FB
#define TEST_ADDRESS_3  0x01FC
#define TEST_ADDRESS_4  0x01FD
#define TEST_ADDRESS_5  0x01FE
#define TEST_ADDRESS_6  0x01FF
#define TEST_ADDRESS_7  0x0200
#define TEST_ADDRESS_8  0x0201
#define TEST_ADDRESS_9  0x0202
#define TEST_ADDRESS_10  0x0203
#define TEST_ADDRESS_11  0x0204
#define TEST_ADDRESS_12  0x0205


///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;


///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][14] =
{
"BRA rel      ",
"BCC rel      ",
"BCS rel      ",
"BNE rel      ",
"BEQ rel      ",
"BVC rel      ",
"BVS rel      ",
"BMI rel      ",
"BPL rel      ",
"BGE rel      ",
"BLE rel      ",
"BLS rel      ",
"BHI rel      ",
"ILLEGAL     ",
"RTN impl     ",
"ILLEGAL     ",

"ST abs       ",
"PSH  ,A      ",
"POP A,       ",
"ILLEGAL     ",
"ILLEGAL     ",
"CLC impl     ",
"SEC impl     ",
"CLI impl     ",
"STI impl     ",
"SEV impl     ",
"CLV impl     ",
"DEX impl     ",
"INX impl     ",
"NOP impl     ",
"WAI impl     ",
"ILLEGAL     ",

"ST abs,X     ",
"PSH  ,s      ",
"POP s,       ",
"ILLEGAL     ",
"ILLEGAL     ",
"ADI  #       ",
"SBI  #       ",
"CPI  #       ",
"ANI  #       ",
"XRI  #       ",
"MVI  #,B     ",
"MVI  #,C     ",
"MVI  #,D     ",
"MVI  #,E     ",
"MVI  #,L     ",
"MVI  #,H     ",

"ILLEGAL     ",
"PSH  ,B      ",
"POP B,       ",
"JPR abs      ",
"CCC abs      ",
"CCS abs      ",
"CNE abs      ",
"CEQ abs      ",
"CVC abs      ",
"CVS abs      ",
"CMI abs      ",
"CPL abs      ",
"CHI abs      ",
"CLE abs      ",
"ILLEGAL     ",
"ILLEGAL     ",

"ILLEGAL     ",
"PSH  ,C      ",
"POP C,       ",
"TST abs      ",
"INC abs      ",
"DEC abs      ",
"RCR abs      ",
"RCL abs      ",
"SAL abs      ",
"ASR abs      ",
"NOT abs      ",
"ROL abs      ",
"ROR abs      ",
"ILLEGAL     ",
"LDX  #       ",
"LODS  #      ",

"STOX abs     ",
"PSH  ,D      ",
"POP D,       ",
"TST abs,X    ",
"INC abs,X    ",
"DEC abs,X    ",
"RCR abs,X    ",
"RCL abs,X    ",
"SAL abs,X    ",
"ASR abs,X    ",
"NOT abs,X    ",
"ROL abs,X    ",
"ROR abs,X    ",
"ILLEGAL     ",
"LDX abs      ",
"LODS abs     ",

"STOX abs,X   ",
"PSH  ,E      ",
"POP E,       ",
"TSTA A,A     ",
"INCA A,A     ",
"DECA A,A     ",
"RCRA A,A     ",
"RCLA A,A     ",
"SALA A,A     ",
"ASRA A,A     ",
"NOTA A,A     ",
"ROLA A,A     ",
"RORA A,A     ",
"ILLEGAL     ",
"LDX abs,X    ",
"LODS abs,X   ",

"ILLEGAL     ",
"PSH  ,L      ",
"POP L,       ",
"ILLEGAL     ",
"TAS impl     ",
"TSA impl     ",
"ILLEGAL     ",
"ILLEGAL     ",
"MOVE A,A     ",
"MOVE B,A     ",
"MOVE C,A     ",
"MOVE D,A     ",
"MOVE E,A     ",
"MOVE L,A     ",
"MOVE H,A     ",
"MOVE M,A     ",

"ILLEGAL     ",
"PSH  ,H      ",
"POP H,       ",
"ILLEGAL     ",
"ILLEGAL     ",
"SWI impl     ",
"RTI impl     ",
"ILLEGAL     ",
"MOVE A,B     ",
"MOVE B,B     ",
"MOVE C,B     ",
"MOVE D,B     ",
"MOVE E,B     ",
"MOVE L,B     ",
"MOVE H,B     ",
"MOVE M,B     ",

"ADC A,B      ",
"SBC A,B      ",
"CMP A,B      ",
"IOR A,B      ",
"AND A,B      ",
"XOR A,B      ",
"BT A,B       ",
"ILLEGAL     ",
"MOVE A,C     ",
"MOVE B,C     ",
"MOVE C,C     ",
"MOVE D,C     ",
"MOVE E,C     ",
"MOVE L,C     ",
"MOVE H,C     ",
"MOVE M,C     ",

"ADC A,C      ",
"SBC A,C      ",
"CMP A,C      ",
"IOR A,C      ",
"AND A,C      ",
"XOR A,C      ",
"BT A,C       ",
"ILLEGAL     ",
"MOVE A,D     ",
"MOVE B,D     ",
"MOVE C,D     ",
"MOVE D,D     ",
"MOVE E,D     ",
"MOVE L,D     ",
"MOVE H,D     ",
"MOVE M,D     ",

"ADC A,D      ",
"SBC A,D      ",
"CMP A,D      ",
"IOR A,D      ",
"AND A,D      ",
"XOR A,D      ",
"BT A,D       ",
"LD  #        ",
"MOVE A,E     ",
"MOVE B,E     ",
"MOVE C,E     ",
"MOVE D,E     ",
"MOVE E,E     ",
"MOVE L,E     ",
"MOVE H,E     ",
"MOVE M,E     ",

"ADC A,E      ",
"SBC A,E      ",
"CMP A,E      ",
"IOR A,E      ",
"AND A,E      ",
"XOR A,E      ",
"BT A,E       ",
"LD abs       ",
"MOVE A,L     ",
"MOVE B,L     ",
"MOVE C,L     ",
"MOVE D,L     ",
"MOVE E,L     ",
"MOVE L,L     ",
"MOVE H,L     ",
"MOVE M,L     ",

"ADC A,L      ",
"SBC A,L      ",
"CMP A,L      ",
"IOR A,L      ",
"AND A,L      ",
"XOR A,L      ",
"BT A,L       ",
"LD abs,X     ",
"MOVE A,H     ",
"MOVE B,H     ",
"MOVE C,H     ",
"MOVE D,H     ",
"MOVE E,H     ",
"MOVE L,H     ",
"MOVE H,H     ",
"MOVE M,H     ",

"ADC A,H      ",
"SBC A,H      ",
"CMP A,H      ",
"IOR A,H      ",
"AND A,H      ",
"XOR A,H      ",
"BT A,H       ",
"ILLEGAL     ",
"MOVE A,M     ",
"MOVE B,M     ",
"MOVE C,M     ",
"MOVE D,M     ",
"MOVE E,M     ",
"MOVE L,M     ",
"MOVE H,M     ",
"MOVE -,-     ",

"ADC A,M      ",
"SBC A,M      ",
"CMP A,M      ",
"IOR A,M      ",
"AND A,M      ",
"XOR A,M      ",
"BT A,M       ",
"ILLEGAL     ",
"ILLEGAL     ",
"ILLEGAL     ",
"JMP abs      ",
"ILLEGAL     ",
"ILLEGAL     ",
"ILLEGAL     ",
"ILLEGAL     ",
"ILLEGAL     ",

};

////////////////////////////////////////////////////////////////////////////////
//                           Simulator/Emulator (Start)                       //
////////////////////////////////////////////////////////////////////////////////
BYTE fetch()
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}

void set_flag_n(BYTE inReg) {
	BYTE reg;
	reg = inReg;

	if ((reg & 0x80) != 0) //msbit set 
	{
		Flags = Flags | FLAG_N;
	}
	else {
		Flags = Flags & (0xFF - FLAG_N);
	}
}

void set_flag_v(BYTE in1, BYTE in2, BYTE out1) {
	BYTE reg1in;
	BYTE reg2in;
	BYTE regOut;

	reg1in = in1;
	reg2in = in2;
	regOut = out1;

	if ((((reg1in & 0x80) == 0x80) && ((reg2in & 0x80) == 0x80) && ((regOut & 0x80) != 0x80)) ||
		(((reg1in & 0x80) != 0x80) && ((reg2in & 0x80) != 0x80) && ((regOut & 0x80) == 0x80))) {
		Flags = Flags | FLAG_V;
	}
	else {
		Flags = Flags & (0xFF - FLAG_V);
	}
}

void set_flag_z(BYTE inReg) {
	BYTE reg;
	reg = inReg;

	if (reg == 0) //zero set 
	{
		Flags = Flags | FLAG_Z;
	}
	else {
		Flags = Flags & (0xFF - FLAG_Z);
	}
}

void Group_1(BYTE opcode) {
	BYTE HB = 0;
	BYTE LB = 0;
	WORD address = 0;
	WORD data = 0;

	WORD temp_word;
	WORD param1;
	WORD param2;
	WORD offset;
	WORD saved_flags;


	switch (opcode) {
		/*----------------------------LOADING AND STORING SLIDE 1----------------------------*/
		/*
	*	Opcode: LD
	*	Description: Loads Memory into Accumulator
	*	Flags: - - T - T - - 0
	*/
	case 0xB7: //LD (#)
		data = fetch();
		Registers[REGISTER_A] = data;

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_C);
		break;
		break;

	case 0xC7://LD (abs)
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Registers[REGISTER_A] = Memory[address];
		}
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0xD7://LD(abs,X)
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Registers[REGISTER_A] = Memory[address];
		}
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*
	 *	Opcode: ST
	 *	Description: Stores Accumulator into Memory
	 *	Flags: - - T - T - - 0
		  */

	case 0x10://ST (abs)
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Registers[REGISTER_A];
		}
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x20://ST (abs,X)
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE)
		{
			Memory[address] = Registers[REGISTER_A];
		}
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*
	*	Opcode: MVI
	*	Description: Loads Memory into register
	*	Flags: - - T - T - - 0
	   */
	case 0x2A: //MVIB
		data = fetch();
		Registers[REGISTER_B] = data;
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x2B: //MVIC
		data = fetch();
		Registers[REGISTER_C] = data;
		set_flag_n(Registers[REGISTER_C]);
		set_flag_z(Registers[REGISTER_C]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x2C: //MVID
		data = fetch();
		Registers[REGISTER_D] = data;
		set_flag_n(Registers[REGISTER_D]);
		set_flag_z(Registers[REGISTER_D]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x2D: //MVIE
		data = fetch();
		Registers[REGISTER_E] = data;
		set_flag_n(Registers[REGISTER_E]);
		set_flag_z(Registers[REGISTER_E]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x2E: //MVIL
		data = fetch();
		Registers[REGISTER_L] = data;
		set_flag_n(Registers[REGISTER_L]);
		set_flag_z(Registers[REGISTER_L]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x2F: //MVIH
		data = fetch();
		Registers[REGISTER_H] = data;
		set_flag_n(Registers[REGISTER_H]);
		set_flag_z(Registers[REGISTER_H]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*
	*	Opcode: LODS
	*	Description: Loads Memory into Stackpointer
	*	Flags: - - T - T - - 0
	*/

	case 0x4F: //LODS (#)
		data = fetch();
		StackPointer = data << 8;
		StackPointer += fetch();
		break;

	case 0x5F: //LODS (abs)
		HB = fetch();
		LB = fetch(); address
			+= (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1)
		{
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}
		break;

	case 0x6F: //LODS (abs, X)
		address += IndexRegister;
		HB = fetch();
		LB = fetch(); address
			+= (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE - 1)
		{
			StackPointer = (WORD)Memory[address] << 8;
			StackPointer += Memory[address + 1];
		}
		break;

		/*
   *	Opcode: LDX
   *	Description: Loads Memory into register X
   *	Flags: - - T - T - - 0
   */

	case 0x4E: //LDX (#)
		data = fetch();
		Registers[IndexRegister] = data;
		set_flag_n(Registers[IndexRegister]);
		set_flag_z(Registers[IndexRegister]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x5E: //LDX (abs)
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[IndexRegister] = Memory[address];
		}
		set_flag_n(Registers[IndexRegister]);
		set_flag_z(Registers[IndexRegister]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x6E: //LDX (abs,X)
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Registers[IndexRegister] = Memory[address];
		}
		set_flag_n(Registers[IndexRegister]);
		set_flag_z(Registers[IndexRegister]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*
	*	Opcode: STOX
	*	Description: Stores register X into Memory
	*	Flags: - - T - T - - 0
	*/

	case 0x50: //STOX (abs)
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[IndexRegister];
		}

		set_flag_n(Registers[IndexRegister]);
		set_flag_z(Registers[IndexRegister]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x60: //STOX (abs, X)
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		if (address >= 0 && address < MEMORY_SIZE) {
			Memory[address] = Registers[IndexRegister];
		}

		set_flag_n(Registers[IndexRegister]);
		set_flag_z(Registers[IndexRegister]);
		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*-----------------------------------FLAGS SLIDE 2-----------------------------------*/
		/*
		*	Opcode: ADC
		*	Description: Register added to Accumulator with Carry
		*	Flags: - - T T T - - T
		*/

	case 0x90: // ADC A,B
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_B];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_B], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;


	case 0xA0: // ADC A,C
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_C];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_C], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xB0: // ADC A,D
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_D];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_D], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xC0: // ADC A,E
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_E];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_E], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xD0: // ADC A,L
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_L];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_L], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xE0: // ADC A,H
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_H];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_H], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xF0: // ADC A,M
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_M];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_M], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

		/*
	  *	Opcode: CMP
	  *	Description: Register compared to Accumulator
	  *	Flags: - - T T T - - T
	  */

	case 0x92: //CMP A,B
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_B];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry ﬂag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(param1, param2, (BYTE)temp_word);
		break;

	case 0xA2: //CMP A,C
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_C];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry ﬂag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(param1, param2, (BYTE)temp_word);
		break;

	case 0xB2: //CMP A,D
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_D];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry ﬂag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(param1, param2, (BYTE)temp_word);
		break;

	case 0xC2: //CMP A,E
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_E];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry ﬂag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(param1, param2, (BYTE)temp_word);
		break;

	case 0xD2: //CMP A,L
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_L];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry ﬂag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(param1, param2, (BYTE)temp_word);
		break;

	case 0xE2: //CMP A,H
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_H];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry ﬂag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(param1, param2, (BYTE)temp_word);
		break;

	case 0xF2: //CMP A,M
		param1 = Registers[REGISTER_A];
		param2 = Registers[REGISTER_M];
		temp_word = (WORD)param1 - (WORD)param2;
		if (temp_word >= 0x100) {
			Flags = Flags | FLAG_C; // Set carry ﬂag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(param1, param2, (BYTE)temp_word);
		break;

		/*
   *	Opcode: TAS
   *	Description: Transfers Accumulator to Status register
   *	Flags: - - - - - - - -
   */

	case 0x74: //TAS (Transfers Status register to Accumulator)
		Flags = Registers[REGISTER_A];
		break;

		/*
		   *Opcode: TSA
		   *Description: Transfers Status register to Accumulator
		   *Flags: - - - - - - - -
		   */

	case 0x75: //TSA  (Transfers Accumulator to Status register)
		Registers[REGISTER_A] = Flags;
		break;

		/*
	*	Opcode: CLC
	*	Description: Clear Carry flag
	*	Flags: - - - - - - - 0
	*	Notes: None
	*/
	case 0x15: //CLC clear carry flag
		Flags = Flags & (0xFF - FLAG_C);
		break;

		/*
	*	Opcode: SEC
	*	Description: Set Carry flag
	*	Flags: - - - - - - - 1
	*	Notes: None
	*/

	case 0x16: //SEC Set carry ﬂag
		Flags = Flags | FLAG_C;
		break;

		/*
   *	Opcode: CLI
   *	Description: Clear Interrupt flag
   *	Flags: 0 - - - - - - -
   *	Notes: None
   */

	case 0x17: //CLI Clear Interupt flag
		Flags = Flags & (0xFF - FLAG_I);
		break;

		/*
	*	Opcode: STI
	*	Description: Set Interrupt flag
	*	Flags: 1 - - - - - - -
	*	Notes: None
	*/

	case 0x18: //STI Set Interupt flag
		Flags = Flags | FLAG_I;
		break;

		/*
		*	Opcode: SEV
		*	Description: Set Overflow flag
		*	Flags: - - - 1 - - - -
		*	Notes: None
		*/

	case 0x19: //SEV Set Overflow flag
		Flags = Flags | FLAG_V;
		break;

		/*
	*	Opcode: CLV
	*	Description: Clear Overflow flag
	*	Flags: - - - 0 - - - -
	*	Notes: None
	*/

	case 0x1A: //CLV Clear Overflow flag
		Flags = Flags & (0xFF - FLAG_V);
		break;

		/*-----------------------------------STACK SLIDE 3-----------------------------------*/
	/*
	*	Opcode: PSH
	*	Description: Pushes Register onto the Stack
	*	Flags: - - - - - - - -
	*	Notes: None
	*/

	case 0x11://PSH A
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE)) {
			Memory[StackPointer] = Registers[REGISTER_A];
			StackPointer--;
		}
		break;

	case 0x21: // PSH FL (flags)     
		Memory[StackPointer] = Flags;  //Move the source data onto the top of the stack 
		StackPointer--;    //Decrement the stackpointer 
		break;

	case 0x31://PSH B
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE)) {
			Memory[StackPointer] = Registers[REGISTER_B];
			StackPointer--;
		}
		break;

	case 0x41://PSH C
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE)) {
			Memory[StackPointer] = Registers[REGISTER_C]; StackPointer--;
		}
		break;

	case 0x51://PSH D
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE)) {
			Memory[StackPointer] = Registers[REGISTER_D]; StackPointer--;
		}
		break;

	case 0x61://PSH E
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE)) {
			Memory[StackPointer] = Registers[REGISTER_E]; StackPointer--;
		}
		break;

	case 0x71://PSH L
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE)) {
			Memory[StackPointer] = Registers[REGISTER_L]; StackPointer--;
		}
		break;

	case 0x81://PSH H
		if ((StackPointer >= 1) && (StackPointer < MEMORY_SIZE)) {
			Memory[StackPointer] = Registers[REGISTER_H]; StackPointer--;
		}
		break;

		/*
	*	Opcode: POP
	*	Description: Pop the top of the Stack into the Register
	*	Flags: - - - - - - - -
	*/

	case 0x12://POP A
		if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1)) {
			StackPointer++;
			Registers[REGISTER_A] = Memory[StackPointer];
		}
		break;

	case 0x22: //POP FL (flags) 
		StackPointer++; //Increase the stackpointer          
		Flags = Memory[StackPointer]; //Pop off of the top of the stack into the variable  
		break;

	case 0x32://POP B
		if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1)) {
			StackPointer++;
			Registers[REGISTER_B] = Memory[StackPointer];
		}
		break;
	case 0x42://POP C
		if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1)) {
			StackPointer++;
			Registers[REGISTER_C] = Memory[StackPointer];
		}
		break;
	case 0x52://POP D
		if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1)) {
			StackPointer++;
			Registers[REGISTER_D] = Memory[StackPointer];
		}
		break;
	case 0x62://POP E
		if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1)) {
			StackPointer++;
			Registers[REGISTER_E] = Memory[StackPointer];
		}
		break;
	case 0x72://POP L
		if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1)) {
			StackPointer++;
			Registers[REGISTER_L] = Memory[StackPointer];
		}
		break;
	case 0x82://POP H
		if ((StackPointer >= 0) && (StackPointer < MEMORY_SIZE - 1)) {
			StackPointer++;
			Registers[REGISTER_H] = Memory[StackPointer];
		}
		break;

		/*
  *	Opcode: JMP
  *	Description: Load Memory into ProgramCounter
  *	Flags: - - - - - - - -
  */

	case 0xFA: // JMP abs 
		HB = fetch();
		LB = fetch();
		address = ((WORD)HB << 8) + (WORD)LB;
		ProgramCounter = address;
		break;

		/*
   *	Opcode: JPR
   *	Description: Jump to subroutine
   *	Flags: - - - - - - - -
   */

	case 0x33: // JPR abs
		HB = fetch();
		LB = fetch();
		address = ((WORD)HB << 8) + (WORD)LB;
		if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
		{
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF); StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF); StackPointer--;
		}
		ProgramCounter = address;
		break;

		/*
	*	Opcode: RTN
	*	Description: Return from subroutine
	*	Flags: - - - - - - - -
	*/

	case 0x0E:// RTN
		StackPointer++;
		HB = Memory[StackPointer];
		StackPointer++; LB = Memory[StackPointer];
		ProgramCounter = ((WORD)HB << 8) + (WORD)LB;
		break;

		/*
  *	Opcode: BRA
  *	Description: Branch always
  *	Flags: - - - - - - - -
  */

	case 0x00: //BRA
		LB = fetch();
		offset = (WORD)LB;
		if ((offset & 0x80) != 0)
		{
			offset + 0xFF00;
		}
		address = ProgramCounter + offset;
		break;

		/*
   *	Opcode: BCC
   *	Description: Branch on Carry clear
   *	Flags: - - - - - - - -
   */

	case 0x01: //BCC carry clear
		LB = fetch();
		if ((Flags & FLAG_C) == 0) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) {
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*
	*	Opcode: BCS
	*	Description: Branch on Carry set
	*	Flags: - - - - - - - -
	*/

	case 0x02: //BCS carry set
		LB = fetch();
		if ((Flags & FLAG_C) == FLAG_C) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) {
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*
   *	Opcode: BNE
   *	Description: Branch on Result not Zero
   *	Flags: - - - - - - - -
   */

	case 0x03: //BNE result not zero
		LB = fetch();
		if ((Flags & FLAG_Z) != 0) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) {
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*
	 *	Opcode: BEQ
	 *	Description: Branch on Result equal to Zero
	 *	Flags: - - - - - - - -
	 */

	case 0x04: //BEQ result equal to zero
		LB = fetch();
		if ((Flags & FLAG_Z) == 0) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) {
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*
   *	Opcode: BVC
   *	Description: Branch on Overflow clear
   *	Flags: - - - - - - - -
   */

	case 0x05: //BVC overflow clear
		LB = fetch();
		if ((Flags & FLAG_V) != FLAG_V) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) {
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;
		/*
   *	Opcode: BVS
   *	Description: Branch on Overflow set
   *	Flags: - - - - - - - -
   */

	case 0x06: //BVS overflow set
		LB = fetch();
		if ((Flags & FLAG_V) == FLAG_V) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) {
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*
	*	Opcode: BMI
	*	Description: Branch on negative result
	*	Flags: - - - - - - - -
	*/

	case 0x07: //BMI negative set
		LB = fetch();
		if ((Flags & FLAG_N) == FLAG_N) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) {
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*
	  *	Opcode: BPL
	  *	Description: Branch on positive result
	  *	Flags: - - - - - - - -
	  */

	case 0x08: //BPL positive set
		LB = fetch();
		if ((Flags & FLAG_N) != FLAG_N) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) {
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*
	   *	Opcode: BGE
	   *	Description: Branch on result less than or equal to zero
	   *	Flags: - - - - - - - -
	   */

	case 0x09: //BGE result less then or equal to zero
		LB = fetch();
		if ((Flags & FLAG_Z) <= 0) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) {
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*
	*	Opcode: BLE
	*	Description: Branch on result greater than or equal to zero
	*	Flags: - - - - - - - -
	*/

	case 0x0A: //BLE result greater then or equal to zero
		LB = fetch();
		if ((Flags & FLAG_Z) >= 0) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) {
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*
	*	Opcode: BLS
	*	Description: Branch on result same or lower
	*	Flags: - - - - - - - -
	*/

	case 0x0B: //BLS result same or lower
		LB = fetch();
		if ((((Flags & FLAG_C) | ((Flags & FLAG_Z))) != 0)) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) {
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*
	*	Opcode: BHI
	*	Description: Branch on result higher
	*	Flags: - - - - - - - -
	*/

	case 0x0C: //BHI result higher
		LB = fetch();
		if ((((Flags & FLAG_C) | ((Flags & FLAG_Z))) == 0)) {
			offset = (WORD)LB;
			if ((offset & 0x80) != 0) {
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

		/*
	   *	Opcode: CCC
	   *	Description: Call on Carry clear
	   *	Flags: - - - - - - - -
	   */

	case 0x34: //CCC
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_C) == 0) // Call on Carry clear
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				// Push return address onto the stack 
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*
	*	Opcode: CCS
	*	Description: Call on Carry set
	*	Flags: - - - - - - - -
	*/

	case 0x35: //CCS
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_C) == FLAG_C) // Call on Carry set
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				// Push return address onto the stack 
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*
  *	Opcode: CNE
  *	Description: Call on result not Zero
  *	Flags: - - - - - - - -
  */

	case 0x36: //CNE
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_Z) != 0) // Call on result not zero
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				// Push return address onto the stack 
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*
   *	Opcode: CEQ
   *	Description: Call on result equal to Zero
   *	Flags: - - - - - - - -
   *	Notes: None
   */

	case 0x37: //CEQ
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_Z) == 0) // Call on result equal to zero 
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				// Push return address onto the stack 
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*
   *	Opcode: CVC
   *	Description: Call on Overflow clear
   *	Flags: - - - - - - - -
   */

	case 0x38: //CVC
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_V) != FLAG_V) // Call on overflow clear 
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				// Push return address onto the stack 
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*
	*	Opcode: CVS
	*	Description: Call on Overflow set
	*	Flags: - - - - - - - -
	*	Notes: None
	*/

	case 0x39: //CVS 
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_V) == FLAG_V) // call on Overflow set
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				// Push return address onto the stack 
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*
	*	Opcode: CMI
	*	Description: Call on negative result
	*	Flags: - - - - - - - -
	*/

	case 0x3A: //CMI
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_N) == FLAG_N) // Call on negative result
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				// Push return address onto the stack 
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*
	*	Opcode: CPL
	*	Description: Call on positive result
	*	Flags: - - - - - - - -
	*/

	case 0x3B: //CPL
		HB = fetch();
		LB = fetch();
		if ((Flags & FLAG_N) != FLAG_N) // Call on positive result
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				// Push return address onto the stack 
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*
	*	Opcode: CHI
	*	Description: Call on result same or lower
	*	Flags: - - - - - - - -
	*/

	case 0x3C: //CHI 
		HB = fetch();
		LB = fetch();
		if ((((Flags & FLAG_C) | ((Flags & FLAG_Z))) != 0)) // Call on result same or lower
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				// Push return address onto the stack 
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*
	*	Opcode: CLE
	*	Description: Call on result higher
	*	Flags: - - - - - - - -
	*	Notes: None
	*/

	case 0x3D: //CLE
		HB = fetch();
		LB = fetch();
		if ((((Flags & FLAG_C) | ((Flags & FLAG_Z))) == 0)) // Call on result higher
		{
			address += (WORD)((WORD)HB << 8) + LB;
			if (address >= 0 && address < MEMORY_SIZE)
			{
				// Push return address onto the stack 
				if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))
				{
					Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
					StackPointer--;
					Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
					StackPointer--;
				}
				ProgramCounter = (WORD)address;
			}
		}
		break;

		/*-------------------------------INC AND LOGIC SLIDE 4-------------------------------*/
	/*
	*	Opcode: INC
	*	Description: Increment Memory or Accumulator
	*	Flags: - - T - T - - -
	*	Notes: None
	*/

	case 0x44: // INC Absolute (abs)
		++Registers[REGISTER_A];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0x54: // INC Absolute (abs,x)
		++Registers[REGISTER_A];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

		/*
	   *	Opcode: INCA
	   *	Description: Increment Memory or Accumulator
	   *	Flags: - - T - T - - -
	   */
	case 0x64: // INCA
		++Registers[REGISTER_A];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

		/*
	*	Opcode: INX
	*	Description: Increments register X
	*	Flags: - - T - - - - -
	*/

	case 0x1C: // INX
		++IndexRegister;
		set_flag_z(IndexRegister);
		break;

		/*
   *	Opcode: DEX
   *	Description: Decrements register X
   *	Flags: - - T - - - - -
   */

	case 0x1B: // DEX
		--IndexRegister;
		set_flag_z(IndexRegister);
		break;

		/*
	  *   Opcode: AND
	  *   Description: Register bitwise and with Accumulator
	  *	Flags: - - T - T - - -
	  */

	case 0x94: // AND A,B
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_B];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xA4: // AND A,C
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_C];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xB4: // AND A,D
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_D];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xC4: // AND A,E
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_E];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xD4: // AND A,L
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_L];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xE4: // AND A,H
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_H];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xF4: // AND A,M
		Registers[REGISTER_A] = Registers[REGISTER_A] & Registers[REGISTER_M];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

		/*
	*   Opcode: BT
	*   Description: Register Bit tested with Accumulator
	*	Flags: - - T - T - - -
	*/
	case 0x96: // BT A,B
		data = Registers[REGISTER_A] & Registers[REGISTER_B];
		set_flag_n(data);
		set_flag_z(data);
		break;

	case 0xA6: // BT A,C
		data = Registers[REGISTER_A] & Registers[REGISTER_C];
		set_flag_n(data);
		set_flag_z(data);
		break;

	case 0xB6: // BT A,D
		data = Registers[REGISTER_A] & Registers[REGISTER_D];
		set_flag_n(data);
		set_flag_z(data);
		break;

	case 0xC6: // BT A,E
		data = Registers[REGISTER_A] & Registers[REGISTER_E];
		set_flag_n(data);
		set_flag_z(data);
		break;

	case 0xD6: // BT A,L
		data = Registers[REGISTER_A] & Registers[REGISTER_L];
		set_flag_n(data);
		set_flag_z(data);
		break;

	case 0xE6: // BT A,H
		data = Registers[REGISTER_A] & Registers[REGISTER_H];
		set_flag_n(data);
		set_flag_z(data);
		break;

	case 0xF6: // BT A,M
		data = Registers[REGISTER_A] & Registers[REGISTER_M];
		set_flag_n(data);
		set_flag_z(data);
		break;

		/*---------------------------------ARITHMATIC SLIDE 5 --------------------------------*/
		  /*
		*   Opcode: SBC
		*   Description: Register subtracted to Accumulator with Carry
		*	Flags: - - T T T - - T
		*/

	case 0x91: // SBC A,B
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_B];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_B], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;


	case 0xA1: // SBC A,C
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_C];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_C], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xB1: // SBC A,D
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_D];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_D], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xC1: // SBC A,E
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_E];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_E], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xD1: // SBC A,L
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_L];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_L], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xE1: // SBC A,H
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)Registers[REGISTER_H];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_H], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xF1: // SBC A,M
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_M];

		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry ﬂag 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry ﬂag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A], Registers[REGISTER_M], (BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

		/*
	*   Opcode: IOR
	*   Description: Register bitwise inclusive or with Accumulator
	*	Flags: - - T - T - - -
	*/

	case 0x93: // IOR A,B
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_B];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xA3: // IOR A,C
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_C];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xB3: // IOR A,D
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_D];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xC3: // IOR A,E
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_E];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xD3: // IOR A,L
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_L];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xE3: // IOR A,H
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_H];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xF3: // IOR A,M
		Registers[REGISTER_A] = Registers[REGISTER_A] | Registers[REGISTER_M];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

		/*
	*   Opcode: XOR
	*   Description: Register bitwise exclusive or with Accumulator
	*	Flags: - - T - T - - -
	*/

	case 0x95: //XOR A-B
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_B];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xA5: //XOR A-C
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_C];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xB5: //XOR A-D
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_D];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xC5: //XOR A-E
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_E];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xD5: //XOR A-L
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_L];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xE5: //XOR A-H
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_H];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0xF5: //XOR A-M
		Registers[REGISTER_A] = Registers[REGISTER_A] ^ Registers[REGISTER_M];
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

		/*
	*	Opcode: NOTA
	*	Description: Negate Memory or Accumulator
	*	Flags: - - T - T - - T
	*/

	case 0x6A: //NOTA
		break;

		/*
  *	Opcode: DEC
  *	Description: Decrement Memory or Accumulator
  *	Flags: - - T - T - - -
  */

	case 0x45: //DEC (abs) 
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;

		Registers[REGISTER_A]--;
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;

	case 0x55: //DEC (abs, X)
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;

		Registers[REGISTER_A]--;
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;

		/*
	*	Opcode: DECA
	*	Description: Decrement Memory or Accumulator
	*	Flags: - - T - T - - -
	*/

	case 0x65: //DECA
		Registers[REGISTER_A]--;
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;

		/*
   *	Opcode: TST
   *	Description: Bit test Memory or Accumulator
   *	Flags: - - T - T - - -
   */

	case 0x43: //TST (abs) 
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;

		Registers[REGISTER_A]--;
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;

	case 0x53: //TST (abs, X)
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;

		Registers[REGISTER_A]--;
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;

		/*
	*	Opcode: TSTA
	*	Description: Bit test Memory or Accumulator
	*	Flags: - - T - T - - -
	*/

	case 0x63: //TSTA
		Registers[REGISTER_A]--;
		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;

		/*
		*	Opcode: SAL
		*	Description: Arithmetic shift left Memory or Accumulator
		*	Flags: - - T - T - - -
		*/
	case 0x48: //SAL (abs) 
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		break;

	case 0x58: //SAL (abs, X)
		address += IndexRegister;
		HB = fetch();
		LB = fetch();
		address += (WORD)((WORD)HB << 8) + LB;
		break;

		/*
		  *	Opcode: SALA
		  *	Description: Arithmetic shift left Memory or Accumulator
		  *	Flags: - - T - T - - T
		  *	Notes: None
		  */

	case 0x68: // SALA
		if ((Registers[REGISTER_A] & 0x01) == 0x01) {
			Flags = Flags | FLAG_C;
		}
		else {
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0x7F;

		if ((Flags & FLAG_N) == FLAG_N) {
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x80;
		}
		break;

		/*
	*	Opcode: ASR
	*	Description: Arithmetic shift right Memory or Accumulator
	*	Flags: - - T - T - - -
	*	Notes: None
	*/

	case 0x49: //ASR (abs) 

		break;

	case 0x59: //ASR  (abs, X)

		break;

		/*
	*	Opcode: ASRA
	*	Description: Arithmetic shift right Memory or Accumulator
	*	Flags: - - T - T - - T
	*	Notes: None
	*/

	case 0x69: //ASRA
		if ((Registers[REGISTER_A] & 0x01) == 0x01) {
			Flags = Flags | FLAG_C;
		}
		else {
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0x7F;

		if ((Flags & FLAG_N) == FLAG_N) {
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x80;
		}

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

		/*-----------------------------------ROTATE SLIDE 6----------------------------------*/
	/*
	*	Opcode: RCLA
	*	Description: Rotate left through carry Memory or Accumulator
	*	Flags: - - T - T - - T
	*/
	case 0x67: // RCLA
		saved_flags = Flags;

		if ((Registers[REGISTER_A] & 0x80) == 0x80) {
			Flags = Flags | FLAG_C;
		}
		else {
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;

		if ((saved_flags & FLAG_C) == FLAG_C) {
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
		}

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

		/*
	*	Opcode: WAI
	*	Description: Wait for interupt
	*	Flags: - - - - - - -
	*/

	case 0x1E:  // WAI
		halt = true;
		break;


	}



}




void Group_2_Move(BYTE opcode) {

	BYTE source = opcode >> 4;
	BYTE dest = opcode & 0x0F;
	int destReg;
	int sourceReg;
	WORD address;

	/*---------------------------------MOVE DEST---------------------------------*/

	switch (dest) {
	case 0x08:
		destReg = REGISTER_A;
		break;

	case 0x09:
		destReg = REGISTER_B;
		break;

	case 0x0A:
		destReg = REGISTER_C;
		break;

	case 0x0B:
		destReg = REGISTER_D;
		break;

	case 0x0C:
		destReg = REGISTER_E;
		break;

	case 0x0D:
		destReg = REGISTER_L;
		break;

	case 0x0E:
		destReg = REGISTER_H;
		break;

	case 0x0F:
		destReg = REGISTER_M;
		break;
	}
	/*--------------------------------MOVE SOURCE--------------------------------*/

	switch (source) {
	case 0x07:
		sourceReg = REGISTER_A;
		break;
	case 0x08:
		sourceReg = REGISTER_B;
		break;
	case 0x09:
		sourceReg = REGISTER_C;
		break;
	case 0x0A:
		sourceReg = REGISTER_D;
		break;
	case 0x0B:
		sourceReg = REGISTER_E;
		break;
	case 0x0C:
		sourceReg = REGISTER_L;
		break;
	case 0x0D:
		sourceReg = REGISTER_H;
		break;
	case 0x0E:
		sourceReg = REGISTER_M;
		break;
	}

	if (sourceReg == REGISTER_M)
	{
		address = Registers[REGISTER_L];
		address += (WORD)Registers[REGISTER_H] << 8;
		Registers[REGISTER_M] = Memory[address];
	}

	Registers[destReg] = Registers[sourceReg];
	if (destReg == REGISTER_M)
	{
		address = Registers[REGISTER_L];
		address += (WORD)Registers[REGISTER_H] << 8;
		Memory[address] = Registers[REGISTER_M];
	}
}


void execute(BYTE opcode)
{
	if (((opcode >= 0x78) && (opcode <= 0x7F))
		|| ((opcode >= 0x88) && (opcode <= 0x8F))
		|| ((opcode >= 0x98) && (opcode <= 0x9F))
		|| ((opcode >= 0xA8) && (opcode <= 0xAF))
		|| ((opcode >= 0xB8) && (opcode <= 0xBF))
		|| ((opcode >= 0xC8) && (opcode <= 0xCF))
		|| ((opcode >= 0xD8) && (opcode <= 0xDF))
		|| ((opcode >= 0xE8) && (opcode <= 0xEF)))
	{
		Group_2_Move(opcode);
	}
	else
	{
		Group_1(opcode);
	}
}



void emulate()
{
	BYTE opcode;
	int sanity;
	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;
	sanity = 0;
	printf("                    A  B  C  D  E  L  H  X    SP\n");

	while ((!halt) && (memory_in_range)) {
		sanity++;
		if (sanity > 500) halt = true;
		printf("%04X ", ProgramCounter);           // Print current address
		opcode = fetch();
		execute(opcode);

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);
		printf("%02X ", Registers[REGISTER_B]);
		printf("%02X ", Registers[REGISTER_C]);
		printf("%02X ", Registers[REGISTER_D]);
		printf("%02X ", Registers[REGISTER_E]);
		printf("%02X ", Registers[REGISTER_L]);
		printf("%02X ", Registers[REGISTER_H]);
		printf("%04X ", IndexRegister);
		printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_I) == FLAG_I)
		{
			printf("I=1 ");
		}
		else
		{
			printf("I=0 ");
		}
		if ((Flags & FLAG_N) == FLAG_N)
		{
			printf("N=1 ");
		}
		else
		{
			printf("N=0 ");
		}
		if ((Flags & FLAG_V) == FLAG_V)
		{
			printf("V=1 ");
		}
		else
		{
			printf("V=0 ");
		}
		if ((Flags & FLAG_Z) == FLAG_Z)
		{
			printf("Z=1 ");
		}
		else
		{
			printf("Z=0 ");
		}
		if ((Flags & FLAG_C) == FLAG_C)
		{
			printf("C=1 ");
		}
		else
		{
			printf("C=0 ");
		}

		printf("\n");  // New line
	}

	printf("\n");  // New line
}


////////////////////////////////////////////////////////////////////////////////
//                            Simulator/Emulator (End)                        //
////////////////////////////////////////////////////////////////////////////////


void initialise_filenames() {
	int i;

	for (i = 0; i < MAX_FILENAME_SIZE; i++) {
		hex_file[i] = '\0';
		trc_file[i] = '\0';
	}
}




int find_dot_position(char* filename) {
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		if (chr == '.') {
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char* filename) {
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0') {
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char* filename) {
	bool exists;
	FILE* ifp;

	exists = false;

	if ((ifp = fopen(filename, "r")) != NULL) {
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char* filename) {
	FILE* ofp;

	if ((ofp = fopen(filename, "w")) != NULL) {
		fclose(ofp);
	}
}



bool getline(FILE* fp, char* buffer) {
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect) {
		c = getc(fp);

		switch (c) {
		case EOF:
			if (i > 0) {
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0) {
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}






void load_and_run(int args, _TCHAR** argv) {
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE* ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	if (args == 2) {
		ln = 0;
		chr = argv[1][ln];
		while (chr != '\0')
		{
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file[ln] = chr;
				trc_file[ln] = chr;
				ln++;
			}
			chr = argv[1][ln];
		}
	}
	else {
		ln = 0;
		chr = '\0';
		while (chr != '\n') {
			chr = getchar();

			switch (chr) {
			case '\n':
				break;
			default:
				if (ln < MAX_FILENAME_SIZE) {
					hex_file[ln] = chr;
					trc_file[ln] = chr;
					ln++;
				}
				break;
			}
		}

	}
	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0) {
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	}
	else {
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0) {
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	}
	else {
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file)) {
		// Clear Registers and Memory

		Registers[REGISTER_A] = 0;
		Registers[REGISTER_B] = 0;
		Registers[REGISTER_C] = 0;
		Registers[REGISTER_D] = 0;
		Registers[REGISTER_E] = 0;
		Registers[REGISTER_L] = 0;
		Registers[REGISTER_H] = 0;
		IndexRegister = 0;
		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;

		for (i = 0; i < MEMORY_SIZE; i++) {
			Memory[i] = 0x00;
		}

		// Load hex file

		if ((ifp = fopen(hex_file, "r")) != NULL) {
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer)) {
				if (sscanf(InputBuffer, "L=%x", &address) == 1) {
					load_at = address;
				}
				else if (sscanf(InputBuffer, "%x", &code) == 1) {
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				}
				else {
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate

		emulate();
	}
	else {
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}

void building(int args, _TCHAR** argv) {
	char buffer[1024];
	load_and_run(args, argv);
	sprintf(buffer, "0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X",
		Memory[TEST_ADDRESS_1],
		Memory[TEST_ADDRESS_2],
		Memory[TEST_ADDRESS_3],
		Memory[TEST_ADDRESS_4],
		Memory[TEST_ADDRESS_5],
		Memory[TEST_ADDRESS_6],
		Memory[TEST_ADDRESS_7],
		Memory[TEST_ADDRESS_8],
		Memory[TEST_ADDRESS_9],
		Memory[TEST_ADDRESS_10],
		Memory[TEST_ADDRESS_11],
		Memory[TEST_ADDRESS_12]
	);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));
}



void test_and_mark() {
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;
	int  passed;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));

	while (!testing_complete) {
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (SOCKADDR*)&client_addr, &len) != SOCKET_ERROR) {
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 1)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1) {
				testing_complete = true;
				printf("Current mark = %d\n", mark);

			}
			else if (sscanf(buffer, "Tests passed %d", &passed) == 1) {
				//testing_complete = true;
				printf("Passed = %d\n", passed);

			}
			else if (strcmp(buffer, "Error") == 0) {
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			}
			else {
				// Clear Registers and Memory

				Registers[REGISTER_A] = 0;
				Registers[REGISTER_B] = 0;
				Registers[REGISTER_C] = 0;
				Registers[REGISTER_D] = 0;
				Registers[REGISTER_E] = 0;
				Registers[REGISTER_L] = 0;
				Registers[REGISTER_H] = 0;
				IndexRegister = 0;
				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i = 0; i < MEMORY_SIZE; i++) {
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				FILE* ofp;
				fopen_s(&ofp, "branch.txt", "a");

				while (!end_of_program) {
					chr = buffer[i];
					switch (chr) {
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1) {
							load_at = address;
						}
						else if (sscanf(InputBuffer, "%x", &code) == 1) {
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE)) {
								Memory[load_at] = (BYTE)code;
								fprintf(ofp, "%02X\n", (BYTE)code);
							}
							load_at++;
						}
						else {
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}
				fclose(ofp);
				// Emulate

				if (load_at > 1) {
					emulate();
					// Send and store results
					sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X",
						Memory[TEST_ADDRESS_1],
						Memory[TEST_ADDRESS_2],
						Memory[TEST_ADDRESS_3],
						Memory[TEST_ADDRESS_4],
						Memory[TEST_ADDRESS_5],
						Memory[TEST_ADDRESS_6],
						Memory[TEST_ADDRESS_7],
						Memory[TEST_ADDRESS_8],
						Memory[TEST_ADDRESS_9],
						Memory[TEST_ADDRESS_10],
						Memory[TEST_ADDRESS_11],
						Memory[TEST_ADDRESS_12]
					);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock) {
		// Creation failed! 
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		if (argc == 2) { building(argc, argv); exit(0); }
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run(argc, argv);
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();


	return 0;
}