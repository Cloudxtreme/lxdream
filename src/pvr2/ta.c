/**
 * $Id: ta.c,v 1.1 2006-02-15 13:11:46 nkeynes Exp $
 *
 * PVR2 Tile Accelerator support. In principle this does a lot more work
 * than is currently implemented - we cheat. A lot.
 *
 * Copyright (c) 2005 Nathan Keynes.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "pvr2/pvr2.h"
#include "asic.h"
#include "dream.h"

/** Tile Accelerator */

struct tacmd {
    uint32_t command;
    uint32_t param1;
    uint32_t param2;
    uint32_t texture;
    float alpha;
    float red;
    float green;
    float blue;
};

struct vertex_type1 {
    uint32_t command;
    float x, y, z;
    uint32_t blank, blank2;
    uint32_t col;
    float f;
};

/**
 * (Re)initialize the tile accelerator in preparation for the next scene.
 * Normally called immediately before commencing polygon transmission.
 */
void pvr2_ta_init( void )
{

}

char pvr2ta_remainder[8];
unsigned int pvr2_last_poly_type = 0;

/**
 * Write a block of data to the tile accelerator, adding the data to the 
 * current scene. We don't make any particular attempt to interpret the data
 * at this stage, deferring that until render time.
 */
void pvr2_ta_write( char *buf, uint32_t length )
{
    int i;
    struct tacmd *cmd_list = (struct tacmd *)buf;
    int count = length >> 5;
    for( i=0; i<count; i++ ){
	unsigned int type = (cmd_list[i].command >> 24) & 0xFF;
	if( type == 0xE0 || type == 0xF0 ) {
	    struct vertex_type1 *vert = (struct vertex_type1 *)&cmd_list[i];
	    DEBUG( "PVR2 vrt: %f %f %f %08X %08X %08X %f", vert->x, vert->y, vert->z, vert->blank, vert->blank2, vert->col, vert->f );
	} else {
	    DEBUG( "PVR2 cmd: %08X %08X %08X %08X %08X %08X %08X %08X", cmd_list[i].command, cmd_list[i].param1, cmd_list[i].param2, cmd_list[i].texture, cmd_list[i].alpha, cmd_list[i].red, cmd_list[i].green, cmd_list[i].blue );
	}
	if( type == 0 ) {
	    /* End of list */
	    switch( pvr2_last_poly_type ) {
	    case 0x80: /* Opaque polys */
		asic_event( EVENT_PVR_OPAQUE_DONE );
		break;
	    case 0x81: /* Opaque poly modifier */
		asic_event( EVENT_PVR_OPAQUEMOD_DONE );
		break;
	    case 0x82: /* Transparent polys */
		asic_event( EVENT_PVR_TRANS_DONE );
		break;
	    case 0x83: /* Transparent poly modifier */
		asic_event( EVENT_PVR_TRANSMOD_DONE );
		break;
	    case 0x84: /* Punchthrough */
		asic_event( EVENT_PVR_PUNCHOUT_DONE );
		break;
	    }
	    pvr2_last_poly_type = 0;
	} else if( type >= 0x80 && type <= 0x84 ) {
	    pvr2_last_poly_type = type;
	}
    }
}