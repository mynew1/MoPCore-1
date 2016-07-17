/*
* Copyright (C) 2011 - 2012 ArkCORE <http://www.arkania.net/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "SpellMgr.h"
#include "Player.h"
#include "Creature.h"
#include "Vehicle.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"

#define SAY_D_A "Ecrouabouille, what are you doing sitting there? You do not recognize the one who is lying?"
#define SAY_D_B  "That's $ N! It is only thanks to $ he: she, if we still breath instead of being pieces of rind grids has Kezan."
#define SAY_D_C  "That's $ N! Sorry, doc, I thought $ ghe death:she death; !"
#define SAY_D_D  "Remains back, I'm $ ghe: she; revive! Hoping that this soggy defibrillator will not kill us all!"
#define SAY_D_E  "Caution! OFF!"
#define SAY_D_F "That's all I can do. C'' is a $ ghe: she; to react now. You hear me, $ N? Come on, wake up now! Do not go to the Light!"
#define SAY_D_G  "You made the right choice. We need you so much, $ N. Try not to get killed here. "
#define SAY_D_H  "There are other survivors that I must attend. I'll see you on the shore."

#define GIZMO 36600
#define SPELL_DEAD_STILL 69010

enum Texts
{
    SAY_GYRO = 0
};

class npc_Zapnozzle : public CreatureScript
{
public:
    npc_Zapnozzle() : CreatureScript("npc_Zapnozzle") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_ZapnozzleAI(creature);
    }

    bool OnQuestReward(Player* player, Creature* creature, const Quest *_Quest, uint32 )
    {
        if (_Quest->GetQuestId() == 14239)
            creature->AI()->DoAction(1);
        return true;
    }

    struct npc_ZapnozzleAI : public ScriptedAI
    {
        npc_ZapnozzleAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset ()
        {
            eventTalk = 0;
            eventTalk2 = 0;
            mui_talk = 2000;
            mui_talk2 = 2000;
            isEventInProgress = false;
            start = false;
        }

        void DoAction(const int32 param)
        {
            if (param == 1)
                isEventInProgress = true;
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
        }

        void JustReachedHome()
        {

        }

        void WaypointReached(uint32)
        {

        }


        void MovementInform(uint32 /*type*/, uint32 id)
        {
            if (id == 1)
                me->DespawnOrUnsummon();
        }

        void UpdateAI(const uint32 diff)
        {
            if (!isEventInProgress)
            {
                if (mui_talk <= diff)
                {
                    mui_talk = 6000;
                    if (!start)
                        player = me->FindNearestPlayer(10);
                    if (!player)
                        return;
                    if (!start)
                    {
                        if (!player->HasAura(SPELL_DEAD_STILL))
                            return;
                        else
                            start = true;
                    }
                    switch (eventTalk)
                    {
                    case 0 :
                        me->MonsterSay(SAY_D_A, LANG_UNIVERSAL, player->GetGUID());
                        break;
                    case 1 :
                        me->MonsterSay(SAY_D_B, LANG_UNIVERSAL, player->GetGUID());
                        break;
                    case 2 :
                        if (Creature *c = me->FindNearestCreature(GIZMO, 10))
                            c->MonsterSay(SAY_D_C, LANG_UNIVERSAL, player->GetGUID());
                        break;
                    case 3 :
                        me->MonsterSay(SAY_D_D, LANG_UNIVERSAL, player->GetGUID());
                        me->CastSpell(player, 54732, true);
                        break;
                    default :
                        break;
                    }
                    eventTalk++;
                }
                else
                    mui_talk -= diff;
            }
            if (!isEventInProgress)
                return;
            if (mui_talk2 <= diff)
            {
                mui_talk2 = 6000;
                if (!start)
                    player = me->FindNearestPlayer(10);
                if (!player)
                    return;
                switch(eventTalk2)
                {
                case 0 :
                    me->MonsterSay(SAY_D_E, LANG_UNIVERSAL, player->GetGUID());
                    me->CastSpell(player, 54732, true);
                    player->RemoveAurasDueToSpell(SPELL_DEAD_STILL);
					player->SetPhaseMask(1, true);
                    break;
                case 1 :
                    me->MonsterSay(SAY_D_F, LANG_UNIVERSAL, player->GetGUID());
                    break;
                case 2 :
                    me->MonsterSay(SAY_D_G, LANG_UNIVERSAL, player->GetGUID());
                    break;
                case 3 :
                    me->CastSpell(player, 54732, true);
                    me->MonsterSay(SAY_D_H, LANG_UNIVERSAL, player->GetGUID());
                    mui_talk2 = 2000;
                    break;
                case 4 :
                    me->SetSpeed(MOVE_SWIM, 2, true);
                    me->GetMotionMaster()->MovePoint(1, 578.49f, 3132.37f, 0.26f);
                    break;
                default :
                    eventTalk2 = 0;
                    isEventInProgress = false;
                    eventTalk = 0;
                    start = false;
                    break;
                }
                eventTalk2++;
            }
            else
                mui_talk2 -= diff;
        }

    private :
        bool isEventInProgress, start;
        uint32 mui_talk, mui_talk2;
        unsigned int eventTalk, eventTalk2;
        Player *player;
    };
};

class npc_Mechumide : public CreatureScript
{
public:
    npc_Mechumide() : CreatureScript("npc_Mechumide") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const*_Quest)
    {
        if (_Quest->GetQuestId() == 14021)
        {
            if (Creature *mineur = player->SummonCreature(35810, player->GetPositionX() + 2, player->GetPositionY(),  player->GetPositionZ() + 2,  player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
                if (Creature *chariot = player->SummonCreature(35814, player->GetPositionX() - 2, player->GetPositionY(),  player->GetPositionZ() + 2,  player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
                {
					mineur->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING); //possibly wrong?
					chariot->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);//possibly wrong?
                    mineur->CastSpell(chariot, 68122, true);
                    chariot->GetMotionMaster()->MoveFollow(mineur, 1.0f, 1.0f);
                    mineur->MonsterSay("Ok, here we go. You cover me, okay?", LANG_UNIVERSAL, 0);
                    CAST_AI(npc_escortAI, (mineur->AI()))->Start(true, true, player->GetGUID(), _Quest);
                }
        }
        return true;
    }
};

class npc_mineur_gob : public CreatureScript
{
public:
    npc_mineur_gob() : CreatureScript("npc_mineur_gob") { }

    struct npc_mineur_gobAI : public npc_escortAI
    {
        npc_mineur_gobAI(Creature* creature) : npc_escortAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset() {}

        void WaypointReached(uint32 i)
        {
            SetRun(false);
            switch(i)
            {
            case 6:
                me->MonsterSay("We touch the jackpot with this place!", LANG_UNIVERSAL, 0);
                //me->LoadEquipment(2901, true);
                me->HandleEmoteCommand(467);
                break;
            case 9:
                me->MonsterSay("Wow, what is that all these cave paintings? Oh, I hear monkeys, the?", LANG_UNIVERSAL, 0);
                //me->LoadEquipment(2901, true);
                me->HandleEmoteCommand(467);
                break;
            case 13:
                me->MonsterSay("Move to the next.", LANG_UNIVERSAL, 0);
                break;
            case 12:
                //me->LoadEquipment(2901, true);
                me->HandleEmoteCommand(467);
                break;
            case 17:
                //me->LoadEquipment(2901, true);
                me->HandleEmoteCommand(467);
                break;
            case 18:
                if (Player *player = me->FindNearestPlayer(20))
                {
                    me->MonsterSay("It will suffice for now. I can go it alone. Thank you for escorting me, $ N.", LANG_UNIVERSAL, player->GetGUID());
                    player->KilledMonsterCredit(35816, 0);
                }
                else
                    me->MonsterSay("It will suffice for now. I can go it alone. Thank you for escorting me, $ N.", LANG_UNIVERSAL, 0);
                if (Creature *c = me->FindNearestCreature(35814, 10))
                    c->DespawnOrUnsummon();
                break;
            default:
                break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void SetHoldState(bool bOnHold)
        {
            SetEscortPaused(bOnHold);
        }

        void UpdateEscortAI(const uint32 diff)
        {
            if (m_ui_attack <= diff)
            {
                if (Creature *cb = me->FindNearestCreature(35812, 10, true))
                    SetHoldState(true);
                else
                    SetHoldState(false);
                m_ui_attack = 1000;
            }
            else m_ui_attack -= diff;

            DoMeleeAttackIfReady();
        }

    private :
        uint32 m_ui_attack;
        uint32 krennansay;
        bool AfterJump;

  };

  CreatureAI* GetAI(Creature* creature) const
  {
    return new npc_mineur_gobAI(creature);
  }

};

#define FLASH_EFFECT 70649

class spell_68281 : public SpellScriptLoader
{
public:
    spell_68281() : SpellScriptLoader("spell_68281") { }

    class  spell_68281SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_68281SpellScript);

        bool Validate(SpellInfo const * spellEntry)
        {
            st = false;
            st2 = false;
            return true;
        }

        bool Load()
        {
            return true;
        }

        void HandleBeforeHit()
        {
            if (st2)
                return;

            if (GetCastItem())
                if (Unit* caster = GetCastItem()->GetOwner())
                    caster->CastSpell(caster, FLASH_EFFECT, true);
            st2 = true;
        }

        void HandleAfterHit()
        {
            if (st)
                return;
            st = true;
        }

        void Unload()
        {
            if (GetCastItem())
                if (Unit* caster = GetCastItem()->GetOwner())
                    caster->RemoveAura(FLASH_EFFECT);
        }

    private :
        bool st, st2;

        void Register()
        {
            BeforeHit += SpellHitFn(spell_68281SpellScript::HandleBeforeHit);
            AfterHit += SpellHitFn(spell_68281SpellScript::HandleAfterHit);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_68281SpellScript();
    }
};

class npc_singe_bombe : public CreatureScript
{
public:
    npc_singe_bombe() : CreatureScript("npc_singe_bombe") { }

    struct npc_singe_bombeAI : public ScriptedAI
    {
        npc_singe_bombeAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            mui_rocket = 45000;
            canCastRocket = true;
            isActive = true;
            isRandomMoving = false;
        }

        void DoAction(const int32 param)
        {
            isActive = false;
            canCastRocket = false;
            me->CastSpell(me, 67919, true);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == 67917)
            {
                isActive = false;
                canCastRocket = false;
                if (me->IsNonMeleeSpellCasted(true))
                    me->InterruptNonMeleeSpells(true);
                if (Creature *t = me->SummonCreature(me->GetEntry(), me->GetPositionX(), me->GetPositionY(),  me->GetPositionZ(),
                                                     me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
                {
                    t->AI()->DoAction(0);
                    t->DespawnOrUnsummon(3000);
                }
                me->DespawnOrUnsummon();
            }
        }

        void JustReachedHome()
        {

        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!who->ToPlayer())
                return ;
            if (!me->IsWithinDistInMap(who, 20.0f))
                return ;
            if (canCastRocket)
                me->CastSpell(me, 8858, true);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!isActive)
                return;
            if (mui_rocket <= diff)
            {
                canCastRocket = true;
                mui_rocket = urand(30000, 45000);
                if (!isRandomMoving)
                {
                    me->GetMotionMaster()->MoveRandom(10.0f);
                    isRandomMoving = true;
                }
            }
            else
                mui_rocket -= diff;
        }

    private:
        uint32 mui_rocket;
        bool canCastRocket;
        bool isActive;
        bool isRandomMoving;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_singe_bombeAI (creature);
    }
};

