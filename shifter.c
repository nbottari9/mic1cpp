#include <strings.h>
#include "globals.h"

void ShiftLeft (DataBusType InputBits, DataBusType ShifterOutput)
{
int i ;

    for (i = 0 ; i < DataWordSize-1 ; i++)
       ShifterOutput[i] = InputBits[i+1] ;

    ShifterOutput[DataWordSize-2] = Zero ;
    
}			/* END ShiftLeft */

void ShiftRight (DataBusType InputBits, DataBusType ShifterOutput)
{
int i ;

    ShifterOutput[0] = Zero ;

    for (i = 1 ; i < DataWordSize-1 ; i++)
        ShifterOutput[i] = InputBits[i-1] ;


}			/* END ShiftRight */

void ActivateShifter (DataBusType InputBits, TwoBits ShiftBits, DataBusType ShifterOutput)
{

    Bit ShiftBit0, ShiftBit1 ;

    ShiftBit0  =  ShiftBits[0] ;
    ShiftBit1  =  ShiftBits[1] ;

    strcpy (ShifterOutput, InputBits) ;

    if ((ShiftBit0 == Zero) && (ShiftBit1 == One))

        ShiftRight (InputBits, ShifterOutput) ;

    else if

      ((ShiftBit0 == One) && (ShiftBit1 == Zero))

         ShiftLeft (InputBits, ShifterOutput) ;

}			/* END ActivateShifter */
