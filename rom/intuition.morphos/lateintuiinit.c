/*
    Copyright � 1995-2003, The AROS Development Team. All rights reserved.
    Copyright � 2001-2003, The MorphOS Development Team. All Rights Reserved.
    $Id$
 
    Late initialization of intuition.
*/

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/alib.h>

#include <exec/alerts.h>
#include <exec/types.h>

#include <aros/config.h>
#include <aros/libcall.h>
#include "intuition_intern.h"
#define NO_PATTERN 1

#if !NO_PATTERN
static void MakeWBPattern(struct Screen *scr, struct IntuitionBase *IntuitionBase);
#endif

/*****************************************************************************
 
    NAME */
#include <graphics/rastport.h>
#include <proto/graphics.h>

AROS_LH1(BOOL , LateIntuiInit,

         /*  SYNOPSIS */
         AROS_LHA(APTR, data, A0),

         /*  LOCATION */
         struct IntuitionBase *, IntuitionBase, 150, Intuition)

/*  FUNCTION
    This function permits late initalization
    of intuition (After dos and after graphics hidds are setup,
    but before starup-sequence is run.
    Can be used to open workbench screen.
 
    INPUTS
    data - unused for now.
 
    RESULT
    success - TRUE if initialization went, FALSE otherwise.
 
    NOTES
    This function is private and AROS specific.
 
    EXAMPLE
 
    BUGS
 
    SEE ALSO
 
    INTERNALS
 
    HISTORY
    29-10-95    digulla automatically created from
                graphics_lib.fd and clib/graphics_protos.h
 
*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct IntuitionBase *,IntuitionBase)

#if 1
    /* shut up the compiler */
    IntuitionBase = IntuitionBase;
    data = data;

    return TRUE;

#else
    struct Screen *screen;

    /* Let LockPubScreen() open the Workbench screen for us. */
    screen = LockPubScreen( NULL );

    if( screen )
    {
    #if !NO_PATTERN
        MakeWBPattern( screen, IntuitionBase );
    #endif
        UnlockPubScreen( NULL, screen );

        return TRUE;
    }

    return FALSE;
#endif

    AROS_LIBFUNC_EXIT
} /* LateIntuiInit */

#if !NO_PATTERN

#define PATTERN_WIDTH 128
#define PATTERN_HEIGHT 128
#define PATTERN_DEPTH 4
#define PATTERN_PLANESIZE (PATTERN_WIDTH / 8 * PATTERN_HEIGHT)

