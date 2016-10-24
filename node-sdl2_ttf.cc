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

#ifndef SDL_TTF_COMPILEDVERSION
#define SDL_TTF_COMPILEDVERSION \
	SDL_VERSIONNUM(SDL_TTF_MAJOR_VERSION, SDL_TTF_MINOR_VERSION, SDL_TTF_PATCHLEVEL)
#endif

#ifndef SDL_TTF_VERSION_ATLEAST
#define SDL_TTF_VERSION_ATLEAST(X, Y, Z) \
	(SDL_TTF_COMPILEDVERSION >= SDL_VERSIONNUM(X, Y, Z))
#endif

namespace node_sdl2_ttf {

static SDL_Color _get_color(v8::Local<v8::Value> value)
{
	SDL_Color color;
	if (value->IsUint32())
	{
		// 0xAABBGGRR
		*((::Uint32*)&color) = NANX_Uint32(value);
		//::Uint32 num = NANX_Uint32(value);
		//color.r = num >> 0;
		//color.g = num >> 8;
		//color.b = num >> 16;
		//color.a = num >> 24;
	}
	else if (value->IsArray())
	{
		// [ 0xRR, 0xGG, 0xBB, 0xAA ]
		v8::Local<v8::Array> arr = v8::Local<v8::Array>::Cast(value);
		color.r = NANX_Uint8(arr->Get(0));
		color.g = NANX_Uint8(arr->Get(1));
		color.b = NANX_Uint8(arr->Get(2));
		color.a = NANX_Uint8(arr->Get(3));
	}
	else if (value->IsObject())
	{
		v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(value);
		node_sdl2::WrapColor* wrap_color = node_sdl2::WrapColor::Unwrap(obj);
		if (wrap_color)
		{
			// new sdl.SDL_Color(0xRR, 0xGG, 0xBB, 0xAA);
			color = wrap_color->GetColor();
		}
		else
		{
			// { r: 0xRR, g: 0xGG, b: 0xBB, a: 0xAA }
			color.r = NANX_Uint8(obj->Get(NANX_SYMBOL("r")));
			color.g = NANX_Uint8(obj->Get(NANX_SYMBOL("g")));
			color.b = NANX_Uint8(obj->Get(NANX_SYMBOL("b")));
			color.a = NANX_Uint8(obj->Get(NANX_SYMBOL("a")));
		}
	}
	return color;
}

// open font

class Task_TTF_OpenFontIndex : public Nanx::SimpleTask
{
public:
	char* m_file;
	int m_ptsize;
	int m_index;
	Nan::Persistent<v8::Function> m_callback;
	TTF_Font* m_font;
public:
	Task_TTF_OpenFontIndex(v8::Local<v8::String> file, v8::Local<v8::Integer> ptsize, v8::Local<v8::Function> callback) :
		m_file(strdup(*v8::String::Utf8Value(file))),
		m_ptsize(NANX_int(ptsize)),
		m_index(0),
		m_font(NULL)
	{
		m_callback.Reset(callback);
	}
	Task_TTF_OpenFontIndex(v8::Local<v8::String> file, v8::Local<v8::Integer> ptsize, v8::Local<v8::Integer> index, v8::Local<v8::Function> callback) :
		m_file(strdup(*v8::String::Utf8Value(file))),
		m_ptsize(NANX_int(ptsize)),
		m_index(NANX_int(index)),
		m_font(NULL)
	{
		m_callback.Reset(callback);
	}
	~Task_TTF_OpenFontIndex()
	{
		free(m_file); m_file = NULL; // strdup
		m_callback.Reset();
		if (m_font) { TTF_CloseFont(m_font); m_font = NULL; }
	}
	void DoWork()
	{
		m_font = TTF_OpenFontIndex(m_file, m_ptsize, m_index);
	}
	void DoAfterWork(int status)
	{
		Nan::HandleScope scope;
		v8::Local<v8::Value> argv[] = { WrapFont::Hold(m_font) };
		Nan::MakeCallback(Nan::GetCurrentContext()->Global(), Nan::New<v8::Function>(m_callback), countof(argv), argv);
		m_font = NULL; // script owns pointer
	}
};

NANX_EXPORT(TTF_LinkedVersion) { Nan::ThrowError("TODO"); }

NANX_EXPORT(TTF_ByteSwappedUNICODE)
{
	int swapped = NANX_int(info[0]);
	TTF_ByteSwappedUNICODE(swapped);
}

NANX_EXPORT(TTF_Init)
{
	int err = TTF_Init();
	if (err < 0)
	{
		printf("TTF_Init error: %d\n", err);
	}
	info.GetReturnValue().Set(Nan::New(err));
}

NANX_EXPORT(TTF_Quit)
{
	TTF_Quit();
}

NANX_EXPORT(TTF_WasInit)
{
	int init = TTF_WasInit();
	info.GetReturnValue().Set(Nan::New(init));
}

NANX_EXPORT(TTF_GetError)
{
	const char* sdl_ttf_error = TTF_GetError();
	info.GetReturnValue().Set(NANX_STRING(sdl_ttf_error));
}

NANX_EXPORT(TTF_ClearError)
{
	SDL_ClearError();
}

NANX_EXPORT(TTF_OpenFont)
{
	v8::Local<v8::String> file = v8::Local<v8::String>::Cast(info[0]);
	v8::Local<v8::Integer> ptsize = v8::Local<v8::Integer>::Cast(info[1]);
	v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(info[2]);
	int err = Nanx::SimpleTask::Run(new Task_TTF_OpenFontIndex(file, ptsize, callback));
	info.GetReturnValue().Set(Nan::New(err));
}

NANX_EXPORT(TTF_OpenFontIndex)
{
	v8::Local<v8::String> file = v8::Local<v8::String>::Cast(info[0]);
	v8::Local<v8::Integer> ptsize = v8::Local<v8::Integer>::Cast(info[1]);
	v8::Local<v8::Integer> index = v8::Local<v8::Integer>::Cast(info[2]);
	v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(info[3]);
	int err = Nanx::SimpleTask::Run(new Task_TTF_OpenFontIndex(file, ptsize, index, callback));
	info.GetReturnValue().Set(Nan::New(err));
}

NANX_EXPORT(TTF_OpenFontRW) { Nan::ThrowError("TODO"); }

NANX_EXPORT(TTF_OpenFontIndexRW) { Nan::ThrowError("TODO"); }

NANX_EXPORT(TTF_CloseFont)
{
	TTF_Font* font = WrapFont::Drop(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	TTF_CloseFont(font);
}

NANX_EXPORT(TTF_GetFontStyle)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int style = TTF_GetFontStyle(font);
	info.GetReturnValue().Set(Nan::New(style));
}

NANX_EXPORT(TTF_SetFontStyle)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int style = NANX_int(info[1]);
	TTF_SetFontStyle(font, style);
	info.GetReturnValue().Set(Nan::New(style));
}

