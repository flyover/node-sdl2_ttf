/**
 * Copyright (c) Flyover Games, LLC.  All rights reserved. 
 *  
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated 
 * documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to 
 * whom the Software is furnished to do so, subject to the 
 * following conditions: 
 *  
 * The above copyright notice and this permission notice shall 
 * be included in all copies or substantial portions of the 
 * Software. 
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY 
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 */

#include "node-sdl2_ttf.h"

#include <v8.h>
#include <node.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include <stdlib.h> // malloc, free
#include <string.h> // strdup

#ifndef strdup
#define strdup(str) strcpy((char*)malloc(strlen(str)+1),str)
#endif

#if defined(__ANDROID__)
#include <android/log.h>
#define printf(...) __android_log_print(ANDROID_LOG_INFO, "printf", __VA_ARGS__)
#endif

#define countof(_a) (sizeof(_a)/sizeof((_a)[0]))

using namespace v8;

namespace node_sdl2_ttf {

static SDL_Color _get_color(Handle<Value> value)
{
	SDL_Color color;
	if (value->IsUint32())
	{
		// 0xAABBGGRR
		*((::Uint32*)&color) = value->Uint32Value();
		//::Uint32 num = value->Uint32Value();
		//color.r = num >> 0;
		//color.g = num >> 8;
		//color.b = num >> 16;
		//color.a = num >> 24;
	}
	else if (value->IsArray())
	{
		// [ 0xRR, 0xGG, 0xBB, 0xAA ]
		Handle<Array> arr = Handle<Array>::Cast(value);
		color.r = arr->Get(0)->Uint32Value();
		color.g = arr->Get(1)->Uint32Value();
		color.b = arr->Get(2)->Uint32Value();
		color.a = arr->Get(3)->Uint32Value();
	}
	else if (value->IsObject())
	{
		Handle<Object> obj = Handle<Object>::Cast(value);
		node_sdl2::WrapColor* n_color = node::ObjectWrap::Unwrap<node_sdl2::WrapColor>(obj);
		if (n_color)
		{
			// new sdl.SDL_Color(0xRR, 0xGG, 0xBB, 0xAA);
			color = n_color->GetColor();
		}
		else
		{
			// { r: 0xRR, g: 0xGG, b: 0xBB, a: 0xAA }
			color.r = obj->Get(NanNew<String>("r"))->Uint32Value();
			color.g = obj->Get(NanNew<String>("g"))->Uint32Value();
			color.b = obj->Get(NanNew<String>("b"))->Uint32Value();
			color.a = obj->Get(NanNew<String>("a"))->Uint32Value();
		}
	}
	return color;
}

// open font

class Task_TTF_OpenFontIndex : public node_sdl2::SimpleTask
{
public:
	char* m_file;
	int m_ptsize;
	int m_index;
	Persistent<Function> m_callback;
	TTF_Font* m_font;
public:
	Task_TTF_OpenFontIndex(Handle<String> file, Handle<Integer> ptsize, Handle<Function> callback) :
		m_file(strdup(*String::Utf8Value(file))), 
		m_ptsize(ptsize->Int32Value()),
		m_index(0),
		m_font(NULL)
	{
		NanAssignPersistent(m_callback, callback);
	}
	Task_TTF_OpenFontIndex(Handle<String> file, Handle<Integer> ptsize, Handle<Integer> index, Handle<Function> callback) :
		m_file(strdup(*String::Utf8Value(file))), 
		m_ptsize(ptsize->Int32Value()),
		m_index(index->Int32Value()),
		m_font(NULL)
	{
		NanAssignPersistent(m_callback, callback);
	}
	~Task_TTF_OpenFontIndex()
	{
		free(m_file); m_file = NULL; // strdup
		NanDisposePersistent(m_callback);
		if (m_font) { TTF_CloseFont(m_font); m_font = NULL; }
	}
	void DoWork()
	{
		m_font = TTF_OpenFontIndex(m_file, m_ptsize, m_index);
	}
	void DoAfterWork(int status)
	{
		NanScope();
		Handle<Value> argv[] = { WrapFont::Hold(m_font) };
		NanMakeCallback(NanGetCurrentContext()->Global(), NanNew<Function>(m_callback), countof(argv), argv);
		m_font = NULL; // script owns pointer
	}
};

MODULE_EXPORT_IMPLEMENT_TODO(TTF_LinkedVersion)

MODULE_EXPORT_IMPLEMENT(TTF_ByteSwappedUNICODE)
{
	NanScope();
	int swapped = args[0]->Int32Value();
	TTF_ByteSwappedUNICODE(swapped);
	NanReturnUndefined();
}

MODULE_EXPORT_IMPLEMENT(TTF_Init)
{
	NanScope();
	int err = TTF_Init();
	if (err < 0)
	{
		printf("TTF_Init error: %d\n", err);
	}
	NanReturnValue(NanNew<Integer>(err));
}

MODULE_EXPORT_IMPLEMENT(TTF_Quit)
{
	NanScope();
	TTF_Quit();
	NanReturnUndefined();
}

MODULE_EXPORT_IMPLEMENT(TTF_WasInit)
{
	NanScope();
	int init = TTF_WasInit();
	NanReturnValue(NanNew<Integer>(init));
}

MODULE_EXPORT_IMPLEMENT(TTF_GetError)
{
	NanScope();
	const char* sdl_ttf_error = TTF_GetError();
	NanReturnValue(NanNew<String>(sdl_ttf_error));
}

MODULE_EXPORT_IMPLEMENT(TTF_ClearError)
{
	NanScope();
	SDL_ClearError();
	NanReturnUndefined();
}

MODULE_EXPORT_IMPLEMENT(TTF_OpenFont)
{
	NanScope();
	Local<String> file = Local<String>::Cast(args[0]);
	Local<Integer> ptsize = Local<Integer>::Cast(args[1]);
	Local<Function> callback = Local<Function>::Cast(args[2]);
	int err = node_sdl2::SimpleTask::Run(new Task_TTF_OpenFontIndex(file, ptsize, callback));
	NanReturnValue(NanNew<v8::Int32>(err));
}

MODULE_EXPORT_IMPLEMENT(TTF_OpenFontIndex)
{
	NanScope();
	Local<String> file = Local<String>::Cast(args[0]);
	Local<Integer> ptsize = Local<Integer>::Cast(args[1]);
	Local<Integer> index = Local<Integer>::Cast(args[2]);
	Local<Function> callback = Local<Function>::Cast(args[3]);
	int err = node_sdl2::SimpleTask::Run(new Task_TTF_OpenFontIndex(file, ptsize, index, callback));
	NanReturnValue(NanNew<v8::Int32>(err));
}

MODULE_EXPORT_IMPLEMENT_TODO(TTF_OpenFontRW)

MODULE_EXPORT_IMPLEMENT_TODO(TTF_OpenFontIndexRW)

MODULE_EXPORT_IMPLEMENT(TTF_CloseFont)
{
	NanScope();
	TTF_Font* font = WrapFont::Drop(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	TTF_CloseFont(font);
	NanReturnUndefined();
}

MODULE_EXPORT_IMPLEMENT(TTF_GetFontStyle)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int style = TTF_GetFontStyle(font);
	NanReturnValue(NanNew<Integer>(style));
}

MODULE_EXPORT_IMPLEMENT(TTF_SetFontStyle)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int style = args[1]->Int32Value();
	TTF_SetFontStyle(font, style);
	NanReturnValue(NanNew<Integer>(style));
}

