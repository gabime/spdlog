#ifndef ____INC___V_STRX_IOMANIP_H___CNI____
#define ____INC___V_STRX_IOMANIP_H___CNI____


enum
{
	IOMID_null		=	0x0000,
	IOMID_flag		=	0x0001,			// Manipulator-Flag für Ausgaben (z.B. dez, hex etc)
	IOMID_clear		=	0x0002,			// Löschen des Puffers
	IOMID_endl		=	0x0003,			// Stellt CR/LF in den Puffer
	IOMID_last
};//VSTR_MANIP_ID;

enum
{
	IOM_null		=	0x0000,
    IOM_dec			=	0x0001,
    IOM_oct			=	0x0002,
    IOM_hex			=	0x0004,

	// Für Vergleiche: Case-Sensitiv / nicht case-sensitiv
	IOM_comp_case	=	0x0008,
	IOM_comp_nocase	=	0x0010,

/*
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
	*/
};


class C_StreamIoManipulator
{
private:
	unsigned int	m_Manip;
	unsigned int	m_ManipId;	


public:
	C_StreamIoManipulator(unsigned int man, unsigned int id){m_Manip = man;m_ManipId = id;};
	C_StreamIoManipulator(){m_Manip = 0; m_ManipId = 0;};
	
	C_StreamIoManipulator& operator=(const C_StreamIoManipulator& ToSet);

	unsigned int g_Manip(void){return m_Manip;};

	unsigned int g_ManipId(void){return m_ManipId;};

	bool IsFlag(void)
	{
		if(m_ManipId==IOMID_flag)
			return true;
		return false;
	};

};


//*****************************************************************************
// Diese Liste enthält die benutzbaren Manipulatoren. Manipulatoren sollten
// immer hier eingepflegt werden, denn zum Definieren eigener ist z.B. zu 
// beachten, daß ein Manipulator nur ein einziges Bit gesetzt haben darf!!!
//*****************************************************************************
extern C_StreamIoManipulator	VSTR_MANIP_HEX;
extern C_StreamIoManipulator	VSTR_MANIP_DEC;
extern C_StreamIoManipulator	VSTR_MANIP_OCT;
extern C_StreamIoManipulator	VSTR_MANIP_CMPCASE;
extern C_StreamIoManipulator	VSTR_MANIP_CMPNOCASE;

extern C_StreamIoManipulator	VSTR_MANIP_CLEAR;
extern C_StreamIoManipulator	VSTR_MANIP_ENDL;



class C_StreamIoFlag
{
private:
	unsigned int m_Manip;

public:
	C_StreamIoFlag(){m_Manip = 0;};
	C_StreamIoFlag& operator=(C_StreamIoManipulator& ToSet);
	
	bool HexMode(void);
	bool DecMode(void);
	bool OctMode(void);
	
};


#endif // ____INC___V_STRX_IOMANIP_H___CNI____
