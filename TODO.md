The following is a list of "stuff that should get done." It's not necessarily a
list of "stuff that *will* get done."

- [ ] Create AviSynth runner that will actually pull a single frame out of the
      AviSynth pipeline and be able to check that against a known good source
      for the test harness
- [ ] With above runner, test multi-threading (needed for VirtualDub "play"
      button and it's a fairly safe guess something else)
- [ ] Fix up the template creation into something that isn't freaking speghetti
      (this probably means keeping templates solely with JSVEnvironment and
      moving all object creation there)
- [ ] Make the templates created come even remotely close to matching the class
      structure that [docs/api/jsvsynth_api.js] pretends exists.
- [ ] Add all the FIXME instances to this list
- [ ] HTML5-style canvas API (hopefully using libcairo)
- [ ] Investigating integrating with node.js (make a node.js module? make the
      plugin work with node.js? I'm not sure)