NANX_EXPORT(TTF_GetFontOutline)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int outline = TTF_GetFontOutline(font);
	info.GetReturnValue().Set(Nan::New(outline));
}

NANX_EXPORT(TTF_SetFontOutline)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int outline = NANX_int(info[1]);
	TTF_SetFontOutline(font, outline);
	info.GetReturnValue().Set(Nan::New(outline));
}

NANX_EXPORT(TTF_GetFontHinting)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int hinting = TTF_GetFontHinting(font);
	info.GetReturnValue().Set(Nan::New(hinting));
}

NANX_EXPORT(TTF_SetFontHinting)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int hinting = NANX_int(info[1]);
	TTF_SetFontHinting(font, hinting);
	info.GetReturnValue().Set(Nan::New(hinting));
}

NANX_EXPORT(TTF_FontHeight)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int height = TTF_FontHeight(font);
	info.GetReturnValue().Set(Nan::New(height));
}

NANX_EXPORT(TTF_FontAscent)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int ascent = TTF_FontAscent(font);
	info.GetReturnValue().Set(Nan::New(ascent));
}

NANX_EXPORT(TTF_FontDescent)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int descent = TTF_FontDescent(font);
	info.GetReturnValue().Set(Nan::New(descent));
}

NANX_EXPORT(TTF_FontLineSkip)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int line_skip = TTF_FontLineSkip(font);
	info.GetReturnValue().Set(Nan::New(line_skip));
}