class spell_weed_whacker : public SpellScriptLoader
{
public:
    spell_weed_whacker() : SpellScriptLoader("spell_weed_whacker") { }

    class spell_weed_whacker_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_weed_whacker_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (GetCastItem())
                if (Unit* caster = GetCastItem()->GetOwner())
                    if (caster->GetTypeId() == TYPEID_PLAYER)
                    {
                        Player *player = caster->ToPlayer();
                        if (player->HasAura(68212))
                            player->RemoveAura(68212);
                        else if (player->GetQuestStatus(14236) == QUEST_STATUS_INCOMPLETE)
                            player->CastSpell(player, 68212, true);
                    }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_weed_whacker_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_weed_whacker_SpellScript();
    }
};

class npc_lianne_gobelin : public CreatureScript
{
public:
    npc_lianne_gobelin() : CreatureScript("npc_lianne_gobelin") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lianne_gobelinAI(creature);
    }

    struct npc_lianne_gobelinAI : public ScriptedAI
    {
        npc_lianne_gobelinAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
            if (me->GetVehicleKit())
                if (Creature *c = me->FindNearestCreature(36042, 10))
                    c->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, me, false);
        }

        void JustDied(Unit* /*killer*/)
        {
            if (Creature *c = me->FindNearestCreature(36042, 100))
                c->ToCreature()->AI()->Talk(irand(0, 7));
        }

        void UpdateAI(const uint32 diff)
        {
        }
    };
};

class npc_killag_sangrecroc : public CreatureScript
{
public:
    npc_killag_sangrecroc() : CreatureScript("npc_killag_sangrecroc") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest *_Quest)
    {
        if (_Quest->GetQuestId() == 14240)
        {
            if (Creature *t = player->SummonCreature(36585, player->GetPositionX(), player->GetPositionY(),  player->GetPositionZ(),
                                                     player->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 300*IN_MILLISECONDS))
            {
                player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, t, false);
                CAST_AI(npc_escortAI, (t->AI()))->Start(false, true, player->GetGUID(), _Quest);
            }
        }
        if (_Quest->GetQuestId() == 14238)
        {
            player->RemoveAura(68338);
            player->RemoveAura(69303);
            for (Unit::ControlList::iterator itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
                if ((*itr)->GetTypeId() == TYPEID_UNIT && (*itr)->GetEntry() == 36100)
                    (*itr)->ToCreature()->DespawnOrUnsummon();
        }
        return true;
    }

    bool OnQuestReward(Player* player, Creature* creature, const Quest *_Quest, uint32 )
    {
        if (_Quest->GetQuestId() == 14238)
        {
            player->RemoveAura(68338);
            player->RemoveAura(69303);
            for (Unit::ControlList::iterator itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
                if ((*itr)->GetTypeId() == TYPEID_UNIT && (*itr)->GetEntry() == 36100)
                    (*itr)->ToCreature()->DespawnOrUnsummon();
        }
        return true;
    }
};

class npc_pant_gob : public CreatureScript
{
public:
    npc_pant_gob() : CreatureScript("npc_pant_gob") { }

    struct npc_pant_gobAI : public npc_escortAI
    {
        npc_pant_gobAI(Creature* creature) : npc_escortAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
                Start(false, true, who->GetGUID());
        }

        void WaypointReached(uint32 i)
        {
            SetRun(true);
            switch(i)
            {
            case 17:
                me->DespawnOrUnsummon();
                break;
            default:
                break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_pant_gobAI (creature);
    }
};

class npc_gyrocoptere_quest_giver : public CreatureScript
{
public:
    npc_gyrocoptere_quest_giver() : CreatureScript("npc_gyrocoptere_quest_giver") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest *_Quest)
    {
        if (_Quest->GetQuestId() == 14242)
        {
            if (Creature *t = player->SummonCreature(39074, creature->GetPositionX(), creature->GetPositionY(),  creature->GetPositionZ(),
                                                     creature->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 300*IN_MILLISECONDS))
            {
                player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, t, false);
                CAST_AI(npc_escortAI, (t->AI()))->Start(false, true, player->GetGUID(), _Quest);
                t->AI()->Talk(SAY_GYRO, player->GetGUID());
            }
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(14242) != QUEST_STATUS_NONE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Would you take a gyrocopter?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        else if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

};

class npc_girocoptere : public CreatureScript
{
public:
    npc_girocoptere() : CreatureScript("npc_girocoptere") { }

    struct npc_girocoptereAI : public npc_escortAI
    {
        npc_girocoptereAI(Creature* creature) : npc_escortAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            _checkQuest = 1000;
            isBoarded = false;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            me->SetCanFly(true);
            me->SetSpeed(MOVE_FLIGHT, 3.0f);
        }

        void WaypointReached(uint32 i)
        {
            me->SetCanFly(true);
            switch(i)
            {
            case 19:
                me->GetVehicleKit()->RemoveAllPassengers();
                me->DespawnOrUnsummon();
                break;
            default:
                break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {

        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
        }

        void UpdateEscortAI(const uint32 diff)
        {
        }

    private :
        uint32 _checkQuest;
        bool isBoarded;

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_girocoptereAI (creature);
    }
};

class npc_tornade_gob : public CreatureScript
{
public:
    npc_tornade_gob() : CreatureScript("npc_tornade_gob") { }

    struct npc_tornade_gobAI : public npc_escortAI
    {
        npc_tornade_gobAI(Creature* creature) : npc_escortAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            _checkQuest = 1000;
            _checkDespawn = 1000;
            isBoarded = false;
            isBoarded2 = false;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (apply)
            {
                me->SetCanFly(true);
                me->SetSpeed(MOVE_FLIGHT, 3.0f);
                isBoarded = true;
                me->AddAura(68436, me);
                Start(false, true, who->GetGUID(), NULL, false, true);
            }
            else
                me->RemoveAura(68436);
        }

        void WaypointReached(uint32 i)
        {
            me->SetCanFly(true);
        }

        void JustDied(Unit* /*killer*/)
        {
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
        }

        void UpdateEscortAI(const uint32 diff)
        {
            if (isBoarded)
            {
                if (isBoarded2)
                {
                    if (_checkDespawn <= diff)
                    {
                        me->GetVehicleKit()->RemoveAllPassengers();
                        me->DespawnOrUnsummon();
                        _checkDespawn = 1000;
                    }
                    else
                        _checkDespawn -= diff;
                }
                else
                {
                    if (_checkQuest <= diff)
                    {
                        if (me->GetVehicleKit())
                            if (Unit *u = me->GetVehicleKit()->GetPassenger(0))
                                if (u->GetTypeId() == TYPEID_PLAYER)
                                {
                                    Player *player = u->ToPlayer();
                                    if (player->GetQuestStatus(14243) == QUEST_STATUS_COMPLETE)
                                    {
                                        isBoarded2 = true;
                                        _checkDespawn = 70000;
                                        SetEscortPaused(true);
                                        me->GetMotionMaster()->MovePoint(1, 862.0f, 2778.87f, 114.0f);
                                    }
                                }
                        _checkQuest = 1000;
                    }
                    else
                        _checkQuest -= diff;
                }
            }
        }

    private :
        uint32 _checkQuest;
        uint32 _checkDespawn;
        bool isBoarded;
        bool isBoarded2;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tornade_gobAI (creature);
    }
};

class gob_fronde_gobelin : public GameObjectScript
{
public:
    gob_fronde_gobelin() : GameObjectScript("gob_fronde_gobelin") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (player->GetQuestStatus(14244) != QUEST_STATUS_NONE)
        {
            if (Creature *t = player->SummonCreature(36514, go->GetPositionX(), go->GetPositionY(),  go->GetPositionZ() + 7,  go->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 300*IN_MILLISECONDS))
                player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, t, false);
            return true;
        }
        return true;
    }
};


class npc_fusee_gob : public CreatureScript
{
public:
    npc_fusee_gob() : CreatureScript("npc_fusee_gob") { }

    struct npc_fusee_gobAI : public npc_escortAI
    {
        npc_fusee_gobAI(Creature* creature) : npc_escortAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            me->SetCanFly(true);
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (apply)
                Start(false, true, who->GetGUID());
            me->SetCanFly(true);
            me->SetSpeed(MOVE_FLIGHT, 5.0f);
        }

        void WaypointReached(uint32 i)
        {
            if (i == 3)
            {
                me->CastSpell(me, 66127, true);
                me->GetVehicleKit()->RemoveAllPassengers();
                me->DespawnOrUnsummon();
            }
        }

        void JustDied(Unit* /*killer*/)
        {
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_fusee_gobAI (creature);
    }
};


class gob_dyn_gobelin : public GameObjectScript
{
public:
    gob_dyn_gobelin() : GameObjectScript("gob_dyn_gobelin") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (player->GetQuestStatus(14245) != QUEST_STATUS_NONE)
        {
            player->CastSpell(player, 68935, true);
           if (Creature *t = player->SummonCreature(9100000, go->GetPositionX(), go->GetPositionY(),  go->GetPositionZ() + 2,  go->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30*IN_MILLISECONDS))
            {
                t->SetPhaseMask(4, true);
                t->CastSpell(t, 71093, true);
                t->CastSpell(t, 71095, true);
                t->CastSpell(t, 71096, true);
                t->CastSpell(t, 71097, true);
            }
            return true;
        }
        return true;
    }
};

