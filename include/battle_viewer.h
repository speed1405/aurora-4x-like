#ifndef BATTLE_VIEWER_H
#define BATTLE_VIEWER_H

#include <string>
#include <vector>
#include "combat.h"

// Windows-only sprite battle viewer.
// On non-Windows platforms this is a no-op.
void showBattleSprites(const std::string& title,
                       const std::vector<CombatFrame>& frames);

#endif // BATTLE_VIEWER_H
