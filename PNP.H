
struct PnP_BIOS_HEADER {
    BYTE  cSignature[4];         /*  0  */
    BYTE  cVersion;              /*  4  */
    BYTE  cLength;               /*  5  */
    WORD  uControlField;         /*  6  */
    BYTE  cChecksum;             /*  8  */
    DWORD fpEvent;               /*  9  */
    WORD  uRealOff;              /* 13  */
    WORD  uRealSeg;              /* 15  */
    WORD  uP16Off;               /* 17  */
    DWORD lP16Bas;               /* 19  */
    DWORD lOEMid;                /* 23  */
    WORD  uRealDataSeg;          /* 27  */
    DWORD lP16DataBas;           /* 29  */
};

/* 00 */ int Get_Number_of_System_Device_Nodes(int, int far *, int far *, unsigned);
/* 01 */ int Get_System_Device_Node(int, int far *, BYTE far *, int, unsigned);
/* 02 */ int Set_System_Device_Node(int, int, BYTE far *, int, unsigned);
/* 03 */ int Get_Event(int, int far *, unsigned);
/* 04 */ int Send_Message(int, int, unsigned);

