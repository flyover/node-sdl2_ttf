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

#ifndef _NODE_SDL2_TTF_H_
#define _NODE_SDL2_TTF_H_

#include <nan.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "node-sdl2.h"

namespace node_sdl2_ttf {

// wrap TTF_Font pointer

class WrapFont : public Nan::ObjectWrap
{
private:
	TTF_Font* m_font;
public:
	WrapFont(TTF_Font* font) : m_font(font) {}
	~WrapFont() { Free(m_font); m_font = NULL; }
public:
	TTF_Font* Peek() { return m_font; }
	TTF_Font* Drop() { TTF_Font* font = m_font; m_font = NULL; return font; }
public:
	static WrapFont* Unwrap(v8::Local<v8::Value> value) { return (value->IsObject())?(Unwrap(v8::Local<v8::Object>::Cast(value))):(NULL); }
	static WrapFont* Unwrap(v8::Local<v8::Object> object) { return Nan::ObjectWrap::Unwrap<WrapFont>(object); }
	static TTF_Font* Peek(v8::Local<v8::Value> value) { WrapFont* wrap = Unwrap(value); return (wrap)?(wrap->Peek()):(NULL); }
public:
	static v8::Local<v8::Value> Hold(TTF_Font* font) { return NewInstance(font); }
	static TTF_Font* Drop(v8::Local<v8::Value> value) { WrapFont* wrap = Unwrap(value); return (wrap)?(wrap->Drop()):(NULL); }
	static void Free(TTF_Font* font)
	{
		if (font) { TTF_CloseFont(font); font = NULL; }
	}
public:
	static v8::Local<v8::Object> NewInstance(TTF_Font* font)
	{
		Nan::EscapableHandleScope scope;
		v8::Local<v8::ObjectTemplate> object_template = GetObjectTemplate();
		v8::Local<v8::Object> instance = object_template->NewInstance();
		WrapFont* wrap = new WrapFont(font);
		wrap->Wrap(instance);
		return scope.Escape(instance);
	}
private:
	static v8::Local<v8::ObjectTemplate> GetObjectTemplate()
	{
		Nan::EscapableHandleScope scope;
		static Nan::Persistent<v8::ObjectTemplate> g_object_template;
		if (g_object_template.IsEmpty())
		{
			v8::Local<v8::ObjectTemplate> object_template = Nan::New<v8::ObjectTemplate>();
			g_object_template.Reset(object_template);
			object_template->SetInternalFieldCount(1);
		}
		v8::Local<v8::ObjectTemplate> object_template = Nan::New<v8::ObjectTemplate>(g_object_template);
		return scope.Escape(object_template);
	}
};

NAN_MODULE_INIT(init);

} // namespace node_sdl2_ttf

#endif // _NODE_SDL2_TTF_H_