NANX_EXPORT(TTF_GetFontKerning)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int kerning = TTF_GetFontKerning(font);
	info.GetReturnValue().Set(Nan::New(kerning));
}

NANX_EXPORT(TTF_SetFontKerning)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int kerning = NANX_int(info[1]);
	TTF_SetFontKerning(font, kerning);
	info.GetReturnValue().Set(Nan::New(kerning));
}

NANX_EXPORT(TTF_FontFaces)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	long faces = TTF_FontFaces(font);
	info.GetReturnValue().Set(Nan::New((int32_t) faces)); // TODO: long
}

NANX_EXPORT(TTF_FontFaceIsFixedWidth)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int fixed_width = TTF_FontFaceIsFixedWidth(font);
	info.GetReturnValue().Set(Nan::New(fixed_width));
}

NANX_EXPORT(TTF_FontFaceFamilyName)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	char* name = TTF_FontFaceFamilyName(font);
	info.GetReturnValue().Set(NANX_STRING(name));
}

NANX_EXPORT(TTF_FontFaceStyleName)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	char* name = TTF_FontFaceStyleName(font);
	info.GetReturnValue().Set(NANX_STRING(name));
}

NANX_EXPORT(TTF_GlyphIsProvided)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	::Uint16 ch = NANX_Uint16(info[1]);
	int provided = TTF_GlyphIsProvided(font, ch);
	info.GetReturnValue().Set(Nan::New(provided));
}

NANX_EXPORT(TTF_GlyphMetrics)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	::Uint16 ch = NANX_Uint16(info[1]);
	int minx = 0, maxx = 0;
	int miny = 0, maxy = 0;
	int advance = 0;
	int err = TTF_GlyphMetrics(font, ch, &minx, &maxx, &miny, &maxy, &advance);
	if (info[2]->IsObject())
	{
		v8::Local<v8::Object> ret = v8::Local<v8::Object>::Cast(info[2]);
		ret->Set(NANX_SYMBOL("minx"), Nan::New(minx));
		ret->Set(NANX_SYMBOL("maxx"), Nan::New(maxx));
		ret->Set(NANX_SYMBOL("miny"), Nan::New(miny));
		ret->Set(NANX_SYMBOL("maxy"), Nan::New(maxy));
		ret->Set(NANX_SYMBOL("advance"), Nan::New(advance));
	}
	info.GetReturnValue().Set(Nan::New(err));
}

NANX_EXPORT(TTF_SizeText)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[1]);
	int w = 0, h = 0;
	int err = TTF_SizeText(font, *v8::String::Utf8Value(text), &w, &h);
	if (info[2]->IsObject())
	{
		v8::Local<v8::Object> ret = v8::Local<v8::Object>::Cast(info[2]);
		ret->Set(NANX_SYMBOL("w"), Nan::New(w));
		ret->Set(NANX_SYMBOL("h"), Nan::New(h));
	}
	info.GetReturnValue().Set(Nan::New(err));
}

NANX_EXPORT(TTF_SizeUTF8)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[1]);
	int w = 0, h = 0;
	int err = TTF_SizeUTF8(font, *v8::String::Utf8Value(text), &w, &h);
	if (info[2]->IsObject())
	{
		v8::Local<v8::Object> ret = v8::Local<v8::Object>::Cast(info[2]);
		ret->Set(NANX_SYMBOL("w"), Nan::New(w));
		ret->Set(NANX_SYMBOL("h"), Nan::New(h));
	}
	info.GetReturnValue().Set(Nan::New(err));
}