static ULONG patterndata[] =
    {
        0xf71ac27c,0x5f149f50,0x0c451000,0xd8a0abd4,0x2917c0cb,0xf5280509,
        0xc6030ba2,0x57569500,0xc23eb7d0,0x4ce3c402,0x3b851206,0xec7e9a18,
        0x8a214240,0x25ee381d,0xb51c3308,0x577007bb,0xc7d69143,0x1ef5ea7e,
        0x6a6b6309,0x1c20a844,0x0968317b,0x5022d9b1,0xf5e9d5ce,0x42f0b1ab,
        0x3b0475b0,0xf5098f56,0x25f0ccaa,0x6c61f7c5,0x22852bf6,0xfa230b58,
        0xcb8b7711,0xb09353db,0x8a52d50a,0xf695002a,0xb193788f,0xf803844e,
        0x00b3ae09,0x4f9c01e7,0xbf132d17,0xf2613cb1,0x70cf5f60,0xf40c4120,
        0x427e5ba5,0x810c10da,0x3acc1d51,0x9c2a73f1,0xbd7a3fef,0xb8aca034,
        0x36081597,0xcd3728ef,0x042b5ed7,0x00732243,0x77e4733b,0xb7ab39df,
        0xf35debfd,0x7bea7c02,0xa81c26f2,0xafb2729a,0xab90fb9a,0xb347dd02,
        0xc5c63ae6,0xbac13e35,0xe70fbbd1,0xafd81905,0x8570caed,0x39cf619e,
        0x84016bd0,0x55bc3004,0x730085a2,0x25d23d47,0x78cd726e,0x746ace82,
        0xca64eaf4,0x1a3b7ede,0xb5edff07,0xfe489be3,0x0bc2bb4a,0x4ad9c39c,
        0x6759ab69,0xf59aeb57,0x1bf9a6a6,0x58147fed,0xdbb37e97,0xc0c1be36,
        0x7b83ed6f,0xed0ff3ef,0x974c8d7d,0xaf25aad1,0x8387bdd3,0x275df27f,
        0x34dec0db,0x9e8bf43b,0x56457ebe,0xb7f572af,0xfa209ff9,0x7fdf47a1,
        0x5e029c33,0xd29eae5f,0xa9a4be33,0xd63beb96,0xd91fbb2b,0x05365fd7,
        0x3f94a7a8,0x562048bd,0x18feceb4,0xcee0ed9f,0x70adaff4,0xb7fdf3ba,
        0x1b748677,0xa1c2df7b,0xf0e05dc9,0x880a3174,0x6532ab75,0x03c7bd9f,
        0x35f62fa3,0x242db6af,0xd2fc69a1,0x05267dfb,0xca0a82af,0xe65007c1,
        0x3d12feaf,0xcaec2523,0x67221610,0xe1280e80,0x88d9ed98,0xcaaf5693,
        0xaa0c8f6f,0xca602d40,0x8539a62c,0x851cdf8b,0x0c6b5d6f,0xd0506b42,
        0x0121a59f,0x88083d17,0x40166eb3,0x4a36de9e,0x04b7ebbe,0x1200e9bc,
        0xd3599633,0x1118ece5,0x8c8118a8,0x3dedc5ca,0xf98248e9,0xe79b0c36,
        0x47a9e0fc,0x3bafafed,0xff127cb3,0x804a78f9,0x8d2c5848,0x6d3337c8,
        0xbd43effe,0x4882435b,0xfe2ae3c3,0x4a77b7ff,0x0085a4be,0xc211b298,
        0xc1e0d5e3,0x38756e74,0x376874ef,0x1c4dcb6e,0xb1719d7a,0xf53cfd45,
        0xf4b20dfc,0x3f5197b9,0x3496b0c2,0xa5b50eff,0x830f0508,0x742df9d5,
        0x4a8f97cc,0x325a64f7,0xcf507bc4,0x7d6d2396,0x35baf587,0x3506327d,
        0x2809cfb7,0x912e31af,0xea89a09b,0xf85e42f6,0x5415731f,0x33d5a057,
        0xa8ea2dc6,0x8b2a274c,0x9b5fbdc0,0x75f9f77e,0x33ee3fa2,0x058a5edb,
        0x5d666788,0xc161f750,0x59045528,0x5535482f,0x363e7ee2,0x2f857206,
        0x7b95ce38,0x151591fe,0xacefd291,0xa3ff9f73,0xc965ed8d,0x366cbeff,
        0x23179b15,0x5732ea16,0x04cf7d58,0x76215b50,0x42a2811a,0x2dfb9f6e,
        0x0e1b5645,0xbbffb7fb,0x07736ee3,0x33de5551,0x138fdedb,0xce359b95,
        0x8b8ed871,0x6ef2cf01,0x6a2ccf41,0xf4a190e1,0x7ffc937e,0xfaf531ff,
        0x14379c65,0x9450f013,0x6b5ab76a,0x2df689af,0x113e7c7f,0x098a4161,
        0xc83dc93c,0xbec62773,0x2fbbffee,0x0bd901f6,0xb447d0fd,0x7fe0d526,
        0x2425ec70,0x0ca9df87,0xcbe94bdd,0x65e01160,0x35fccae0,0x0bef7a82,
        0x3a8bcff8,0x2bfecfcd,0x3f777e61,0x7b8cffb6,0xba670bd5,0xfec24805,
        0xc276b591,0x770e7ebb,0xb25a6fef,0x6c20c68b,0xdfef19ae,0x6ff8ff37,
        0xdffe64d7,0xd39713a4,0x7efe7c37,0x4eba1439,0xabda1c64,0x17143701,
        0xf4efb41c,0xc8d9c33f,0x2431b90b,0x0dffb298,0x4b93d799,0x9fae82eb,
        0xe9650198,0xa7ae0c30,0xcf34e337,0x9a7747d5,0x534c23a3,0x3d6838a3,
        0x0d25ac78,0xfa62ef20,0x5dea7b83,0x3a0fe143,0x1902e5da,0x0563b4cd,
        0xaa91de42,0x714f9534,0x1d98cbfe,0x3788aea6,0x9c205b82,0x1bd1c148,
        0x113f77a6,0xfaadef4c,0xf0ff7224,0xcf2147b0,0x66d223e0,0x5f4effc6,
        0x185e845c,0xc750dd24,0xfdcdaf11,0x65373fa1,0x20da002e,0xdd8b4a23,
        0xadb57c02,0xdae6eec9,0x7394c432,0xfe1eb8d1,0xb71ff21d,0x9ac5f952,
        0x5d4329f2,0xe93677c0,0xe8a7e13d,0xf3cdeee0,0x9593cfb2,0x8449b986,
        0x4259b059,0x727625bb,0x2ae3ad1d,0xaeda1ffe,0x7d05c94e,0x949f2981,
        0xbd891e92,0xd309dfec,0x7896b93b,0x6f1c7921,0x5be65ccb,0x5f164444,
        0xeeb2f539,0xcd3c2041,0x7a86452f,0xfa05e897,0x2b50f1cf,0xf778cfe7,
        0xeac43f6d,0xc37882a9,0xfcf0cfbe,0xc33514d8,0x80c6797f,0xcfbc0514,
        0x76e3441c,0xb5a097be,0x830422c0,0xaf8c961c,0x03d37dec,0xd0321767,
        0x15ec0197,0x939cc7e5,0x23dbc859,0xc6717be7,0x960854fb,0xe105b94a,
        0x1b17a0d2,0xa6e83694,0x0c01f3c2,0xa46c34d5,0xc628fd67,0xda6d35d8,
        0x680405b7,0x52100c00,0x6f814dfe,0x298323bc,0x3440172d,0x11a10636,
        0xbea47d9a,0x6205dcca,0x2d15b91f,0x0e204406,0xb9278a32,0x94b47b32,
        0x160993b4,0x45179808,0x2d15fcd7,0xe6ca2987,0x037e278a,0x308a0c98,
        0x033d330e,0xf371e209,0x04de4cc8,0xc5580344,0x1f4bfa29,0x9acbc4a8,
        0x835ebb8b,0x89298110,0x3e33404f,0x3792052d,0x00403adf,0x92861602,
        0x71aef18c,0x21446c85,0x1c8829c5,0x0901c914,0x7831a38c,0xaed7f338,
        0x311a0380,0x78d2ef2d,0x50cd1339,0x58822204,0x02284bd8,0x68c1bbdd,
        0x02bb8378,0xc9004000,0x0890e920,0x1d42a82c,0x2f27069b,0xdf87e260,
        0x4d2d2d5a,0x3dcc4830,0x8b5c1b44,0x3e080206,0x95860d33,0x2ece2064,
        0x9c2c12eb,0x2fee4ba4,0x7d1083a0,0xfc1886a6,0x2f30d63a,0x2cbbac90,
        0x7d4da722,0x21480e0c,0x79cf509e,0x55075a4a,0x614d534c,0x0463dd3a,
        0xe5e2390e,0x7323b006,0xc771fb54,0xebca7b22,0x5aa21133,0x88476016,
        0xe51da7b3,0xcd8bbc76,0xc2c2706f,0x4153d049,0x9c711da7,0x1ed10764,
        0xba6b6038,0x0545df2c,0xf0353426,0x5c8fba20,0xe6b2ef2e,0xb21a89b0,
        0x40da2e70,0x2e443149,0x1b3baef0,0xffaff429,0x141890dd,0x1223adda,
        0xc06e16eb,0xc0939ca4,0x1ce02063,0xd25201f1,0x08be6dd2,0x0c0ce5f5,
        0xea2dcf23,0xe2aff802,0x0179c1d4,0x810cc742,0xacf28a22,0x619b009d,
        0x1b44cc9a,0x10658f9e,0x8e077c13,0xa7f6588b,0xe6a4cc6f,0x5a9ed535,
        0x781ff047,0x0050e48f,0x72783f70,0x3074a828,0xa83ac454,0x6d43ff1d,
        0xb4e0a2ea,0x329ca0ad,0xc1608b57,0x37aefdc1,0x3bc320b6,0x55adc0c5,
        0xa8804b0a,0x526bd185,0x1a822ed5,0xe25dbf0f,0xbd03980a,0xc615221b,
        0x29e2e17e,0x46eda917,0x0a270c61,0x047942b8,0x77c56cfe,0x5ddc413c,
        0xd41640f2,0x81f1b301,0x6b999dc2,0xe483ad1b,0x983b69f2,0x3437a02c,
        0x769b3184,0x0c12b7ee,0xfdbaa351,0x24370aa1,0xac635b0a,0xcc6daadf,
        0xc0335178,0x7c1082b0,0xba8c7f88,0xa4c34c18,0xd944b3f3,0xc0709405,
        0x50c0898c,0x055d3ffe,0x4a1a4361,0x9de367e9,0x3041485d,0x8907d3ad,
        0x1b342234,0x8bd3a727,0xaf724c94,0x113fa361,0x82c8c207,0x76394322,
        0xa0c3b465,0xa29e06c3,0x70134a4c,0xc428029a,0xca13dafc,0x4542e7a4,
        0x80a201ca,0xd89fbd10,
        0xefeffff3,0xafffbfef,0xfffbffff,0xdffff7ff,0xfef7fff7,0x37f87eff,
        0xff7ff7df,0xffffebff,0xfdffc9ef,0xefe3fffe,0xfbffffff,0xfbffe6ff,
        0xffffffdf,0xefeffffd,0xfeffffff,0xffffdf77,0xeaebeebe,0xdffffdff,
        0xfdbefffe,0xffbf5fff,0xf77fcfff,0xfffde7ff,0xfff7fdff,0x57fffbbf,
        0xff7b8fbf,0xfff7f79f,0x7fefadff,0x7fffe7fb,0xff7fdffb,0xffffffff,
        0xfffffffb,0xb3ffeee7,0x7fefbf77,0xff6fffe5,0xfffff9ff,0xfffffff7,
        0xffffeeff,0xff7ffffb,0xffeffff7,0xfffefe9f,0xbfffbfed,0xffffbdfe,
        0xff97eeed,0xfffffcff,0xfff7dfff,0xdffdfffd,0xf6ef7fef,0xff7ffdff,
        0xffffddff,0xfff9fcff,0xffdfffdf,0xfffdfbbf,0xfffffeff,0xf7f7fdff,
        0xf77fffff,0xfffffffd,0xfbeffd7f,0xefcffffe,0xaffeefff,0xff7ffbfd,
        0xfff9ffff,0xff7dfe35,0xffffffdf,0xafffe6fa,0xf7cf7fff,0xfeffe9be,
        0xbfe9ffff,0xddbbcffb,0xffffffb5,0xffaffdf7,0x7fdffef7,0x7f57feff,
        0xfddfbffb,0xbbdefffe,0xbfffffff,0xff77ff9f,0xfffd77f6,0x7ff7f3bf,
        0xff7ffffd,0xffeffbff,0xeffeffdf,0x7fffffff,0xebffffff,0xfaffffff,
        0xfbffed7f,0xfffff3ff,0xffbffbfd,0xffffffff,0xf3fbfdff,0x7ffbf37f,
        0x773fffdb,0xfffffb7f,0xf7b9fefe,0xbdfdf3bf,0xffffffff,0xff7f7fdf,
        0xbfffdcff,0xffffaf5f,0xff7ffefb,0xdfffbfff,0xdbffffff,0x3fff5fff,
        0x3ef7ffff,0xffffecff,0xdb3ffffc,0xffefffff,0x736fffff,0xffff7ffa,
        0xdebffff7,0xdfdeffff,0xffffffff,0xbfff7df7,0xfeffd7f5,0xffffffdf,
        0x7ffdefff,0xefffffaf,0xfdfffb7e,0xfdffffff,0xffffffdf,0xffffffff,
        0xfffffff8,0xfbdffbef,0xfeffffe7,0xffffffff,0xf7fffff7,0xfddfef9f,
        0xbdff7fff,0xffbfffff,0xffffb7f7,0xffffff9f,0x3bfebfff,0xfffffbbf,
        0xfefdbfff,0xfffffd3f,0x7d7dbfff,0xfffffd77,0xff7fffff,0xfffff9fc,
        0xfbfa77ff,0xfebffffe,0xffddff7f,0xffffe7ca,0xfff5cbf7,0xfdff7ff7,
        0xfffbffff,0xffffefed,0xfffdffff,0xffeeffff,0xffefffff,0xffdfffdb,
        0xebff3ffe,0xfffffff7,0xffebfffd,0xffbfffff,0xfffefffe,0xfefeffef,
        0xfefffedf,0xfbffffff,0xffdfb7ef,0xfffff7ff,0xdfffffdf,0xf7fffffe,
        0xffffeffd,0xffbfffff,0xffef7fff,0xefff7fff,0xfff3d7df,0xfffdffff,
        0xfffe7dff,0xf39eaff7,0xffbffffb,0xfffdffff,0xfbddffff,0xbfcfff7f,
        0xfff5ffff,0xeefefffd,0xfffffbff,0xfbdfdffe,0xffefffff,0xdcf5bfff,
        0x77fbefff,0xffefffde,0x7ffffdff,0xf7f9ef7f,0xcfefffff,0xffdfffff,
        0xfffef7f7,0xffe1cff7,0xffffdfdf,0xafffbfff,0xfffefff6,0x3b87dfff,
        0xf7fefff7,0xffff7fff,0xeeefd3fd,0xffffefff,0xbfffefff,0xff7f7fff,
        0xffdff5fd,0xffbfdeff,0xfaff7fbf,0x66fedb7f,0xffffffff,0xfdffffff,
        0xffffffdf,0xfffdffff,0xffffff6f,0xf7f7ffff,0xffefffff,0xfebfdbfe,
        0xfbbffffb,0xfeffffff,0xffefffff,0xfdffffff,0xffdebf7f,0xffffffff,
        0xfbffbfff,0xe7ee3ffe,0x7bfebf6b,0xfffbffff,0xffff7fff,0xffffdfff,
        0xfff7fdff,0xfffeffbf,0xffffffff,0xfff7ffff,0xffffffff,0xffddeeff,
        0xfbef77ff,0xfffdfffb,0xfffdffdf,0xffffffff,0xcf7fffff,0xefffbffe,
        0xfefbffff,0xfbffffff,0xcfff7fff,0xf7ff7fff,0xfbe7ebdf,0xffff7ffe,
        0x3ffefeff,0xffffffff,0xb7deeffb,0xfffff9fc,0xbfffdfff,0x7ffffaff,
        0xffffffff,0xffbffffb,0xbfffffff,0x7fbf1bf9,0xbfffefff,0xff3fbfff,
        0x3fffff5f,0xfaf5ffbf,0x5fffff7f,0xfddfcf7f,0xbffbffdf,0xffbfbfeb,
        0xbeffffff,0xf7ffffff,0xdfb5e3ff,0x9a7f7fd7,0xff7defff,0xfffffffd,
        0xff27effb,0xfefeff27,0xffebfbff,0xfb3effff,0xf902efdf,0x7d7ff4df,
        0xfffeff7f,0xff7cfffb,0xfd98cfff,0xfff9feb7,0xfffffffb,0xffffffb7,
        0xd37fffff,0xfaffffdf,0xffff7ff7,0xdfffffff,0xf7dfffbf,0x7feeffff,
        0xf7dfffff,0xffeffffb,0xfffffeff,0x7dffff9f,0xfcdfffff,0xfffff7ff,
        0xeffffffe,0xfbffeeff,0xef9fffff,0xffffcfff,0xffdffdff,0xdfe5fdff,
        0xfd7dfdff,0xfffffffd,0xebf7fffd,0xffcdffff,0xfdffffff,0xfefafbff,
        0xd7fffff7,0x7efff7cf,0x7efffddf,0xbf9d1fdf,0xfffdfffe,0xfc9ffdfd,
        0xfdf7ffde,0xf3ffdf2f,0xffd7fefb,0xffffffff,0xffffffff,0x7fff7fbf,
        0xfbf7fafb,0xffffffbf,0xffffabff,0xfdffdf7e,0xb7ffefef,0xfffffb7f,
        0xffffbfef,0xfcfffd7f,0xeefff7ff,0xf7fffbed,0xaffff7ff,0xf1ebfeff,
        0xdbff7fdf,0xffffff9e,0xff7fdf1f,0xffb7fddf,0xffff4eff,0xffffff77,
        0xfddffe7f,0xffeffdff,0xffefff7e,0xfeffbfd7,0x5bffbffb,0xfffffeff,
        0xefffdefd,0xfeffffff,0xf7ff3fc6,0xff9fffff,0x3efffdf7,0xffd3ffff,
        0xbfffffff,0xfdfffbff,0xff6fffff,0x6ffffff3,0xfbbffffd,0xfeeef9ff,
        0xfffffdfd,0xffff3ff7,0xf6ffff9e,0xf3ffffff,0xefff9fff,0xffffffcf,
        0xefffffbf,0xffe8efff,0xd7f7ffff,0xf5fffefd,0xfdffffbf,0xff7dffff,
        0xffff3fff,0xfffefdff,0xfbedfeff,0xfaefffff,0xffffffff,0xffffffd7,
        0xfffdffff,0xf7d6feef,0xffff7fff,0xffff7fd7,0xffffffff,0xef79efff,
        0xfffeffff,0xf7fefff7,0xfffff7fa,0xf7fdffef,0xf7f1ffff,0xbffbffff,
        0xffefffff,0x8fffffdf,0x6ecfffff,0x7fffffff,0xffdfbfef,0xdf7fde7f,
        0xfefffffe,0xffbfffff,0xffff37df,0xf2fffffc,0xdffffff7,0xffffffff,
        0xbefaffef,0xfffbbfff,0x7bafeffc,0xbffe3ffd,0x7ff9ff8e,0xfff3dffb,
        0xffeffbeb,0x7fffffff,0xfeefcfdf,0xffef7ffb,0xfeff27fa,0xffffffff,
        0xfaa3ffdd,0xfff7fff3,0xfff5ffff,0xfffbfff7,0xffefffff,0xff9c3ff7,
        0xffbefeff,0xffffffff,0xfbfdfffb,0xfe3fffff,0xffdffff7,0xfffbffff,
        0xffffffff,0xfff4ffbf,0xffbfffef,0xffefefff,0xfefdfdff,0xff2efdff,
        0xffdfff7f,0xffbdefd7,0x6fdbfffb,0xfb7bfbff,0xfbdfffef,0xcffffadf,
        0xff37ffcf,0xf9bbf2ff,0xf7efdfff,0xdb73d7ff,0xffd7ffff,0xffffeffd,
        0xff9fffff,0xbf6fffff,0xfffffffe,0xfdfec7ff,0xfffffff3,0xfffcffaf,
        0xfff7ff7f,0xffffffff,0xfffaffd7,0x7efdffbf,0xffeffedf,0xffffdf66,
        0xfffffdf5,0xf7dfbe6f,0xffff7fff,0xdf7bfffc,0xbfffafff,0xfffefeff,
        0xffffffff,0xfffffff8,0xfdffffff,0xffb4fffd,0xffffff7f,0xfffffde7,
        0xfbffefff,0xff5dfffd,0xfeefffff,0xf7f2ffbf,0xf7feffdf,0xfa6fff6d,
        0xfffefdff,0xff9fef7f,0xe7bdffbf,0xfdddf7df,0xf7fff7ff,0xfdfddfff,
        0xddffefff,0xffff7fff,0xf7fdf3ff,0xfbbffdef,0xbfffffff,0xffffffff,
        0xefffbfef,0xff3dcffe,0xffeffbfd,0xbffffddf,0xffffffdf,0xcfff5fdb,
        0xfffffffb,0xfffbf7ff,0xffffffbe,0xdbfeff5f,0xffdffffb,0xf3ff7fff,
        0xfffffee7,0xefdfffcf,0xfffffeff,0xffffffff,0xffbffece,0xfffffbff,
        0xffff7fff,0xfbaffdff,0x7fdbffdf,0xefeffbdf,0xffbeffff,0x7fffffaf,
        0xfefffbff,0xffdffbdb,0xfbbf7fff,0xffffff7f,0xfd7f75ff,0xffd7fff3,
        0xff3ffbbe,0xff7ffeff,0xeffcfdff,0xf7f7ffff,0xf5df75fb,0xbffff7ff,
        0xfffdffff,0xffefffdf,
        0xffffffff,0xffffbfff,0xffffffff,0xdfffffff,0xfff7ffff,0xf7f87fff,
        0xff7fffff,0xffffffff,0xffffffff,0xefe3fffe,0xfbffffff,0xfffffeff,
        0xffffffff,0xefeffffd,0xffffffff,0xffffffff,0xefffffff,0xdfffffff,
        0xffffffff,0xffbfffff,0xff7fffff,0xffffffff,0xfffffdff,0x57fffbbf,
        0xff7fffbf,0xffffffdf,0x7fffedff,0x7ffff7ff,0xffffffff,0xffffffff,
        0xfffffffb,0xb3ffffff,0xffffff7f,0xffffffef,0xfffff9ff,0xffffffff,
        0xffffeeff,0xffffffff,0xfffffff7,0xfffffebf,0xffffffed,0xfffffdfe,
        0xffffffed,0xfffffcff,0xffffdfff,0xdffffffd,0xffff7fef,0xfffffdff,
        0xffffddff,0xfffffcff,0xffffffdf,0xfffffbff,0xffffffff,0xf7fffdff,
        0xf77fffff,0xffffffff,0xfbffffff,0xeffffffe,0xaffeffff,0xff7fffff,
        0xffffffff,0xfffdfe35,0xffffffdf,0xafffffff,0xf7ffffff,0xffffe8be,
        0xbfe9ffff,0xddbfffff,0xffffffb7,0xfffffdf7,0x7fdffeff,0x7f7ffeff,
        0xffffffff,0xbbfffffe,0xbfffffff,0xff7fffff,0xfffffffe,0x7ffff3bf,
        0xff7ffffd,0xfffffbff,0xffffffff,0x7fffffff,0xfbffffff,0xfaffffff,
        0xfbffed7f,0xfffff3ff,0xfffffffd,0xffffffff,0xf3fffdff,0x7ffff37f,
        0x77ffffdb,0xffffff7f,0xf7fdfefe,0xbffdf3bf,0xffffffff,0xffff7fff,
        0xffffdcff,0xffffaf5f,0xfffffefb,0xdfffffff,0xdbffffff,0x3fff5fff,
        0x3ff7ffff,0xffffecff,0xdbfffffc,0xffefffff,0x73efffff,0xfffffffa,
        0xdffffff7,0xffdeffff,0xffffffff,0xbfff7df7,0xfffffff5,0xffffffdf,
        0x7fffefff,0xefffffaf,0xfffffbff,0xfdffffff,0xffffffff,0xffffffff,
        0xffffbfff,0xfbffffef,0xfffffff7,0xffffffff,0xffffffff,0xffffff9f,
        0xbfffffff,0xffffffff,0xffffb7ff,0xffffff9f,0x3fffffff,0xfffffbff,
        0xfffdbfff,0xfffffd3f,0x7d7fffff,0xffffffff,0xffffffff,0xfffff9fc,
        0xfbfbf7ff,0xffbfffff,0xffddffff,0xffffe7ca,0xfff7cbff,0xffff7ff7,
        0xfffbffff,0xffffefed,0xffffffff,0xffeeffff,0xffefffff,0xffffffdb,
        0xfffffffe,0xffffffff,0xffebffff,0xffffffff,0xfffffffe,0xfeffffff,
        0xffffffff,0xfbffffff,0xfffff7ef,0xffffffff,0xffffffff,0xf7ffffff,
        0xffffeffd,0xffffffff,0xffffffff,0xefff7fff,0xffffd7df,0xfffdffff,
        0xffffffff,0xf3deeff7,0xffffffff,0xfffdffff,0xffffffff,0xbfcfff7f,
        0xfffdffff,0xfffeffff,0xfffffbff,0xfbdfdffe,0xffffffff,0xfff5bfff,
        0xfffbefff,0xffefffde,0xfffffdff,0xf7f9ff7f,0xffefffff,0xffdfffff,
        0xfffef7ff,0xffe1fff7,0xffffdfff,0xffffffff,0xfffefff6,0x3f87ffff,
        0xffffffff,0xffffffff,0xeeefd3fd,0xffffffff,0xffffefff,0xff7fffff,
        0xffdffffd,0xffbffeff,0xfeff7fff,0x76ffdb7f,0xffffffff,0xfdffffff,
        0xffffffdf,0xffffffff,0xffffffef,0xf7ffffff,0xffefffff,0xfebfdbff,
        0xfbbffffb,0xfeffffff,0xffefffff,0xfdffffff,0xfffebf7f,0xffffffff,
        0xffffbfff,0xf7feffff,0x7bfebf6b,0xffffffff,0xffff7fff,0xffffdfff,
        0xfffffdff,0xfffeffff,0xffffffff,0xffffffff,0xffffffff,0xfffdffff,
        0xffefffff,0xfffdffff,0xfffdffdf,0xffffffff,0xffffffff,0xeffffffe,
        0xfefbffff,0xfbffffff,0xffff7fff,0xffffffff,0xfbe7ebdf,0xffff7fff,
        0xfffeffff,0xffffffff,0xb7deefff,0xffffffff,0xffffdfff,0x7fffffff,
        0xffffffff,0xffbfffff,0xffffffff,0x7fbf1ff9,0xbfffffff,0xff3fbfff,
        0xffffff5f,0xfafdffbf,0x7fffff7f,0xfdffffff,0xfffbffdf,0xffbfbfeb,
        0xffffffff,0xf7ffffff,0xdfb5e3ff,0x9a7f7fd7,0xff7defff,0xffffffff,
        0xff27effb,0xfefeff27,0xffebfbff,0xfb3fffff,0xf902efdf,0x7d7ff4df,
        0xffffff7f,0xff7fffff,0xfd98cfff,0xfff9feb7,0xfffffffb,0xffffffff,
        0xd37fffff,0xfaffffdf,0xffff7ff7,0xdfffffff,0xf7dfffff,0x7feeffff,
        0xffdfffff,0xffffffff,0xffffffff,0x7dffffbf,0xfcdfffff,0xffffffff,
        0xeffffffe,0xfbffeeff,0xff9fffff,0xffffffff,0xffdfffff,0xdfe5fdff,
        0xfd7ffdff,0xfffffffd,0xebf7fffd,0xffcdffff,0xfdffffff,0xfefbfbff,
        0xd7ffffff,0x7efff7ff,0x7efffddf,0xbfdf1fff,0xfffdfffe,0xfc9ffdfd,
        0xfdffffde,0xf3ffdfef,0xffd7fffb,0xffffffff,0xffffffff,0x7fff7fff,
        0xfff7fffb,0xffffffff,0xffffefff,0xffffffff,0xbfffffef,0xffffffff,
        0xffffbfef,0xffffffff,0xfeffffff,0xf7fffffd,0xafffffff,0xffffffff,
        0xffff7fdf,0xffffffbe,0xff7fffdf,0xffbfffdf,0xffff7fff,0xffffff77,
        0xfdffffff,0xffffffff,0xffffff7f,0xfefffff7,0xdffffffb,0xffffffff,
        0xfffffeff,0xfeffffff,0xffffffc6,0xffffffff,0xfefffdf7,0xffffffff,
        0xffffffff,0xffffffff,0xffefffff,0x6fffffff,0xfffffffd,0xffefffff,
        0xfffffdff,0xffffffff,0xffffff9f,0xffffffff,0xffff9fff,0xffffffff,
        0xffffffbf,0xffffffff,0xfff7ffff,0xf7ffffff,0xffffffbf,0xffffffff,
        0xffff3fff,0xffffffff,0xfffffeff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffff7fff,0xffff7fff,0xffffffff,0xffffffff,
        0xfffeffff,0xf7fefff7,0xffffffff,0xfffdffff,0xfff1ffff,0xbfffffff,
        0xffffffff,0xffffffff,0x7ecfffff,0x7fffffff,0xffffffff,0xffffffff,
        0xfefffffe,0xffbfffff,0xffffffff,0xfffffffc,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xfbfffffc,0xbffe3fff,0xffffffbf,0xffffffff,
        0xffeffbeb,0x7fffffff,0xffffcfff,0xffffffff,0xfffff7fa,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffefffff,0xffffffff,
        0xfffeffff,0xffffffff,0xfffdffff,0xffffffff,0xfffffff7,0xffffffff,
        0xffffffff,0xffffffff,0xffffffef,0xffffffff,0xfefdfdff,0xffffffff,
        0xffffff7f,0xfffdffff,0xffffffff,0xfffffbff,0xffffffef,0xfffffbff,
        0xffffffff,0xfffff3ff,0xffffffff,0xfffff7ff,0xffdfffff,0xffffefff,
        0xffffffff,0xffffffff,0xffffffff,0xfffec7ff,0xfffffff3,0xfffcffff,
        0xffffff7f,0xffffffff,0xfffbffd7,0xfffdffff,0xfffffeff,0xffffdfff,
        0xfffffdff,0xf7ffbfff,0xffff7fff,0xffffffff,0xffffefff,0xfffeffff,
        0xffffffff,0xffffffff,0xffffffff,0xfff4fffd,0xffffff7f,0xffffffff,
        0xffffefff,0xffddfffd,0xffefffff,0xf7feffff,0xffffffff,0xffefffed,
        0xfffeffff,0xffffffff,0xffbfffff,0xffddffdf,0xffffffff,0xfffdffff,
        0xfdffefff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xfffdffff,0xffffffff,0xfffffddf,0xffffffff,0xffffffff,
        0xffffffff,0xfffbf7ff,0xffffffff,0xffffffff,0xfffffffb,0xffffffff,
        0xffffffff,0xffdfffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0x7fdbffff,0xffefffff,0xffffffff,0xffffffaf,
        0xfffffbff,0xffdfffff,0xffff7fff,0xffffff7f,0xfffff7ff,0xfffffff3,
        0xffffffff,0xfffffeff,0xffffffff,0xf7ffffff,0xffdfffff,0xfffff7ff,
        0xffffffff,0xffffffdf,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,
        0xffffffff,0xffffffff,
    };