class npc_phaseswift : public CreatureScript
{
public:
    npc_phaseswift() : CreatureScript("npc_phaseswift") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_phaseswiftAI(creature);
    }

    struct npc_phaseswiftAI : public ScriptedAI
    {
        npc_phaseswiftAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset ()
        {
            mui_talk = 5000;
            cnt = 6;
        }

        void DoAction(const int32 param)
        {
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
        }

        void JustReachedHome()
        {

        }

        void WaypointReached(uint32)
        {

        }


        void MovementInform(uint32 /*type*/, uint32 id)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            if (mui_talk <= diff)
            {
                if (Unit *p = me->ToTempSummon()->GetSummoner())
                    if (p->GetTypeId() == TYPEID_PLAYER)
                    {
                        std::set<uint32> terrainswap;
                        std::set<uint32> phaseId;
                        terrainswap.insert(661);
                        phaseId.insert(180);
                        p->ToPlayer()->CastSpell(p->ToPlayer(), 68750, true);
                        p->ToPlayer()->KilledMonsterCredit(38024, 1);
                        p->ToPlayer()->GetSession()->SendSetPhaseShift(phaseId, terrainswap);
                    }
                me->DespawnOrUnsummon();
                mui_talk = 6000;
            }
            else
                mui_talk -= diff;
        }

    private :
        uint32 mui_talk;
        int cnt;
    };
};

class npc_poule : public CreatureScript
{
public:
    npc_poule() : CreatureScript("npc_poule") { }

    struct npc_pouleAI : public ScriptedAI
    {
        npc_pouleAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->GetMotionMaster()->MoveRandom(10.0f);
        }

        void DoAction(const int32 param)
        {
            me->CastSpell(me, 67919, true);
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == 67917)
            {
                if (me->IsNonMeleeSpellCasted(true))
                    me->InterruptNonMeleeSpells(true);
                if (Creature *t = me->SummonCreature(me->GetEntry(), me->GetPositionX(), me->GetPositionY(),  me->GetPositionZ(),
                                                     me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
                {
                    t->AI()->DoAction(0);
                    t->DespawnOrUnsummon(3000);
                }
                me->DespawnOrUnsummon();
            }
        }

        void UpdateAI(const uint32 diff)
        {
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_pouleAI (creature);
    }
};

#define GO_PIEGE 201972

class spell_egg_gob : public SpellScriptLoader
{
public:
    spell_egg_gob() : SpellScriptLoader("spell_egg_gob") { }

    class spell_egg_gobSpellScript : public SpellScript
    {
        PrepareSpellScript(spell_egg_gobSpellScript);

        bool Validate(SpellInfo const * spellEntry)
        {
            return true;
        }


        bool Load()
        {
            if (Unit* caster = GetCastItem()->GetOwner())
            {
                if (GameObject* go = caster->FindNearestGameObject(GO_PIEGE, 5))
                {
                    if (Creature *c = go->FindNearestCreature(38187, 10))
                        c->AI()->DoAction(1);
                }
            }
            return true;
        }


        void HandleActivateGameobject(SpellEffIndex effIndex)
        {

        }


        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_egg_gobSpellScript::HandleActivateGameobject,EFFECT_0,SPELL_EFFECT_ACTIVATE_OBJECT);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_egg_gobSpellScript();
    }
};

class npc_raptore_gob : public CreatureScript
{
public:
    npc_raptore_gob() : CreatureScript("npc_raptore_gob") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_raptore_gobAI(creature);
    }

    struct npc_raptore_gobAI : public ScriptedAI
    {
        npc_raptore_gobAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset ()
        {
            start = true;
            me->GetMotionMaster()->MoveRandom(20);
        }

        void MovementInform(uint32 /*type*/, uint32 id)
        {
            if (id == 42 && !start)
            {
                if (Player *player = me->FindNearestPlayer(10))
                    player->SummonGameObject(201974, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, 0, 0, 0, 0, 10*IN_MILLISECONDS);
                me->Kill(me);
            }
        }

        void DoAction(const int32 param)
        {
            if (param == 1)
            {
                if (go = me->FindNearestGameObject(GO_PIEGE, 25))
                {
                    start = false;
                    me->CastSpell(me, 66726, true);
                    me->GetMotionMaster()->MovePoint(42, go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            DoMeleeAttackIfReady();
        }

    private :
        bool start;
        GameObject* go;

    };
};

class SparkSearcher
{
public:
    SparkSearcher(Creature const* source, float range) : _source(source), _range(range) {}

    bool operator()(Unit* unit)
    {
        if (!unit->isAlive())
            return false;

        switch (unit->GetEntry())
        {
            case 38318:
                break;
            default:
                return false;
        }

        if (!unit->IsWithinDist(_source, _range, false))
            return false;

        return true;
    }

private:
    Creature const* _source;
    float _range;
};

class gob_spark_gobelin : public GameObjectScript
{
public:
    gob_spark_gobelin() : GameObjectScript("gob_spark_gobelin") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (player->GetQuestStatus(24817) == QUEST_STATUS_INCOMPLETE)
        {
            if (Creature *t = player->SummonCreature(38318, go->GetPositionX(), go->GetPositionY(),  go->GetPositionZ() + 2,  go->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30*IN_MILLISECONDS))
            {
                std::list<Creature*> temp;
                SparkSearcher check(t, 100.0f);
                MoPCore::CreatureListSearcher<SparkSearcher> searcher(t, temp, check);
                t->VisitNearbyGridObject(100.0f, searcher);
                for (std::list<Creature*>::iterator itr = temp.begin(); itr != temp.end(); ++itr)
                    if ((*itr)->ToTempSummon())
                        if ((*itr)->ToTempSummon()->GetSummoner())
                            if ((*itr)->ToTempSummon()->GetSummoner()->GetTypeId() == TYPEID_PLAYER)
                                if ((*itr)->ToTempSummon()->GetSummoner()->GetGUID() == player->GetGUID() && (*itr)->GetGUID() != t->GetGUID())
                                    (*itr)->DespawnOrUnsummon();
                player->EnterVehicle(t);
            }
            return true;
        }
        return true;
    }
};

class npc_young_naga_gob : public CreatureScript
{
public:
    npc_young_naga_gob() : CreatureScript("npc_young_naga_gob") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_young_naga_gobAI(creature);
    }

    struct npc_young_naga_gobAI : public ScriptedAI
    {
        npc_young_naga_gobAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset ()
        {
            ui_findPlayer = 1000;
            me->GetMotionMaster()->MoveRandom(5);
        }

        void UpdateAI(const uint32 diff)
        {
            if (ui_findPlayer <= diff)
            {
                if (Player *player = me->FindNearestPlayer(10))
                {
                    if (player->HasAura(71914) && player->GetQuestStatus(24864) == QUEST_STATUS_INCOMPLETE)
                    {
                        if (Creature *naga = player->SummonCreature(44589, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 40*IN_MILLISECONDS))
                        {
                            naga->CastSpell(naga, 71917, true);
                            naga->GetMotionMaster()->MoveFollow(player, urand (urand(urand(1, 2), urand(3, 4)), urand(5, 6)),  urand(urand(urand(1, 2), urand(3, 4)), urand(5, 6)));
                            player->KilledMonsterCredit(38413, 1);
                        }
                        me->DespawnOrUnsummon();
                    }
                }
                ui_findPlayer = 1000;
            }
            else
                ui_findPlayer -= diff;

            DoMeleeAttackIfReady();
        }

    private :
        uint32 ui_findPlayer;

    };
};

#define SPELL_CRACK_INVOK 72058

// (Chef des nagas)
#define NAGA_SAY_A "WHO OSE?"
#define NAGA_SAY_B "Small goblins? I remember the creation of your race."
#define NAGA_SAY_C "Your threats do not impress me, nor the naga. Get ready to disappear from this reality."
#define NAGA_SAY_D  "Now, young $ N, you will die!"
//					(Crack)
//-Quand on rend la quete d'avant
#define QUEST_RENDER_CRACK "I want to let these little demons, $ N. Nagas will never attack until we have grandchildren."

//-Quand on commence la quete
#define QUEST_RESET_CRACK "You are $ gpret, ready to force their leader to get $ gmy friend: my friend; ?"

//Quand on se rend syur place
#define CRACK_PROVOC "Come on, Chief naga, come out of your hiding and surrendered for $ N and Bilgewater Cartel!"
#define CRACK_EVADE "Hula treasure, it smells bad. I am size!"

#define NPC_CRACK 39198

class YoungNagaSearcher
{
public:
    YoungNagaSearcher(Creature const* source, float range) : _source(source), _range(range) {}

    bool operator()(Unit* unit)
    {
        if (!unit->isAlive())
            return false;

        switch (unit->GetEntry())
        {
            case 44580:
                break;
            case 44579:
                break;
            case 44578:
                break;
            case 38412:
                break;
            default:
                return false;
        }

        if (!unit->IsWithinDist(_source, _range, false))
            return false;

        return true;
    }

private:
    Creature const* _source;
    float _range;
};

class npc_megs_isle_gob : public CreatureScript
{
public:
    npc_megs_isle_gob() : CreatureScript("npc_megs_isle_gob") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest *_Quest)
    {
        if (_Quest->GetQuestId() == 24868)
            player->SummonCreature(NPC_CRACK, player->GetPositionX(), player->GetPositionY(),  player->GetPositionZ(),  player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
        else if (_Quest->GetQuestId() == 24897)
            player->CastSpell(player, 68481, true);
        return true;
    }

    bool OnQuestReward(Player* player, Creature* creature, const Quest *_Quest, uint32 )
    {
        if (_Quest->GetQuestId() == 24864)
        {
            std::list<Creature*> temp;
            YoungNagaSearcher check(creature, 900.0f);
            MoPCore::CreatureListSearcher<YoungNagaSearcher> searcher(creature, temp, check);
            creature->VisitNearbyGridObject(900.0f, searcher);
            for (std::list<Creature*>::iterator itr = temp.begin(); itr != temp.end(); ++itr)
                if ((*itr)->ToTempSummon())
                    if ((*itr)->ToTempSummon()->GetSummoner())
                        if ((*itr)->ToTempSummon()->GetSummoner()->GetTypeId() == TYPEID_PLAYER)
                            if ((*itr)->ToTempSummon()->GetSummoner()->GetGUID() == player->GetGUID())
                                (*itr)->DespawnOrUnsummon();
        }
        return true;
    }
};


