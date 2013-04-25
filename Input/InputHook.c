/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		InputHook.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI input handler.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "InputHook.h"
#include "Element.h"
#include "Renderer.h"

static MGuiElement*	hovered			= NULL; // Element being hovered currently
static MGuiElement*	pressed			= NULL; // Element being pressed down currently
static MGuiElement*	dragged			= NULL;	// Element that is being dragged
static MGuiElement*	mousefocus		= NULL; // The element that has the mouse focus
static MGuiElement*	kbfocus			= NULL;	// The element that has the keyboard focus

void mgui_input_initialize_hooks( void )
{
	input_add_hook( INPUT_CHARACTER, mgui_input_handle_char );
	input_add_hook( INPUT_KEY_UP, mgui_input_handle_key_up );
	input_add_hook( INPUT_KEY_DOWN, mgui_input_handle_key_down );
	input_add_hook( INPUT_MOUSE_MOVE, mgui_input_handle_mouse_move );
	input_add_hook( INPUT_MOUSE_WHEEL, mgui_input_handle_mouse_wheel );
	input_add_hook( INPUT_LBUTTON_UP, mgui_input_handle_lmb_up );
	input_add_hook( INPUT_LBUTTON_DOWN, mgui_input_handle_lmb_down );
}

void mgui_input_shutdown_hooks( void )
{
	input_remove_hook( INPUT_CHARACTER, mgui_input_handle_char );
	input_remove_hook( INPUT_KEY_UP, mgui_input_handle_key_up );
	input_remove_hook( INPUT_KEY_DOWN, mgui_input_handle_key_down );
	input_remove_hook( INPUT_MOUSE_MOVE, mgui_input_handle_mouse_move );
	input_remove_hook( INPUT_MOUSE_WHEEL, mgui_input_handle_mouse_wheel );
	input_remove_hook( INPUT_LBUTTON_UP, mgui_input_handle_lmb_up );
	input_remove_hook( INPUT_LBUTTON_DOWN, mgui_input_handle_lmb_down );
}

void mgui_input_cleanup_references( MGuiElement* element )
{
	if ( element == hovered ) hovered = NULL;
	if ( element == pressed ) pressed = NULL;
	if ( element == dragged ) dragged = NULL;
	if ( element == mousefocus ) mousefocus = NULL;
	if ( element == kbfocus ) kbfocus = NULL;
}

MGuiElement* mgui_get_focus( void )
{
	return kbfocus;
}

void mgui_set_focus( MGuiElement* element )
{
	MGuiEvent guievent;

	if ( kbfocus )
	{
		if ( kbfocus->event_handler )
		{
			guievent.type = EVENT_FOCUS_EXIT;
			guievent.element = kbfocus;
			guievent.data = kbfocus->event_data;

			kbfocus->event_handler( &guievent );
		}

		kbfocus->flags_int &= ~INTFLAG_FOCUS;
		kbfocus = NULL;
	}

	if ( !element ) return;

	if ( BIT_ON( element->flags, FLAG_KBCTRL ) &&
		 BIT_ON( element->flags, FLAG_VISIBLE ) )
	{
		kbfocus = element;
		element->flags_int |= INTFLAG_FOCUS;

		if ( kbfocus->event_handler )
		{
			guievent.type = EVENT_FOCUS_ENTER;
			guievent.element = kbfocus;
			guievent.data = element->event_data;

			kbfocus->event_handler( &guievent );
		}
	}

	mgui_force_redraw();
}

static bool mgui_input_handle_char( input_event_t* event )
{
	if ( kbfocus )
	{
		kbfocus->on_character( kbfocus, (char_t)event->keyboard.key );
	}

	return true;
}

static bool mgui_input_handle_key_up( input_event_t* event )
{
	if ( kbfocus )
	{
		kbfocus->on_key_press( kbfocus, event->keyboard.key, false );
	}

	return true;
}

static bool mgui_input_handle_key_down( input_event_t* event )
{
	if ( kbfocus )
	{
		kbfocus->on_key_press( kbfocus, event->keyboard.key, true );
	}

	return true;
}