MODULE_EXPORT_IMPLEMENT(TTF_GetFontOutline)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int outline = TTF_GetFontOutline(font);
	NanReturnValue(NanNew<Integer>(outline));
}

MODULE_EXPORT_IMPLEMENT(TTF_SetFontOutline)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int outline = args[1]->Int32Value();
	TTF_SetFontOutline(font, outline);
	NanReturnValue(NanNew<Integer>(outline));
}

MODULE_EXPORT_IMPLEMENT(TTF_GetFontHinting)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int hinting = TTF_GetFontHinting(font);
	NanReturnValue(NanNew<Integer>(hinting));
}

MODULE_EXPORT_IMPLEMENT(TTF_SetFontHinting)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int hinting = args[1]->Int32Value();
	TTF_SetFontHinting(font, hinting);
	NanReturnValue(NanNew<Integer>(hinting));
}

MODULE_EXPORT_IMPLEMENT(TTF_FontHeight)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int height = TTF_FontHeight(font);
	NanReturnValue(NanNew<Integer>(height));
}

MODULE_EXPORT_IMPLEMENT(TTF_FontAscent)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int ascent = TTF_FontAscent(font);
	NanReturnValue(NanNew<Integer>(ascent));
}

MODULE_EXPORT_IMPLEMENT(TTF_FontDescent)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int descent = TTF_FontDescent(font);
	NanReturnValue(NanNew<Integer>(descent));
}

