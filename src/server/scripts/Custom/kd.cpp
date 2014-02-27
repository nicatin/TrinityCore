diff -r 2d23fc779565 src/server/game/Scripting/ScriptLoader.cpp 
--- a/src/server/game/Scripting/ScriptLoader.cpp    Mon Sep 12 23:41:29 2011 -0700 
+++ b/src/server/game/Scripting/ScriptLoader.cpp    Thu Sep 15 21:44:36 2011 +0300 
@@ -23,6 +23,7 @@ 
    void AddSC_example_gossip_codebox(); 
    void AddSC_example_misc(); 
    void AddSC_example_commandscript(); 
+void AddSC_DuelReset(); 
       
    // spells 
    void AddSC_deathknight_spell_scripts(); 
@@ -1229,6 +1230,6 @@ 
    { 
    #ifdef SCRIPTS 
        /* This is where custom scripts should be added. */ 
- 
+    AddSC_DuelReset(); 
    #endif 
    } 
diff -r 2d23fc779565 src/server/game/World/World.cpp 
--- a/src/server/game/World/World.cpp    Mon Sep 12 23:41:29 2011 -0700 
+++ b/src/server/game/World/World.cpp    Thu Sep 15 21:44:36 2011 +0300 
@@ -1184,6 +1184,11 @@ 
       
        // MySQL ping time interval 
        m_int_configs[CONFIG_DB_PING_INTERVAL] = ConfigMgr::GetIntDefault("MaxPingTime", 30); 
+       
+    //Reset Duel Cooldown 
+    m_bool_configs[CONFIG_DUEL_RESET_COOLDOWN_ON_START] = ConfigMgr::GetBoolDefault("DuelReset.Cooldown.OnStart", false); 
+    m_bool_configs[CONFIG_DUEL_RESET_COOLDOWN_ON_FINISH] = ConfigMgr::GetBoolDefault("DuelReset.Cooldown.OnFinish", false); 
+    m_bool_configs[CONFIG_DUEL_RESET_COOLDOWN_ONLY_IN_ELWYNN_AND_DUROTAR] = ConfigMgr::GetBoolDefault("DuelReset.Cooldown.Only.in.Elwynn.and.Durotar", false); 
+    m_bool_configs[CONFIG_DUEL_RESET_COOLDOWN_MAX_ENERGY_ON_START] = ConfigMgr::GetBoolDefault("DuelReset.Cooldown.Max.Energy.OnStart", false); 
+    m_bool_configs[CONFIG_DUEL_RESET_COOLDOWN_RESET_ENERGY_ON_START] = ConfigMgr::GetBoolDefault("DuelReset.Cooldown.Reset.Energy.OnStart", false); 
       
        sScriptMgr->OnConfigLoad(reload); 
    } 
diff -r 2d23fc779565 src/server/game/World/World.h 
--- a/src/server/game/World/World.h    Mon Sep 12 23:41:29 2011 -0700 
+++ b/src/server/game/World/World.h    Thu Sep 15 21:44:36 2011 +0300 
@@ -163,6 +163,9 @@ 
        CONFIG_QUEST_IGNORE_AUTO_ACCEPT, 
        CONFIG_QUEST_IGNORE_AUTO_COMPLETE, 
        CONFIG_WARDEN_ENABLED, 
+    CONFIG_DUEL_RESET_COOLDOWN_ON_START, 
+    CONFIG_DUEL_RESET_COOLDOWN_ON_FINISH, 
+    CONFIG_DUEL_RESET_COOLDOWN_ONLY_IN_ELWYNN_AND_DUROTAR, 
+    CONFIG_DUEL_RESET_COOLDOWN_RESET_ENERGY_ON_START, 
+    CONFIG_DUEL_RESET_COOLDOWN_MAX_ENERGY_ON_START, 
        BOOL_CONFIG_VALUE_COUNT 
    }; 
       
diff -r 2d23fc779565 src/server/scripts/Custom/CMakeLists.txt 
--- a/src/server/scripts/Custom/CMakeLists.txt    Mon Sep 12 23:41:29 2011 -0700 
+++ b/src/server/scripts/Custom/CMakeLists.txt    Thu Sep 15 21:44:36 2011 +0300 
@@ -10,6 +10,7 @@ 
       
    set(scripts_STAT_SRCS 
      ${scripts_STAT_SRCS} 
+  Custom/duel_reset.cpp 
    ) 
       
    message("  -> Prepared: Custom") 