class npc_crack_isle_gob : public CreatureScript
{
public:
  npc_crack_isle_gob() : CreatureScript("npc_crack_isle_gob") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_crack_isle_gobAI(creature);
    }

    struct npc_crack_isle_gobAI : public ScriptedAI
    {
        npc_crack_isle_gobAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset ()
        {
            playerGUID = 0;
            if (Unit *p = me->ToTempSummon()->GetSummoner())
                if (p->GetTypeId() == TYPEID_PLAYER)
                    playerGUID = p->GetGUID();
            if (playerGUID == 0)
            {
                me->DespawnOrUnsummon();
                return;
            }
            if (Player *player =  Unit::GetPlayer(*me, playerGUID))
            {
                me->GetMotionMaster()->MoveFollow(player, 1.0f, 1.0f);
                me->MonsterSay(QUEST_RESET_CRACK, LANG_UNIVERSAL, player->GetGUID());
            }
            else
                me->DespawnOrUnsummon();
            ui_findPlayer = 1000;
            start = false;
            event = false;
            combats = false;
            mui_event = 2000;
            event_p = 0;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!start)
            {
                if (ui_findPlayer <= diff)
                {
                    if (Creature *zone = me->FindNearestCreature(38450, 3))
                    {
                        if (Player *player =  Unit::GetPlayer(*me, playerGUID))
                        {
                            me->MonsterSay(CRACK_PROVOC, LANG_UNIVERSAL, player->GetGUID());
                            if (naga = player->SummonCreature(38448, zone->GetPositionX(), zone->GetPositionY(), zone->GetPositionZ() + 2, zone->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60*IN_MILLISECONDS))
                                naga->setFaction(35);
                        }
                        zone->DespawnOrUnsummon();
                        start = true;
                        event = true;
                    }
                    ui_findPlayer = 1000;
                }
                else
                    ui_findPlayer -= diff;
            }
            else if (event)
            {
                if (mui_event <= diff)
                {
                    mui_event  = 4000;
                    switch (event_p)
                    {
                    case 0 :
                        if (Player *player =  Unit::GetPlayer(*me, playerGUID))
                            naga->MonsterYell(NAGA_SAY_A, LANG_UNIVERSAL, player->GetGUID());
                        break;
                    case 1 :
                        if (Player *player =  Unit::GetPlayer(*me, playerGUID))
                            naga->MonsterYell(NAGA_SAY_B, LANG_UNIVERSAL, player->GetGUID());
                        break;
                    case 2 :
                        if (Player *player =  Unit::GetPlayer(*me, playerGUID))
                            naga->MonsterYell(NAGA_SAY_C, LANG_UNIVERSAL, player->GetGUID());
                        break;
                    case 3 :
                        if (Player *player =  Unit::GetPlayer(*me, playerGUID))
                            naga->MonsterYell(NAGA_SAY_D, LANG_UNIVERSAL, player->GetGUID());
                        break;
                    default :
                        if (Player *player =  Unit::GetPlayer(*me, playerGUID))
                            me->MonsterYell(CRACK_EVADE, LANG_UNIVERSAL, player->GetGUID());
                        mui_event = 2000;
                        event = false;
                        combats = true;
                        break;
                    }
                    event_p++;
                }
                else mui_event -= diff;
            }
            else if (combats)
            {
                if (mui_event <= diff)
                {
                    combats = false;
                    naga->setFaction(14);
                    mui_event  = 4000;
                    me->GetMotionMaster()->MovePoint(1, me->GetHomePosition());
                }
                else
                    mui_event -= diff;
            }
            else
            {
                if (mui_event <= diff)
                    me->DespawnOrUnsummon();
                else
                    mui_event -= diff;
            }
        }

    private :
        uint32 ui_findPlayer;
        Creature *naga;
        uint64 playerGUID;
        bool start, event, combats;
        uint32 mui_event;
        int event_p;
    };
};


class npc_canon_gob : public CreatureScript
{
public:
    npc_canon_gob() : CreatureScript("npc_canon_gob") { }

    struct npc_canon_gobAI : public npc_escortAI
    {
        npc_canon_gobAI(Creature* creature) : npc_escortAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            _checkQuest = 1000;
            isBoarded = false;
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage)
        {
            damage = 0;
        }
        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
        }

        void WaypointReached(uint32 i)
        {
        }

        void JustDied(Unit* /*killer*/)
        {
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
        }

        void UpdateEscortAI(const uint32 diff)
        {
            if (_checkQuest <= diff)
            {
                if (!isBoarded)
                {
                    if (Creature *player = me->FindNearestCreature(38745,1))
                    {
                        player->EnterVehicle(me);
                        isBoarded = true;
                    }
                    else if (Creature *player = me->FindNearestCreature(38409,1))
                    {
                        player->EnterVehicle(me);
                        isBoarded = true;
                    }
                }
                else
                {
                    if (Creature *oomlot = me->FindNearestCreature(38531,80))
                    {
                        me->CastSpell(oomlot, 72206, true);
                        isBoarded = true;
                    }
                }
                _checkQuest = urand(7000, 13333);
            }
            else _checkQuest -= diff;
        }

    private :
        uint32 _checkQuest;
        bool isBoarded;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_canon_gobAI (creature);
    }
};

class npc_oom_isle_gob : public CreatureScript
{
public:
    npc_oom_isle_gob() : CreatureScript("npc_oom_isle_gob") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_oom_isle_gobAI(creature);
    }

    struct npc_oom_isle_gobAI : public ScriptedAI
    {
        npc_oom_isle_gobAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset ()
        {
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == 72207)
                caster->Kill(me);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return ;
            DoMeleeAttackIfReady();
        }

    };
};

class npc_ceint : public CreatureScript
{
public:
    npc_ceint() : CreatureScript("npc_ceint") { }

    bool OnQuestReward(Player* player, Creature* creature, const Quest *_Quest, uint32 )
    {
        if (_Quest->GetQuestId() == 24942)
        {
            if (Creature *c = player->FindNearestCreature(38802, 10))
            {
                player->RemoveAura(72889);
                player->RemoveAura(72885);
                c->Kill(c);
            }
        }
        return true;
    }
};

class spell_boot_gob : public SpellScriptLoader
{
public:
    spell_boot_gob() : SpellScriptLoader("spell_boot_gob") { }


    class spell_boot_gobSpellScript : public SpellScript
    {
        PrepareSpellScript(spell_boot_gobSpellScript);

        bool Validate(SpellInfo const * spellEntry)
        {
            st = false;
            return true;
        }


        bool Load()
        {
            return true;
        }

        void HandleOnHit()
        {
            if (Unit* caster = GetCastItem()->GetOwner())
                if (caster->GetTypeId() == TYPEID_PLAYER)
                    caster->ToPlayer()->GetMotionMaster()->MoveJump(1480.31f, 1269.97f, 110.0f, 50.0f, 50.0f, 300.0f);
        }

    private :
        bool st;

        void Register()
        {
            OnHit += SpellHitFn(spell_boot_gobSpellScript::HandleOnHit);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_boot_gobSpellScript();
    }
};

class npc_izzy_airplane : public CreatureScript
{
public:
    npc_izzy_airplane() : CreatureScript("npc_izzy_airplane") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest *_Quest)
    {
        if (_Quest->GetQuestId() == 25023)
        {
            if (Creature *airplane = player->SummonCreature(38929, creature->GetPositionX(), creature->GetPositionY(),  creature->GetPositionZ(),  creature->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            {
                player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 2, airplane, false);
                if (Creature *t = player->SummonCreature(38869, creature->GetPositionX(),  creature->GetPositionY(),  creature->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 750000))
                    t->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, airplane, false);
            }
        }
        return true;
    }
};

class npc_avion_gob : public CreatureScript
{
public:
    npc_avion_gob() : CreatureScript("npc_avion_gob") { }

    struct npc_avion_gobAI : public npc_escortAI
    {
        npc_avion_gobAI(Creature* creature) : npc_escortAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            me->SetCanFly(true);
            me->SetSpeed(MOVE_FLIGHT, 6.0f);
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
                if (apply)
                    Start(false, true, who->GetGUID());
        }

        void WaypointReached(uint32 i)
        {
            me->SetCanFly(true);
            me->SetSpeed(MOVE_FLIGHT, 6.0f);
            Creature *chipie = me->FindNearestCreature(38869, 5, true);
            if (!chipie)
                return;
            switch(i)
            {
                case 1:
                    if (Player *player = me->FindNearestPlayer(10))
                        chipie->MonsterSay("Hang on $ N! We need makes a clearance here, NOW!", LANG_UNIVERSAL, player->GetGUID());
                    break;
                case 4 :
                    chipie->MonsterSay("OH MY GOD! BEHIND YOU!", LANG_UNIVERSAL, 0);
                    break;
                case 8:
                    chipie->MonsterSay("WE BURN !!", LANG_UNIVERSAL, 0);
                    break;
                case 17 :
                    Creature *chipie;
                    Player *player;
                    if (Unit *unit = me->GetVehicleKit()->GetPassenger(0))
                        chipie = unit->ToCreature();
                    if (Unit *unit = me->GetVehicleKit()->GetPassenger(1))
                        player = unit->ToPlayer();
                    if (chipie && player)
                    {
                        me->GetVehicleKit()->RemoveAllPassengers();
                        player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 3, me, false);
                        chipie->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, me, false);
                        me->RemoveAura(73149);
                    }
                    chipie->MonsterSay("Climb in the back. We're going. I know where are our old friends the orcs", LANG_UNIVERSAL, 0);
                    break;
                case 26 :
                    me->GetVehicleKit()->RemoveAllPassengers();
                    if (Creature *chip = me->FindNearestCreature(38869, 5, true))
                     chip->DespawnOrUnsummon();
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_avion_gobAI (creature);
    }
};


class npc_tremblement_volcano : public CreatureScript
{
public:
    npc_tremblement_volcano() : CreatureScript("npc_tremblement_volcano") { }

    struct npc_tremblement_volcanoAI : public ScriptedAI
    {
        npc_tremblement_volcanoAI(Creature* creature) : ScriptedAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            mui_soufle = 2000;
        }