static bool mgui_input_handle_mouse_move( input_event_t* event )
{
	uint16 x, y;
	MGuiElement* element;
	MGuiEvent guievent;

	x = event->mouse.x;
	y = event->mouse.y;

	if ( dragged )
	{
		dragged->on_mouse_drag( dragged, x, y );
		mgui_force_redraw();
	}

	element = mgui_get_element_at( x, y );
	if ( element == hovered ) return true;

	if ( hovered )
	{
		hovered->flags_int &= ~INTFLAG_HOVER;
		hovered->on_mouse_leave( hovered );

		if ( hovered->event_handler )
		{
			guievent.type = EVENT_HOVER_LEAVE;
			guievent.element = hovered;
			guievent.data = hovered->event_data;
			guievent.mouse.x = x;
			guievent.mouse.y = y;

			hovered->event_handler( &guievent );
		}
	}

	if ( ( hovered = element ) != NULL )
	{
		hovered->flags_int |= INTFLAG_HOVER;
		hovered->on_mouse_enter( hovered );

		if ( hovered->event_handler )
		{
			guievent.type = EVENT_HOVER_ENTER;
			guievent.element = hovered;
			guievent.data = hovered->event_data;
			guievent.mouse.x = x;
			guievent.mouse.y = y;

			hovered->event_handler( &guievent );
		}
	}

	mgui_force_redraw();

	return true;
}

static bool mgui_input_handle_mouse_wheel( input_event_t* event )
{
	UNREFERENCED_PARAM( event );
	return true;
}

static bool mgui_input_handle_lmb_up( input_event_t* event )
{
	uint16 x, y;
	MGuiElement* element;
	MGuiEvent guievent;

	x = event->mouse.x;
	y = event->mouse.y;

	dragged = NULL;

	element = mgui_get_element_at( x, y );

	if ( pressed )
	{
		pressed->flags_int &= ~INTFLAG_PRESSED;
		pressed->on_mouse_release( pressed, MOUSE_LBUTTON, x, y );

		if ( pressed->event_handler )
		{
			guievent.type = EVENT_RELEASE;
			guievent.element = pressed;
			guievent.data = pressed->event_data;
			guievent.mouse.x = x;
			guievent.mouse.y = y;

			pressed->event_handler( &guievent );
		}

		pressed = NULL;

		mgui_force_redraw();
	}

	return true;
}

static bool mgui_input_handle_lmb_down( input_event_t* event )
{
	uint16 x, y;
	MGuiElement* element;
	MGuiEvent guievent;

	x = event->mouse.x;
	y = event->mouse.y;

	dragged = NULL;
	element = mgui_get_element_at( x, y );

	// Remove old keyboard focus
	if ( kbfocus && kbfocus != element )
	{
		if ( kbfocus->event_handler )
		{
			guievent.type = EVENT_FOCUS_EXIT;
			guievent.element = kbfocus;
			guievent.data = kbfocus->event_data;

			kbfocus->event_handler( &guievent );
		}

		kbfocus->flags_int &= ~INTFLAG_FOCUS;
		kbfocus = NULL;

		mgui_force_redraw();
	}

	// Remove old pressed focus
	if ( pressed )
	{
		pressed->flags_int &= ~INTFLAG_PRESSED;
		pressed->on_mouse_release( pressed, MOUSE_LBUTTON, x, y );

		if ( pressed->event_handler )
		{
			guievent.type = EVENT_RELEASE;
			guievent.element = pressed;
			guievent.data = pressed->event_data;
			guievent.mouse.x = x;
			guievent.mouse.y = y;

			pressed->event_handler( &guievent );
		}

		mgui_force_redraw();
	}

	// If a new element is being pressed handle focus/event stuff
	if ( ( pressed = element ) != NULL )
	{
		pressed->flags_int |= INTFLAG_PRESSED;
		pressed->on_mouse_click( pressed, MOUSE_LBUTTON, x, y );

		if ( BIT_ON( pressed->flags, FLAG_DRAGGABLE ) )
		{
			dragged = pressed;
		}

		if ( pressed->event_handler )
		{
			guievent.type = EVENT_CLICK;
			guievent.element = pressed;
			guievent.data = pressed->event_data;
			guievent.mouse.x = x;
			guievent.mouse.y = y;

			pressed->event_handler( &guievent );
		}

		if ( BIT_ON( element->flags, FLAG_KBCTRL ) )
		{
			kbfocus = element;
			element->flags_int |= INTFLAG_FOCUS;

			if ( kbfocus->event_handler )
			{
				guievent.type = EVENT_FOCUS_ENTER;
				guievent.element = kbfocus;
				guievent.data = kbfocus->event_data;

				kbfocus->event_handler( &guievent );
			}
		}

		mgui_force_redraw();
	}

	return true;
}
