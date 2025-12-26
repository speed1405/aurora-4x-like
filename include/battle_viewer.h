#ifndef BATTLE_VIEWER_H
#define BATTLE_VIEWER_H

#include <string>
#include <vector>
#include "combat.h"

// Windows-only sprite battle viewer.
// On non-Windows platforms this is a no-op.
#ifdef _WIN32
void showBattleSprites(const std::string& title,
                       const std::vector<CombatFrame>& frames);
#else
inline void showBattleSprites(const std::string&, const std::vector<CombatFrame>&) {}
#endif

#endif // BATTLE_VIEWER_H