        void JustDied(Unit* killer)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            if (mui_soufle <= diff)
            {
                me->CastSpell(me, 69235, true);
                mui_soufle = 3000;
            }
            else
                mui_soufle -= diff;
        }

    private :
        uint32 mui_soufle;

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tremblement_volcanoAI (creature);
    }
};

class npc_meteor2_gob : public CreatureScript
{
public:
    npc_meteor2_gob() : CreatureScript("npc_meteor2_gob") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_meteor2_gobAI(creature);
    }

    struct npc_meteor2_gobAI : public ScriptedAI
    {
        npc_meteor2_gobAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset ()
        {
            _a = urand(15000, 20200);
            _b = 600000;
            _c = 600000;
        }

        void JustReachedHome()
        {

        }

        void UpdateAI(const uint32 diff)
        {
            if (_a <= diff)
            {
                me->CastSpell(me, 93668, true);
                _a = urand(15000, 20200);
                _b = 800;
            }
            else _a -= diff;
            if (_b <= diff)
            {
                me->CastSpell(me, 87701, true);
                _b = 600000;
                _c = 500;
            }
            else _b -= diff;
            if (_c <= diff)
            {
                me->CastSpell(me, 69235, true);
                _c = 600000;
            }
            else _c -= diff;
        }

    private :
        uint32 _a, _b, _c;
    };
};

class npc_explosion_volcano : public CreatureScript
{
public:
    npc_explosion_volcano() : CreatureScript("npc_explosion_volcano") { }

    struct npc_explosion_volcanoAI : public ScriptedAI
    {
        npc_explosion_volcanoAI(Creature* creature) : ScriptedAI(creature) {}

        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
            mui_soufle = urand(1100, 2000);
        }

        void JustDied(Unit* killer)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            if (mui_soufle <= diff)
            {
                me->CastSpell(me, 73193, true);
                mui_soufle = urand(4000, 5200);
            }
            else
                mui_soufle -= diff;
        }

    private :
        uint32 mui_soufle;

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_explosion_volcanoAI (creature);
    }
};



class npc_killag_2 : public CreatureScript
{
public:
    npc_killag_2() : CreatureScript("npc_killag_2") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest *_Quest)
    {
        if (_Quest->GetQuestId() == 25100)
        {
            if (Creature *pant = player->SummonCreature(39152, player->GetPositionX(), player->GetPositionY(),
                                                        player->GetPositionZ(),  player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
                player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, pant, false);
        }
        return true;
    }
};

class npc_Chariot : public CreatureScript
{
public:
    npc_Chariot() : CreatureScript("npc_Chariot") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest *_Quest)
    {
        if (_Quest->GetQuestId() == 25184)
        {
            if (Creature *chariot = player->SummonCreature(39329, creature->GetPositionX(), creature->GetPositionY(),  creature->GetPositionZ(),  creature->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            {
                int cnt = 1;
				chariot->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING); //possibly wrong??
                player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, chariot, false);
                for (Unit::ControlList::iterator itr = player->m_Controlled.begin(); itr != player->m_Controlled.end(); ++itr)
                    if ((*itr)->GetTypeId() == TYPEID_UNIT && ((*itr)->GetEntry() == 34957 || (*itr)->GetEntry() == 39199 || (*itr)->GetEntry() == 34959 || (*itr)->GetEntry() == 39205))
                    {
                        cnt++;
                        (*itr)->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, cnt, chariot, false);
                    }
                CAST_AI(npc_escortAI, (chariot->AI()))->Start(false, true, player->GetGUID(), _Quest);
            }
        }
        return true;
    }
};

class npc_Chariot2 : public CreatureScript
{
public:
    npc_Chariot2() : CreatureScript("npc_Chariot2") { }

    struct npc_Chariot2AI : public npc_escortAI
    {
        npc_Chariot2AI(Creature* creature) : npc_escortAI(creature) {}

        uint32 krennansay;
        bool AfterJump;
        void AttackStart(Unit* /*who*/) {}
        void EnterCombat(Unit* /*who*/) {}
        void EnterEvadeMode() {}

        void Reset()
        {
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply)
        {
            if (who->GetTypeId() == TYPEID_UNIT)
                if (!apply)
                    who->ToCreature()->DespawnOrUnsummon();
        }

