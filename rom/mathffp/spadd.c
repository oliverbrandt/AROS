/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$
    $Log$
    Revision 1.4  1997/09/16 23:00:46  bergers
    Added the missing AROS_LIBFUNC_INITs and EXITs

    Revision 1.3  1997/07/21 20:56:40  bergers
    *** empty log message ***

    Revision 1.2  1997/06/25 21:36:44  bergers
    Corrections

    Revision 1.1  1997/05/30 20:50:57  aros
    *** empty log message ***


    Desc:
    Lang: english
*/
#include <libraries/mathffp.h>
#include <aros/libcall.h>
#include <proto/mathffp.h>
#include <proto/exec.h>
#include <exec/types.h>
#include "mathffp_intern.h"

/*****************************************************************************

    NAME */

        AROS_LH2(LONG, SPAdd,

/*  SYNOPSIS */
        AROS_LHA(LONG, fnum1, D1),
        AROS_LHA(LONG, fnum2, D0),

/*  LOCATION */
        struct MathBase *, MathBase, 11, Mathffp)

/*  FUNCTION
        Calculate the sum of two ffp numbers

    INPUTS
        fnum1  - ffp number to add
        fnum2  - ffp number to add

    RESULT
        sum of fnum1 and fnu2

        Flags:
          zero     : result is zero
          negative : result is negative
          overflow : result is too large or too small for ffp format

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO


    INTERNALS
      ALGORITHM:
        Adapt the exponent of the ffp-number with the smaller
        exponent to the ffp-number with the larger exponent.
        Therefore rotate the mantisse of the ffp-number with the
        smaller exponents by n bits, where n is the absolute value
        of the difference of the exponents.
        The exponent of the target ffp-number is set to the larger
        exponent plus 1.
        Additionally rotate both numbers by one bit to the right so
        you can catch a result >1 in the MSB.
        If the signs of the two numbers are equal then simply add
        the two mantisses. The result of the mantisses will be
        [0.5 .. 2[. Check the MSB. If zero, then the result is < 1
        and therefore subtract 1 from the exponent. Normalize the
        mantisse of the result by rotating it one bit to the left.
        Check the mantisse for 0.
        If the signs of the two numbers are different then subtract
        the ffp-number with the neagtive sign from the other one.
        The result of the mantisse will be [-1..1[. If the MSB of
        the result is set, then the result is below zero and therefore
        you have to calculate the absolute value of the mantisse.
        Check the mantisse for zero. Normalize the mantisse by
        rotating it to the left and decreasing the exponent for every
        rotation.
        Test the exponent of the result for an overflow.
        That`s it!

    HISTORY

******************************************************************************/

{
AROS_LIBFUNC_INIT
  LONG Res;
  ULONG Mant1, Mant2;
  char Shift;
  char Exponent;

  SetSR(0, Zero_Bit | Overflow_Bit | Negative_Bit );

  Mant1 = fnum1 & FFPMantisse_Mask;
  Mant2 = fnum2 & FFPMantisse_Mask;
  Shift = ((char)fnum1 & FFPExponent_Mask) -
          ((char)fnum2 & FFPExponent_Mask);

  if (Shift > 0)
  {
    if (Shift >= 31)
      Mant2 = 0;
    else
      Mant2 >>= (Shift + 1);
    Mant1 >>= 1;
    Exponent = (fnum1 & FFPExponent_Mask) + 1;
  }
  else
  {
    if (Shift <= -31)
      Mant1 = 0;
    else
      Mant1 >>= (-Shift + 1);
    Mant2 >>= 1;
    Exponent = (fnum2 & FFPExponent_Mask) + 1;
  }

  /* sign(fnum1) == sign(fnum2)
  ** simple addition
  ** 0.5 <= res < 2
  */
  if ( ((BYTE) fnum1 & FFPSign_Mask) - ((BYTE) fnum2 & FFPSign_Mask) == 0)
  {
    Res = fnum1 & FFPSign_Mask;
    Mant1 += Mant2;
    if ((LONG) Mant1 > 0)
    {
      Exponent --;
      Mant1 +=Mant1;
    }

    if (0 == Mant1)
    {
      SetSR(Zero_Bit, Zero_Bit | Negative_Bit | Overflow_Bit);
      return 0;
    }
  }
    /* second case: sign(fnum1) != sign(fnum2)
    ** -1 <= res < 1
    */
  else
  {
    if ((char) fnum1 < 0)
      Mant1 = Mant2 - Mant1;
    else /* fnum2 < 0 */
      Mant1 = Mant1 - Mant2;
    /* if the result is below zero */
    if ((LONG) Mant1 < 0)
    {
      Res = FFPSign_Mask;
      Mant1 =-Mant1;
      SetSR(Negative_Bit, Zero_Bit | Negative_Bit | Overflow_Bit);
    }
    else
      Res = 0;
      
    /* test the result for zero, has to be done before normalizing
    ** the mantisse
    */
    if (0 == Mant1)
    {
      SetSR(Zero_Bit, Zero_Bit | Overflow_Bit | Negative_Bit);
      return 0;
    }
    /* normalize the mantisse */
      while ((LONG) Mant1 > 0)
      {
        Mant1 += Mant1;  /* one bit to the left. */
        Exponent --;
      }

  } /* else */

  if ((char) Exponent < 0)
  {
    SetSR(Overflow_Bit, Zero_Bit | Overflow_Bit);
    /* do NOT change Negative_Bit! */
    return (Res | (FFPMantisse_Mask | FFPExponent_Mask));
  }

  Res |= (Mant1 & FFPMantisse_Mask) | Exponent;
  return Res;
AROS_LIBFUNC_EXIT
} /* SPAdd */

