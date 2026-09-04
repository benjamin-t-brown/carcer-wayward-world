module;
#include <cstddef>

export module carcer.actions.world:WorldSetActionMode;
export import carcer.state;
import carcer.game.map;
import carcer.model.templates;
import bmin.string_interop;
#include "macros.h"

export {

namespace state {

namespace actions {

struct WorldSetActionModeCtx {
  bmin::String spellId;
  bmin::String chId;
};

class WorldSetActionMode : public AbstractAction {
  model::WorldActionMode mode = model::WorldActionMode::NONE;
  WorldSetActionModeCtx ctx;

  void act() override {
    if (!state) {
      return;
    }
    game::resolveWorldActionMode(
        state->world, state->player, mode, ctx.spellId, ctx.chId);
  }

public:
  explicit WorldSetActionMode(model::WorldActionMode _mode,
                              const WorldSetActionModeCtx& _ctx = {})
      : mode(_mode), ctx(_ctx) {}
};

} // namespace actions

} // namespace state

} // export