        void WaypointReached(uint32 i)
        {
            switch(i)
            {
            case 12:
                me->GetVehicleKit()->RemoveAllPassengers();
                if (Creature *t = me->SummonCreature(41505, me->GetPositionX(), me->GetPositionY(),  me->GetPositionZ(),  me->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 10*IN_MILLISECONDS))
                {
                    t->CastSpell(t, 93569, true);
                    t->CastSpell(t, 71094, true);
                }
                me->DespawnOrUnsummon();
                if (Unit *player = me->ToTempSummon()->GetSummoner())
                    player->GetMotionMaster()->MoveJump(2354.36f, 1943.21f, 24.0f, 20.0f, 20.0f);
                break;
            default:
                break;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
        }

        void OnCharmed(bool /*apply*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            npc_escortAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_Chariot2AI (creature);
    }
};

class gob_red_but : public GameObjectScript
{
public:
    gob_red_but() : GameObjectScript("gob_red_but") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (player->GetQuestStatus(25207) == QUEST_STATUS_INCOMPLETE)
        {
            if (player->GetPhaseMask() == 32768)
                return false;
            player->CastSpell(player, 69077, true);
            player->AddAura(90418, player);
            player->KilledMonsterCredit(39393, 0);
            if (Creature *t = player->SummonCreature(41505, 2477.0f, 2082.0f,  14.0f,  go->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 10*IN_MILLISECONDS))
            {
                t->SetPhaseMask(32768, true);
                t->CastSpell(t, 93569, true);
                t->CastSpell(t, 71094, true);
            }
            if (Creature *t = player->SummonCreature(41505, 2499.28f, 2091.48f,  17.0f,  go->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 10*IN_MILLISECONDS))
            {
                t->SetPhaseMask(32768, true);
                t->CastSpell(t, 93569, true);
                t->CastSpell(t, 71094, true);
            }
            if (Creature *t = player->SummonCreature(41505, 2450.424f, 2068.89f,  28.0f,  go->GetOrientation(), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 10*IN_MILLISECONDS))
            {
                t->SetPhaseMask(32768, true);
                t->SetCanFly(true);
                t->CastSpell(t, 93569, true);
                t->CastSpell(t, 71094, true);
            }
            return true;
        }
        return true;
    }
};

class npc_grilly_2 : public CreatureScript
{
public:
    npc_grilly_2() : CreatureScript("npc_grilly_2") { }

    bool OnQuestAccept(Player* player, Creature* creature, const Quest *_Quest)
    {
        if (_Quest->GetQuestId() == 25213)
        {
            if (Creature *pant = player->SummonCreature(47956, player->GetPositionX(), player->GetPositionY(),
                                                        player->GetPositionZ(),  player->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
                player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, pant, false);
        }
        return true;
    }
};

class npc_Prince : public CreatureScript
{
public:
    npc_Prince() : CreatureScript("npc_Prince") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_PrinceAI(creature);
    }

    struct npc_PrinceAI : public ScriptedAI
    {
        npc_PrinceAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset ()
        {
            eventTalk = 0;
            eventTalk2 = 0;
            mui_talk = 2000;
            mui_talk2 = 2000;
            isEventInProgress = false;
            start = false;
            end = false;
            mui1 = 10000;
            mui2 = 30000;
            mui3 = 50000;
            mui4 = 40000;
            mui5 = 60000;
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (isEventInProgress)
                return;
            if (who->ToPlayer())
                return;
            if (!me->IsWithinDistInMap(who, 5.0f))
                return ;
            if (end)
                return;
            if (who->GetEntry() == 39592)
            {
                isEventInProgress = true;
                me->setFaction(14);
                //DoScriptText(-1039585, me);
            }
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage)
        {
            if (end)
            {
                damage = 0;
                return;
            }
            if (damage >= me->GetHealth())
            {
                damage = 0;
                me->setFaction(35);
                //DoScriptText(-1039588, me);
                end = true;
                if (Creature *c = me->FindNearestCreature(39592, 30))
                    c->DespawnOrUnsummon();
                if (Player *pl = me->FindNearestPlayer(10))
                    pl->KilledMonsterCredit(39582, 0);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!isEventInProgress)
                return;
            if (end)
            {
                if (mui_talk <= diff)
                {
                    mui_talk = 10000;
                    switch (eventTalk)
                    {
                    case 0 :
                        //		  DoScriptText(-1039590, me);
                        break;
                    case 1 :
                        if (Creature *c = me->FindNearestCreature(39594, 30))
                        {
                            //		      DoScriptText(-1039591, c);
                        }
                        break;
                    case 2 :
                        //		  DoScriptText(-1039589, me);
                        break;
                    case 3 :
                        if (Creature *c = me->FindNearestCreature(39594, 30))
                        {
                            //		      DoScriptText(-1039592, c);
                        }
                        break;
                    case 4 :
                        if (Creature *c = me->FindNearestCreature(39594, 30))
                        {
                            //		      DoScriptText(-1039593, c);
                        }
                        break;
                    case 5 :
                        me->DespawnOrUnsummon();
                        if (Creature *c = me->FindNearestCreature(39594, 30))
                        {
                            c->DespawnOrUnsummon();
                        }
                        break;
                    default :
                        break;
                    }
                    eventTalk++;
                }
                else
                    mui_talk -= diff;
            }
            else
            {
                if (!me->getVictim())
                    return;
                if (mui1 <= diff)
                {
                    switch (urand(0, 4))
                    {
                    case 0 :
                        me->CastSpell(me->getVictim(), 74005, true);
                        //		  DoScriptText(-1039583, me);
                        break;
                    case 1 :
                        me->CastSpell(me->getVictim(), 74000, true);
                        //		  DoScriptText(-1039584, me);
                        break;
                    case 2 :
                        me->CastSpell(me->getVictim(), 74004, true);
                        //		  DoScriptText(-1039586, me);
                        break;
                    case 3 :
                        me->CastSpell(me->getVictim(), 81000, true);
                        //		  DoScriptText(-1039582, me);
                        break;
                    case 4 :
                        me->CastSpell(me->getVictim(), 74003, true);
                        break;
                    default :
                        me->CastSpell(me->getVictim(), 74003, true);
                        break;
                    }
                    mui1 = 5000;
                }
                else
                    mui1 -= diff;
            }
            DoMeleeAttackIfReady();
        }

    private :
        bool isEventInProgress, start, end;
        uint32 mui_talk, mui_talk2, mui1, mui2, mui3, mui4, mui5;
        unsigned int eventTalk, eventTalk2;
        Player *player;
    };
};


class npc_boot : public CreatureScript
{
public:
  npc_boot() : CreatureScript("npc_boot") { }

  bool OnQuestAccept(Player* player, Creature* creature, const Quest *_Quest)
  {
    if (_Quest->GetQuestId() == 25265)
    {
        player->AddAura(72971, player);
        player->CastSpell(player, 67789, true);
        player->AddAura(90418, player);
        player->GetMotionMaster()->MoveJump(2352.31f, 2483.97f, 13.0f, 15.0f, 20.0f, 150.0f);
    }

    if (_Quest->GetQuestId() == 25066)
    {
        if (Creature *t = player->SummonCreature(39074, player->GetPositionX(), player->GetPositionY(),  player->GetPositionZ(),
                                                 player->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3*IN_MILLISECONDS))
        {
            player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, t, false);
            t->SetCanFly(true);
            t->SetSpeed(MOVE_FLIGHT, 6.0f);
        }
    }

    if (_Quest->GetQuestId() == 25251)
    {
        if (Creature *t = player->SummonCreature(39592, player->GetPositionX(), player->GetPositionY(),  player->GetPositionZ(),
                                                 player->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3*IN_MILLISECONDS))
            player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, t, false);
    }

    return true;
  }
};

class BootSearcher
{
public:
    bool operator()(WorldObject* object)
    {
        if (!object)
            return true;
        Unit* unit = object->ToUnit();
        if (!unit || !unit->isAlive() || unit->GetTypeId() == TYPEID_PLAYER)
            return true;

        if (unit->ToCreature())
        {
            switch (unit->ToCreature()->GetEntry())
            {
                case 38753:
                    return false;
                default:
                    break;
            }
        }
        return true;
    }
};

class spell_boot_damage : public SpellScriptLoader
{
public:
    spell_boot_damage() : SpellScriptLoader("spell_boot_damage") { }

    class spell_boot_damage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_boot_damage_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(BootSearcher());
            if (GetCaster())
                if (!GetCaster()->HasAura(72887))
                {
                    GetCaster()->RemoveAura(72885);
                    targets.clear();
                }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_boot_damage_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_boot_damage_SpellScript();
    }
};

// 288 cinematic + spell to org
class npc_sassy_handwrench : public CreatureScript
{
public:
	npc_sassy_handwrench() : CreatureScript("npc_sassy_handwrench") { }

	bool OnGossipHello(Player* player, Creature* creature) override
	{
			// 1440.35 -5014.26 12.2184 1.75891  orgrimmar
			player->TeleportTo(1, 1440.35f, -5014.26f, 12.2184f, 1.75891f);
			// cinematic id is 288
			player->SendMovieStart(288);
		return true;
	}
};


// #########################################################  fight novice vs tiki_target

class npc_novice_darkspear_warrior : public CreatureScript
{
public:
	npc_novice_darkspear_warrior() : CreatureScript("npc_novice_darkspear_warrior") { }

	enum eFightVersusTikiTarget
	{
		NPC_NOVICE_DARKSPEAR_WARRIOR = 38268,
		NPC_TIKI_TARGET = 38038,
	};

	struct npc_novice_darkspear_warriorAI : public ScriptedAI
	{
		npc_novice_darkspear_warriorAI(Creature* creature) : ScriptedAI(creature) { }

		uint32 _timer;

		void Reset()
		{
			_timer = urand(1800, 2200);
		}

		void UpdateAI(uint32 diff)
		{
			if (!UpdateVictim())
			{
				if (Creature* tiki = me->FindNearestCreature(NPC_TIKI_TARGET, 3.0f))
				{
					if (_timer <= diff)
					{
						me->SetFacingTo(me->GetAngle(tiki));
						me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
						_timer = urand(1800, 2200);
					}
					else
						_timer -= diff;
				}
			}
			else
				DoMeleeAttackIfReady();

		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_novice_darkspear_warriorAI(creature);
	}
};

// #########################################################  Quest Class Chain: A Rough Start

enum eQuestChainStart
{
	NPC_DOCILE_ISLAND_BOAR = 38141,
	NPC_WILDMANE_CAT = 38046,
	SPELL_LEAPING_RUSH = 75002,
	SPELL_WILD_POUNCE = 71232,
	SPELL_PERMANENT_FEIGN_DEATH = 29266,
	SPELL_FEIGN_DEATH = 71598,
	SPELL_SWIPE = 31279,
};

class npc_docile_island_boar : public CreatureScript
{
public:
	npc_docile_island_boar() : CreatureScript("npc_docile_island_boar") { }

	bool OnDummyEffect(Unit* caster, uint32 spellId, SpellEffIndex effIndex, Creature* target)
	{
		if (spellId == SPELL_WILD_POUNCE && effIndex == 1)
		{
			CAST_AI(npc_docile_island_boarAI, target->AI())->StartAnim(caster->GetGUID());
			return true;
		}
		return false;
	}


	struct npc_docile_island_boarAI : public ScriptedAI
	{
		npc_docile_island_boarAI(Creature* creature) : ScriptedAI(creature) { }

		uint32 m_timer;
		uint32 m_phase;

		void Reset() override
		{
			m_timer = 0;
			m_phase = 0;
		}

		void StartAnim(uint64 guid)
		{
			m_phase = 5; m_timer = 100;
		}

		void DamageTaken(Unit* attacker, uint32& damage) override
		{
		}

		void SpellHit(Unit* caster, SpellInfo const* spell) override
		{
			if (spell->Id == SPELL_LEAPING_RUSH)
			{
				caster->GetMotionMaster()->MoveIdle();
				m_phase = 2; m_timer = 5000;
			}
		}

		void UpdateAI(uint32 diff) override
		{
			if (m_timer <= diff)
			{
				m_timer = 1000;
				DoWork();
			}
			else
				m_timer -= diff;

			if (!UpdateVictim())
				return;
			else
				DoMeleeAttackIfReady();
		}

		void DoWork()
		{
			switch (m_phase)
			{
			case 0:
				break;
			case 1:
			{
				// me->RemoveAura(SPELL_FEIGN_DEATH);
			}
			case 2:
			{
				UpdateVictim();
				m_phase = 0; m_timer = 0;
				break;
			}
			// here start phase for spell 71232, bunch boar down
			case 5:
				me->GetMotionMaster()->MoveIdle();
				me->HandleEmoteState(EMOTE_STATE_SLEEP);
				m_phase = 6; m_timer = 10000;
				break;
			case 6:
				UpdateVictim();
				m_phase = 0; m_timer = 0;
				break;
			default:
				break;
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_docile_island_boarAI(creature);
	}
};



// #########################################################  Quest Class Chain: The Basics: Hitting Things

enum TrollSpells
{
	// Tiki Target
	SPELL_TIKI_TARGET_VISUAL_1 = 71064,
	SPELL_TIKI_TARGET_VISUAL_2 = 71065,
	SPELL_TIKI_TARGET_VISUAL_3 = 71066,
	SPELL_TIKI_TARGET_VISUAL_DIE = 71240,
};

class npc_tiki_target : public CreatureScript
{
public:
	npc_tiki_target() : CreatureScript("npc_tiki_target") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_tiki_targetAI(creature);
	}

	struct npc_tiki_targetAI : public ScriptedAI
	{
		npc_tiki_targetAI(Creature* creature) : ScriptedAI(creature) { }

		void Reset()
		{
			if (!me->HasAura(SPELL_TIKI_TARGET_VISUAL_1) && !me->HasAura(SPELL_TIKI_TARGET_VISUAL_2) && !me->HasAura(SPELL_TIKI_TARGET_VISUAL_3))
				DoCast(me, RAND(SPELL_TIKI_TARGET_VISUAL_1, SPELL_TIKI_TARGET_VISUAL_2, SPELL_TIKI_TARGET_VISUAL_3));
		}

		void EnterCombat(Unit* /*who*/)
		{
			if (!me->HasAura(SPELL_TIKI_TARGET_VISUAL_1) && !me->HasAura(SPELL_TIKI_TARGET_VISUAL_2) && !me->HasAura(SPELL_TIKI_TARGET_VISUAL_3))
				DoCast(me, RAND(SPELL_TIKI_TARGET_VISUAL_1, SPELL_TIKI_TARGET_VISUAL_2, SPELL_TIKI_TARGET_VISUAL_3));
		}

		void UpdateAI(uint32 /*diff*/)
		{
			if (me->HealthBelowPct(30))
				DoCast(me, SPELL_TIKI_TARGET_VISUAL_DIE);
		}
	};
};

// #########################################################   Quest Class Chain: Proving Pit


enum TrollCreatures
{
	NPC_SPITESCALE_SCOUT = 38142,
	NPC_DARKSPEAR_JAILOR = 39062,
	NPC_CAPTIVE_SPIESCALE_SCOUT = 38142,
	NPC_LEGATI_ROGUE_TRAINER = 38244,
};

Position const TrollwayPos[8] =
{
	// First Darkspear Jailor 
	{ -1159.222f, -5519.436f, 12.128f, 0.000f }, // pos on side
	{ -1152.798f, -5519.026f, 11.984f, 0.000f }, // pos on cage
	{ -1150.308f, -5521.526f, 11.307f, 4.760f }, // pos summit scout
	{ -1146.754f, -5530.905f, 08.106f, 2.982f }, // pos center place
	// Second Darkspear Jailor 
	{ -1137.437f, -5430.574f, 13.640f, 0.000f }, // pos on side
	{ -1136.318f, -5417.105f, 13.270f, 0.000f }, // pos on cage
	{ -1137.858f, -5414.610f, 13.038f, 3.252f }, // pos summit scout
	{ -1148.483f, -5417.083f, 10.598f, 5.067f }, // pos center place
};

class npc_captive_spitescale_scout : public CreatureScript
{
public:
	npc_captive_spitescale_scout() : CreatureScript("npc_captive_spitescale_scout") { }

	enum Events
	{
		EVENT_GO_TO_CENTER = 1,
		EVENT_WAIT_ON_FIGHT = 2,
		EVENT_FIGHT_UNTIL_DEATH = 3,
	};

	struct npc_captive_spitescale_scoutAI : public ScriptedAI
	{
		npc_captive_spitescale_scoutAI(Creature* creature) : ScriptedAI(creature) { }

