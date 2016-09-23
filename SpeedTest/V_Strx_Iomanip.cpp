#include "V_Strx_Iomanip.h"

/*
// Flag-Werte, übernommen vom IOS Objekt, noch nicht alle sind
// implementiert.
enum
{
	IOM_null		=	0x0000,
	IOM_skipws		=	0x0001,
    IOM_left		=	0x0002,
    IOM_right		=	0x0004,
    IOM_internal	=	0x0008,
    IOM_dec			=	0x0010,
    IOM_oct			=	0x0020,
    IOM_hex			=	0x0040,
    IOM_showbase	=	0x0080,
    IOM_showpoint	=	0x0100,
    IOM_uppercase	=	0x0200,
    IOM_showpos		=	0x0400,
    IOM_scientific	=	0x0800,
    IOM_fixed		=	0x1000,
    IOM_unitbuf		=	0x2000,
    IOM_stdio		=	0x4000
};
*/
/*
enum
{
	IOMID_null		=	0x0000,
	IOMID_flag		=	0x0001,			// Manipulator-Flag für Ausgaben (z.B. dez, hex etc)
	IOMID_clear		=	0x0002,			// Löschen des Puffers
	IOMID_endl		=	0x0003,			// Stellt CR/LF in den Puffer
	IOMID_last
}VSTR_MANIP_ID;
*/

//*****************************************************************************
// Manipulatoren, die Ausgabeflags setzen
//*****************************************************************************
C_StreamIoManipulator	VSTR_MANIP_HEX(IOM_hex,IOMID_flag);
C_StreamIoManipulator	VSTR_MANIP_DEC(IOM_dec,IOMID_flag);
C_StreamIoManipulator	VSTR_MANIP_OCT(IOM_oct,IOMID_flag);
C_StreamIoManipulator	VSTR_MANIP_CMPCASE(IOM_comp_case,IOMID_flag);
C_StreamIoManipulator	VSTR_MANIP_CMPNOCASE(IOM_comp_nocase,IOMID_flag);

//*****************************************************************************
// Maniopulatoren, die direkte Aktionen auslösen (löschen, LF etc.)
//*****************************************************************************
// Löschen des Puffers
C_StreamIoManipulator	VSTR_MANIP_CLEAR(IOM_null,IOMID_clear);
// CR/LF in den Puffer stellen
C_StreamIoManipulator	VSTR_MANIP_ENDL(IOM_null,IOMID_endl);

/*
bool C_StreamIoManipulator::IsFlag(void)
{
	if(m_ManipId==IOMID_flag)
		return true;
	return false;
};
*/


//*****************************************************************************
// Der zugewiesene Manipulator 
//*****************************************************************************
C_StreamIoFlag& C_StreamIoFlag::operator=(C_StreamIoManipulator& ToSet)
{
	// Kein Flag -> nix zu setzen!
	if(!ToSet.IsFlag())
		return *this;

	// Bei der Zuweisung werden einzelne Bits gesetzt, in der Regel ohne 
	// die anderen zu manipulieren. Bei bestimmten Bits müssen einander 
	// ausschließende Flags gelöscht werden.
	
	switch(ToSet.g_Manip())
	{
	case IOM_dec:
	case IOM_oct:
	case IOM_hex:
		{
			unsigned long df = ~(IOM_dec|IOM_oct|IOM_hex);
			m_Manip &= df;
		};
		break;
	};

	m_Manip |= ToSet.g_Manip();

	return *this;
};



//*****************************************************************************
//*****************************************************************************
bool C_StreamIoFlag::HexMode(void)
{
	if(m_Manip&IOM_hex)
		return true;
	return false;
};

//*****************************************************************************
//*****************************************************************************
bool C_StreamIoFlag::DecMode(void)
{
	if(m_Manip&IOM_dec)
		return true;
	return false;
};


//*****************************************************************************
//*****************************************************************************
bool C_StreamIoFlag::OctMode(void)
{
	if(m_Manip&IOM_oct)
		return true;
	return false;
};