diff -r 2d23fc779565 src/server/scripts/Custom/duel_reset.cpp 
--- /dev/null    Thu Jan 01 00:00:00 1970 +0000 
+++ b/src/server/scripts/Custom/duel_reset.cpp    Thu Sep 15 21:44:36 2011 +0300 
@@ -0,0 +1,92 @@ 
+#include "ScriptPCH.h" 
+#include "ScriptMgr.h" 
+#include "Config.h" 
+ 
+class duel_reset : public PlayerScript 
+{ 
+public: 
+    duel_reset() : PlayerScript("duel_reset") {} 
+ 
+    void OnDuelEnd(Player *winner, Player *looser, DuelCompleteType type) 
+    { 
+        if (sWorld->getBoolConfig(CONFIG_DUEL_RESET_COOLDOWN_ON_FINISH)) 
+        { 
+            if (sWorld->getBoolConfig(CONFIG_DUEL_RESET_COOLDOWN_ONLY_IN_ELWYNN_AND_DUROTAR)) 
+            { 
+                if(winner->GetZoneId() == 14 || winner->GetZoneId() == 12) 
+                { 
+                    if (type == DUEL_WON) 
+                    { 
+                        winner->RemoveArenaSpellCooldowns(); 
+                        looser->RemoveArenaSpellCooldowns(); 
+                        winner->SetHealth(winner->GetMaxHealth()); 
+                        looser->SetHealth(looser->GetMaxHealth()); 
+ 
+                        if (winner->getPowerType() == POWER_MANA)    
+                            winner->SetPower(POWER_MANA, winner->GetMaxPower(POWER_MANA)); 
+ 
+                        if (looser->getPowerType() == POWER_MANA)    
+                            looser->SetPower(POWER_MANA, looser->GetMaxPower(POWER_MANA)); 
+                    } 
+                    winner->HandleEmoteCommand(EMOTE_ONESHOT_CHEER); 
+                } 
+            } 
+ 
+            else 
+                if (type == DUEL_WON) 
+                { 
+                    winner->RemoveArenaSpellCooldowns(); 
+                    looser->RemoveArenaSpellCooldowns(); 
+                    winner->SetHealth(winner->GetMaxHealth()); 
+                    looser->SetHealth(looser->GetMaxHealth()); 
+ 
+                    if (winner->getPowerType() == POWER_MANA)    
+                        winner->SetPower(POWER_MANA, winner->GetMaxPower(POWER_MANA)); 
+ 
+                    if (looser->getPowerType() == POWER_MANA)    
+                        looser->SetPower(POWER_MANA, looser->GetMaxPower(POWER_MANA)); 
+                } 
+                winner->HandleEmoteCommand(EMOTE_ONESHOT_CHEER); 
+        } 
+    } 
+ 
+    void OnDuelStart(Player *player1, Player *player2) 
+    { 
+        if (sWorld->getBoolConfig(CONFIG_DUEL_RESET_COOLDOWN_ON_START)) 
+        { 
+            if (sWorld->getBoolConfig(CONFIG_DUEL_RESET_COOLDOWN_ONLY_IN_ELWYNN_AND_DUROTAR)) 
+            { 
+                if(player1->GetZoneId() == 14 || player1->GetZoneId() == 12) 
+                { 
+                    player1->RemoveArenaSpellCooldowns(); 
+                    player2->RemoveArenaSpellCooldowns(); 
+                    player1->SetHealth(player1->GetMaxHealth()); 
+                    player2->SetHealth(player2->GetMaxHealth()); 
+ 
+                    if (sWorld->getBoolConfig(CONFIG_DUEL_RESET_COOLDOWN_RESET_ENERGY_ON_START)) 
+                    { 
+                        switch (player1->getPowerType()) 
+                        { 
+                        case POWER_RAGE: 
+                            player1->SetPower(POWER_RAGE, 0); 
+                            break; 
+                        case POWER_RUNIC_POWER: 
+                            player1->SetPower(POWER_RUNIC_POWER, 0); 
+                            break; 
+                        default: 
+                            break; 
+                        } 
+ 
+                        switch (player2->getPowerType()) 
+                        { 
+                        case POWER_RAGE: 
+                            player2->SetPower(POWER_RAGE, 0); 
+                            break; 
+                        case POWER_RUNIC_POWER: 
+                            player2->SetPower(POWER_RUNIC_POWER, 0); 
+                            break; 
+                        default: 
+                            break; 
+                        } 
+                    } 
+ 
+                    if (sWorld->getBoolConfig(CONFIG_DUEL_RESET_COOLDOWN_MAX_ENERGY_ON_START)) 
+                    { 
+                        switch (player1->getPowerType()) 
+                        { 
+                        case POWER_MANA: 
+                            player1->SetPower(POWER_MANA, player1->GetMaxPower(POWER_MANA)); 
+                            break; 
+                        case POWER_RAGE: 
+                            player1->SetPower(POWER_RAGE, player1->GetMaxPower(POWER_RAGE)); 
+                            break; 
+                        case POWER_RUNIC_POWER: 
+                            player1->SetPower(POWER_RUNIC_POWER, player1->GetMaxPower(POWER_RUNIC_POWER)); 
+                            break; 
+                        default: 
+                            break; 
+                        } 
+ 
+                        switch (player2->getPowerType()) 
+                        { 
+                        case POWER_MANA: 
+                            player2->SetPower(POWER_MANA, player2->GetMaxPower(POWER_MANA)); 
+                           break; 
+                       case POWER_RAGE: 
+                           player2->SetPower(POWER_RAGE, player2->GetMaxPower(POWER_RAGE)); 
+                            break; 
+                        case POWER_RUNIC_POWER: 
+                            player2->SetPower(POWER_RUNIC_POWER, player2->GetMaxPower(POWER_RUNIC_POWER)); 
+                            break; 
+                        default: 
+                            break; 
+                        } 
+                    } 
+                } 
+            } 
+ 
+            else 
+            { 
+                player1->RemoveArenaSpellCooldowns(); 
+                player2->RemoveArenaSpellCooldowns(); 
+                player1->SetHealth(player1->GetMaxHealth()); 
+                player2->SetHealth(player2->GetMaxHealth()); 
+ 
+                if (sWorld->getBoolConfig(CONFIG_DUEL_RESET_COOLDOWN_MAX_ENERGY_ON_START)) 
+                { 
+                    switch (player1->getPowerType()) 
+                    { 
+                    case POWER_MANA: 
+                        player1->SetPower(POWER_MANA, player1->GetMaxPower(POWER_MANA)); 
+                        break; 
+                    case POWER_RAGE: 
+                        player1->SetPower(POWER_RAGE, player1->GetMaxPower(POWER_RAGE)); 
+                        break; 
+                    case POWER_RUNIC_POWER: 
+                        player1->SetPower(POWER_RUNIC_POWER, player1->GetMaxPower(POWER_RUNIC_POWER)); 
+                        break; 
+                    default: 
+                        break; 
+                    } 
+ 
+                    switch (player2->getPowerType()) 
+                    { 
+                    case POWER_MANA: 
+                        player2->SetPower(POWER_MANA, player2->GetMaxPower(POWER_MANA)); 
+                        break; 
+                    case POWER_RAGE: 
+                        player2->SetPower(POWER_RAGE, player2->GetMaxPower(POWER_RAGE)); 
+                        break; 
+                    case POWER_RUNIC_POWER: 
+                        player2->SetPower(POWER_RUNIC_POWER, player2->GetMaxPower(POWER_RUNIC_POWER)); 
+                        break; 
+                    default: 
+                        break; 
+                    } 
+                } 
+            } 
+        } 
+    } 
+}; 
+ 
+void AddSC_DuelReset() 
+{ 
+    new duel_reset(); 
+} 
+ 
diff -r 2d23fc779565 src/server/worldserver/worldserver.conf.dist 
--- a/src/server/worldserver/worldserver.conf.dist    Mon Sep 12 23:41:29 2011 -0700 
+++ b/src/server/worldserver/worldserver.conf.dist    Thu Sep 15 21:44:36 2011 +0300 
@@ -2771,3 +2771,33 @@ 
       
    # 
    ################################################################################################### 