MODULE_EXPORT_IMPLEMENT(TTF_FontLineSkip)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int line_skip = TTF_FontLineSkip(font);
	NanReturnValue(NanNew<Integer>(line_skip));
}

MODULE_EXPORT_IMPLEMENT(TTF_GetFontKerning)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int kerning = TTF_GetFontKerning(font);
	NanReturnValue(NanNew<Integer>(kerning));
}

MODULE_EXPORT_IMPLEMENT(TTF_SetFontKerning)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int kerning = args[1]->Int32Value();
	TTF_SetFontKerning(font, kerning);
	NanReturnValue(NanNew<Integer>(kerning));
}

MODULE_EXPORT_IMPLEMENT(TTF_FontFaces)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	long faces = TTF_FontFaces(font);
	NanReturnValue(NanNew<Integer>((int32_t) faces)); // TODO: long
}

MODULE_EXPORT_IMPLEMENT(TTF_FontFaceIsFixedWidth)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int fixed_width = TTF_FontFaceIsFixedWidth(font);
	NanReturnValue(NanNew<Integer>(fixed_width));
}

MODULE_EXPORT_IMPLEMENT(TTF_FontFaceFamilyName)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	char* name = TTF_FontFaceFamilyName(font);
	NanReturnValue(NanNew<String>(name));
}

MODULE_EXPORT_IMPLEMENT(TTF_FontFaceStyleName)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	char* name = TTF_FontFaceStyleName(font);
	NanReturnValue(NanNew<String>(name));
}

MODULE_EXPORT_IMPLEMENT(TTF_GlyphIsProvided)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	::Uint16 ch = (::Uint16) args[1]->Uint32Value();
	int provided = TTF_GlyphIsProvided(font, ch);
	NanReturnValue(NanNew<Integer>(provided));
}

MODULE_EXPORT_IMPLEMENT(TTF_GlyphMetrics)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	::Uint16 ch = (::Uint16) args[1]->Uint32Value();
	int minx = 0, maxx = 0;
	int miny = 0, maxy = 0;
	int advance = 0;
	int err = TTF_GlyphMetrics(font, ch, &minx, &maxx, &miny, &maxy, &advance);
	if (args[2]->IsObject())
	{
		Local<Object> ret = Local<Object>::Cast(args[2]);
		ret->Set(NanNew<String>("minx"), NanNew<Integer>(minx));
		ret->Set(NanNew<String>("maxx"), NanNew<Integer>(maxx));
		ret->Set(NanNew<String>("miny"), NanNew<Integer>(miny));
		ret->Set(NanNew<String>("maxy"), NanNew<Integer>(maxy));
		ret->Set(NanNew<String>("advance"), NanNew<Integer>(advance));
	}
	NanReturnValue(NanNew<Integer>(err));
}

MODULE_EXPORT_IMPLEMENT(TTF_SizeText)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	Local<String> text = Local<String>::Cast(args[1]);
	int w = 0, h = 0;
	int err = TTF_SizeText(font, *NanAsciiString(text), &w, &h);
	if (args[2]->IsObject())
	{
		Local<Object> ret = Local<Object>::Cast(args[2]);
		ret->Set(NanNew<String>("w"), NanNew<Integer>(w));
		ret->Set(NanNew<String>("h"), NanNew<Integer>(h));
	}
	NanReturnValue(NanNew<Integer>(err));
}

