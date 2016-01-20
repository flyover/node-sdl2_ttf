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

var node_sdl2_ttf = null;
try {
  node_sdl2_ttf = node_sdl2_ttf || require('./build/Release/node-sdl2_ttf.node');
} catch (err) {}
try {
  node_sdl2_ttf = node_sdl2_ttf || process._linkedBinding('node_sdl2_ttf');
} catch (err) {}
try {
  node_sdl2_ttf = node_sdl2_ttf || process.binding('node_sdl2_ttf');
} catch (err) {}
module.exports = node_sdl2_ttf;

node_sdl2_ttf.version = node_sdl2_ttf.version || node_sdl2_ttf.SDL_TTF_MAJOR_VERSION + "." + node_sdl2_ttf.SDL_TTF_MINOR_VERSION + "." + node_sdl2_ttf.SDL_TTF_PATCHLEVEL;

node_sdl2_ttf.TTF_CheckError = node_sdl2_ttf.TTF_CheckError || function() {
  var error = node_sdl2_ttf.TTF_GetError();
  node_sdl2_ttf.TTF_ClearError();
  if (error) {
    console.error("SDL_ttf", error);
  }
  return error;
};

/// var node_sdl2_ttf = require('@flyover/node-sdl2_ttf');
/// var sdl_ttf = node_sdl2_ttf.TTF();
/// node_sdl2_ttf.TTF_* -> sdl_ttf.*
node_sdl2_ttf.TTF = function(out) {
  out = out || {};
  var re = /^(TTF_)(.*)/;
  for (var key in node_sdl2_ttf) {
    var match = key.match(re);
    if (match && match[2]) {
      //console.log(key, match[2]);
      out[match[2]] = node_sdl2_ttf[key];
    } else {
      //console.log("!!!", key);
      out[key] = node_sdl2_ttf[key];
    }
  }
  return out;
}

//node_sdl2_ttf.Mix();
