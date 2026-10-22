/* [library:window] Linked after Emscripten's code (--post-js: CMakeLists.txt, tools/test_miniwin/build_web.sh).

   The game draws with WebGL from its own thread (a worker's: port/web/README.md), into a picture of its own on the
   page's thread that is copied to the canvas each frame (OFFSCREEN_FRAMEBUFFER). When the canvas changes its size (the
   page's, a phone or a tablet turned, full screen) Emscripten gives that picture the new size in place
   (GL.resizeOffscreenFramebuffer: texImage2D on the texture the framebuffer holds), and in WebKit (Safari, iPhones,
   iPads) nothing drawn reached it any more after that: the screen stayed black (tools/web/presenttest.py). This makes
   a new texture and depth buffer of the new size for it instead, as it was first made. */
if (typeof GL !== 'undefined' && GL.resizeOffscreenFramebuffer) {
  GL.resizeOffscreenFramebuffer = (context) => {
    const gl = context.GLctx;
    const framebuffer = gl.getParameter(gl.FRAMEBUFFER_BINDING);
    gl.bindFramebuffer(gl.FRAMEBUFFER, context.defaultFbo);
    if (context.defaultColorTarget) {
      const bound = gl.getParameter(gl.TEXTURE_BINDING_2D);
      const old = context.defaultColorTarget;
      const texture = gl.createTexture();
      gl.bindTexture(gl.TEXTURE_2D, texture);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
      gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.drawingBufferWidth, gl.drawingBufferHeight, 0, gl.RGBA,
                    gl.UNSIGNED_BYTE, null);
      gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);
      context.defaultColorTarget = texture;
      gl.deleteTexture(old);
      gl.bindTexture(gl.TEXTURE_2D, bound === old ? texture : bound);
    }
    if (context.defaultDepthTarget) {
      const bound = gl.getParameter(gl.RENDERBUFFER_BINDING);
      const old = context.defaultDepthTarget;
      const depth = gl.createRenderbuffer();
      gl.bindRenderbuffer(gl.RENDERBUFFER, depth);
      gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, gl.drawingBufferWidth, gl.drawingBufferHeight);
      gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, depth);
      context.defaultDepthTarget = depth;
      gl.deleteRenderbuffer(old);
      gl.bindRenderbuffer(gl.RENDERBUFFER, bound === old ? depth : bound);
    }
    gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffer);
  };
}
