// The Node.js web build's start (preset web-node, for -port-selftest): there's no page here, but SDL's web code
// looks for one; its dummy video and audio drivers then stand in for the browser's.
if (typeof window === 'undefined') {
  globalThis.window = globalThis;
  globalThis.location = { search: '', hash: '', href: 'file:///' };
}
Module.preRun = Module.preRun || [];
Module.preRun.push(() => {
  ENV.SDL_VIDEO_DRIVER = 'dummy';
  ENV.SDL_AUDIO_DRIVER = 'dummy';
});