static const ULONG patterncoltab[] =
    {
        (16L << 16) + 0,    /* 16 colors, loaded at index 0 */

        0xB3B3B3B3, 0xB3B3B3B3, 0xB3B3B3B3, /* Grey70   */
        0x00000000, 0x00000000, 0x00000000, /* Black    */
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, /* White    */
        0x66666666, 0x88888888, 0xBBBBBBBB, /* AMIGA Blue   */

        0x00000000, 0x00000000, 0xFFFFFFFF, /* Blue     */
        0x00000000, 0xFFFFFFFF, 0x00000000, /* Green    */
        0xFFFFFFFF, 0x00000000, 0x00000000, /* Red      */
        0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, /* Cyan     */

        0x33333333, 0x33333333, 0x33333333, /* Pattern Col 1 */
        0xcdcdcdcd, 0x6c6c6c6c, 0xc7c7c7c7, /* Pattern Col 2 */
        0x8e8e8e8e, 0x85858585, 0x93939393, /* Pattern Col 3 */
        0x22222222, 0x22222222, 0x22222222, /* Pattern Col 4 */

        0x77777777, 0x77777777, 0x77777777, /* Pattern Col 5 */
        0x66666666, 0x66666666, 0x66666666, /* Pattern Col 6 */
        0x55555555, 0x55555555, 0x55555555, /* Pattern Col 7 */
        0x44444444, 0x44444444, 0x44444444, /* Pattern Col 8 */

        0L      /* Termination */
    };