MODULE_EXPORT_IMPLEMENT(TTF_SizeUTF8)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	Local<String> text = Local<String>::Cast(args[1]);
	int w = 0, h = 0;
	int err = TTF_SizeUTF8(font, *String::Utf8Value(text), &w, &h);
	if (args[2]->IsObject())
	{
		Local<Object> ret = Local<Object>::Cast(args[2]);
		ret->Set(NanNew<String>("w"), NanNew<Integer>(w));
		ret->Set(NanNew<String>("h"), NanNew<Integer>(h));
	}
	NanReturnValue(NanNew<Integer>(err));
}

MODULE_EXPORT_IMPLEMENT_TODO(TTF_SizeUNICODE)

MODULE_EXPORT_IMPLEMENT(TTF_RenderText_Solid)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	Local<String> text = Local<String>::Cast(args[1]);
	SDL_Color fg = _get_color(args[2]);
	SDL_Surface* surface = TTF_RenderText_Solid(font, *NanAsciiString(text), fg);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

MODULE_EXPORT_IMPLEMENT(TTF_RenderUTF8_Solid)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	Local<String> text = Local<String>::Cast(args[1]);
	SDL_Color fg = _get_color(args[2]);
	SDL_Surface* surface = TTF_RenderText_Solid(font, *String::Utf8Value(text), fg);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

MODULE_EXPORT_IMPLEMENT_TODO(TTF_RenderUNICODE_Solid)

MODULE_EXPORT_IMPLEMENT(TTF_RenderGlyph_Solid)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	::Uint16 ch = (::Uint16) args[1]->Uint32Value();
	SDL_Color fg = _get_color(args[2]);
	SDL_Surface* surface = TTF_RenderGlyph_Solid(font, ch, fg);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

MODULE_EXPORT_IMPLEMENT(TTF_RenderText_Shaded)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	Local<String> text = Local<String>::Cast(args[1]);
	SDL_Color fg = _get_color(args[2]);
	SDL_Color bg = _get_color(args[3]);
	SDL_Surface* surface = TTF_RenderText_Shaded(font, *NanAsciiString(text), fg, bg);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

MODULE_EXPORT_IMPLEMENT(TTF_RenderUTF8_Shaded)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	Local<String> text = Local<String>::Cast(args[1]);
	SDL_Color fg = _get_color(args[2]);
	SDL_Color bg = _get_color(args[3]);
	SDL_Surface* surface = TTF_RenderUTF8_Shaded(font, *String::Utf8Value(text), fg, bg);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

MODULE_EXPORT_IMPLEMENT_TODO(TTF_RenderUNICODE_Shaded)

MODULE_EXPORT_IMPLEMENT(TTF_RenderGlyph_Shaded)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	::Uint16 ch = (::Uint16) args[1]->Uint32Value();
	SDL_Color fg = _get_color(args[2]);
	SDL_Color bg = _get_color(args[3]);
	SDL_Surface* surface = TTF_RenderGlyph_Shaded(font, ch, fg, bg);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

// extern DECLSPEC SDL_Surface * SDLCALL TTF_RenderText_Blended(TTF_Font *font, const char *text, SDL_Color fg);
MODULE_EXPORT_IMPLEMENT(TTF_RenderText_Blended)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	Local<String> text = Local<String>::Cast(args[1]);
	SDL_Color fg = _get_color(args[2]);
	SDL_Surface* surface = TTF_RenderUTF8_Blended(font, *NanAsciiString(text), fg);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

// extern DECLSPEC SDL_Surface * SDLCALL TTF_RenderUTF8_Blended(TTF_Font *font, const char *text, SDL_Color fg);
MODULE_EXPORT_IMPLEMENT(TTF_RenderUTF8_Blended)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	Local<String> text = Local<String>::Cast(args[1]);
	SDL_Color fg = _get_color(args[2]);
	SDL_Surface* surface = TTF_RenderUTF8_Blended(font, *String::Utf8Value(text), fg);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

MODULE_EXPORT_IMPLEMENT_TODO(TTF_RenderUNICODE_Blended)

MODULE_EXPORT_IMPLEMENT(TTF_RenderGlyph_Blended)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	::Uint16 ch = (::Uint16) args[1]->Uint32Value();
	SDL_Color fg = _get_color(args[2]);
	SDL_Surface* surface = TTF_RenderGlyph_Blended(font, ch, fg);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

