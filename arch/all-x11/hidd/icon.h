/*  GIMP header image file format (INDEXED): /usr/local/aros/AROS/config/x11/hidd/icon.h  */

static unsigned int width = 40;
static unsigned int height = 40;

/*  Call this macro repeatedly.  After each use, the pixel data can be extracted  */

#define HEADER_PIXEL(data,pixel) {\
  pixel[0] = header_data_cmap[(unsigned char)data[0]][0]; \
  pixel[1] = header_data_cmap[(unsigned char)data[0]][1]; \
  pixel[2] = header_data_cmap[(unsigned char)data[0]][2]; \
  data ++; }

static char header_data_cmap[256][3] = {
	{255,255,255},
	{254,254,254},
	{250,222,222},
	{245,207,207},
	{250,198,198},
	{250,126,126},
	{246,145,145},
	{246,173,173},
	{249,166,166},
	{246,154,154},
	{241,185,185},
	{251,250,250},
	{246,190,190},
	{246,104,104},
	{254, 18, 18},
	{254, 24, 24},
	{250, 82, 82},
	{254, 14, 14},
	{238,140,140},
	{226,202,202},
	{218, 54, 54},
	{212,144,144},
	{226,194,194},
	{254,214,214},
	{254,160,160},
	{254, 72, 72},
	{254,  2,  2},
	{250, 90, 90},
	{242,214,214},
	{243,242,242},
	{239,226,226},
	{238,162,162},
	{223,  3,  3},
	{206,106,106},
	{194,126,126},
	{184, 98, 98},
	{249,120,120},
	{248,158,158},
	{238,238,238},
	{238,234,234},
	{250, 94, 94},
	{242, 74, 74},
	{234,202,202},
	{210,162,162},
	{200,114,114},
	{174, 52, 52},
	{162,104,104},
	{199,173,173},
	{249,132,132},
	{238,230,230},
	{246,150,150},
	{242,  2,  2},
	{238,114,114},
	{226,226,226},
	{214,214,214},
	{198, 86, 86},
	{186,  2,  2},
	{160, 94, 94},
	{142,106,106},
	{194,154,154},
	{254, 50, 50},
	{252, 36, 36},
	{238,152,152},
	{222,222,222},
	{209,207,207},
	{198,142,142},
	{181,  2,  2},
	{170,  4,  4},
	{144,124,124},
	{122, 78, 78},
	{254, 42, 42},
	{236,  2,  2},
	{230,190,190},
	{219,218,218},
	{192,182,182},
	{182,  6,  6},
	{160,  2,  2},
	{146, 50, 50},
	{122,122,122},
	{110, 58, 58},
	{254, 56, 56},
	{236,178,178},
	{224, 94, 94},
	{174, 26, 26},
	{142,  2,  2},
	{128,106,106},
	{ 98, 90, 90},
	{134, 92, 92},
	{247,246,246},
	{242,222,222},
	{250, 62, 62},
	{242,202,202},
	{254,  6,  6},
	{226,170,170},
	{218,  2,  2},
	{202,  3,  3},
	{198,  2,  2},
	{182, 18, 18},
	{134, 70, 70},
	{118, 70, 70},
	{104, 54, 54},
	{ 76, 42, 42},
	{230,230,230},
	{222,110,110},
	{207,  2,  2},
	{174,  2,  2},
	{146,146,146},
	{130,130,130},
	{110, 94, 94},
	{ 94,  2,  2},
	{ 66,  2,  2},
	{134,134,134},
	{236,220,220},
	{254, 30, 30},
	{210, 66, 66},
	{193,  2,  2},
	{142,142,142},
	{126,126,126},
	{110,110,110},
	{ 86, 14, 14},
	{ 70,  2,  2},
	{ 82, 67, 67},
	{250,242,242},
	{210, 10, 10},
	{166,  3,  3},
	{154,  2,  2},
	{150, 91, 91},
	{118,118,118},
	{ 98, 98, 98},
	{ 82, 28, 28},
	{ 66, 22, 22},
	{238, 10, 10},
	{230, 70, 70},
	{222,138,138},
	{210,186,186},
	{138, 80, 80},
	{ 94, 94, 94},
	{ 62,  2,  2},
	{ 62, 10, 10},
	{186,186,186},
	{250,111,111},
	{242, 30, 30},
	{230,  2,  2},
	{182,106,106},
	{179, 59, 59},
	{162, 26, 26},
	{126, 88, 88},
	{115,114,114},
	{102,102,102},
	{ 87, 86, 86},
	{ 67, 34, 34},
	{166,154,154},
	{242,122,122},
	{186,126,126},
	{174,174,174},
	{167,166,166},
	{155,154,154},
	{146,134,134},
	{134,102,102},
	{122, 50, 50},
	{102, 46, 46},
	{ 90, 54, 54},
	{ 73, 46, 46},
	{ 64, 27, 27},
	{ 66, 45, 45},
	{154,130,130},
	{162,162,162},
	{170,170,170},
	{158,158,158},
	{150,142,142},
	{104, 20, 20},
	{ 98,  2,  2},
	{ 78,  2,  2},
	{ 66, 66, 66},
	{ 62, 62, 62},
	{158,142,142},
	{230,162,162},
	{230, 18, 18},
	{214, 26, 26},
	{174, 38, 38},
	{114, 98, 98},
	{103,  3,  3},
	{ 83,  3,  3},
	{ 66, 61, 61},
	{186,166,166},
	{240,198,198},
	{214,202,202},
	{218,130,130},
	{206, 82, 82},
	{202, 54, 54},
	{162, 78, 78},
	{102, 78, 78},
	{202,202,202},
	{202, 74, 74},
	{194,194,194},
	{186,158,158},
	{152,114,114},
	{190,142,142},
	{186,118,118},
	{178,178,178},
	{108,106,106},
	{166, 10, 10},
	{174,138,138},
	{150,150,150},
	{138,138,138},
	{126,  2,  2},
	{111,  2,  2},
	{ 78, 36, 36},
	{ 70, 30, 30},
	{ 70, 22, 22},
	{ 62, 18, 18},
	{ 70, 18, 18},
	{ 70, 12, 12},
	{162, 50, 50},
	{166,134,134},
	{122, 42, 42},
	{ 74,  2,  2},
	{ 79, 78, 78},
	{146, 22, 22},
	{150, 54, 54},
	{104, 12, 12},
	{ 70, 70, 70},
	{ 76,  6,  6},
	{ 74, 74, 74},
	{190,190,190},
	{130,114,114},
	{134, 14, 14},
	{122, 62, 62},
	{118, 94, 94},
	{102, 74, 74},
	{ 77, 11, 11},
	{ 75, 29, 29},
	{ 90, 90, 90},
	{170,146,146},
	{ 86, 42, 42},
	{ 90, 36, 36},
	{ 78, 60, 60},
	{ 82, 82, 82},
	{ 90, 62, 62},
	{ 90,  2,  2},
	{ 72, 64, 64},
	{ 79, 54, 54},
	{ 78, 48, 48},
	{206,182,182},
	{ 78, 22, 22},
	{ 82, 22, 22},
	{ 70, 54, 54},
	{198,198,198},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255},
	{255,255,255}
	};
