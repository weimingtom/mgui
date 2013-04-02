/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Text.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		A struct for text buffer and its properties.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "Text.h"
#include "Renderer.h"
#include "Skin.h"
#include "Stringy/Stringy.h"
#include "Platform/Alloc.h"
#include <stdio.h>
#include <assert.h>

extern MGuiRenderer* renderer;

MGuiText* mgui_text_create( void )
{
	MGuiText* text;
	text = (MGuiText*)mem_alloc_clean( sizeof(*text) );

	text->alignment = ALIGN_CENTER;
	text->colour.hex = COL_TEXT;

	return text;
}

void mgui_text_destroy( MGuiText* text )
{
	assert( text != NULL );

	if ( text->buffer )
		mem_free( text->buffer );

	mem_free( text );
}

void mgui_text_set_buffer( MGuiText* text, const char_t* fmt, ... )
{
	size_t	len;
	char_t	tmp[512];
	va_list	marker;

	assert( text != NULL );

	if ( text->buffer )
		mem_free( text->buffer );

	va_start( marker, fmt );
	len = msnprintf( tmp, lengthof(tmp), fmt, marker );
	va_end( marker );

	len = math_max( len, text->bufsize );

	text->buffer = mstrdup( tmp, len );
	text->len = mstrlen( text->buffer );
	text->bufsize = len + 1;

	mgui_text_update_dimensions( text );
}

void mgui_text_set_buffer_s( MGuiText* text, const char_t* str )
{
	size_t len;

	assert( text != NULL );

	if ( text->buffer )
		mem_free( text->buffer );

	len = mstrlen( str );
	len = math_max( len, text->bufsize );

	text->buffer = mstrdup( str, len );
	text->len = mstrlen( text->buffer );
	text->bufsize = len + 1;

	mgui_text_update_dimensions( text );
}

void mgui_text_set_buffer_va( MGuiText* text, const char_t* fmt, va_list list )
{
	size_t	len;
	char_t	tmp[512];

	assert( text != NULL );

	if ( text->buffer )
		mem_free( text->buffer );

	len = msnprintf( tmp, lengthof(tmp), fmt, list );
	len = math_max( len, text->bufsize );

	text->buffer = mstrdup( tmp, len );
	text->len = mstrlen( text->buffer );
	text->bufsize = len + 1;

	mgui_text_update_dimensions( text );
}

void mgui_text_update_dimensions( MGuiText* text )
{
	uint32 w, h;

	assert( text != NULL );

	renderer->measure_text( text->font->data, text->buffer, &w, &h );
	h -= 2;

	text->size.x = (uint16)w;
	text->size.y = (uint16)h;

	mgui_text_update_position( text );
}

void mgui_text_update_position( MGuiText* text )
{
	uint16 x, y, w, h;

	assert( text != NULL );
	assert( text->bounds != NULL );

	x = text->bounds->x + text->pad.left;
	y = text->bounds->y + text->pad.top - text->font->size;
	w = text->bounds->w - text->pad.left - text->pad.right;
	h = text->bounds->h - text->pad.top - text->pad.bottom;

	switch ( text->alignment )
	{
	case ALIGN_LEFT:
	case ALIGN_LEFT|ALIGN_CENTERV:
		text->pos.x = x;
		text->pos.y = y + text->size.y + ( h - text->size.y ) / 2;
		break;

	case ALIGN_LEFT|ALIGN_TOP:
		text->pos.x = x;
		text->pos.y = y + text->size.y;
		break;

	case ALIGN_TOP:
	case ALIGN_TOP|ALIGN_CENTERH:
		text->pos.x = x + ( w - text->size.x ) / 2;
		text->pos.y = y + text->size.y;
		break;

	case ALIGN_RIGHT|ALIGN_TOP:
		text->pos.x = x + w - text->size.x;
		text->pos.y = y + text->size.y;
		break;

	case ALIGN_RIGHT:
	case ALIGN_RIGHT|ALIGN_CENTERV:
		text->pos.x = x + w - text->size.x;
		text->pos.y = y + text->size.y + ( h - text->size.y ) / 2;
		break;

	case ALIGN_RIGHT|ALIGN_BOTTOM:
		text->pos.x = x + w - text->size.x;
		text->pos.y = y + h;
		break;

	case ALIGN_BOTTOM:
	case ALIGN_BOTTOM|ALIGN_CENTERH:
		text->pos.x = x + ( w - text->size.x ) / 2;
		text->pos.y = y + h;
		break;

	case ALIGN_LEFT|ALIGN_BOTTOM:
		text->pos.x = x;
		text->pos.y = y + h;
		break;

	case ALIGN_CENTER:
	case ALIGN_CENTERH:
	case ALIGN_CENTERV:
	default:
		text->pos.x = x + ( w - text->size.x ) / 2;
		text->pos.y = y + text->size.y + ( h - text->size.y ) / 2;
		break;
	}
}

uint32 mgui_text_get_closest_char( MGuiText* text, uint16 x, uint16 y )
{
	uint32 dist, ch, i, tmp;
	uint16 cx, cy;

	// We only calculate the dist along the x axis for now
	UNREFERENCED_PARAM( y );

	dist = 0xFFFF;
	ch = 0;

	assert( text != NULL );

	for ( i = 0; i < text->len + 1; i++ )
	{
		mgui_text_get_char_pos( text, i, &cx, &cy );
		tmp = math_abs( cx - x );

		if ( tmp < dist )
		{
			dist = tmp;
			ch = i;
		}
	}

	return ch;
}

void mgui_text_get_char_pos( MGuiText* text, uint32 idx, uint16* x, uint16* y )
{
	char_t tmp[512];
	uint32 w, h;

	assert( text != NULL );

	if ( text->len == 0 || idx == 0 )
	{
		*x = 0;
		*y = text->font->size;
		return;
	}

	mstrcpy( tmp, text->buffer, math_min( idx+1, lengthof(tmp) ) );
	renderer->measure_text( text->font->data, tmp, &w, &h );

	*x = (uint16)w;
	*y = (uint16)h;
}