		uint8 m_ScoutIndex;
		EventMap events;

		void Reset() override
		{
			m_ScoutIndex = 0;
		}

		void StartEvents(uint8 index)
		{
			m_ScoutIndex = index;
			events.ScheduleEvent(EVENT_GO_TO_CENTER, 100);
		}

		void JustDied(Unit* killer) override
		{
			Talk(1);
		}

		void UpdateAI(uint32 diff) override
		{
			events.Update(diff);
			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_GO_TO_CENTER:
				{
					Talk(0);
					me->GetMotionMaster()->MovePoint(7, TrollwayPos[m_ScoutIndex + 3]);
					events.ScheduleEvent(EVENT_WAIT_ON_FIGHT, 6000);
					break;
				}
				case EVENT_WAIT_ON_FIGHT:
				{
					if (me->isInCombat())
					{
						events.ScheduleEvent(EVENT_FIGHT_UNTIL_DEATH, 1000);
						return;
					}
					if (Player* player = me->FindNearestPlayer(25.0f))
					{
						Talk(0);
						me->setFaction(2224);
						me->Attack(player, true);
						player->Attack(me, true);
						events.ScheduleEvent(EVENT_WAIT_ON_FIGHT, 8000);
						return;
					}
					me->DespawnOrUnsummon();
					break;
				}
				case EVENT_FIGHT_UNTIL_DEATH:
				{
					if (me->GetDistance2d(TrollwayPos[m_ScoutIndex + 3].m_positionX, TrollwayPos[m_ScoutIndex + 3].m_positionY) > 20.0f)
						me->DespawnOrUnsummon();

					if (!me->isInCombat())
						me->DespawnOrUnsummon();

					if (Unit* unit = me->getVictim())
						if (Player* player = unit->ToPlayer())
						{
							events.ScheduleEvent(EVENT_FIGHT_UNTIL_DEATH, 1000);
							return;
						}
					break;
				}
				default:
					printf("Event default %u \n", eventId);
					break;
				}
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_captive_spitescale_scoutAI(creature);
	}
};

class npc_darkspear_jailor : public CreatureScript
{
public:
	npc_darkspear_jailor() : CreatureScript("npc_darkspear_jailor"){ }

	enum TrollQuests
	{
		// Proving Pit
		QUEST_PROVING_PIT_ROGU = 24774,
		QUEST_PROVING_PIT_MAGE = 24754,
		QUEST_PROVING_PIT_SHAM = 24762,
		QUEST_PROVING_PIT_HUNT = 24780,
		QUEST_PROVING_PIT_PRIE = 24786,
		QUEST_PROVING_PIT_WARR = 24642,
		QUEST_PROVING_PIT_DRUI = 24768,
		QUEST_PROVING_PIT_WARL = 26276,
	};

	enum Events
	{
		// Darkspear Jailor
		EVENT_MOVE_TO_CAGE_1 = 1,
		EVENT_MOVE_TO_CAGE_2,
		EVENT_OPEN_CAGE,
		EVENT_MOVE_BACK_1,
		EVENT_MOVE_BACK_2,
		EVENT_SUMMON_SPITESCALE_SCOUT,
		EVENT_RESET_POS,
	};