static char header_data[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,2,3,4,5,6,7,
	8,9,10,2,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,11,2,3,12,13,14,15,10,16,
	17,18,19,20,21,22,11,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,23,24,3,25,26,26,27,28,29,30,
	25,5,31,32,33,34,35,30,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,1,24,17,36,13,26,17,37,38,29,39,40,
	26,26,41,42,43,44,45,46,47,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,1,48,17,9,38,38,7,36,49,38,38,50,26,
	26,51,52,53,54,55,56,57,58,59,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,1,8,17,8,38,38,30,60,26,61,5,12,14,26,
	26,51,62,63,64,65,66,67,68,69,47,1,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,11,70,6,38,38,39,25,26,26,26,26,50,10,13,
	61,71,72,73,64,74,75,76,77,78,79,30,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,1,29,37,30,38,38,36,26,26,26,26,80,39,38,38,
	39,81,82,21,47,74,83,76,84,85,86,87,1,1,1,1,
	1,1,1,1,1,1,1,1,
	1,88,89,15,90,3,91,92,26,26,26,26,7,39,38,38,
	38,93,94,95,96,97,57,57,98,99,100,101,73,1,1,1,
	1,1,1,1,1,1,1,1,
	1,29,36,26,26,17,5,14,26,26,26,80,39,38,38,38,
	102,103,104,95,56,105,46,106,107,108,109,110,111,1,1,1,
	1,1,1,1,1,1,1,1,
	11,112,14,26,26,25,38,89,48,113,26,8,38,38,38,102,
	73,114,95,115,66,105,57,116,117,118,119,120,121,11,1,1,
	1,1,1,1,1,1,1,1,
	122,50,26,26,26,81,38,38,38,39,9,6,30,38,102,73,
	64,123,115,66,124,125,126,111,127,128,129,110,130,63,1,1,
	1,1,1,1,1,1,1,1,
	24,25,26,26,60,38,38,38,38,38,36,51,131,132,133,134,
	43,96,66,105,76,125,135,78,118,136,101,137,138,139,1,1,
	1,1,1,1,1,1,1,1,
	140,37,14,26,48,38,38,38,38,39,141,142,32,94,95,95,
	55,143,144,145,125,84,146,147,148,149,150,137,137,151,1,1,
	1,1,1,1,1,1,1,1,
	152,39,4,80,91,38,38,38,38,81,71,32,94,104,95,115,
	153,154,155,156,157,158,159,160,161,162,150,163,164,165,1,1,
	1,1,1,1,1,1,1,1,
	18,39,38,3,13,112,38,38,102,82,94,104,95,95,115,66,
	166,167,168,169,111,127,170,171,172,110,163,173,174,175,1,1,
	1,1,1,1,1,1,1,1,
	176,73,102,176,142,177,103,19,73,178,104,104,96,115,66,179,
	166,156,106,111,78,180,181,182,120,137,163,174,183,184,1,1,
	1,1,1,1,1,1,1,1,
	185,186,73,187,32,94,32,123,188,189,104,115,66,105,105,190,
	156,116,111,78,147,191,182,120,137,137,150,183,183,186,1,1,
	1,1,1,1,1,1,1,1,
	11,47,192,33,104,95,104,95,193,194,195,35,179,76,76,196,
	116,107,78,118,148,161,120,137,137,110,164,174,121,88,1,1,
	1,1,1,1,1,1,1,1,
	1,44,197,55,96,96,115,56,198,199,167,155,166,169,126,135,
	85,127,200,128,149,163,110,137,110,137,173,173,68,1,1,1,
	1,1,1,1,1,1,1,1,
	1,72,201,45,66,66,66,105,202,167,166,156,203,204,87,205,
	206,181,170,129,207,208,209,210,211,211,150,212,151,194,54,102,
	29,1,1,1,1,1,1,1,
	1,11,144,213,214,213,124,76,156,156,106,116,111,107,215,206,
	181,171,182,216,130,173,173,173,173,208,120,207,217,149,149,136,
	118,111,154,54,1,1,1,1,
	1,1,19,218,169,106,196,219,116,116,111,107,78,118,220,181,
	109,182,120,120,162,221,221,221,173,216,222,174,174,173,174,173,
	173,223,217,136,117,224,88,1,
	1,1,1,197,146,111,107,225,226,227,228,118,200,229,171,182,
	216,120,110,230,173,221,223,223,231,222,164,174,174,174,174,174,
	174,174,174,174,221,232,204,29,
	1,1,1,11,233,200,118,180,206,206,181,220,234,235,230,120,
	216,120,120,162,223,221,223,236,216,162,174,174,174,174,174,174,
	174,174,174,174,174,174,237,154,
	1,1,1,1,1,233,238,149,109,239,239,182,216,121,223,240,
	241,162,231,207,101,242,162,236,174,174,174,174,174,174,174,174,
	174,174,174,174,174,174,223,203,
	1,1,1,1,1,1,243,160,211,182,216,172,231,223,217,223,
	223,121,212,172,172,222,236,173,174,174,174,174,174,173,174,174,
	174,174,174,174,173,221,136,194,
	1,1,1,1,1,1,1,38,228,217,161,231,242,217,217,217,
	217,129,182,172,244,183,174,174,174,174,174,174,174,174,174,174,
	173,174,174,173,221,136,167,11,
	1,1,1,1,1,1,1,1,194,128,223,236,244,245,245,241,
	246,101,207,246,173,174,174,174,174,173,174,174,174,174,173,174,
	173,173,223,149,78,192,1,1,
	1,1,1,1,1,1,1,1,1,247,117,136,223,173,183,183,
	183,174,173,174,174,173,174,174,174,174,174,173,173,173,173,223,
	237,148,116,247,88,1,1,1,
	1,1,1,1,1,1,1,1,1,1,88,192,203,147,136,149,
	217,223,221,173,173,221,221,223,221,221,223,237,232,128,127,168,
	199,73,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,29,63,247,
	154,156,78,127,127,127,78,127,107,168,167,194,192,39,1,1,
	1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
	};
