/*
    (C) 2001 AROS - The Amiga Research OS
    $Id$

    Desc: 
    Lang: English
*/

#include <proto/exec.h>

#include "camd_intern.h"


/* 
	CLSemaphore must be obtained first.
*/

void UnlinkMidiLink(
	struct MidiLink *midilink,
	BOOL unlinkfromnode,
	struct CamdBase *CamdBase
){
	struct MidiCluster *cluster=midilink->ml_Location;
	int type=midilink->ml_Node.ln_Type;
	struct MyMidiCluster *mycluster=(struct MyMidiCluster *)cluster;

	if(cluster!=NULL){
		if(type==NT_USER-MLTYPE_Receiver){
			ObtainExclusiveSem(&mycluster->mutex);
		}
		Remove(&midilink->ml_Node);
	}

	if(unlinkfromnode==TRUE){
		Remove((struct Node *)&midilink->ml_OwnerNode);
	}

	if(cluster!=NULL){
		if(type==NT_USER-MLTYPE_Receiver){
			ReleaseExclusiveSem(&mycluster->mutex);
		}
		D(bug("here3\n"));
		LinkHasBeenRemovedFromCluster(cluster,CamdBase);
	}

}

