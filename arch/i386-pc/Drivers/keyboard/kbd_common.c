/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc: PS/2 mouse driver.
    Lang: English.
*/

#include "kbd.h"

void mouse_usleep(ULONG usec);

unsigned char handle_kbd_event(void)
{
    unsigned char status = kbd_read_status();
    unsigned int work = 10000;
		
    while (status & KBD_STATUS_OBF)
    {
        unsigned char scancode;
					
        scancode = kbd_read_input();

        status = kbd_read_status();
	if(!work--)
        {
	    //printf(KERN_ERR "pc_keyb: controller jammed (0x%02X).\n",status);
            break;
	}
    }
    return status;
}															

void kb_wait(void)
{
    ULONG timeout = 1000; /* 1 sec should be enough */
    
    do
    {
	unsigned char status = handle_kbd_event();
	if (! (status & KBD_STATUS_IBF))
	    return;
	
	mouse_usleep(1000);
	timeout--;
    } while (timeout);
}

void kbd_write_cmd(int cmd)
{
    kb_wait();
    kbd_write_command(KBD_CTRLCMD_WRITE_MODE);
    kb_wait();
    kbd_write_output(cmd);
}

void aux_write_ack(int val)
{
    kb_wait();
    kbd_write_command(KBD_CTRLCMD_WRITE_MOUSE);
    kb_wait();;
    kbd_write_output(val);
    kb_wait();
}

void kbd_write_output_w(int data)
{
    kb_wait();
    kbd_write_output(data);
}

void kbd_write_command_w(int data)
{
    kb_wait();
    kbd_write_command(data);
}

#define KBD_NO_DATA	(-1)
#define KBD_BAD_DATA	(-2)

int kbd_read_data(void)
{
    LONG	retval = KBD_NO_DATA;
    UBYTE	status;
    
    status = kbd_read_status();
    if (status & KBD_STATUS_OBF)
    {
        UBYTE	data = kbd_read_input();
	
        retval = data;
        if (status & (KBD_STATUS_GTO | KBD_STATUS_PERR))
            retval = KBD_BAD_DATA;
    }
    
    return retval;
}

void kbd_clear_input(void)
{
    int maxread = 100;

    do
    {
        if (kbd_read_data() == KBD_NO_DATA)
            break;
    } while (--maxread);
}

int kbd_wait_for_input(void)
{
    ULONG timeout = 1000;

    do
    {
        int retval = kbd_read_data();
        if (retval >= 0)
            return retval;
        mouse_usleep(1000);
    } while(--timeout);
    return -1;
}

