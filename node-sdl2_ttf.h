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

#include <v8.h>
#include <node.h>
#include <nan.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "node-sdl2.h"

namespace node_sdl2_ttf {

// wrap TTF_Font pointer

class WrapFont : public node_sdl2::SimpleWrap<WrapFont,TTF_Font*>
{
	public: static void InitTemplate(v8::Handle<v8::ObjectTemplate> tpl) {}
	public: static void Free(TTF_Font* font)
	{
		if (font) { TTF_CloseFont(font); font = NULL; }
	}
};

#if NODE_VERSION_AT_LEAST(0,11,0)
void init(v8::Handle<v8::Object> exports, v8::Handle<v8::Value> module, v8::Handle<v8::Context> context);
#else
void init(v8::Handle<v8::Object> exports/*, v8::Handle<v8::Value> module*/);
#endif

} // namespace node_sdl2_ttf

#endif // _NODE_SDL2_TTF_H_