NANX_EXPORT(TTF_SizeUNICODE) { Nan::ThrowError("TODO"); }

NANX_EXPORT(TTF_RenderText_Solid)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[1]);
	SDL_Color fg = _get_color(info[2]);
	SDL_Surface* surface = TTF_RenderText_Solid(font, *v8::String::Utf8Value(text), fg);
	info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
}

NANX_EXPORT(TTF_RenderUTF8_Solid)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[1]);
	SDL_Color fg = _get_color(info[2]);
	SDL_Surface* surface = TTF_RenderText_Solid(font, *v8::String::Utf8Value(text), fg);
	info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
}

NANX_EXPORT(TTF_RenderUNICODE_Solid) { Nan::ThrowError("TODO"); }

NANX_EXPORT(TTF_RenderGlyph_Solid)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	::Uint16 ch = NANX_Uint16(info[1]);
	SDL_Color fg = _get_color(info[2]);
	SDL_Surface* surface = TTF_RenderGlyph_Solid(font, ch, fg);
	info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
}

NANX_EXPORT(TTF_RenderText_Shaded)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[1]);
	SDL_Color fg = _get_color(info[2]);
	SDL_Color bg = _get_color(info[3]);
	SDL_Surface* surface = TTF_RenderText_Shaded(font, *v8::String::Utf8Value(text), fg, bg);
	info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
}

NANX_EXPORT(TTF_RenderUTF8_Shaded)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[1]);
	SDL_Color fg = _get_color(info[2]);
	SDL_Color bg = _get_color(info[3]);
	SDL_Surface* surface = TTF_RenderUTF8_Shaded(font, *v8::String::Utf8Value(text), fg, bg);
	info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
}

NANX_EXPORT(TTF_RenderUNICODE_Shaded) { Nan::ThrowError("TODO"); }

NANX_EXPORT(TTF_RenderGlyph_Shaded)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	::Uint16 ch = NANX_Uint16(info[1]);
	SDL_Color fg = _get_color(info[2]);
	SDL_Color bg = _get_color(info[3]);
	SDL_Surface* surface = TTF_RenderGlyph_Shaded(font, ch, fg, bg);
	info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
}

// extern DECLSPEC SDL_Surface * SDLCALL TTF_RenderText_Blended(TTF_Font *font, const char *text, SDL_Color fg);
NANX_EXPORT(TTF_RenderText_Blended)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[1]);
	SDL_Color fg = _get_color(info[2]);
	SDL_Surface* surface = TTF_RenderUTF8_Blended(font, *v8::String::Utf8Value(text), fg);
	info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
}

// extern DECLSPEC SDL_Surface * SDLCALL TTF_RenderUTF8_Blended(TTF_Font *font, const char *text, SDL_Color fg);
NANX_EXPORT(TTF_RenderUTF8_Blended)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[1]);
	SDL_Color fg = _get_color(info[2]);
	SDL_Surface* surface = TTF_RenderUTF8_Blended(font, *v8::String::Utf8Value(text), fg);
	info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
}

NANX_EXPORT(TTF_RenderUNICODE_Blended) { Nan::ThrowError("TODO"); }

NANX_EXPORT(TTF_RenderGlyph_Blended)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	::Uint16 ch = NANX_Uint16(info[1]);
	SDL_Color fg = _get_color(info[2]);
	SDL_Surface* surface = TTF_RenderGlyph_Blended(font, ch, fg);
	if (surface == NULL)
	{
		info.GetReturnValue().SetNull();
	}
	else
	{
		info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
	}
}

NANX_EXPORT(TTF_RenderText_Blended_Wrapped)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[1]);
	SDL_Color fg = _get_color(info[2]);
	::Uint32 wrapLength = NANX_Uint32(info[3]);
	SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, *v8::String::Utf8Value(text), fg, wrapLength);
	info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
}