MODULE_EXPORT_IMPLEMENT(TTF_RenderText_Blended_Wrapped)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	Local<String> text = Local<String>::Cast(args[1]);
	SDL_Color fg = _get_color(args[2]);
	::Uint32 wrapLength = args[3]->Uint32Value();
	SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, *NanAsciiString(text), fg, wrapLength);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

MODULE_EXPORT_IMPLEMENT(TTF_RenderUTF8_Blended_Wrapped)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	Local<String> text = Local<String>::Cast(args[1]);
	SDL_Color fg = _get_color(args[2]);
	::Uint32 wrapLength = args[3]->Uint32Value();
	SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, *String::Utf8Value(text), fg, wrapLength);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

MODULE_EXPORT_IMPLEMENT_TODO(TTF_RenderUNICODE_Blended_Wrapped)

MODULE_EXPORT_IMPLEMENT(TTF_RenderText)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	Local<String> text = Local<String>::Cast(args[1]);
	SDL_Color fg = _get_color(args[2]);
	SDL_Color bg = _get_color(args[3]);
	SDL_Surface* surface = TTF_RenderText(font, *NanAsciiString(text), fg, bg);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

MODULE_EXPORT_IMPLEMENT(TTF_RenderUTF8)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	Local<String> text = Local<String>::Cast(args[1]);
	SDL_Color fg = _get_color(args[2]);
	SDL_Color bg = _get_color(args[3]);
	SDL_Surface* surface = TTF_RenderUTF8(font, *String::Utf8Value(text), fg, bg);
	NanReturnValue(node_sdl2::WrapSurface::Hold(surface));
}

MODULE_EXPORT_IMPLEMENT_TODO(TTF_RenderUNICODE)

MODULE_EXPORT_IMPLEMENT(TTF_GetFontKerningSize)
{
	NanScope();
	TTF_Font* font = WrapFont::Peek(args[0]); if (!font) { return NanThrowError(NanNew<String>("null object")); }
	int prev_index = args[1]->Int32Value();
	int index = args[2]->Int32Value();
	int size = TTF_GetFontKerningSize(font, prev_index, index);
	NanReturnValue(NanNew<Integer>(size));
}