struct LayerHookMsg
{
    struct Layer     *lay;      /* not valid for layerinfo backfill hook!!! */
    struct Rectangle  bounds;
    LONG    	      offsetx;
    LONG    	      offsety;
};

static struct Hook backfillhook;
static struct BitMap *patternbm;

extern ULONG HookEntry();

static void MyBackfillFunc(struct Hook *hook,struct RastPort *rp, struct LayerHookMsg *msg)
{
    struct IntuitionBase *IntuitionBase = (struct IntuitionBase *)hook->h_Data;
    struct RastPort 	  myrp;
    WORD    	    	  x1,y1,x2,y2,px,py,pw,ph;

    myrp = *rp;

    myrp.Layer = 0;

    x1 = msg->bounds.MinX;
    y1 = msg->bounds.MinY;
    x2 = msg->bounds.MaxX;
    y2 = msg->bounds.MaxY;

    px = x1 % PATTERN_WIDTH;

    pw = PATTERN_WIDTH - px;

    do
    {
        y1 = msg->bounds.MinY;
        py = y1  % PATTERN_HEIGHT;

        ph = PATTERN_HEIGHT - py;

        if (pw > (x2 - x1 + 1)) pw = x2 - x1 + 1;

        do
        {
            if (ph > (y2 - y1 + 1)) ph = y2 - y1 + 1;

            BltBitMap(patternbm,
                      px,
                      py,
                      rp->BitMap,
                      x1,
                      y1,
                      pw,
                      ph,
                      192,
                      255,
                      0);

            y1 += ph;

            py = 0;
            ph = PATTERN_HEIGHT;

        }
        while (y1 <= y2); /* while(y1 < y2) */

        x1 += pw;

        px = 0;
        pw = PATTERN_WIDTH;

    }
    while (x1 <= x2); /* while (x1 < x2) */

}