+ 
+################################################################################################### 
+# DUEL RESET COOLDOWN BY DIMITRO 
+# 
+#     DuelReset.Cooldown.OnStart 
+#     Reseting cooldown and restore health to start of duel. 
+#     Default: 0 - Disable 
+#              1 - Enable 
+# 
+ 
+DuelReset.Cooldown.OnStart = 0 
+ 
+#     DuelReset.Cooldown.OnFinish 
+#     Reseting cooldown and restore health to finish of duel. 
+#     Default: 0 - Disable 
+#              1 - Enable 
+# 
+ 
+DuelReset.Cooldown.OnFinish = 0 
+ 
+#     DuelReset.Cooldown.Only.in.Elwynn.and.Durotar 
+#     Reseting cooldown only in Elwynn forest and Durotar locations. 
+#     Default: 0 - Disable 
+#              1 - Enable 
+# 
+ 
+DuelReset.Cooldown.Only.in.Elwynn.and.Durotar = 0 
+ 
+#     DuelReset.Cooldown.Max.Energy.OnStart 
+#     Sets rage, mana and runic power to max on duel start. 
+#     Default: 0 - Disable 
+#              1 - Enable 
+# 
+ 
+DuelReset.Cooldown.Max.Energy.OnStart = 0 
+ 
+#     DuelReset.Cooldown.Reset.Energy.OnStart 
+#     Resets rage and runic power on duel start. 
+#     Default: 0 - Disable 
+#              1 - Enable 
+# 
+ 
+DuelReset.Cooldown.Reset.Energy.OnStart = 0 
+ 
+# 
+###################################################################################################
