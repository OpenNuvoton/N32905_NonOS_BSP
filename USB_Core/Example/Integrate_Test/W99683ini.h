/**************************************************************************//**
 * @file     w99683ini.h
 * @brief    W99683 USB device driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

UINT8  InitRegValue[] =
{
//CR0000, CR0001, CR0002, CR0003, CR0004, CR0005, CR0006, CR0007,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0008, CR0009, CR000A, CR000B, CR000C, CR000D, CR000E, CR000F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0010, CR0011, CR0012, CR0013, CR0014, CR0015, CR0016, CR0017,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0018, CR0019, CR001A, CR001B, CR001C, CR001D, CR001E, CR001F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0020, CR0021, CR0022, CR0023, CR0024, CR0025, CR0026, CR0027,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0028, CR0029, CR002A, CR002B, CR002C, CR002D, CR002E, CR002F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0030, CR0031, CR0032, CR0033, CR0034, CR0035, CR0036, CR0037,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0038, CR0039, CR003A, CR003B, CR003C, CR003D, CR003E, CR003F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0040, CR0041, CR0042, CR0043, CR0044, CR0045, CR0046, CR0047,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0048, CR0049, CR004A, CR004B, CR004C, CR004D, CR004E, CR004F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0050, CR0051, CR0052, CR0053, CR0054, CR0055, CR0056, CR0057,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0058, CR0059, CR005A, CR005B, CR005C, CR005D, CR005E, CR005F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0060, CR0061, CR0062, CR0063, CR0064, CR0065, CR0066, CR0067,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0068, CR0069, CR006A, CR006B, CR006C, CR006D, CR006E, CR006F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0070, CR0071, CR0072, CR0073, CR0074, CR0075, CR0076, CR0077,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0078, CR0079, CR007A, CR007B, CR007C, CR007D, CR007E, CR007F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0080, CR0081, CR0082, CR0083, CR0084, CR0085, CR0086, CR0087,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0088, CR0089, CR008A, CR008B, CR008C, CR008D, CR008E, CR008F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0090, CR0091, CR0092, CR0093, CR0094, CR0095, CR0096, CR0097,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0098, CR0099, CR009A, CR009B, CR009C, CR009D, CR009E, CR009F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR00A0, CR00A1, CR00A2, CR00A3, CR00A4, CR00A5, CR00A6, CR00A7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR00A8, CR00A9, CR00AA, CR00AB, CR00Ac, CR00AD, CR00AE, CR00AF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR00B0, CR00B1, CR00B2, CR00B3, CR00B4, CR00B5, CR00B6, CR00B7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR00B8, CR00B9, CR00BA, CR00BB, CR00BC, CR00BD, CR00BE, CR00BF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR00C0, CR00C1, CR00C2, CR00C3, CR00C4, CR00C5, CR00C6, CR00C7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR00C8, CR00C9, CR00CA, CR00CB, CR00CC, CR00CD, CR00CE, CR00CF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR00D0, CR00D1, CR00D2, CR00D3, CR00D4, CR00D5, CR00D6, CR00D7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR00D8, CR00D9, CR00DA, CR00DB, CR00DC, CR00DD, CR00DE, CR00DF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR00E0, CR00E1, CR00E2, CR00E3, CR00E4, CR00E5, CR00E6, CR00E7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR00E8, CR00E9, CR00EA, CR00EB, CR00EC, CR00ED, CR00EE, CR00EF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR00F0, CR00F1, CR00F2, CR00F3, CR00F4, CR00F5, CR00F6, CR00F7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR00F8, CR00F9, CR00FA, CR00FB, CR00FC, CR00FD, CR00FE, CR00FF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0100, CR0101, CR0102, CR0103, CR0104, CR0105, CR0106, CR0107,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0108, CR0109, CR010A, CR010B, CR010C, CR010D, CR010E, CR010F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0110, CR0111, CR0112, CR0113, CR0114, CR0115, CR0116, CR0117,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0118, CR0119, CR011A, CR011B, CR011C, CR011D, CR011E, CR011F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0120, CR0121, CR0122, CR0123, CR0124, CR0125, CR0126, CR0127,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0128, CR0129, CR012A, CR012B, CR012C, CR012D, CR012E, CR012F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0130, CR0131, CR0132, CR0133, CR0134, CR0135, CR0136, CR0137,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0138, CR0139, CR013A, CR013B, CR013C, CR013D, CR013E, CR013F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0140, CR0141, CR0142, CR0143, CR0144, CR0145, CR0146, CR0147,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0148, CR0149, CR014A, CR014B, CR014C, CR014D, CR014E, CR014F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0150, CR0151, CR0152, CR0153, CR0154, CR0155, CR0156, CR0157,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0158, CR0159, CR015A, CR015B, CR015C, CR015D, CR015E, CR015F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0160, CR0161, CR0162, CR0163, CR0164, CR0165, CR0166, CR0167,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0168, CR0169, CR016A, CR016B, CR016C, CR016D, CR016E, CR016F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0170, CR0171, CR0172, CR0173, CR0174, CR0175, CR0176, CR0177,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0178, CR0179, CR017A, CR017B, CR017C, CR017D, CR017E, CR017F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0180, CR0181, CR0182, CR0183, CR0184, CR0185, CR0186, CR0187,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0188, CR0189, CR018A, CR018B, CR018C, CR018D, CR018E, CR018F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0190, CR0191, CR0192, CR0193, CR0194, CR0195, CR0196, CR0197,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0198, CR0199, CR019A, CR019B, CR019C, CR019D, CR019E, CR019F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR01A0, CR01A1, CR01A2, CR01A3, CR01A4, CR01A5, CR01A6, CR01A7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR01A8, CR01A9, CR01AA, CR01AB, CR01Ac, CR01AD, CR01AE, CR01AF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR01B0, CR01B1, CR01B2, CR01B3, CR01B4, CR01B5, CR01B6, CR01B7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR01B8, CR01B9, CR01BA, CR01BB, CR01BC, CR01BD, CR01BE, CR01BF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR01C0, CR01C1, CR01C2, CR01C3, CR01C4, CR01C5, CR01C6, CR01C7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR01C8, CR01C9, CR01CA, CR01CB, CR01CC, CR01CD, CR01CE, CR01CF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR01D0, CR01D1, CR01D2, CR01D3, CR01D4, CR01D5, CR01D6, CR01D7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR01D8, CR01D9, CR01DA, CR01DB, CR01DC, CR01DD, CR01DE, CR01DF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR01E0, CR01E1, CR01E2, CR01E3, CR01E4, CR01E5, CR01E6, CR01E7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR01E8, CR01E9, CR01EA, CR01EB, CR01EC, CR01ED, CR01EE, CR01EF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR01F0, CR01F1, CR01F2, CR01F3, CR01F4, CR01F5, CR01F6, CR01F7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR01F8, CR01F9, CR01FA, CR01FB, CR01FC, CR01FD, CR01FE, CR01FF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0200, CR0201, CR0202, CR0203, CR0204, CR0205, CR0206, CR0207,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0208, CR0209, CR020A, CR020B, CR020C, CR020D, CR020E, CR020F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0210, CR0211, CR0212, CR0213, CR0214, CR0215, CR0216, CR0217,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0218, CR0219, CR021A, CR021B, CR021C, CR021D, CR021E, CR021F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0220, CR0221, CR0222, CR0223, CR0224, CR0225, CR0226, CR0227,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0228, CR0229, CR022A, CR022B, CR022C, CR022D, CR022E, CR022F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0230, CR0231, CR0232, CR0233, CR0234, CR0235, CR0236, CR0237,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0238, CR0239, CR023A, CR023B, CR023C, CR023D, CR023E, CR023F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0240, CR0241, CR0242, CR0243, CR0244, CR0245, CR0246, CR0247,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0248, CR0249, CR024A, CR024B, CR024C, CR024D, CR024E, CR024F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0250, CR0251, CR0252, CR0253, CR0254, CR0255, CR0256, CR0257,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0258, CR0259, CR025A, CR025B, CR025C, CR025D, CR025E, CR025F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0260, CR0261, CR0262, CR0263, CR0264, CR0265, CR0266, CR0267,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0268, CR0269, CR026A, CR026B, CR026C, CR026D, CR026E, CR026F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0270, CR0271, CR0272, CR0273, CR0274, CR0275, CR0276, CR0277,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0278, CR0279, CR027A, CR027B, CR027C, CR027D, CR027E, CR027F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0280, CR0281, CR0282, CR0283, CR0284, CR0285, CR0286, CR0287,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0288, CR0289, CR028A, CR028B, CR028C, CR028D, CR028E, CR028F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0290, CR0291, CR0292, CR0293, CR0294, CR0295, CR0296, CR0297,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0298, CR0299, CR029A, CR029B, CR029C, CR029D, CR029E, CR029F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR02A0, CR02A1, CR02A2, CR02A3, CR02A4, CR02A5, CR02A6, CR02A7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR02A8, CR02A9, CR02AA, CR02AB, CR02Ac, CR02AD, CR02AE, CR02AF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR02B0, CR02B1, CR02B2, CR02B3, CR02B4, CR02B5, CR02B6, CR02B7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR02B8, CR02B9, CR02BA, CR02BB, CR02BC, CR02BD, CR02BE, CR02BF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR02C0, CR02C1, CR02C2, CR02C3, CR02C4, CR02C5, CR02C6, CR02C7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR02C8, CR02C9, CR02CA, CR02CB, CR02CC, CR02CD, CR02CE, CR02CF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR02D0, CR02D1, CR02D2, CR02D3, CR02D4, CR02D5, CR02D6, CR02D7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR02D8, CR02D9, CR02DA, CR02DB, CR02DC, CR02DD, CR02DE, CR02DF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR02E0, CR02E1, CR02E2, CR02E3, CR02E4, CR02E5, CR02E6, CR02E7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR02E8, CR02E9, CR02EA, CR02EB, CR02EC, CR02ED, CR02EE, CR02EF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR02F0, CR02F1, CR02F2, CR02F3, CR02F4, CR02F5, CR02F6, CR02F7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR02F8, CR02F9, CR02FA, CR02FB, CR02FC, CR02FD, CR02FE, CR02FF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0300, CR0301, CR0302, CR0303, CR0304, CR0305, CR0306, CR0307,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0308, CR0309, CR030A, CR030B, CR030C, CR030D, CR030E, CR030F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0310, CR0311, CR0312, CR0313, CR0314, CR0315, CR0316, CR0317,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0318, CR0319, CR031A, CR031B, CR031C, CR031D, CR031E, CR031F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0320, CR0321, CR0322, CR0323, CR0324, CR0325, CR0326, CR0327,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0328, CR0329, CR032A, CR032B, CR032C, CR032D, CR032E, CR032F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0330, CR0331, CR0332, CR0333, CR0334, CR0335, CR0336, CR0337,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0338, CR0339, CR033A, CR033B, CR033C, CR033D, CR033E, CR033F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0340, CR0341, CR0342, CR0343, CR0344, CR0345, CR0346, CR0347,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0348, CR0349, CR034A, CR034B, CR034C, CR034D, CR034E, CR034F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0350, CR0351, CR0352, CR0353, CR0354, CR0355, CR0356, CR0357,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0358, CR0359, CR035A, CR035B, CR035C, CR035D, CR035E, CR035F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0360, CR0361, CR0362, CR0363, CR0364, CR0365, CR0366, CR0367,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0368, CR0369, CR036A, CR036B, CR036C, CR036D, CR036E, CR036F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0370, CR0371, CR0372, CR0373, CR0374, CR0375, CR0376, CR0377,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0378, CR0379, CR037A, CR037B, CR037C, CR037D, CR037E, CR037F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0380, CR0381, CR0382, CR0383, CR0384, CR0385, CR0386, CR0387,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0388, CR0389, CR038A, CR038B, CR038C, CR038D, CR038E, CR038F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0390, CR0391, CR0392, CR0393, CR0394, CR0395, CR0396, CR0397,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0398, CR0399, CR039A, CR039B, CR039C, CR039D, CR039E, CR039F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR03A0, CR03A1, CR03A2, CR03A3, CR03A4, CR03A5, CR03A6, CR03A7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR03A8, CR03A9, CR03AA, CR03AB, CR03Ac, CR03AD, CR03AE, CR03AF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR03B0, CR03B1, CR03B2, CR03B3, CR03B4, CR03B5, CR03B6, CR03B7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR03B8, CR03B9, CR03BA, CR03BB, CR03BC, CR03BD, CR03BE, CR03BF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR03C0, CR03C1, CR03C2, CR03C3, CR03C4, CR03C5, CR03C6, CR03C7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR03C8, CR03C9, CR03CA, CR03CB, CR03CC, CR03CD, CR03CE, CR03CF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR03D0, CR03D1, CR03D2, CR03D3, CR03D4, CR03D5, CR03D6, CR03D7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR03D8, CR03D9, CR03DA, CR03DB, CR03DC, CR03DD, CR03DE, CR03DF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR03E0, CR03E1, CR03E2, CR03E3, CR03E4, CR03E5, CR03E6, CR03E7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR03E8, CR03E9, CR03EA, CR03EB, CR03EC, CR03ED, CR03EE, CR03EF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR03F0, CR03F1, CR03F2, CR03F3, CR03F4, CR03F5, CR03F6, CR03F7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR03F8, CR03F9, CR03FA, CR03FB, CR03FC, CR03FD, CR03FE, CR03FF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0400, CR0401, CR0402, CR0403, CR0404, CR0405, CR0406, CR0407,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0408, CR0409, CR040A, CR040B, CR040C, CR040D, CR040E, CR040F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0410, CR0411, CR0412, CR0413, CR0414, CR0415, CR0416, CR0417,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0418, CR0419, CR041A, CR041B, CR041C, CR041D, CR041E, CR041F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0420, CR0421, CR0422, CR0423, CR0424, CR0425, CR0426, CR0427,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0428, CR0429, CR042A, CR042B, CR042C, CR042D, CR042E, CR042F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0430, CR0431, CR0432, CR0433, CR0434, CR0435, CR0436, CR0437,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0438, CR0439, CR043A, CR043B, CR043C, CR043D, CR043E, CR043F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0440, CR0441, CR0442, CR0443, CR0444, CR0445, CR0446, CR0447,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0448, CR0449, CR044A, CR044B, CR044C, CR044D, CR044E, CR044F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0450, CR0451, CR0452, CR0453, CR0454, CR0455, CR0456, CR0457,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0458, CR0459, CR045A, CR045B, CR045C, CR045D, CR045E, CR045F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0460, CR0461, CR0462, CR0463, CR0464, CR0465, CR0466, CR0467,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0468, CR0469, CR046A, CR046B, CR046C, CR046D, CR046E, CR046F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0470, CR0471, CR0472, CR0473, CR0474, CR0475, CR0476, CR0477,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0478, CR0479, CR047A, CR047B, CR047C, CR047D, CR047E, CR047F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0480, CR0481, CR0482, CR0483, CR0484, CR0485, CR0486, CR0487,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0488, CR0489, CR048A, CR048B, CR048C, CR048D, CR048E, CR048F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0490, CR0491, CR0492, CR0493, CR0494, CR0495, CR0496, CR0497,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0498, CR0499, CR049A, CR049B, CR049C, CR049D, CR049E, CR049F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR04A0, CR04A1, CR04A2, CR04A3, CR04A4, CR04A5, CR04A6, CR04A7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR04A8, CR04A9, CR04AA, CR04AB, CR04Ac, CR04AD, CR04AE, CR04AF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR04B0, CR04B1, CR04B2, CR04B3, CR04B4, CR04B5, CR04B6, CR04B7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR04B8, CR04B9, CR04BA, CR04BB, CR04BC, CR04BD, CR04BE, CR04BF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR04C0, CR04C1, CR04C2, CR04C3, CR04C4, CR04C5, CR04C6, CR04C7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR04C8, CR04C9, CR04CA, CR04CB, CR04CC, CR04CD, CR04CE, CR04CF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR04D0, CR04D1, CR04D2, CR04D3, CR04D4, CR04D5, CR04D6, CR04D7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR04D8, CR04D9, CR04DA, CR04DB, CR04DC, CR04DD, CR04DE, CR04DF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR04E0, CR04E1, CR04E2, CR04E3, CR04E4, CR04E5, CR04E6, CR04E7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR04E8, CR04E9, CR04EA, CR04EB, CR04EC, CR04ED, CR04EE, CR04EF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR04F0, CR04F1, CR04F2, CR04F3, CR04F4, CR04F5, CR04F6, CR04F7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR04F8, CR04F9, CR04FA, CR04FB, CR04FC, CR04FD, CR04FE, CR04FF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0500, CR0501, CR0502, CR0503, CR0504, CR0505, CR0506, CR0507,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0508, CR0509, CR050A, CR050B, CR050C, CR050D, CR050E, CR050F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0510, CR0511, CR0512, CR0513, CR0514, CR0515, CR0516, CR0517,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0518, CR0519, CR051A, CR051B, CR051C, CR051D, CR051E, CR051F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0520, CR0521, CR0522, CR0523, CR0524, CR0525, CR0526, CR0527,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0528, CR0529, CR052A, CR052B, CR052C, CR052D, CR052E, CR052F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0530, CR0531, CR0532, CR0533, CR0534, CR0535, CR0536, CR0537,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0538, CR0539, CR053A, CR053B, CR053C, CR053D, CR053E, CR053F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0540, CR0541, CR0542, CR0543, CR0544, CR0545, CR0546, CR0547,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0548, CR0549, CR054A, CR054B, CR054C, CR054D, CR054E, CR054F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0550, CR0551, CR0552, CR0553, CR0554, CR0555, CR0556, CR0557,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0558, CR0559, CR055A, CR055B, CR055C, CR055D, CR055E, CR055F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0560, CR0561, CR0562, CR0563, CR0564, CR0565, CR0566, CR0567,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0568, CR0569, CR056A, CR056B, CR056C, CR056D, CR056E, CR056F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0570, CR0571, CR0572, CR0573, CR0574, CR0575, CR0576, CR0577,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0578, CR0579, CR057A, CR057B, CR057C, CR057D, CR057E, CR057F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0580, CR0581, CR0582, CR0583, CR0584, CR0585, CR0586, CR0587,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0588, CR0589, CR058A, CR058B, CR058C, CR058D, CR058E, CR058F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0590, CR0591, CR0592, CR0593, CR0594, CR0595, CR0596, CR0597,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0598, CR0599, CR059A, CR059B, CR059C, CR059D, CR059E, CR059F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR05A0, CR05A1, CR05A2, CR05A3, CR05A4, CR05A5, CR05A6, CR05A7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR05A8, CR05A9, CR05AA, CR05AB, CR05Ac, CR05AD, CR05AE, CR05AF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR05B0, CR05B1, CR05B2, CR05B3, CR05B4, CR05B5, CR05B6, CR05B7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR05B8, CR05B9, CR05BA, CR05BB, CR05BC, CR05BD, CR05BE, CR05BF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR05C0, CR05C1, CR05C2, CR05C3, CR05C4, CR05C5, CR05C6, CR05C7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR05C8, CR05C9, CR05CA, CR05CB, CR05CC, CR05CD, CR05CE, CR05CF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR05D0, CR05D1, CR05D2, CR05D3, CR05D4, CR05D5, CR05D6, CR05D7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR05D8, CR05D9, CR05DA, CR05DB, CR05DC, CR05DD, CR05DE, CR05DF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR05E0, CR05E1, CR05E2, CR05E3, CR05E4, CR05E5, CR05E6, CR05E7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR05E8, CR05E9, CR05EA, CR05EB, CR05EC, CR05ED, CR05EE, CR05EF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR05F0, CR05F1, CR05F2, CR05F3, CR05F4, CR05F5, CR05F6, CR05F7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR05F8, CR05F9, CR05FA, CR05FB, CR05FC, CR05FD, CR05FE, CR05FF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0600, CR0601, CR0602, CR0603, CR0604, CR0605, CR0606, CR0607,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0608, CR0609, CR060A, CR060B, CR060C, CR060D, CR060E, CR060F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0610, CR0611, CR0612, CR0613, CR0614, CR0615, CR0616, CR0617,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0618, CR0619, CR061A, CR061B, CR061C, CR061D, CR061E, CR061F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0620, CR0621, CR0622, CR0623, CR0624, CR0625, CR0626, CR0627,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0628, CR0629, CR062A, CR062B, CR062C, CR062D, CR062E, CR062F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0630, CR0631, CR0632, CR0633, CR0634, CR0635, CR0636, CR0637,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0638, CR0639, CR063A, CR063B, CR063C, CR063D, CR063E, CR063F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0640, CR0641, CR0642, CR0643, CR0644, CR0645, CR0646, CR0647,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0648, CR0649, CR064A, CR064B, CR064C, CR064D, CR064E, CR064F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0650, CR0651, CR0652, CR0653, CR0654, CR0655, CR0656, CR0657,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0658, CR0659, CR065A, CR065B, CR065C, CR065D, CR065E, CR065F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0660, CR0661, CR0662, CR0663, CR0664, CR0665, CR0666, CR0667,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0668, CR0669, CR066A, CR066B, CR066C, CR066D, CR066E, CR066F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0670, CR0671, CR0672, CR0673, CR0674, CR0675, CR0676, CR0677,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0678, CR0679, CR067A, CR067B, CR067C, CR067D, CR067E, CR067F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0680, CR0681, CR0682, CR0683, CR0684, CR0685, CR0686, CR0687,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0688, CR0689, CR068A, CR068B, CR068C, CR068D, CR068E, CR068F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0690, CR0691, CR0692, CR0693, CR0694, CR0695, CR0696, CR0697,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR0698, CR0699, CR069A, CR069B, CR069C, CR069D, CR069E, CR069F,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR06A0, CR06A1, CR06A2, CR06A3, CR06A4, CR06A5, CR06A6, CR06A7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR06A8, CR06A9, CR06AA, CR06AB, CR06Ac, CR06AD, CR06AE, CR06AF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR06B0, CR06B1, CR06B2, CR06B3, CR06B4, CR06B5, CR06B6, CR06B7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR06B8, CR06B9, CR06BA, CR06BB, CR06BC, CR06BD, CR06BE, CR06BF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR06C0, CR06C1, CR06C2, CR06C3, CR06C4, CR06C5, CR06C6, CR06C7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR06C8, CR06C9, CR06CA, CR06CB, CR06CC, CR06CD, CR06CE, CR06CF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR06D0, CR06D1, CR06D2, CR06D3, CR06D4, CR06D5, CR06D6, CR06D7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR06D8, CR06D9, CR06DA, CR06DB, CR06DC, CR06DD, CR06DE, CR06DF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR06E0, CR06E1, CR06E2, CR06E3, CR06E4, CR06E5, CR06E6, CR06E7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR06E8, CR06E9, CR06EA, CR06EB, CR06EC, CR06ED, CR06EE, CR06EF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR06F0, CR06F1, CR06F2, CR06F3, CR06F4, CR06F5, CR06F6, CR06F7,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,

//CR06F8, CR06F9, CR06FA, CR06FB, CR06FC, CR06FD, CR06FE, CR06FF,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
};