	bool OnGossipHello(Player* player, Creature* creature)
	{
		if (player->GetQuestStatus(QUEST_PROVING_PIT_ROGU) == QUEST_STATUS_INCOMPLETE ||
			player->GetQuestStatus(QUEST_PROVING_PIT_MAGE) == QUEST_STATUS_INCOMPLETE ||
			player->GetQuestStatus(QUEST_PROVING_PIT_SHAM) == QUEST_STATUS_INCOMPLETE ||
			player->GetQuestStatus(QUEST_PROVING_PIT_HUNT) == QUEST_STATUS_INCOMPLETE ||
			player->GetQuestStatus(QUEST_PROVING_PIT_PRIE) == QUEST_STATUS_INCOMPLETE ||
			player->GetQuestStatus(QUEST_PROVING_PIT_WARR) == QUEST_STATUS_INCOMPLETE ||
			player->GetQuestStatus(QUEST_PROVING_PIT_DRUI) == QUEST_STATUS_INCOMPLETE ||
			player->GetQuestStatus(QUEST_PROVING_PIT_WARL) == QUEST_STATUS_INCOMPLETE)
		{
			player->ADD_GOSSIP_ITEM_DB(10974, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
		}
		player->SEND_GOSSIP_MENU(15251, creature->GetGUID());
		return true;
	}

	bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
	{
		printf("OnGossipSelect: action %u \n", action);
		if (action == GOSSIP_ACTION_INFO_DEF + 1)
		{
			player->PlayerTalkClass->ClearMenus();
			player->CLOSE_GOSSIP_MENU();
			player->KilledMonsterCredit(NPC_DARKSPEAR_JAILOR);
			CAST_AI(npc_darkspear_jailorAI, creature->GetAI())->StartEvents();
		}

		return true;
	}

	struct npc_darkspear_jailorAI : public ScriptedAI
	{
		npc_darkspear_jailorAI(Creature* creature) : ScriptedAI(creature) { }

		uint8 m_JailorIndex;
		bool m_activated;
		EventMap events;

		void Reset() override
		{
			me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
			m_JailorIndex = 0;
			m_activated = false;
		}

		void StartEvents()
		{
			if (Creature* npc = me->FindNearestCreature(NPC_LEGATI_ROGUE_TRAINER, 30.0f, true))
				m_JailorIndex = 4;

			if (!m_activated)
			{
				Talk(0);
				me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
				m_activated = true;
				events.ScheduleEvent(EVENT_MOVE_TO_CAGE_1, 100);
			}
		}

		void UpdateAI(uint32 diff) override
		{
			events.Update(diff);
			while (uint32 eventId = events.ExecuteEvent())
			{
				switch (eventId)
				{
				case EVENT_MOVE_TO_CAGE_1:
					me->GetMotionMaster()->MovePoint(20, TrollwayPos[m_JailorIndex + 0]);
					events.ScheduleEvent(EVENT_MOVE_TO_CAGE_2, 4000);
					break;
				case EVENT_MOVE_TO_CAGE_2:
					me->GetMotionMaster()->MovePoint(21, TrollwayPos[m_JailorIndex + 1]);
					events.ScheduleEvent(EVENT_OPEN_CAGE, 6000);
					break;
				case EVENT_OPEN_CAGE:
					if (GameObject* cage = me->FindNearestGameObject(201968, 10.0f))
						cage->UseDoorOrButton();
					events.ScheduleEvent(EVENT_SUMMON_SPITESCALE_SCOUT, 500);
					events.ScheduleEvent(EVENT_MOVE_BACK_1, 2500);
					break;
				case EVENT_MOVE_BACK_1:
					me->GetMotionMaster()->MovePoint(22, TrollwayPos[m_JailorIndex + 0]);
					events.ScheduleEvent(EVENT_MOVE_BACK_2, 6000);
					break;
				case EVENT_MOVE_BACK_2:
					me->GetMotionMaster()->MoveTargetedHome();
					events.ScheduleEvent(EVENT_RESET_POS, 3000);
					break;
				case EVENT_RESET_POS:
					me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
					Reset();
					break;
				case EVENT_SUMMON_SPITESCALE_SCOUT:
					if (Creature* creature = me->SummonCreature(NPC_SPITESCALE_SCOUT, TrollwayPos[m_JailorIndex + 2], TEMPSUMMON_CORPSE_DESPAWN))
					{
						CAST_AI(npc_captive_spitescale_scout::npc_captive_spitescale_scoutAI, creature->AI())->StartEvents(m_JailorIndex);
					}
					break;
				}
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const override
	{
		return new npc_darkspear_jailorAI(creature);
	}
};

// #########################################################  Quest Class Chain: The Arts of a <class>

class npc_wounded_darkspear_watcher : public CreatureScript
{
public:
	npc_wounded_darkspear_watcher() : CreatureScript("npc_wounded_darkspear_watcher"){ }

	enum eQuest
	{
		NPC_WOUNDET_DARKSPEAR_WATCHER = 47057,
		QUEST_THE_ART_OF_A_PRIEST = 24784,
		SPELL_FLASH_HEAL = 2061,
		QUEST_THE_ART_OF_A_DRUID = 24766,
		SPELL_REJUVENATION = 774,

	};

	struct npc_wounded_darkspear_watcherAI : public ScriptedAI
	{
		npc_wounded_darkspear_watcherAI(Creature* creature) : ScriptedAI(creature) { }

		void SpellHit(Unit* caster, SpellInfo const* spell) override
		{
			if (Player* player = caster->ToPlayer())
			{
				if (player->GetQuestStatus(QUEST_THE_ART_OF_A_PRIEST) == QUEST_STATUS_INCOMPLETE)
					if (spell->Id == SPELL_FLASH_HEAL)
					{
						player->KilledMonsterCredit(NPC_WOUNDET_DARKSPEAR_WATCHER);
						me->DespawnOrUnsummon();
					}
				if (player->GetQuestStatus(QUEST_THE_ART_OF_A_DRUID) == QUEST_STATUS_INCOMPLETE)
					if (spell->Id == SPELL_REJUVENATION)
					{
						player->KilledMonsterCredit(NPC_WOUNDET_DARKSPEAR_WATCHER);
						me->DespawnOrUnsummon();
					}
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const override
	{
		return new npc_wounded_darkspear_watcherAI(creature);
	}
};

// #########################################################  Quest 24623 Save the Young:

class npc_lost_bloodtalon_hatchling : public CreatureScript
{
public:
	npc_lost_bloodtalon_hatchling() : CreatureScript("npc_lost_bloodtalon_hatchling") { }

	enum eHatchling
	{
		NPC_BLOODTALON_HATCHLING = 39157,
		QUEST_SAVING_THE_YOUNG = 24623,
		SPELL_BLOODTALON_WHISTLE = 70874,
	};

	struct npc_lost_bloodtalon_hatchlingAI : public ScriptedAI
	{
		npc_lost_bloodtalon_hatchlingAI(Creature* creature) : ScriptedAI(creature) { }

		uint32 m_timer;
		uint32 m_phase;
		Player* m_player;

		void Reset() override
		{
			m_timer = 0;
			m_phase = 0;
			m_player = NULL;
		}

		void SpellHit(Unit* caster, SpellInfo const* spell)
		{
			if (spell->Id == SPELL_BLOODTALON_WHISTLE && m_phase == 0)
				if (m_player = caster->ToPlayer())
					if (m_player->GetQuestStatus(QUEST_SAVING_THE_YOUNG) == QUEST_STATUS_INCOMPLETE)
						if (me->GetDistance2d(m_player) < 15.0f)
						{
							me->GetMotionMaster()->MoveFollow(m_player, 2.0f, frand(0.0f, 6.28f));
							m_player->KilledMonsterCredit(NPC_BLOODTALON_HATCHLING);
							m_timer = 1000;
							m_phase = 1;
						}
		}

		void UpdateAI(uint32 diff) override
		{
			if (m_timer <= diff)
			{
				m_timer = 1000;
				DoWork();
			}
			else
				m_timer -= diff;

			if (!UpdateVictim())
				return;
			else
				DoMeleeAttackIfReady();
		}

		void DoWork()
		{
			switch (m_phase)
			{
			case 1:
			{
				if (!m_player)
					me->DespawnOrUnsummon();
				if (m_player->isDead() || m_player->GetQuestStatus(QUEST_SAVING_THE_YOUNG) == QUEST_STATUS_REWARDED)
					me->DespawnOrUnsummon();
				break;
			}
			default:
				break;
			}
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_lost_bloodtalon_hatchlingAI(creature);
	}
};

// #########################################################  Quest 24626 Young and Vicious

class npc_swiftclaw_38002 : public CreatureScript
{
public:
	npc_swiftclaw_38002() : CreatureScript("npc_swiftclaw_38002") { }

	struct npc_swiftclaw_38002AI : public ScriptedAI
	{
		npc_swiftclaw_38002AI(Creature* creature) : ScriptedAI(creature) { }

		void StartAnim(Player* player, Creature* raptor)
		{
			player->EnterVehicle(me, 1);
			me->SetSpeed(MOVE_RUN, 1.2f, true);
		}

		void UpdateAI(uint32 diff) override
		{
			if (!UpdateVictim())
				return;
			else
				DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_swiftclaw_38002AI(creature);
	}
};

class at_raptor_pens : public AreaTriggerScript
{
public:
	at_raptor_pens() : AreaTriggerScript("at_raptor_pens") { }

	enum eAreaTrigger
	{
		QUEST_YOUNG_AND_VICIOUS = 24626,
		NPC_SWIFTCLAW = 38002,
	};

	bool OnTrigger(Player* player, const AreaTriggerEntry* at) override
	{
		if (player && player->GetQuestStatus(QUEST_YOUNG_AND_VICIOUS) == QUEST_STATUS_INCOMPLETE)
			if (Vehicle* vehicle = player->GetVehicle())
				if (Unit* base = vehicle->GetBase())
					if (Creature* raptor = base->ToCreature())
					{
						player->Dismount();
						player->KilledMonsterCredit(NPC_SWIFTCLAW);
						raptor->DespawnOrUnsummon();
					}

		return false;
	}
};


// #########################################################  showfight between darkspear and spitescale 

class npc_spitescale_showfight : public CreatureScript // 38300, 38301, 38302
{
public:
	npc_spitescale_showfight() : CreatureScript("npc_spitescale_showfight") { }

	struct npc_spitescale_showfightAI : public ScriptedAI
	{
		npc_spitescale_showfightAI(Creature* creature) : ScriptedAI(creature) { }

		float m_health;

		void Reset() override
		{
			m_health = frand(40.0f, 75.0f);
		}

		void DamageTaken(Unit* attacker, uint32& damage) override
		{
			bool sf = false;
			if (attacker->GetEntry() == 38324) sf = true;
			if (attacker->GetEntry() == 38326) sf = true;
			if (sf)
			{
				if (me->GetHealthPct() < m_health)
					damage = 0;
				else
					damage /= 10;
			}
		}

		void UpdateAI(uint32 diff) override
		{
			if (!UpdateVictim())
				return;
			else
				DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_spitescale_showfightAI(creature);
	}
};

class npc_darkspear_showfight : public CreatureScript // 38324, 38326
{
public:
	npc_darkspear_showfight() : CreatureScript("npc_darkspear_showfight") { }

	struct npc_darkspear_showfightAI : public ScriptedAI
	{
		npc_darkspear_showfightAI(Creature* creature) : ScriptedAI(creature) { }

		float m_health;

		void Reset() override
		{
			m_health = frand(40.0f, 75.0f);
		}

		void DamageTaken(Unit* attacker, uint32& damage) override
		{
			bool sf = false;
			if (attacker->GetEntry() == 38300) sf = true;
			if (attacker->GetEntry() == 38301) sf = true;
			if (attacker->GetEntry() == 38302) sf = true;
			if (sf)
			{
				if (me->GetHealthPct() < m_health)
					damage = 0;
				else
					damage /= 10;
			}
		}

		void UpdateAI(uint32 diff) override
		{
			if (!UpdateVictim())
				return;
			else
				DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_darkspear_showfightAI(creature);
	}
};

// ######################################################### quest 25035 break the lines  
// ToDo: showfight between darkspear and spitescale 
// ToDo: mount raptor summoned by jornun

class npc_jornun_38989 : public CreatureScript
{
public:
	npc_jornun_38989() : CreatureScript("npc_jornun_38989"){ }

	enum eQuest25035
	{
		npc_jornun = 38989,
		npc_morakki = 38442,
		quest_break_the_lines = 25035,
	};

	bool OnGossipHello(Player* player, Creature* creature)
	{
		if (player && player->GetQuestStatus(quest_break_the_lines) == QUEST_STATUS_INCOMPLETE)
		{
			player->KilledMonsterCredit(npc_jornun);
			creature->AI()->Talk(0);
		}

		return false;
	}

};

class npc_morakki_38442 : public CreatureScript
{
public:
	npc_morakki_38442() : CreatureScript("npc_morakki_38442"){ }

	enum eQuest25035
	{
		npc_jornun = 38989,
		npc_morakki = 38442,
		quest_break_the_lines = 25035,
	};

	bool OnGossipHello(Player* player, Creature* creature)
	{
		if (player && player->GetQuestStatus(quest_break_the_lines) == QUEST_STATUS_INCOMPLETE)
		{
			player->KilledMonsterCredit(npc_morakki);
			return true;
		}

		return false;
	}

};

// ######################################################### quest 24814 
// ToDo: videos not ready..i want sniffs..
// ToDo: fight with Sea Witch 

class npc_voljin_38225 : public CreatureScript
{
public:
	npc_voljin_38225() : CreatureScript("npc_voljin_38225"){ }

	enum eQuest25035
	{
		NPC_ZUNI = 38423,
		NPC_VANIRA = 38437,
		NPC_ZARJIRA = 38306,
		NPC_VOLJIN = 38225,
		QUEST_AN_ANCIENT_ENEMY = 24814,
	};

	bool OnGossipHello(Player* player, Creature* creature)
	{
		if (player &&  player->GetQuestStatus(QUEST_AN_ANCIENT_ENEMY) == QUEST_STATUS_INCOMPLETE)
			player->KilledMonsterCredit(NPC_VOLJIN);

		return false;
	}

	bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
	{
		if (player && player->GetQuestStatus(QUEST_AN_ANCIENT_ENEMY) == QUEST_STATUS_INCOMPLETE)
		{
			player->KilledMonsterCredit(NPC_ZARJIRA);
			player->TeleportTo(1, -1304.986f, -5589.265f, 23.72f, 3.85f);
			return true;
		}

		return false;
	}

	struct npc_voljin_38225AI : public ScriptedAI
	{
		npc_voljin_38225AI(Creature* creature) : ScriptedAI(creature) { }

		void Reset() override
		{

		}

		void UpdateAI(uint32 diff) override
		{
			if (!UpdateVictim())
				return;
			else
				DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_voljin_38225AI(creature);
	}
};

class npc_voljin_38966 : public CreatureScript
{
public:
	npc_voljin_38966() : CreatureScript("npc_voljin_38966"){ }

	enum eQuest25035
	{

		QUEST_AN_ANCIENT_ENEMY = 24814,
	};

	bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 /*opt*/)
	{

		return false;
	}

	struct npc_voljin_38966AI : public ScriptedAI
	{
		npc_voljin_38966AI(Creature* creature) : ScriptedAI(creature) { }

		void Reset() override
		{

		}

		void UpdateAI(uint32 diff) override
		{
			if (!UpdateVictim())
				return;
			else
				DoMeleeAttackIfReady();
		}
	};

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_voljin_38966AI(creature);
	}
};

// ######################################################### sen'jin

void AddSC_lost_isle()
{
    new npc_Zapnozzle();
    new npc_singe_bombe();
    new npc_mineur_gob();
    new npc_Mechumide();
    new spell_68281();
    new spell_weed_whacker();
    new npc_lianne_gobelin();
    new npc_killag_sangrecroc();
    new npc_pant_gob();
    new npc_gyrocoptere_quest_giver();
    new npc_girocoptere();
    new npc_tornade_gob();
    new gob_fronde_gobelin();
    new npc_fusee_gob();
    new npc_phaseswift();
    new gob_dyn_gobelin();
    new npc_poule();
    new npc_raptore_gob();
    new gob_spark_gobelin();
    new npc_young_naga_gob();
    new npc_megs_isle_gob();
    new npc_crack_isle_gob();
    new npc_canon_gob();
    new spell_boot_damage();
    new spell_boot_gob();
    new npc_oom_isle_gob();
    new npc_ceint();
    new npc_izzy_airplane();
    new npc_avion_gob();
    new npc_meteor2_gob();
    new npc_tremblement_volcano();
    new npc_explosion_volcano();
    new npc_killag_2();
    new npc_Chariot();
    new npc_Chariot2();
    new gob_red_but();
    new npc_grilly_2();
    new npc_Prince();
    new npc_boot();
	new npc_sassy_handwrench();
	new npc_novice_darkspear_warrior();
	new npc_tiki_target();
	new npc_docile_island_boar();
	new npc_darkspear_jailor();
	new npc_captive_spitescale_scout();
	new npc_wounded_darkspear_watcher();
	new npc_lost_bloodtalon_hatchling();
	new npc_swiftclaw_38002();
	new at_raptor_pens();
	new npc_spitescale_showfight();
	new npc_darkspear_showfight();
	new npc_jornun_38989();
	new npc_morakki_38442();
	new npc_voljin_38225();
	new npc_voljin_38966();
}
