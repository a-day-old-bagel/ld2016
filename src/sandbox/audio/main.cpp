/*
 * Copyright (c) 2016 Jonathan Glines, Galen Cochrane
 * Jonathan Glines <jonathan@glines.net>
 * Galen Cochrane <galencochrane@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <cstdlib>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../../common/debug.h"
#include "../../common/game.h"
#include "../../common/meshObject.h"
#include "../../common/scene.h"

#include "../../common/ecs/ecsHelpers.h"
#include "../../common/ecs/ecsSystem_movement.h"
#include "../../common/ecs/ecsSystem_controls.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

using namespace ld2016;
using namespace ecs;

class AudioDemo : public Game {
  private:
  public:
    Delegate<bool(SDL_Event&)> systemsHandlerDlgt;
    AudioDemo(int argc, char **argv)
        : Game(argc, argv, "Pyramid Game")
    {
      systemsHandlerDlgt = DELEGATE(&AudioDemo::systemsHandler, this);
    }
    EcsResult init() {
      return ECS_SUCCESS;
    }
    bool systemsHandler(SDL_Event& event) {
      return false;
    }
    void tick(float dt) {
    }
};

Uint8 *gameMusic;
Uint32 gameMusicLength;

void audioCallback(
    void *userdata,
    Uint8 *stream,
    int len)
{
  static int current = 0;
  SDL_memset(stream, 0, len);
  while (len > 0) {
    int chunk = min(len, gameMusicLength - current);
    SDL_MixAudio(
        stream,
        gameMusic + current,
        chunk,
        SDL_MIX_MAXVOLUME);
    stream += chunk;
    len -= chunk;
    current = (current + chunk) % gameMusicLength;
    fprintf(stderr, "Copied audio to stream.\n");
  }
}

void main_loop(void *instance) {
  AudioDemo *demo = (AudioDemo *) instance;
  float dt;
  demo->mainLoop(demo->systemsHandlerDlgt, dt);
  demo->tick(dt);
}

int main(int argc, char **argv) {
  AudioDemo demo(argc, argv);
  EcsResult status = demo.init();
  if (status.isError()) { fprintf(stderr, "%s", status.toString().c_str()); }

  int count = SDL_GetNumAudioDevices(0);
  fprintf(stderr, "Number of audio devices: %d\n", count);
  for (int i = 0; i < count; ++i) {
    fprintf(stderr, "Audio device %d: %s\n", i, SDL_GetAudioDeviceName(i, 0));
  }
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID dev;
  SDL_memset(&want, 0, sizeof(want));
  want.freq = 4800;
  want.format = AUDIO_F32;
  want.channels = 2;
  want.samples = 4096;
  want.callback = NULL;

  SDL_LoadWAV(
      "./assets/audio/TitleScreen.wav",
      &want,
      &gameMusic,
      &gameMusicLength);

  dev = SDL_OpenAudioDevice(
      NULL,  // device
      0,  // is capture
      &want,  // desired
      &have,  // obtained
      0  // allowed changes
      );
  if (dev == 0) {
    fprintf(stderr, "Failed to open SDL audio device: %s\n", SDL_GetError());
  } else {
    SDL_QueueAudio(
        dev,
        gameMusic,
        gameMusicLength);
    SDL_PauseAudioDevice(dev, 0);  // start audio
  }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(main_loop, (void*)&demo, 0, 1);
#else
    while (1) {
      main_loop(&demo);
      // TODO: Wait for VSync? Or should we poll input faster than that?
    }
#endif

  SDL_CloseAudioDevice(dev);

  return EXIT_SUCCESS;
}