NANX_EXPORT(TTF_RenderUTF8_Blended_Wrapped)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[1]);
	SDL_Color fg = _get_color(info[2]);
	::Uint32 wrapLength = NANX_Uint32(info[3]);
	SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, *v8::String::Utf8Value(text), fg, wrapLength);
	info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
}

NANX_EXPORT(TTF_RenderUNICODE_Blended_Wrapped) { Nan::ThrowError("TODO"); }

NANX_EXPORT(TTF_RenderText)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[1]);
	SDL_Color fg = _get_color(info[2]);
	SDL_Color bg = _get_color(info[3]);
	SDL_Surface* surface = TTF_RenderText(font, *v8::String::Utf8Value(text), fg, bg);
	info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
}

NANX_EXPORT(TTF_RenderUTF8)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[1]);
	SDL_Color fg = _get_color(info[2]);
	SDL_Color bg = _get_color(info[3]);
	SDL_Surface* surface = TTF_RenderUTF8(font, *v8::String::Utf8Value(text), fg, bg);
	info.GetReturnValue().Set(node_sdl2::WrapSurface::Hold(surface));
}

NANX_EXPORT(TTF_RenderUNICODE) { Nan::ThrowError("TODO"); }

NANX_EXPORT(TTF_GetFontKerningSize)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	int prev_index = NANX_int(info[1]);
	int index = NANX_int(info[2]);
	int size = TTF_GetFontKerningSize(font, prev_index, index);
	info.GetReturnValue().Set(Nan::New(size));
}

#if SDL_TTF_VERSION_ATLEAST(2, 0, 14)
NANX_EXPORT(TTF_GetFontKerningSizeGlyphs)
{
	TTF_Font* font = WrapFont::Peek(info[0]); if (!font) { return Nan::ThrowError("null object"); }
	Uint16 prev_index = NANX_Uint16(info[1]);
	Uint16 index = NANX_Uint16(info[2]);
	int size = TTF_GetFontKerningSizeGlyphs(font, prev_index, index);
	info.GetReturnValue().Set(Nan::New(size));
}
#endif