#if NODE_VERSION_AT_LEAST(0,11,0)
void init(Handle<Object> exports, Handle<Value> module, Handle<Context> context)
#else
void init(Handle<Object> exports/*, Handle<Value> module*/)
#endif
{
	NanScope();

	// SDL_ttf.h

	MODULE_CONSTANT(exports, SDL_TTF_MAJOR_VERSION);
	MODULE_CONSTANT(exports, SDL_TTF_MINOR_VERSION);
	MODULE_CONSTANT(exports, SDL_TTF_PATCHLEVEL);

	MODULE_CONSTANT(exports, UNICODE_BOM_NATIVE);
	MODULE_CONSTANT(exports, UNICODE_BOM_SWAPPED);

	MODULE_CONSTANT(exports, TTF_STYLE_NORMAL);
	MODULE_CONSTANT(exports, TTF_STYLE_BOLD);
	MODULE_CONSTANT(exports, TTF_STYLE_ITALIC);
	MODULE_CONSTANT(exports, TTF_STYLE_UNDERLINE);
	MODULE_CONSTANT(exports, TTF_STYLE_STRIKETHROUGH);

	MODULE_CONSTANT(exports, TTF_HINTING_NORMAL);
	MODULE_CONSTANT(exports, TTF_HINTING_LIGHT);
	MODULE_CONSTANT(exports, TTF_HINTING_MONO);
	MODULE_CONSTANT(exports, TTF_HINTING_NONE);

	MODULE_EXPORT_APPLY(exports, TTF_LinkedVersion);
	MODULE_EXPORT_APPLY(exports, TTF_ByteSwappedUNICODE);
	MODULE_EXPORT_APPLY(exports, TTF_Init);
	MODULE_EXPORT_APPLY(exports, TTF_Quit);
	MODULE_EXPORT_APPLY(exports, TTF_WasInit);
	MODULE_EXPORT_APPLY(exports, TTF_GetError);
	MODULE_EXPORT_APPLY(exports, TTF_ClearError);
	MODULE_EXPORT_APPLY(exports, TTF_OpenFont);
	MODULE_EXPORT_APPLY(exports, TTF_OpenFontIndex);
	MODULE_EXPORT_APPLY(exports, TTF_OpenFontRW);
	MODULE_EXPORT_APPLY(exports, TTF_OpenFontIndexRW);
	MODULE_EXPORT_APPLY(exports, TTF_CloseFont);
	MODULE_EXPORT_APPLY(exports, TTF_GetFontStyle);
	MODULE_EXPORT_APPLY(exports, TTF_SetFontStyle);
	MODULE_EXPORT_APPLY(exports, TTF_GetFontOutline);
	MODULE_EXPORT_APPLY(exports, TTF_SetFontOutline);
	MODULE_EXPORT_APPLY(exports, TTF_GetFontHinting);
	MODULE_EXPORT_APPLY(exports, TTF_SetFontHinting);
	MODULE_EXPORT_APPLY(exports, TTF_FontHeight);
	MODULE_EXPORT_APPLY(exports, TTF_FontAscent);
	MODULE_EXPORT_APPLY(exports, TTF_FontDescent);
	MODULE_EXPORT_APPLY(exports, TTF_FontLineSkip);
	MODULE_EXPORT_APPLY(exports, TTF_GetFontKerning);
	MODULE_EXPORT_APPLY(exports, TTF_SetFontKerning);
	MODULE_EXPORT_APPLY(exports, TTF_FontFaces);
	MODULE_EXPORT_APPLY(exports, TTF_FontFaceIsFixedWidth);
	MODULE_EXPORT_APPLY(exports, TTF_FontFaceFamilyName);
	MODULE_EXPORT_APPLY(exports, TTF_FontFaceStyleName);
	MODULE_EXPORT_APPLY(exports, TTF_GlyphIsProvided);
	MODULE_EXPORT_APPLY(exports, TTF_GlyphMetrics);
	MODULE_EXPORT_APPLY(exports, TTF_SizeText);
	MODULE_EXPORT_APPLY(exports, TTF_SizeUTF8);
	MODULE_EXPORT_APPLY(exports, TTF_SizeUNICODE);
	MODULE_EXPORT_APPLY(exports, TTF_RenderText_Solid);
	MODULE_EXPORT_APPLY(exports, TTF_RenderUTF8_Solid);
	MODULE_EXPORT_APPLY(exports, TTF_RenderUNICODE_Solid);
	MODULE_EXPORT_APPLY(exports, TTF_RenderGlyph_Solid);
	MODULE_EXPORT_APPLY(exports, TTF_RenderText_Shaded);
	MODULE_EXPORT_APPLY(exports, TTF_RenderUTF8_Shaded);
	MODULE_EXPORT_APPLY(exports, TTF_RenderUNICODE_Shaded);
	MODULE_EXPORT_APPLY(exports, TTF_RenderGlyph_Shaded);
	MODULE_EXPORT_APPLY(exports, TTF_RenderText_Blended);
	MODULE_EXPORT_APPLY(exports, TTF_RenderUTF8_Blended);
	MODULE_EXPORT_APPLY(exports, TTF_RenderUNICODE_Blended);
	MODULE_EXPORT_APPLY(exports, TTF_RenderGlyph_Blended);
	MODULE_EXPORT_APPLY(exports, TTF_RenderText_Blended_Wrapped);
	MODULE_EXPORT_APPLY(exports, TTF_RenderUTF8_Blended_Wrapped);
	MODULE_EXPORT_APPLY(exports, TTF_RenderUNICODE_Blended_Wrapped);
	MODULE_EXPORT_APPLY(exports, TTF_RenderText);
	MODULE_EXPORT_APPLY(exports, TTF_RenderUTF8);
	MODULE_EXPORT_APPLY(exports, TTF_RenderUNICODE);
	MODULE_EXPORT_APPLY(exports, TTF_GetFontKerningSize);
}

} // namespace node_sdl2_ttf

#if NODE_VERSION_AT_LEAST(0,11,0)
NODE_MODULE_CONTEXT_AWARE_BUILTIN(node_sdl2_ttf, node_sdl2_ttf::init)
#else
NODE_MODULE(node_sdl2_ttf, node_sdl2_ttf::init)
#endif