static void InitBackfillHook(struct IntuitionBase *IntuitionBase)
{
    backfillhook.h_Entry = HookEntry;
    backfillhook.h_SubEntry = (HOOKFUNC)MyBackfillFunc;
    backfillhook.h_Data = IntuitionBase;
}

static void MakeWBPattern(struct Screen *scr, struct IntuitionBase *IntuitionBase)
{
    static struct BitMap bm;

    struct Window *tempwin;
    struct TagItem wintags[] =
    {
        {WA_PubScreen   ,(IPTR)scr	    	},
        {WA_Left        ,0              	},
        {WA_Top         ,0              	},
        {WA_Width       ,scr->Width             },
        {WA_Height      ,scr->Height            },
        {WA_Borderless  ,TRUE                   },
        {WA_Backdrop    ,TRUE                   },
        {WA_BackFill    ,(IPTR)LAYERS_NOBACKFILL},
        {TAG_DONE                       	}
    };
    WORD i;

    InitBitMap(&bm, PATTERN_DEPTH, PATTERN_WIDTH, PATTERN_HEIGHT);
    for(i = 0; i < PATTERN_DEPTH; i++)
    {
        bm.Planes[i] = (((UBYTE *)patterndata) + (i * PATTERN_PLANESIZE));
    }

    LoadRGB32(&scr->ViewPort, patterncoltab);


    /* nlorentz: Allocate a bitmap with the screen bitmap as friend. This
       way the HIDD knows how to blit from this bitmap onto the screen.

    */
    patternbm = AllocBitMap(PATTERN_WIDTH, PATTERN_HEIGHT, PATTERN_DEPTH,
    	    	    	    0, scr->RastPort.BitMap);

    if (NULL != patternbm)
    {
        /* Blit the pattern into the allocated bitmap */
        BltBitMap(&bm , 0, 0, patternbm,
                  0, 0, PATTERN_WIDTH, PATTERN_HEIGHT,
                  0xC0, 0xFF, NULL);


        InitBackfillHook(IntuitionBase);
        InstallLayerInfoHook(&scr->LayerInfo, &backfillhook);
        tempwin = OpenWindowTagList(0,wintags);
        if (tempwin) CloseWindow(tempwin);

    }
    else
    {
        /* If this doesn't succeed, then there is someting seriously wrong */
        kprintf("COULD NOT CREATE PATTERN BITMAP in rom/intuition/lateintuiinit.c\n");
        Alert(AT_DeadEnd);
    }
}

#endif