NAN_MODULE_INIT(init)
{
	// SDL_ttf.h

	NANX_CONSTANT(target, SDL_TTF_MAJOR_VERSION);
	NANX_CONSTANT(target, SDL_TTF_MINOR_VERSION);
	NANX_CONSTANT(target, SDL_TTF_PATCHLEVEL);

	NANX_CONSTANT(target, UNICODE_BOM_NATIVE);
	NANX_CONSTANT(target, UNICODE_BOM_SWAPPED);

	NANX_CONSTANT(target, TTF_STYLE_NORMAL);
	NANX_CONSTANT(target, TTF_STYLE_BOLD);
	NANX_CONSTANT(target, TTF_STYLE_ITALIC);
	NANX_CONSTANT(target, TTF_STYLE_UNDERLINE);
	NANX_CONSTANT(target, TTF_STYLE_STRIKETHROUGH);

	NANX_CONSTANT(target, TTF_HINTING_NORMAL);
	NANX_CONSTANT(target, TTF_HINTING_LIGHT);
	NANX_CONSTANT(target, TTF_HINTING_MONO);
	NANX_CONSTANT(target, TTF_HINTING_NONE);

	NANX_EXPORT_APPLY(target, TTF_LinkedVersion);
	NANX_EXPORT_APPLY(target, TTF_ByteSwappedUNICODE);
	NANX_EXPORT_APPLY(target, TTF_Init);
	NANX_EXPORT_APPLY(target, TTF_Quit);
	NANX_EXPORT_APPLY(target, TTF_WasInit);
	NANX_EXPORT_APPLY(target, TTF_GetError);
	NANX_EXPORT_APPLY(target, TTF_ClearError);
	NANX_EXPORT_APPLY(target, TTF_OpenFont);
	NANX_EXPORT_APPLY(target, TTF_OpenFontIndex);
	NANX_EXPORT_APPLY(target, TTF_OpenFontRW);
	NANX_EXPORT_APPLY(target, TTF_OpenFontIndexRW);
	NANX_EXPORT_APPLY(target, TTF_CloseFont);
	NANX_EXPORT_APPLY(target, TTF_GetFontStyle);
	NANX_EXPORT_APPLY(target, TTF_SetFontStyle);
	NANX_EXPORT_APPLY(target, TTF_GetFontOutline);
	NANX_EXPORT_APPLY(target, TTF_SetFontOutline);
	NANX_EXPORT_APPLY(target, TTF_GetFontHinting);
	NANX_EXPORT_APPLY(target, TTF_SetFontHinting);
	NANX_EXPORT_APPLY(target, TTF_FontHeight);
	NANX_EXPORT_APPLY(target, TTF_FontAscent);
	NANX_EXPORT_APPLY(target, TTF_FontDescent);
	NANX_EXPORT_APPLY(target, TTF_FontLineSkip);
	NANX_EXPORT_APPLY(target, TTF_GetFontKerning);
	NANX_EXPORT_APPLY(target, TTF_SetFontKerning);
	NANX_EXPORT_APPLY(target, TTF_FontFaces);
	NANX_EXPORT_APPLY(target, TTF_FontFaceIsFixedWidth);
	NANX_EXPORT_APPLY(target, TTF_FontFaceFamilyName);
	NANX_EXPORT_APPLY(target, TTF_FontFaceStyleName);
	NANX_EXPORT_APPLY(target, TTF_GlyphIsProvided);
	NANX_EXPORT_APPLY(target, TTF_GlyphMetrics);
	NANX_EXPORT_APPLY(target, TTF_SizeText);
	NANX_EXPORT_APPLY(target, TTF_SizeUTF8);
	NANX_EXPORT_APPLY(target, TTF_SizeUNICODE);
	NANX_EXPORT_APPLY(target, TTF_RenderText_Solid);
	NANX_EXPORT_APPLY(target, TTF_RenderUTF8_Solid);
	NANX_EXPORT_APPLY(target, TTF_RenderUNICODE_Solid);
	NANX_EXPORT_APPLY(target, TTF_RenderGlyph_Solid);
	NANX_EXPORT_APPLY(target, TTF_RenderText_Shaded);
	NANX_EXPORT_APPLY(target, TTF_RenderUTF8_Shaded);
	NANX_EXPORT_APPLY(target, TTF_RenderUNICODE_Shaded);
	NANX_EXPORT_APPLY(target, TTF_RenderGlyph_Shaded);
	NANX_EXPORT_APPLY(target, TTF_RenderText_Blended);
	NANX_EXPORT_APPLY(target, TTF_RenderUTF8_Blended);
	NANX_EXPORT_APPLY(target, TTF_RenderUNICODE_Blended);
	NANX_EXPORT_APPLY(target, TTF_RenderGlyph_Blended);
	NANX_EXPORT_APPLY(target, TTF_RenderText_Blended_Wrapped);
	NANX_EXPORT_APPLY(target, TTF_RenderUTF8_Blended_Wrapped);
	NANX_EXPORT_APPLY(target, TTF_RenderUNICODE_Blended_Wrapped);
	NANX_EXPORT_APPLY(target, TTF_RenderText);
	NANX_EXPORT_APPLY(target, TTF_RenderUTF8);
	NANX_EXPORT_APPLY(target, TTF_RenderUNICODE);
	NANX_EXPORT_APPLY(target, TTF_GetFontKerningSize);
	#if SDL_TTF_VERSION_ATLEAST(2, 0, 14)
	NANX_EXPORT_APPLY(target, TTF_GetFontKerningSizeGlyphs);
	#endif
}

} // namespace node_sdl2_ttf

NODE_MODULE(node_sdl2_ttf, node_sdl2_ttf::init)
