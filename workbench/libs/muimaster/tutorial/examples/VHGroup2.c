/*
    Copyright � 2003, The AROS Development Team.
    All rights reserved.

    $Id$
*/

/* "muizunesupport.h" contains misc includes and 
   init stuff which is not important at the moment. */
   
#include "muizunesupport.h"


/* Objects */

Object *app;
Object *WD_Main;
Object *BT_11_13;
Object *BT_21,    *BT_22,    *BT_23;
Object *BT_31,    *BT_32_33;


/****************************************************************
 Allocalte resources for gui
*****************************************************************/

BOOL init_gui(void)
{
    app = ApplicationObject,
    	      MUIA_Application_Title      , (IPTR) "VHGroup2",
	      MUIA_Application_Version    , (IPTR) "$VER: VHGroup2 0.1 (14.01.03)",
	      MUIA_Application_Copyright  , (IPTR) "� 2003, The AROS Development Team", 
	      MUIA_Application_Author     , (IPTR) "The AROS Development Team",
	      MUIA_Application_Description, (IPTR) "Layout with VGroup + HGroup 2",
	      MUIA_Application_Base       , (IPTR) "VHGroup2",
		
    	      SubWindow, WD_Main = WindowObject,
                  MUIA_Window_Title, (IPTR) "Layout with VGroup + HGroup 2",

                  /* 
		     Start VGroup of window here. All other GUI objects are childs
		     of this VGroup. 
		  */
		  
   		  WindowContents, VGroup,
		  
		      /* 
		          Layout: VGroup + HGroup for a more complex layout

		          A group could have any size and can contain any number of
		          GUI objects. This allows more complex layouts.
		     		           
			  |              Button 1.1 - 1.3              |
			  | Button 2.1 |  | Button 2.2 |  | Button 2.3 |
			  |    Button 3.1    |  |   Button 3.2 - 3.3   |
		      */
		      
		      
		      /* Horizontal line 1 */
		      
     	    	      Child, HGroup,
		          Child, BT_11_13 = SimpleButton("Button 1.1 - 1.3"),
		      End, /* HGroup */ 	


		      /* Horizontal line 2 */
		      
     	    	      Child, HGroup,
		          Child, BT_21 = SimpleButton("Button 2.1"),
			  Child, BT_22 = SimpleButton("Button 2.2"),
			  Child, BT_23 = SimpleButton("Button 2.3"),
		      End, /* HGroup */ 		      
		      
		        
		      /* Horizontal line 3 */
		      
     	    	      Child, HGroup,
		          Child, BT_31    = SimpleButton("Button 3.1"),
			  Child, BT_32_33 = SimpleButton("Button 3.2 - 3.3"),
		      End, /* HGroup */ 
		      
		      			
    	          End, /* VGroup */
		 
              End, /* WindowObject */

    	  End; /* ApplicationObject */

    if(app)
    {
        /* Quit application if the windowclosegadget or the esc key is pressed. */
	
	DoMethod(WD_Main, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, 
	         app, 2, 
		 MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

	return(TRUE);
    }
    
    return(FALSE);
} /* init_gui(void) */


/****************************************************************
 Deallocates all gui resources
*****************************************************************/

void deinit_gui(void)
{
    if(app){MUI_DisposeObject(app);}
} /* deinit_gui(void) */



/****************************************************************
 The message loop
*****************************************************************/

void loop(void)
{
    ULONG sigs = 0;

    while((LONG) DoMethod(app, MUIM_Application_NewInput, &sigs) != MUIV_Application_ReturnID_Quit)
    {
	if (sigs)
	{
	    sigs = Wait(sigs | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D);
	    if(sigs & SIGBREAKF_CTRL_C){break;}
	    if(sigs & SIGBREAKF_CTRL_D){break;}
	}
    }
} /* loop(void)*/


/****************************************************************
 The main entry point
*****************************************************************/

int main(int argc, char *argv[])
{
    if(open_libs())
    {
    	if(init_gui())
    	{
      	    set(WD_Main, MUIA_Window_Open, TRUE);
	    
    	    if(xget(WD_Main, MUIA_Window_Open))
	    {
                loop();
	    }

      	    set(WD_Main, MUIA_Window_Open, FALSE);

	    deinit_gui();
    	}
	
	close_libs();
    }
} /* main(int argc, char *argv[]) */
